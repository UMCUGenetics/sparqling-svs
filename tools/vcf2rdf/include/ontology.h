/* Copyright (C) 2018  Roel Janssen <roel@gnu.org>
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

#ifndef ONTOLOGY_H
#define ONTOLOGY_H

#include <stdbool.h>
#include <stdint.h>
#include <raptor2.h>
#include <htslib/vcf.h>

/* These string constants can be used to concatenate strings at compile-time. */
#define URI_W3            "http://www.w3.org"
#define URI_ONTOLOGY      "http://sparqling-genomics/vcf2rdf"
#define URI_MASTER        "http://sparqling-genomics"

#define STR_PREFIX_MASTER              URI_MASTER "/"
#define STR_PREFIX_BASE                URI_ONTOLOGY "/"
#define STR_PREFIX_SAMPLE              URI_MASTER "/Sample/"
#define STR_PREFIX_VCF_HEADER          URI_ONTOLOGY "/HeaderItem/"
#define STR_PREFIX_VCF_HEADER_INFO     URI_ONTOLOGY "/InfoItem/"
#define STR_PREFIX_VCF_HEADER_FORMAT   URI_ONTOLOGY "/FormatItem/"
#define STR_PREFIX_VCF_HEADER_FORMAT_GT URI_ONTOLOGY "/FormatItem/GT/"
#define STR_PREFIX_VCF_HEADER_FILTER   URI_ONTOLOGY "/FilterItem/"
#define STR_PREFIX_VCF_HEADER_ALT      URI_ONTOLOGY "/AltItem/"
#define STR_PREFIX_VCF_HEADER_CONTIG   URI_ONTOLOGY "/ContigItem/"
#define STR_PREFIX_VARIANT_CALL        URI_ONTOLOGY "/VariantCall/"
#define STR_PREFIX_SEQUENCE            URI_ONTOLOGY "/Sequence/"
#define STR_PREFIX_ORIGIN              URI_MASTER "/Origin/"
#define STR_PREFIX_RDF                 URI_W3 "/1999/02/22-rdf-syntax-ns#"
#define STR_PREFIX_RDFS                URI_W3 "/2000/01/rdf-schema#"
#define STR_PREFIX_OWL                 URI_W3 "/2002/07/owl#"
#define STR_PREFIX_XSD                 URI_W3 "/2001/XMLSchema#"
#define STR_PREFIX_FALDO               "http://biohackathon.org/resource/faldo#"
#define STR_PREFIX_REFERENCE           "http://www.ncbi.nlm.nih.gov/nuccore/"

typedef enum
{
  PREFIX_BASE = 0,
  PREFIX_MASTER,
  PREFIX_SAMPLE,
  PREFIX_VCF_HEADER,
  PREFIX_VCF_HEADER_INFO,
  PREFIX_VCF_HEADER_FORMAT,
  PREFIX_VCF_HEADER_FORMAT_GT,
  PREFIX_VCF_HEADER_FILTER,
  PREFIX_VCF_HEADER_ALT,
  PREFIX_VCF_HEADER_CONTIG,
  PREFIX_VARIANT_CALL,
  PREFIX_SEQUENCE,
  PREFIX_ORIGIN,
  PREFIX_RDF,
  PREFIX_RDFS,
  PREFIX_OWL,
  PREFIX_XSD,
  PREFIX_FALDO,
  PREFIX_REFERENCE
} ontology_prefix;

typedef enum
{
  CLASS_RDF_TYPE = 0,
  CLASS_ORIGIN,
  CLASS_VCF_HEADER,
  CLASS_VCF_HEADER_INFO,
  CLASS_VCF_HEADER_FORMAT,
  CLASS_VCF_HEADER_FILTER,
  CLASS_VCF_HEADER_ALT,
  CLASS_VCF_HEADER_CONTIG,
  CLASS_SAMPLE,
  CLASS_VARIANT_CALL,
  CLASS_HETEROZYGOUS,
  CLASS_MULTIZYGOUS,
  CLASS_NULLIZYGOUS,
  CLASS_HOMOZYGOUS,
  CLASS_HOMOZYGOUS_REFERENCE,
  CLASS_HOMOZYGOUS_ALTERNATIVE
} ontology_class;

typedef enum
{
  PREDICATE_RDF_TYPE = 0,
  PREDICATE_SHA256SUM,
  PREDICATE_CONVERTED_BY,
  PREDICATE_VERSION_INFO,
  PREDICATE_FILENAME,
  PREDICATE_ORIGINATED_FROM,
  PREDICATE_SAMPLE,
  PREDICATE_VARIANT_ID,
  PREDICATE_CHROMOSOME,
  PREDICATE_POSITION,
  PREDICATE_REF,
  PREDICATE_ALT,
  PREDICATE_QUAL,
  PREDICATE_FILTER,
  PREDICATE_PLOIDY,
  PREDICATE_FOUND_IN
} ontology_predicate;

typedef struct
{
  raptor_term **classes;
  raptor_term **predicates;
  raptor_uri  **prefixes;
  raptor_uri **xsds;
  int32_t     classes_length;
  int32_t     predicates_length;
  int32_t     prefixes_length;
  int32_t     xsds_length;
} ontology_t;

#define XSD_STRING              BCF_HT_STR
#define XSD_INTEGER             BCF_HT_INT
#define XSD_FLOAT               BCF_HT_REAL
#define XSD_BOOLEAN             BCF_HT_FLAG

bool ontology_init (ontology_t **ontology_ptr);
void ontology_free (ontology_t *ontology);

raptor_term* term (int32_t index, char *suffix);

/* The following marcros can be used to construct terms (nodes) and URIs.
 * These assume 'config.raptor_world', 'config.uris', 'config.ontology',
 * and 'config.raptor_serializer' exist and have been initialized.
 */
#define uri(index, suffix)                                      \
  raptor_new_uri_relative_to_base (config.raptor_world,         \
                                   config.uris[index],          \
                                   str)

#define class(index)      config.ontology->classes[index]
#define predicate(index)  config.ontology->predicates[index]

#define literal(str, datatype)                                  \
  raptor_new_term_from_literal                                  \
  (config.raptor_world, (unsigned char *)str,                   \
   config.ontology->xsds[datatype],                             \
   NULL)

#define register_statement(stmt)                                \
  raptor_serializer_serialize_statement                         \
  (config.raptor_serializer, stmt);                             \
  raptor_free_statement (stmt)

#define register_statement_reuse_subject(stmt)                  \
  raptor_serializer_serialize_statement                         \
  (config.raptor_serializer, stmt);                             \
  stmt->subject = NULL;                                         \
  raptor_free_statement (stmt)

#define register_statement_reuse_predicate(stmt)                \
  raptor_serializer_serialize_statement                         \
  (config.raptor_serializer, stmt);                             \
  stmt->predicate = NULL;                                       \
  raptor_free_statement (stmt)

#define register_statement_reuse_object(stmt)                   \
  raptor_serializer_serialize_statement                         \
  (config.raptor_serializer, stmt);                             \
  stmt->object = NULL;                                          \
  raptor_free_statement (stmt)

#define register_statement_reuse_subject_predicate(stmt)        \
  raptor_serializer_serialize_statement                         \
  (config.raptor_serializer, stmt);                             \
  stmt->subject = NULL;                                         \
  stmt->predicate = NULL;                                       \
  raptor_free_statement (stmt)

#define register_statement_reuse_subject_object(stmt)           \
  raptor_serializer_serialize_statement                         \
  (config.raptor_serializer, stmt);                             \
  stmt->subject = NULL;                                         \
  stmt->object = NULL;                                          \
  raptor_free_statement (stmt)

#define register_statement_reuse_predicate_object(stmt)         \
  raptor_serializer_serialize_statement                         \
  (config.raptor_serializer, stmt);                             \
  stmt->predicate = NULL;                                       \
  stmt->object = NULL;                                          \
  raptor_free_statement (stmt)

#define register_statement_reuse_all(stmt)                      \
  raptor_serializer_serialize_statement                         \
  (config.raptor_serializer, stmt);                             \
  stmt->subject = NULL;                                         \
  stmt->predicate = NULL;                                       \
  stmt->object = NULL;                                          \
  raptor_free_statement (stmt)


#endif  /* ONTOLOGY_H */
