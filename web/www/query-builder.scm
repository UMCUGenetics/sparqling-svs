;;; Copyright © 2020, 2021  Roel Janssen <roel@gnu.org>
;;;
;;; This program is free software: you can redistribute it and/or
;;; modify it under the terms of the GNU Affero General Public License
;;; as published by the Free Software Foundation, either version 3 of
;;; the License, or (at your option) any later version.
;;;
;;; This program is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
;;; Affero General Public License for more details.
;;;
;;; You should have received a copy of the GNU Affero General Public
;;; License along with this program.  If not, see
;;; <http://www.gnu.org/licenses/>.

(define-module (www query-builder)

  #:export (build-query
            safe-for-query?
            contains-non-az09space))

(define %Az09space
  (string->char-set
   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 "))

(define (contains-non-az09space input)
  (not (string-every %Az09space input)))

(define %query-escape (string->char-set "(){}?#")) ; Also include ";" and "."?

(define (safe-for-query? input)
  (not (string-any %query-escape input)))

(define (build-query format-string . args)
  "Returns a string that is guaranteed to not contain a query
injection, or throws UNSAFE-FOR-QUERY exception otherwise."
  (apply format (append `(#f ,format-string)
                        (map (lambda (arg)
                               (cond
                                [(and (string? arg)
                                      (not (safe-for-query? arg)))
                                 (throw 'unsafe-for-query arg)]
                                [(and (char? arg)
                                      (char-set-contains? %query-escape arg))
                                 (throw 'unsafe-for-query arg)]
                                [else arg]))
                             args))))
