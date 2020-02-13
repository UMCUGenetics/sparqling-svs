;;; Copyright © 2020 Roel Janssen <roel@gnu.org>
;;;
;;; This program is free software; you can redistribute it and/or modify it
;;; under the terms of the GNU General Public License as published by
;;; the Free Software Foundation; either version 3 of the License, or (at
;;; your option) any later version.
;;;
;;; This program is distributed in the hope that it will be useful, but
;;; WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;
;;; You should have received a copy of the GNU General Public License
;;; along with this program.  If not, see <http://www.gnu.org/licenses/>.

(define-module (sparql parser)
  #:use-module (oop goops)
  #:export (<query>

            query-type
            set-query-type!

            query-base
            set-query-base!

            query-prefixes
            set-query-prefixes!

            query-quads
            set-query-quads!

            query-triplets
            set-query-triplets!

            parse-query))

(define-class <query> ()

  (type     #:init-value #nil
            #:getter query-type
            #:setter set-query-type!)

  (base     #:init-value #nil
            #:getter query-base
            #:setter set-query-base!)

  (prefixes #:init-value '()
            #:getter query-prefixes
            #:setter set-query-prefixes!)

  (quads    #:init-value '()
            #:getter query-quads
            #:setter set-query-quads!)

  (triplets #:init-value '()
            #:getter query-triplets
            #:setter set-query-triplets!))

(define (parse-query query)
  "Returns an instace of <query>."

  (define (string-contains-ci-surrounded-by-whitespace s1 s2 start)
    (let [(pstart (string-contains-ci s1 s2 start))]
      (cond
       [(not pstart)                            #f]
       [(= pstart 0)                            pstart]
       [(and (> pstart 0)

             (char-whitespace?
              (string-ref s1 (- pstart 1)))

             (catch 'out-of-range
               (lambda _
                 (char-whitespace?
                  (string-ref s1
                    (+ pstart
                       (string-length s2)))))
               (lambda _ #t)))                   pstart]
       [else                                     #f])))

  (define (read-prefixes out text start)
    (let* [(prefix-start  (string-contains-ci-surrounded-by-whitespace
                           text "prefix" start))
           (shortcode-end (when prefix-start
                            (string-index text #\: (+ prefix-start 7))))
           (uri-start     (unless (unspecified? shortcode-end)
                            (string-index text #\< (+ shortcode-end 1))))
           (uri-end       (unless (unspecified? uri-start)
                            (string-index text #\> (+ uri-start 1))))]
      (if (unspecified? uri-end)
          start
          (begin
            (set-query-prefixes! out
              (cons `(;; Cut out the shortcode.
                      ,(string->symbol
                        (string-trim-both
                         (string-copy text (+ prefix-start 7) shortcode-end)))
                      .
                      ;; Cut out the URI.
                      ,(string-copy text (+ uri-start 1) uri-end))
                    (query-prefixes out)))
            (read-prefixes out text (+ uri-end 1))))))

  (define (determine-query-type out text start)
    (let [(select-type    (string-contains-ci-surrounded-by-whitespace
                           text "select" start))
          (insert-type    (string-contains-ci-surrounded-by-whitespace
                           text "insert" start))
          (delete-type    (string-contains-ci-surrounded-by-whitespace
                           text "delete" start))
          (clear-type     (string-contains-ci-surrounded-by-whitespace
                           text "clear graph" start))]
      (set-query-type! out
       (cond
        [(and select-type
              (not insert-type)
              (not delete-type)
              (not clear-type))
         'SELECT]
        [(and insert-type
              delete-type
              (not select-type)
              (not clear-type))
         'INSERT-DELETE]
        [(and insert-type
              (not select-type)
              (not clear-type))
         'INSERT]
        [(and delete-type
              (not select-type)
              (not clear-type))
         'DELETE]
        [(and clear-type
              (not select-type)
              (not insert-type)
              (not delete-type))
         'CLEAR]
        [else 'UNKNOWN]))))

  (let* [(out (make <query>))]
    ;; The following functions write their findings to ‘out’ as side-effects.
    (let ((after-prefixes-position (read-prefixes out query 0)))
      (determine-query-type out query after-prefixes-position))
    out))


