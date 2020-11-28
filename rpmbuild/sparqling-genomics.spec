Name:           sparqling-genomics
Version:        0.99.12
Release:        1%{?dist}
Summary:        SPARQLing-genomics
License:        GPLv3+
URL:            https://www.sparqling-genomics.org
Source0:        https://github.com/UMCUGenetics/%{name}/releases/download/%{version}/%{name}-%{version}.tar.gz
BuildRequires:  gcc make pkgconfig curl guile-devel raptor2-devel libxml2-devel gnutls-devel libpng-devel
Requires:       guile raptor2 curl libxml2 gnutls libpng

%description
SPARQLing genomics is a combination of tools and practices to create a
knowledge graph to make discovering, connecting, and collaborating easy.


%prep
%autosetup


%build
%configure
%make_build


%install
rm -rf $RPM_BUILD_ROOT
%make_install


%files
/usr/bin/bam2rdf
/usr/bin/ega2rdf
/usr/bin/folder2rdf
/usr/bin/json2rdf
/usr/bin/sg-auth-manager
/usr/bin/sg-web
/usr/bin/sg-web-test
/usr/bin/table2rdf
/usr/bin/vcf2rdf
/usr/bin/virtuoso-config
/usr/bin/xml2rdf
/usr/lib64/guile/2.0/extensions/libhashing.a
/usr/lib64/guile/2.0/extensions/libhashing.la
/usr/lib64/guile/2.0/extensions/libhashing.so
/usr/lib64/guile/2.0/extensions/libhashing.so.0
/usr/lib64/guile/2.0/extensions/libhashing.so.0.0.0
/usr/lib64/guile/2.0/extensions/libpdf_report.a
/usr/lib64/guile/2.0/extensions/libpdf_report.la
/usr/lib64/guile/2.0/extensions/libpdf_report.so
/usr/lib64/guile/2.0/extensions/libpdf_report.so.0
/usr/lib64/guile/2.0/extensions/libpdf_report.so.0.0.0
/usr/lib64/guile/2.0/site-ccache/auth-manager/api.go
/usr/lib64/guile/2.0/site-ccache/auth-manager/config-reader.go
/usr/lib64/guile/2.0/site-ccache/auth-manager/config.go
/usr/lib64/guile/2.0/site-ccache/auth-manager/permission-check.go
/usr/lib64/guile/2.0/site-ccache/auth-manager/virtuoso.go
/usr/lib64/guile/2.0/site-ccache/json.go
/usr/lib64/guile/2.0/site-ccache/json/builder.go
/usr/lib64/guile/2.0/site-ccache/json/parser.go
/usr/lib64/guile/2.0/site-ccache/json/syntax.go
/usr/lib64/guile/2.0/site-ccache/ldap/authenticate.go
/usr/lib64/guile/2.0/site-ccache/logger.go
/usr/lib64/guile/2.0/site-ccache/sparql/driver.go
/usr/lib64/guile/2.0/site-ccache/sparql/lang.go
/usr/lib64/guile/2.0/site-ccache/sparql/parser.go
/usr/lib64/guile/2.0/site-ccache/sparql/stream.go
/usr/lib64/guile/2.0/site-ccache/sparql/util.go
/usr/lib64/guile/2.0/site-ccache/test/endpoint.go
/usr/lib64/guile/2.0/site-ccache/test/sparql-parser.go
/usr/lib64/guile/2.0/site-ccache/www/base64.go
/usr/lib64/guile/2.0/site-ccache/www/components/project-graphs.go
/usr/lib64/guile/2.0/site-ccache/www/components/query-history.go
/usr/lib64/guile/2.0/site-ccache/www/components/rdf-stores.go
/usr/lib64/guile/2.0/site-ccache/www/components/sessions.go
/usr/lib64/guile/2.0/site-ccache/www/config-reader.go
/usr/lib64/guile/2.0/site-ccache/www/config.go
/usr/lib64/guile/2.0/site-ccache/www/db/api.go
/usr/lib64/guile/2.0/site-ccache/www/db/cache.go
/usr/lib64/guile/2.0/site-ccache/www/db/connections.go
/usr/lib64/guile/2.0/site-ccache/www/db/exploratory.go
/usr/lib64/guile/2.0/site-ccache/www/db/forms.go
/usr/lib64/guile/2.0/site-ccache/www/db/graphs.go
/usr/lib64/guile/2.0/site-ccache/www/db/orcid.go
/usr/lib64/guile/2.0/site-ccache/www/db/projects.go
/usr/lib64/guile/2.0/site-ccache/www/db/prompt.go
/usr/lib64/guile/2.0/site-ccache/www/db/queries.go
/usr/lib64/guile/2.0/site-ccache/www/db/reports.go
/usr/lib64/guile/2.0/site-ccache/www/db/sessions.go
/usr/lib64/guile/2.0/site-ccache/www/forms.go
/usr/lib64/guile/2.0/site-ccache/www/hashing.go
/usr/lib64/guile/2.0/site-ccache/www/pages.go
/usr/lib64/guile/2.0/site-ccache/www/pages/automate.go
/usr/lib64/guile/2.0/site-ccache/www/pages/collect.go
/usr/lib64/guile/2.0/site-ccache/www/pages/connection-failure.go
/usr/lib64/guile/2.0/site-ccache/www/pages/create-project.go
/usr/lib64/guile/2.0/site-ccache/www/pages/dashboard.go
/usr/lib64/guile/2.0/site-ccache/www/pages/edit-connection.go
/usr/lib64/guile/2.0/site-ccache/www/pages/error.go
/usr/lib64/guile/2.0/site-ccache/www/pages/exploratory.go
/usr/lib64/guile/2.0/site-ccache/www/pages/import.go
/usr/lib64/guile/2.0/site-ccache/www/pages/login.go
/usr/lib64/guile/2.0/site-ccache/www/pages/project-details.go
/usr/lib64/guile/2.0/site-ccache/www/pages/prompt-session-table.go
/usr/lib64/guile/2.0/site-ccache/www/pages/prompt.go
/usr/lib64/guile/2.0/site-ccache/www/pages/query-history.go
/usr/lib64/guile/2.0/site-ccache/www/pages/query.go
/usr/lib64/guile/2.0/site-ccache/www/pages/report.go
/usr/lib64/guile/2.0/site-ccache/www/pages/structure.go
/usr/lib64/guile/2.0/site-ccache/www/query-builder.go
/usr/lib64/guile/2.0/site-ccache/www/reports.go
/usr/lib64/guile/2.0/site-ccache/www/requests-api.go
/usr/lib64/guile/2.0/site-ccache/www/requests-beacon.go
/usr/lib64/guile/2.0/site-ccache/www/requests.go
/usr/lib64/guile/2.0/site-ccache/www/util.go
/usr/lib64/systemd/system/sg-auth-manager.service
/usr/lib64/systemd/system/sg-web.service
/usr/share/guile/site/2.0/auth-manager/api.scm
/usr/share/guile/site/2.0/auth-manager/config-reader.scm
/usr/share/guile/site/2.0/auth-manager/config.scm
/usr/share/guile/site/2.0/auth-manager/permission-check.scm
/usr/share/guile/site/2.0/auth-manager/virtuoso.scm
/usr/share/guile/site/2.0/json.scm
/usr/share/guile/site/2.0/json/builder.scm
/usr/share/guile/site/2.0/json/parser.scm
/usr/share/guile/site/2.0/json/syntax.scm
/usr/share/guile/site/2.0/ldap/authenticate.scm
/usr/share/guile/site/2.0/logger.scm
/usr/share/guile/site/2.0/sparql/driver.scm
/usr/share/guile/site/2.0/sparql/lang.scm
/usr/share/guile/site/2.0/sparql/parser.scm
/usr/share/guile/site/2.0/sparql/stream.scm
/usr/share/guile/site/2.0/sparql/util.scm
/usr/share/guile/site/2.0/test/endpoint.scm
/usr/share/guile/site/2.0/test/sparql-parser.scm
/usr/share/guile/site/2.0/www/base64.scm
/usr/share/guile/site/2.0/www/components/project-graphs.scm
/usr/share/guile/site/2.0/www/components/query-history.scm
/usr/share/guile/site/2.0/www/components/rdf-stores.scm
/usr/share/guile/site/2.0/www/components/sessions.scm
/usr/share/guile/site/2.0/www/config-reader.scm
/usr/share/guile/site/2.0/www/config.scm
/usr/share/guile/site/2.0/www/db/api.scm
/usr/share/guile/site/2.0/www/db/cache.scm
/usr/share/guile/site/2.0/www/db/connections.scm
/usr/share/guile/site/2.0/www/db/exploratory.scm
/usr/share/guile/site/2.0/www/db/forms.scm
/usr/share/guile/site/2.0/www/db/graphs.scm
/usr/share/guile/site/2.0/www/db/orcid.scm
/usr/share/guile/site/2.0/www/db/projects.scm
/usr/share/guile/site/2.0/www/db/prompt.scm
/usr/share/guile/site/2.0/www/db/queries.scm
/usr/share/guile/site/2.0/www/db/reports.scm
/usr/share/guile/site/2.0/www/db/sessions.scm
/usr/share/guile/site/2.0/www/forms.scm
/usr/share/guile/site/2.0/www/hashing.scm
/usr/share/guile/site/2.0/www/pages.scm
/usr/share/guile/site/2.0/www/pages/automate.scm
/usr/share/guile/site/2.0/www/pages/collect.scm
/usr/share/guile/site/2.0/www/pages/connection-failure.scm
/usr/share/guile/site/2.0/www/pages/create-project.scm
/usr/share/guile/site/2.0/www/pages/dashboard.scm
/usr/share/guile/site/2.0/www/pages/edit-connection.scm
/usr/share/guile/site/2.0/www/pages/error.scm
/usr/share/guile/site/2.0/www/pages/exploratory.scm
/usr/share/guile/site/2.0/www/pages/import.scm
/usr/share/guile/site/2.0/www/pages/login.scm
/usr/share/guile/site/2.0/www/pages/project-details.scm
/usr/share/guile/site/2.0/www/pages/prompt-session-table.scm
/usr/share/guile/site/2.0/www/pages/prompt.scm
/usr/share/guile/site/2.0/www/pages/query-history.scm
/usr/share/guile/site/2.0/www/pages/query.scm
/usr/share/guile/site/2.0/www/pages/report.scm
/usr/share/guile/site/2.0/www/pages/structure.scm
/usr/share/guile/site/2.0/www/query-builder.scm
/usr/share/guile/site/2.0/www/reports.scm
/usr/share/guile/site/2.0/www/requests-api.scm
/usr/share/guile/site/2.0/www/requests-beacon.scm
/usr/share/guile/site/2.0/www/requests.scm
/usr/share/guile/site/2.0/www/util.scm
/usr/share/sparqling-genomics/deployment/virtuoso-machine.scm
/usr/share/sparqling-genomics/ontologies/sparqling-genomics.ttl
/usr/share/sparqling-genomics/web/static/css/datatables.min.css
/usr/share/sparqling-genomics/web/static/css/jquery-ui.min.css
/usr/share/sparqling-genomics/web/static/css/main.css
/usr/share/sparqling-genomics/web/static/fonts/FiraMono-Bold.ttf
/usr/share/sparqling-genomics/web/static/fonts/FiraMono-Regular.ttf
/usr/share/sparqling-genomics/web/static/fonts/Roboto-Bold.ttf
/usr/share/sparqling-genomics/web/static/fonts/Roboto-Light.ttf
/usr/share/sparqling-genomics/web/static/fonts/Roboto-LightItalic.ttf
/usr/share/sparqling-genomics/web/static/images/favicon.ico
/usr/share/sparqling-genomics/web/static/images/icons/check-white.png
/usr/share/sparqling-genomics/web/static/images/icons/check-white.svg
/usr/share/sparqling-genomics/web/static/images/icons/check.png
/usr/share/sparqling-genomics/web/static/images/icons/check.svg
/usr/share/sparqling-genomics/web/static/images/icons/checkmark.svg
/usr/share/sparqling-genomics/web/static/images/icons/locked.png
/usr/share/sparqling-genomics/web/static/images/icons/locked.svg
/usr/share/sparqling-genomics/web/static/images/icons/pdf.png
/usr/share/sparqling-genomics/web/static/images/icons/pdf.svg
/usr/share/sparqling-genomics/web/static/images/icons/plus.png
/usr/share/sparqling-genomics/web/static/images/icons/plus.svg
/usr/share/sparqling-genomics/web/static/images/icons/return-white.png
/usr/share/sparqling-genomics/web/static/images/icons/return-white.svg
/usr/share/sparqling-genomics/web/static/images/icons/return.png
/usr/share/sparqling-genomics/web/static/images/icons/return.svg
/usr/share/sparqling-genomics/web/static/images/icons/save.png
/usr/share/sparqling-genomics/web/static/images/icons/save.svg
/usr/share/sparqling-genomics/web/static/images/icons/unlocked.png
/usr/share/sparqling-genomics/web/static/images/icons/unlocked.svg
/usr/share/sparqling-genomics/web/static/images/icons/up-white.png
/usr/share/sparqling-genomics/web/static/images/icons/up-white.svg
/usr/share/sparqling-genomics/web/static/images/icons/up.png
/usr/share/sparqling-genomics/web/static/images/icons/up.svg
/usr/share/sparqling-genomics/web/static/images/icons/x-white.png
/usr/share/sparqling-genomics/web/static/images/icons/x-white.svg
/usr/share/sparqling-genomics/web/static/images/icons/x.png
/usr/share/sparqling-genomics/web/static/images/icons/x.svg
/usr/share/sparqling-genomics/web/static/images/logo.png
/usr/share/sparqling-genomics/web/static/images/orcid-logo.png
/usr/share/sparqling-genomics/web/static/images/sort_asc.png
/usr/share/sparqling-genomics/web/static/images/sort_asc_disabled.png
/usr/share/sparqling-genomics/web/static/images/sort_both.png
/usr/share/sparqling-genomics/web/static/images/sort_desc.png
/usr/share/sparqling-genomics/web/static/images/ui-icons_444444_256x240.png
/usr/share/sparqling-genomics/web/static/images/ui-icons_555555_256x240.png
/usr/share/sparqling-genomics/web/static/images/ui-icons_777620_256x240.png
/usr/share/sparqling-genomics/web/static/images/ui-icons_777777_256x240.png
/usr/share/sparqling-genomics/web/static/images/ui-icons_cc0000_256x240.png
/usr/share/sparqling-genomics/web/static/images/ui-icons_ffffff_256x240.png
/usr/share/sparqling-genomics/web/static/js/ace/ace.js
/usr/share/sparqling-genomics/web/static/js/ace/ext-language_tools.js
/usr/share/sparqling-genomics/web/static/js/ace/ext-searchbox.js
/usr/share/sparqling-genomics/web/static/js/ace/mode-sparql.js
/usr/share/sparqling-genomics/web/static/js/ace/theme-crimson_editor.js
/usr/share/sparqling-genomics/web/static/js/base32.js
/usr/share/sparqling-genomics/web/static/js/connections.js
/usr/share/sparqling-genomics/web/static/js/d3.min.js
/usr/share/sparqling-genomics/web/static/js/exploratory.js
/usr/share/sparqling-genomics/web/static/js/import.js
/usr/share/sparqling-genomics/web/static/js/jquery-3.2.1.min.js
/usr/share/sparqling-genomics/web/static/js/jquery-ui.min.js
/usr/share/sparqling-genomics/web/static/js/jquery.dataTables.min.js
/usr/share/sparqling-genomics/web/static/js/plottable-query.js
/usr/share/sparqling-genomics/web/static/js/projects.js
/usr/share/sparqling-genomics/web/static/js/prompt.js
/usr/share/sparqling-genomics/web/static/js/query-editor.js
/usr/share/sparqling-genomics/web/static/js/query-history.js
/usr/share/sparqling-genomics/web/static/js/sessions.js

%license COPYING
%doc doc/sparqling-genomics.pdf
%config
/etc/sparqling-genomics/sg-auth-manager.xml
/etc/sparqling-genomics/sg-web.xml

%changelog
* Wed Nov 25 2020 root
- 
