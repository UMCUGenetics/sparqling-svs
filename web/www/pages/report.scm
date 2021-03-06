;;; Copyright © 2019  Roel Janssen <roel@gnu.org>
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

(define-module (www pages report)
  #:use-module (www db reports)
  #:use-module (www pages)
  #:use-module (www util)

  #:export (page-report))

(define* (page-report request-path username hash #:key (post-data ""))
  (let ((reports        (reports-for-project hash))
        (sweave-reports (r-sweave-reports-for-project hash)))
    (page-root-template username "Report" request-path
     `((h2 "Report")
       ,(if (and (null? reports)
                 (null? sweave-reports))
            '(p "No reporting modules have been configured.")
            `(,(if (not (null? reports))
                   (map (lambda (report)
                          (let ((report-overview (assoc-ref report 'report-overview))
                                (title           (assoc-ref report 'title)))
                            `((h3 ,title)
                              ,(report-overview))))
                        reports)
                   '())
              ,(if (not (null? sweave-reports))
                   `((h3 "Sweave reports")
                     (table (@ (class "item-table"))
                            (tr (th "Name")
                                (th "Actions"))
                            ,(map (lambda (report)
                                    `(tr (td ,(basename (assoc-ref report 'filename) ".Rnw"))
                                         (td (@ (class "button-column right-button-column"))
                                             (a (@ (href ,(string-append
                                                           "/sweave-report/" hash
                                                           "/" (assoc-ref report 'md5) ".pdf")))
                                                ,(icon 'pdf)))))
                                  sweave-reports))))))))))
