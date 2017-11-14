/*
 * Copyright (C) 2017  Roel Janssen <roel@gnu.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>
#include <unistd.h>

#include <htslib/vcf.h>
#include <pthread.h>

#include "RuntimeConfiguration.h"
#include "GenomePosition.h"
#include "Variant.h"
#include "Origin.h"
#include "Sample.h"
#include "VcfHeader.h"
#include "ui.h"

#ifdef __GNUC__
#define UNUSED __attribute__((__unused__))
#else
#define UNUSED
#endif

extern RuntimeConfiguration program_config;
extern int hts_verbose;
static pthread_mutex_t output_mutex;
const char DEFAULT_GRAPH_LOCATION[] = "http://localhost:8890/";

typedef struct
{
  bool is_reversed;
  bool is_left_of_ref;
  char *chromosome;
  int32_t chromosome_len;
  int32_t position;
} BndProperties;

void
bnd_properties_init (BndProperties *properties)
{
  if (properties == NULL) return;

  properties->is_reversed = false;
  properties->is_left_of_ref = false;
  properties->chromosome = NULL;
  properties->chromosome_len = 0;
  properties->position = 0;
}

bool
parse_properties (BndProperties *properties,
                  const char *ref, int32_t ref_len,
                  const char *alt, int32_t alt_len)
{
  if (properties == NULL || ref == NULL || alt == NULL) return false;

  char bracket;

  /* Determine direction and inner-position.
   * ------------------------------------------------------------------------ */
  if (alt[0] == ']')
    {
      properties->is_reversed = false;
      properties->is_left_of_ref = true;
      bracket = ']';
    }
  else if (alt[0] == '[')
    {
      properties->is_reversed = true;
      properties->is_left_of_ref = true;
      bracket = '[';
    }
  else if (alt[alt_len - 1] == '[')
    {
      properties->is_reversed = false;
      properties->is_left_of_ref = false;
      bracket = '[';
    }
  else if (alt[alt_len - 1] == ']')
    {
      properties->is_reversed = true;
      properties->is_left_of_ref = false;
      bracket = ']';
    }

  /* Determine second chromosome and position.
   * ------------------------------------------------------------------------ */
  char *first_bracket = strchr (alt, bracket);
  char *last_bracket = strchr (first_bracket + 1, bracket);
  char *separator = strchr (alt, ':');

  if (first_bracket == NULL || last_bracket == NULL || separator == NULL)
    return false;

  properties->chromosome_len = separator - first_bracket - 1;
  int32_t position_len = last_bracket - separator - 1;

  properties->chromosome = calloc (sizeof (char), properties->chromosome_len+1);

  if (properties->chromosome == NULL)
    return false;

  char pos[sizeof (char) * position_len + 1];
  memset (pos, 0, position_len + 1);

  memcpy (properties->chromosome, first_bracket+1, properties->chromosome_len);
  memcpy (pos, separator + 1, position_len);

  properties->position = atoi (pos);
  return properties;
}

bool
determine_confidence_interval (FaldoExactPosition *base,
                               const char *property,
                               FaldoExactPosition *begin,
                               FaldoExactPosition *end)
{
  int32_t *cipos = NULL;
  int32_t cipos_len = 0;
  bcf_get_info_int32 (header, buffer, property, &cipos, &cipos_len);

  if (cipos_len > 0 && cipos)
    {
      begin->position -= cipos[0];
      end->position   += cipos[1];
    }
  else
    return false;

  if (cipos)
    {
      free (cipos);
      cipos = NULL;
    }

  base->before = begin;
  base->after = end;

  return true;
}

/*----------------------------------------------------------------------------.
 | HANDLERS FOR SPECIFIC VCF RECORD TYPES                                     |
 '----------------------------------------------------------------------------*/

void
handle_record (Origin *origin, bcf_hdr_t *header, bcf1_t *buffer)
{
  /* One of: VCF_REF, VCF_SNP, VCF_MNP, VCF_INDEL, VCF_OTHER, VCF_BND. */
  int variant_type = bcf_get_variant_types (buffer);

  /* Handle the program options for leaving out FILTER fields.
   * ------------------------------------------------------------------------ */
  if (program_config.filter &&
      bcf_has_filter (header, buffer, program_config.filter) == 1)
    {
      pthread_mutex_lock (&output_mutex);
      printf ("# Skipping %s record,\n", program_config.filter);
      pthread_mutex_unlock (&output_mutex);
      return;
    }

  if (program_config.keep &&
      bcf_has_filter (header, buffer, program_config.keep) != 1)
    {
      pthread_mutex_lock (&output_mutex);
      printf ("# Skipping record without %s.\n", program_config.keep);
      pthread_mutex_unlock (&output_mutex);
      return;
    }

  Variant variant;
  variant_initialize (&variant, variant_type);

  /* Unpack up and including the ALT field. */
  bcf_unpack (buffer, BCF_UN_STR);

  /* If the allele information is still missing after unpacking the buffer,
   * we will end up without REF information.  So, let's skip such records. */
  if (buffer->d.allele == NULL)
    {
      pthread_mutex_lock (&output_mutex);
      printf ("# Skipping record because of missing allele information.\n");
      pthread_mutex_unlock (&output_mutex);
      return;
    }

  char *ref = buffer->d.allele[0];
  int32_t ref_len = buffer->rlen;

  /* TODO: Only the first alternative allele is considered.  When multiple
   * alleles are specified, we need to provide variants for each allele. */
  char *alt = buffer->d.allele[1];
  int32_t alt_len = strlen (alt);

  bcf_info_t *svtype_info = bcf_get_info (header, buffer, "SVTYPE");
  char *svtype = NULL;
  if (svtype_info != NULL)
    svtype = (char *)header->id[BCF_DT_ID][svtype_info->key].key;

  /* Gather the positions.
   * ------------------------------------------------------------------------ */
  FaldoExactPosition start_position, end_position;
  faldo_position_initialize ((FaldoBaseType *)&start_position, FALDO_EXACT_POSITION);
  faldo_position_initialize ((FaldoBaseType *)&end_position, FALDO_EXACT_POSITION);

  variant.start_position = &start_position;
  variant.end_position = &end_position;

  start_position.position = buffer->pos;
  start_position.chromosome = (char *)bcf_seqname (header, buffer);
  start_position.chromosome_len = strlen (start_position.chromosome);

  variant.length = end - buffer->pos;

  /* For BND, the chromosome for the end position can be different. */
  if (svtype != NULL && !strcmp (svtype, "BND"))
    {
      BndProperties properties;
      bnd_properties_init (&properties);

      if (!parse_properties (&properties, ref, ref_len, alt, alt_len))
        puts ("# WARNING: Failed to parse complex rearrangement.");
      else
        {
          variant.is_complex_rearrangement = TRUE;
          variant.is_reversed = properties.is_reversed;
          variant.is_left_of_ref = properties.is_left_of_ref;

          end_position.chromosome = properties.chromosome;
          end_position.position = properties.position;
        }
    }
  else
    {
      /* For anything other than complex rearrangements, we can assume the
       * chromosome of the start position is the same as the chromosome for the
       * end position. */
      end_position.chromosome = start_position.chromosome;

      /* Usually, an END info-field is provided. */
      if (bcf_get_info_int32 (header, buffer, "END", &buf, &buf_len) > 0)
        end_position.position = ((int32_t *)buf)[0];
      /* One possible fallback is to infer it from the SVLEN property. */
      else if (bcf_get_info_int32 (header, buffer, "SVLEN", &buf, &buf_len) > 0)
        end_position.position = start_position.position + ((int32_t *)buf)[0];
    }

  /* When the positions are not on the same chromosome, our length indication
   * does not make any sense.  What also doesn't make sense is a length of 0. */
  if (strcmp (end_position.chromosome, start_position.chromosome))
    variant.length = 0;

  FaldoExactPosition end_position;
  faldo_position_initialize ((FaldoBaseType *)&end_position, FALDO_EXACT_POSITION);
  end_position.position = end;
  end_position.chromosome = chr2;
  end_position.chromosome_len = strlen (chr2);

  FaldoInBetweenPosition confidence_position;
  faldo_position_initialize ((FaldoBaseType *)&confidence_position,
                             FALDO_IN_BETWEEN_POSITION);

  /* Handle the confidence interval for the start position.
   * ------------------------------------------------------------------------ */
  determine_confidence_interval (variant.start_position,
                                 "CIPOS",
                                 variant.cipos->before,
                                 variant.cipos->after);

  determine_confidence_interval (variant.end_position,
                                 "CIEND",
                                 variant.ciend->before,
                                 variant.ciend->after);

  pthread_mutex_lock (&output_mutex);
  faldo_position_print ((FaldoBaseType *)&variant.cipos);
  faldo_position_print ((FaldoBaseType *)&variant.ciend);
  //faldo_position_print ((FaldoBaseType *)&confidence_position);
  faldo_exact_position_print (variant.start_position);
  faldo_exact_position_print (variant.end_position);
  pthread_mutex_unlock (&output_mutex);

  faldo_exact_position_reset (&ci_start_position);
  faldo_exact_position_reset (&ci_end_position);

  /* Probe for more fields/information.
   * ------------------------------------------------------------------------ */
  void *buf = NULL;
  int32_t buf_len = 0;

  int32_t mapq = 0;
  if (bcf_get_info_int32 (header, buffer, "MAPQ", &buf, &buf_len) > 0)
    mapq = ((int32_t *)buf)[0];

  int32_t paired_end_support = 0;
  if (bcf_get_info_int32 (header, buffer, "PE", &buf, &buf_len) > 0)
    paired_end_support = ((int32_t *)buf)[0];

  int32_t split_read_support = 0;
  if (bcf_get_info_int32 (header, buffer, "SR", &buf, &buf_len) > 0)
    split_read_support = ((int32_t *)buf)[0];

  float split_read_consensus_alignment_quality = 0;
  if (bcf_get_info_float (header, buffer, "SRQ", &buf, &buf_len) > 0)
    split_read_consensus_alignment_quality = ((float *)buf)[0];

  int32_t read_count = 0;
  if (bcf_get_format_int32 (header, buffer, "RC", &buf, &buf_len) > 0)
    read_count = ((int32_t *)buf)[0];

  int32_t hq_reference_pairs = 0;
  if (bcf_get_format_int32 (header, buffer, "DR", &buf, &buf_len) > 0)
    hq_reference_pairs = ((int32_t *)buf)[0];

  int32_t hq_variant_pairs = 0;
  if (bcf_get_format_int32 (header, buffer, "DV", &buf, &buf_len) > 0)
    hq_variant_pairs = ((int32_t *)buf)[0];

  int32_t hq_ref_junction_reads = 0;
  if (bcf_get_format_int32 (header, buffer, "RR", &buf, &buf_len) > 0)
    hq_ref_junction_reads = ((int32_t *)buf)[0];

  int32_t hq_var_junction_reads = 0;
  if (bcf_get_format_int32 (header, buffer, "RV", &buf, &buf_len) > 0)
    hq_var_junction_reads = ((int32_t *)buf)[0];
  
  variant.origin = origin;
  variant.position = (FaldoBaseType *)&start_position;
  variant.confidence_interval = (FaldoBaseType *)&confidence_position;
  if (!variant_gather_data (&variant, header, buffer))
    fprintf (stderr, "# Couldn't gather variant data.\n");

  pthread_mutex_lock (&output_mutex);
  variant_print (&variant, header);
  pthread_mutex_unlock (&output_mutex);

  faldo_exact_position_reset (&start_position);
  faldo_in_between_position_reset (&confidence_position);
}

typedef struct
{
  htsFile *vcf_stream;
  bcf_hdr_t *vcf_header;
  bcf1_t **buffers;
  Origin *origin;
  int32_t thread_number;
  int32_t jobs_to_run;
} htslib_data_t;

void *
run_jobs_in_thread (void *data)
{
  htslib_data_t *pack = (htslib_data_t *)data;
  if (pack == NULL) return NULL;

  bcf_hdr_t *vcf_header = pack->vcf_header;
  Origin *origin = pack->origin;

  /* Process the records. */
  int32_t i = 0;
  int variant_type = 0;

  for (; i < pack->jobs_to_run; i++)
    {
      bcf1_t *buffer = pack->buffers[i + (pack->thread_number * pack->jobs_to_run)];
      handle_record (origin, vcf_header, buffer);
    }

  return NULL;
}


/*----------------------------------------------------------------------------.
 | HANDLE THE HEADER FIELDS                                                   |
 '----------------------------------------------------------------------------*/

void
handle_header (bcf_hdr_t *vcf_header, Origin *origin)
{
  if (vcf_header == NULL)
    return;

  int32_t i;
  for (i = 0; i < vcf_header->nhrec; i++)
    {
      /* Handle simple key-value fields.
       * ------------------------------------------------------------------- */
      if (vcf_header->hrec[i]->value)
        {
          VcfHeader h;
          initialize_VcfHeader (&h, HEADER_TYPE_GENERIC);
          h.origin = origin;
          h.key = vcf_header->hrec[i]->key;
          h.key_len = strlen (vcf_header->hrec[i]->key);
          h.value = vcf_header->hrec[i]->value;
          h.value_len = strlen (vcf_header->hrec[i]->value);
          h.origin = origin;

          print_VcfHeader (&h);
        }

      /* Handle INFO fields.
       * ------------------------------------------------------------------- */
      else if (!strcmp (vcf_header->hrec[i]->key, "INFO"))
        {
          int32_t j;
          VcfInfoField info_field;
          initialize_VcfHeader ((VcfHeader *)&info_field, HEADER_TYPE_INFO);
          info_field.origin = origin;

          for (j = 0; j < vcf_header->hrec[i]->nkeys; j++)
            {
              if (!strcmp (vcf_header->hrec[i]->keys[j], "ID"))
                info_field.id = vcf_header->hrec[i]->vals[j];
              else if (!strcmp (vcf_header->hrec[i]->keys[j], "Number"))
                info_field.number = vcf_header->hrec[i]->vals[j];
              else if (!strcmp (vcf_header->hrec[i]->keys[j], "Type"))
                info_field.type = vcf_header->hrec[i]->vals[j];
              else if (!strcmp (vcf_header->hrec[i]->keys[j], "Description"))
                info_field.description = vcf_header->hrec[i]->vals[j];
            }

          print_VcfHeader ((VcfHeader *)&info_field);
        }

      /* Handle FILTER fields.
       * ------------------------------------------------------------------- */
      else if (!strcmp (vcf_header->hrec[i]->key, "FILTER"))
        {
          int32_t j;
          VcfFilterField filter_field;
          initialize_VcfHeader ((VcfHeader *)&filter_field, HEADER_TYPE_FILTER);
          filter_field.origin = origin;

          for (j = 0; j < vcf_header->hrec[i]->nkeys; j++)
            {
              if (!strcmp (vcf_header->hrec[i]->keys[j], "ID"))
                filter_field.id = vcf_header->hrec[i]->vals[j];
              else if (!strcmp (vcf_header->hrec[i]->keys[j], "Description"))
                filter_field.description = vcf_header->hrec[i]->vals[j];
            }

          print_VcfHeader ((VcfHeader *)&filter_field);
        }

      /* Handle ALT fields.
       * ------------------------------------------------------------------- */
      else if (!strcmp (vcf_header->hrec[i]->key, "ALT"))
        {
          int32_t j;
          VcfAltField alt_field;
          initialize_VcfHeader ((VcfHeader *)&alt_field, HEADER_TYPE_ALT);
          alt_field.origin = origin;

          for (j = 0; j < vcf_header->hrec[i]->nkeys; j++)
            {
              if (!strcmp (vcf_header->hrec[i]->keys[j], "ID"))
                alt_field.id = vcf_header->hrec[i]->vals[j];
              else if (!strcmp (vcf_header->hrec[i]->keys[j], "Description"))
                alt_field.description = vcf_header->hrec[i]->vals[j];
            }

          print_VcfHeader ((VcfHeader *)&alt_field);
        }

      /* Handle FORMAT fields.
       * ------------------------------------------------------------------- */
      else if (!strcmp (vcf_header->hrec[i]->key, "FORMAT"))
        {
          int32_t j;
          VcfFormatField format_field;
          initialize_VcfHeader ((VcfHeader *)&format_field, HEADER_TYPE_FORMAT);
          format_field.origin = origin;

          for (j = 0; j < vcf_header->hrec[i]->nkeys; j++)
            {
              if (!strcmp (vcf_header->hrec[i]->keys[j], "ID"))
                format_field.id = vcf_header->hrec[i]->vals[j];
              else if (!strcmp (vcf_header->hrec[i]->keys[j], "Number"))
                format_field.number = vcf_header->hrec[i]->vals[j];
              else if (!strcmp (vcf_header->hrec[i]->keys[j], "Type"))
                format_field.type = vcf_header->hrec[i]->vals[j];
              else if (!strcmp (vcf_header->hrec[i]->keys[j], "Description"))
                format_field.description = vcf_header->hrec[i]->vals[j];
            }

          print_VcfHeader ((VcfHeader *)&format_field);
        }

      /* Handle 'contig' fields.
       * ------------------------------------------------------------------- */
      else if (!strcmp (vcf_header->hrec[i]->key, "contig"))
        {
          int32_t j;
          VcfContigField contig_field;
          initialize_VcfHeader ((VcfHeader *)&contig_field, HEADER_TYPE_CONTIG);
          contig_field.origin = origin;

          for (j = 0; j < vcf_header->hrec[i]->nkeys; j++)
            {
              if (!strcmp (vcf_header->hrec[i]->keys[j], "ID"))
                contig_field.id = vcf_header->hrec[i]->vals[j];
              else if (!strcmp (vcf_header->hrec[i]->keys[j], "length"))
                contig_field.length = atoi (vcf_header->hrec[i]->vals[j]);
              else if (!strcmp (vcf_header->hrec[i]->keys[j], "assembly"))
                contig_field.assembly = vcf_header->hrec[i]->vals[j];
            }

          print_VcfHeader ((VcfHeader *)&contig_field);
        }
      else
        fprintf (stderr, "Error: Encountered an unknown header item '%s'.\n",
                 vcf_header->hrec[i]->key);
    }
}

/*----------------------------------------------------------------------------.
 | MAIN PROGRAM                                                               |
 '----------------------------------------------------------------------------*/

int
main (int argc, char **argv)
{
  program_config.filter = NULL;
  program_config.keep = NULL;
  program_config.input_file = NULL;
  program_config.reference = "unknown";
  program_config.graph_location = (char *)DEFAULT_GRAPH_LOCATION;
  program_config.threads = 2;
  program_config.jobs_per_thread = 500;

  pthread_mutex_init(&output_mutex, NULL);

  /*--------------------------------------------------------------------------.
   | PROCESS COMMAND-LINE OPTIONS                                             |
   '--------------------------------------------------------------------------*/

  if (argc > 1)
    {
      int arg = 0;
      int index = 0;

      /* Program options
       * ------------------------------------------------------------------- */
      static struct option options[] =
	{
          { "input-file",        required_argument, 0, 'i' },
          { "filter",            required_argument, 0, 'f' },
          { "keep",              required_argument, 0, 'k' },
          { "publish-to",        required_argument, 0, 'p' },
          { "reference",         required_argument, 0, 'r' },
          { "caller",            required_argument, 0, 'c' },
          { "threads",           required_argument, 0, 't' },
	  { "help",              no_argument,       0, 'h' },
	  { "version",           no_argument,       0, 'v' },
	  { 0,                   0,                 0, 0   }
	};

      while ( arg != -1 )
	{
	  /* Make sure to list all short options in the string below. */
	  arg = getopt_long (argc, argv, "i:f:p:k:r:c:t:vh", options, &index);
          switch (arg)
            {
            case 'c': program_config.caller = optarg;                break;
            case 'f': program_config.filter = optarg;                break;
            case 'p': program_config.graph_location = optarg;        break;
            case 'i': program_config.input_file = optarg;            break;
            case 'k': program_config.keep = optarg;                  break;
            case 'r': program_config.reference = optarg;             break;
            case 't': program_config.threads = atoi(optarg);         break;
            case 'h': show_help ();                                  break;
            case 'v': show_version ();                               break;
            }
        }
    }
  else
    show_help ();

  /*--------------------------------------------------------------------------.
   | HANDLE AN INPUT FILE                                                     |
   '--------------------------------------------------------------------------*/

  if (program_config.input_file)
    {
      /* Show warnings for missing options that lead to missing data.
       * -------------------------------------------------------------------- */
      if (program_config.reference == NULL)
        fputs ("Warning: No --reference has been specified.  "
               "This may lead to incomplete and/or ambiguous information "
               "in the database.\n", stderr);

      if (program_config.caller == NULL)
        fputs ("Warning: No --caller has been specified.  "
               "This may lead to incomplete and/or ambiguous information "
               "in the database.\n", stderr);

      /* Disable the output messages from HTSLib.
       * -------------------------------------------------------------------- */
      hts_verbose = 0;

      /* Determine whether the input file is a VCF or compressed VCF.
       * -------------------------------------------------------------------- */
      int32_t input_file_len = strlen (program_config.input_file);
      bool is_vcf = false;
      bool is_gzip_vcf = false;
      bool is_bcf = false;
      bool is_gzip_bcf = false;

      if (input_file_len > 3)
        is_vcf = !strcmp (program_config.input_file + input_file_len - 3, "vcf");

      if (input_file_len > 3)
        is_bcf = !strcmp (program_config.input_file + input_file_len - 3, "bcf");

      if (!is_vcf && !is_bcf && input_file_len > 6)
        is_gzip_vcf = !strcmp (program_config.input_file + input_file_len - 6, "vcf.gz");

      if (!is_vcf && !is_bcf && input_file_len > 6)
        is_gzip_bcf = !strcmp (program_config.input_file + input_file_len - 6, "bcf.gz");

      if (!(is_bcf || is_gzip_bcf || is_vcf || is_gzip_vcf))
        return print_file_format_error ();

      /* Prepare the buffers needed to read the VCF file.
       * -------------------------------------------------------------------- */
      bcf_hdr_t *vcf_header = NULL;
      htsFile *vcf_stream = NULL;

      if (is_vcf)
        vcf_stream = hts_open (program_config.input_file, "r");
      else if (is_gzip_vcf)
        vcf_stream = hts_open (program_config.input_file, "rz");
      else if (is_bcf)
        vcf_stream = hts_open (program_config.input_file, "rbu");
      else if (is_gzip_bcf)
        vcf_stream = hts_open (program_config.input_file, "rb");

      if (vcf_stream == NULL)
        return print_vcf_file_error (program_config.input_file);

      vcf_header = bcf_hdr_read (vcf_stream);
      if (vcf_header == NULL)
        {
          hts_close (vcf_stream);
          return print_vcf_header_error (program_config.input_file);
        }

      /* Write the prefix of the Turtle output.
       * -------------------------------------------------------------------- */
      puts ("@prefix vcf:     <http://semweb.op.umcutrecht.nl/vcf/> .\n"
            "@prefix smo:     <http://semweb.op.umcutrecht.nl/smo/> .\n"
            "@prefix faldo:   <http://biohackathon.org/resource/faldo#> .\n"
            "@prefix rdf:     <http://www.w3.org/1999/02/22-rdf-syntax-ns#> .\n"
            "@prefix rdfs:    <http://www.w3.org/2000/01/rdf-schema#> .");

      printf ("@prefix : <%s> .\n",                     program_config.graph_location);
      printf ("@prefix v: <%sVariant/> .\n",            program_config.graph_location);
      printf ("@prefix p: <%sPosition/> .\n",           program_config.graph_location);
      printf ("@prefix ep: <%sExactPosition/> .\n",     program_config.graph_location);
      printf ("@prefix ip: <%sInBetweenPosition/> .\n", program_config.graph_location);
      printf ("@prefix rp: <%sRangedPosition/> .\n",    program_config.graph_location);
      printf ("@prefix s: <%sSample/> .\n",             program_config.graph_location);
      printf ("@prefix h: <%sVcfHeader/> .\n",          program_config.graph_location);
      printf ("@prefix o: <%sOrigin/> .\n",           program_config.graph_location);

      /* There seem to be slight differences between the way URIs for GRCh38
       * and GRCh37.  Also, the GRCh37 is only available for download from their FTP
       * server.  So it cannot be queried by their own web-based ontology browser. */
      if (!strcmp (program_config.reference, "GRCh37") ||
          !strcmp (program_config.reference, "grch37"))
        {
          program_config.reference = "grch37";
          puts ("@prefix grch37: <http://rdf.ebi.ac.uk/resource/ensembl/83/chromosome:GRCh37:> .");
        }
      else if (!strcmp (program_config.reference, "GRCh38") ||
               !strcmp (program_config.reference, "grch38"))
        {
          program_config.reference = "grch38";
          puts ("@prefix grch38: <http://rdf.ebi.ac.uk/resource/ensembl/90/homo_sapiens/GRCh38/> .");
        }

      puts ("");

      /* Add origin to database.
       * -------------------------------------------------------------------- */
      Origin o;
      initialize_Origin (&o);

      if (program_config.input_file[0] == '/')
        set_Origin_filename (&o, program_config.input_file);
      else
        {
          char *cwd = getcwd (NULL, 0);

          /* Degrade to a relative filename if we cannot allocate the memory
           * for an absolute path, because it's better than nothing. */
          if (cwd == NULL)
            set_Origin_filename (&o, program_config.input_file);
          else
            {
              int32_t full_path_len = strlen (cwd) + strlen (program_config.input_file) + 2;
              char full_path[full_path_len + 1];
              memset (full_path, 0, full_path_len);

              if (snprintf (full_path, full_path_len, "%s/%s",
                            cwd, program_config.input_file) == full_path_len)
                set_Origin_filename (&o, full_path);
              else
                /* Degrade to a relative filename if we cannot allocate the
                 * memory for an absolute path, because it's better than
                 * nothing. */
                set_Origin_filename (&o, program_config.input_file);

              free (cwd);
              cwd = NULL;
            }
        }

      print_Origin (&o);
      free (o.filename);
      o.filename = NULL;
      o.filename_len = 0;

      /* Add header lines to database.
       * -------------------------------------------------------------------- */
      handle_header (vcf_header, &o);
      
      /* Set up threads and per-thread storage.
       * -------------------------------------------------------------------- */
      pthread_t threads[program_config.threads];
      memset (threads, 0, sizeof (pthread_t) * program_config.threads);

      int32_t tbuffers_len = program_config.threads * program_config.jobs_per_thread;
      bcf1_t *tbuffers[tbuffers_len];

      int32_t j = 0;
      for (; j < tbuffers_len; j++)
        {
          tbuffers[j] = bcf_init ();
          if (tbuffers[j] == NULL)
            return print_memory_error (program_config.input_file);
        }

      /* Divide the work over the threads.
       * -------------------------------------------------------------------- */
      int32_t queue = 0;
      while (bcf_read (vcf_stream, vcf_header, tbuffers[queue]) == 0)
        {
          if (queue >= (program_config.jobs_per_thread * program_config.threads - 2))
            {
              htslib_data_t pack[j];
              /* Spawn the threads. */
              for (j = 0; j < program_config.threads; j++)
                {
                  pack[j].vcf_header = vcf_header;
                  pack[j].vcf_stream = vcf_stream;
                  pack[j].buffers = tbuffers;
                  pack[j].thread_number = j;
                  pack[j].jobs_to_run = program_config.jobs_per_thread;
                  pack[j].origin = &o;

                  pthread_create (&(threads[j]), NULL, run_jobs_in_thread, (void *)&(pack[j]));
                }

              /* Wait for all to complete. */
              for (j = 0; j < program_config.threads; j++)
                pthread_join (threads[j], NULL);

              /* Reset the queue */
              queue = 0;
            }
          else
            queue++;
        }

      if (queue > 0)
        {
          for (j = 0; j < queue; j++)
            {
              htslib_data_t pack;
              pack.vcf_header = vcf_header;
              pack.vcf_stream = vcf_stream;
              pack.buffers = tbuffers;
              pack.thread_number = 0;
              pack.jobs_to_run = 1;
              pack.origin = &o;

              /* Run them in a single thread. */
              run_jobs_in_thread (&pack);
            }
        }

      for (j = 0; j < tbuffers_len; j++)
        bcf_destroy (tbuffers[j]);

      bcf_hdr_destroy (vcf_header);
      hts_close (vcf_stream);
    }

  return 0;
}
