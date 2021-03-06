;;; Copyright © 2016, 2017, 2018, 2019, 2020  Roel Janssen <roel@gnu.org>
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

(define-module (www pages)
  #:use-module (srfi srfi-1)
  #:use-module (www config)
  #:use-module (www util)
  #:use-module (www db projects)
  #:use-module (sparql driver)
  #:use-module (web response)
  #:use-module (ice-9 rdelim)
  #:export (page-root-template
            page-empty-template
            page-menu
            dependent-scripts-and-styles))

(define page-title-prefix (string-append (www-name) " | "))

(define pages
  '(("/project-details" "Overview")
    ("/collect"         "Collect")
    ("/structure"       "Structure")
    ("/query"           "Query")
    ("/report"          "Report")
    ("/automate"        "Automate")))

(define (page-static-menu request-path)
  `(ul (@ (role "menubar"))
    ,(map (lambda (page)
            (let ((page-name (string-titlecase
                              (string-map
                               (lambda (char)
                                 (if (eq? char #\-) #\space char))
                               page) 0 1)))
              (cond
               [(string-suffix? page request-path)
                `(li (@ (role "menuitem")
                        (class "active")) ,page-name)]
               [else
                `(li (@ (role "menuitem"))
                     (a (@ (href ,(string-append "/spage/" page)))
                        ,page-name))])))
          (static-page-modules))
   (li (@ (role "menuitem")
          (class ,(string-append
                   (if (and (string? request-path)
                            (string= "/login" request-path))
                        "active " "")
                   "login")))
       (a (@ (href "/login")) "Log in"))))

(define (page-menu username request-path)
  `(ul (@ (role "menubar"))
    (li (@ (role "menuitem")
           (class ,(if (or (string-prefix? "/edit-connection" request-path)
                           (string-prefix? "/edit-form" request-path)
                           (string= request-path "/dashboard")) "active" "")))
        ,(if (string= request-path "/dashboard")
             "Dashboard"
             (if (or (string-prefix? "/edit-connection" request-path)
                     (string-prefix? "/edit-form" request-path))
                 `(a (@ (href "/dashboard")) "← Go back")
                 `(a (@ (href "/dashboard")) "Dashboard"))))
   ,(map (lambda (project)
           (cond
            [(string-suffix? (project-id project) request-path)
             `(li (@ (role "menuitem")
                     (class "active")) ,(project-name project))]
            [else
             `(li (@ (role "menuitem"))
               (a (@ (href ,(string-append "/project-details/"
                                           (project-id project))))
                  ,(project-name project)))]))
         (projects-by-user username))
   (li (@ (role "menuitem")
          (class ,(if (string= "/create-project" request-path)
                      "active new-menu-item"
                      "new-menu-item")))
       (a (@ (href "/create-project")) ,(icon 'plus #t)))
   (li (@ (role "menuitem")
          (class "logout")) (a (@ (href "/logout")) "Log out"))))

(define (page-submenu username request-path)
  (let* [(project-id (basename request-path))
         (show-menu-item
          (lambda (item show-spacer?)
            (cond
             [(string= (substring (car item) 1)
                       (car (string-split (substring request-path 1) #\/)))
              `(,(if show-spacer? `(li (@ (class "spacer")) "→") '())
                (li (@ (role "menuitem")
                       (class "active")) ,(cadr item)))]
             [else
              `(,(if show-spacer? `(li (@ (class "spacer")) "→") '())
                (li (@ (role "menuitem"))
                 (a (@ (href ,(string-append (car item) "/" project-id)))
                    ,(cadr item))))])))]
    (cond
     ;; Sub-pages for the “structure” page.
     [(or (string-prefix? "/exploratory" request-path)
          (string-prefix? "/prompt" request-path))
      `(ul (@ (role "menubar"))
        (li (@ (role "menuitem"))
         (a (@ (href ,(string-append "/structure/" project-id)))
            "← Go back")))]

     ;; Sub-pages for the “collect” page.
     [(or (string-prefix? "/forms" request-path)
          (string-prefix? "/create-form" request-path)
          (string-prefix? "/import" request-path))
      `(ul (@ (role "menubar"))
        (li (@ (role "menuitem"))
            (a (@ (href ,(string-append "/collect/" project-id)))
               "← Go back")))]

     ;; The regular submenu.
     [else
      `(ul (@ (role "menubar"))
        ,(show-menu-item (car pages) #f)
        ,(map (lambda (item) (show-menu-item item #t)) (cdr pages)))])))

(define (show-footer)
  `(div (@ (role "contentinfo")
           (id "footer"))
        (p "v" ,(www-version) " | "
           (a (@ (href "https://github.com/UMCUgenetics/sparqling-genomics")
                 (target "_blank"))
              "Source code"))))

(define* (dependent-scripts-and-styles dependencies #:optional (out '()))
  (if (null? dependencies)
      out
      (let ((dependency (car dependencies))
            (remainder  (cdr dependencies)))
        (cond
         ((eq? 'jquery dependency)
          (dependent-scripts-and-styles remainder
            (cons '(script (@ (src "/static/js/jquery-3.2.1.min.js")) "") out)))
         ((eq? 'jquery-ui dependency)
          (dependent-scripts-and-styles remainder
           (cons* `(link (@ (rel "stylesheet") (type "text/css")
                            (href "/static/css/jquery-ui.min.css")))
                  `(script (@ (src "/static/js/jquery-ui.min.js")) "")
                  out)))
         ((eq? 'prompt dependency)
          (dependent-scripts-and-styles remainder
            (cons '(script (@ (src "/static/js/prompt.js")) "") out)))
         ((eq? 'projects dependency)
          (dependent-scripts-and-styles remainder
            (cons '(script (@ (src "/static/js/projects.js")) "") out)))
         ((eq? 'sessions dependency)
          (dependent-scripts-and-styles remainder
            (cons '(script (@ (src "/static/js/sessions.js")) "") out)))
         ((eq? 'connections dependency)
          (dependent-scripts-and-styles remainder
            (cons '(script (@ (src "/static/js/connections.js")) "") out)))
         ((eq? 'import dependency)
          (dependent-scripts-and-styles remainder
            (cons '(script (@ (src "/static/js/import.js")) "") out)))
         ((eq? 'exploratory dependency)
          (dependent-scripts-and-styles remainder
            (cons* '(script (@ (src "/static/js/base32.js")) "")
                   '(script (@ (src "/static/js/exploratory.js")) "")
                   out)))
         ((eq? 'd3 dependency)
          (dependent-scripts-and-styles remainder
            (cons '(script (@ (src "/static/js/d3.min.js")) "") out)))
         ((eq? 'plottable-query dependency)
          (dependent-scripts-and-styles remainder
            (cons '(script (@ (src "/static/js/plottable-query.js")) "") out)))
         ((eq? 'datatables dependency)
          (dependent-scripts-and-styles remainder
            (cons* '(link (@ (rel "stylesheet")
                             (type "text/css")
                             (href "/static/css/datatables.min.css")))
                   '(script (@ (src "/static/js/jquery.dataTables.min.js")) "")
                   out)))
         ((eq? 'ace dependency)
          (dependent-scripts-and-styles remainder
            (cons* '(script (@ (src "/static/js/ace/ace.js")) "")
                   '(script (@ (src "/static/js/ace/ext-language_tools.js")) "")
                   out)))
         ((eq? 'forms dependency)
          (dependent-scripts-and-styles remainder
            (cons* '(script (@ (src "/static/js/base32.js")) "")
                   '(script (@ (src "/static/js/forms.js")) "")
                   out)))
         (else
          (dependent-scripts-and-styles remainder out))))))

(define* (page-root-template username title request-path content-tree #:key (dependencies '()))
  `((html (@ (lang "en"))
     (head
      (title ,(string-append page-title-prefix title))
      (meta (@ (http-equiv "Content-Type") (content "text/html; charset=utf-8")))
      (link (@ (rel "icon")
               (type "image/x-icon")
               (href "/static/images/favicon.ico")))
      ,(dependent-scripts-and-styles (reverse dependencies))
      (link (@ (rel "stylesheet")
               (href "/static/css/main.css")
               (type "text/css")
               (media "screen"))))
     (body
      (div (@ (id "wrapper"))
           (div (@ (role "banner")
                   (id "header"))
                (div (@ (class "title")
                        (style "text-align: center"))
                     (img (@ (src "/static/images/logo.png")
                             (alt ,(www-name)))))
                (div (@ (role "navigation")
                        (class "menu")) ,(page-menu username request-path))
                ,(if (any (lambda (x) x)
                          (map (lambda (path)
                                 (string-prefix? path request-path))
                               '("/project-details/"  "/collect/"
                                 "/structure/"        "/query/"
                                 "/report/"           "/automate/"
                                 "/exploratory/"      "/prompt/"
                                 "/forms/"            "/create-form/"
                                 "/import/")))
                     `(div (@ (class "submenu"))
                           ,(page-submenu username request-path))
                     `()))
           (div (@ (role "main")
                   (id "content")) ,content-tree)
           ,(show-footer))))))

(define* (page-empty-template title request-path content-tree #:key (dependencies '()))
  `((html (@ (lang "en"))
     (head
      (title ,(string-append page-title-prefix title))
      (meta (@ (http-equiv "Content-Type") (content "text/html; charset=utf-8")))
      (link (@ (rel "icon")
               (type "image/x-icon")
               (href "/static/images/favicon.ico")))
      ,(dependent-scripts-and-styles (reverse dependencies))
      (link (@ (rel "stylesheet")
               (href "/static/css/main.css")
               (type "text/css"))))
     (body
      (div (@ (id "wrapper"))
           (div (@ (role "banner")
                   (id "header"))
                (div (@ (class "title")
                        (style "text-align: center"))
                     (img (@ (src "/static/images/logo.png")
                             (alt ,(www-name)))))
                (div (@ (role "navigation")
                        (class "menu"))
                     ,(page-static-menu request-path)))
           (div (@ (role "main")
                   (id "content")) ,content-tree)
           ,(show-footer))))))
