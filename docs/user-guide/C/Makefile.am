SUBDIRS = figures

figdir = figures
docname = glom
lang = C
omffile = glom-C.omf
entities = legal.xml
include $(top_srcdir)/xmldocs.make
dist-hook: app-dist-hook



# Build a local copy (yelp normally transforms+renders it at runtime)

DOCBOOK_STYLESHEET ?= http://docbook.sourceforge.net/release/xsl/current/html/chunk.xsl

html/index.html: glom.xml
	-rm -rf html
	$(mkinstalldirs) html
	xsltproc -o html/ --xinclude --catalogs $(DOCBOOK_STYLESHEET) $<

validate_original: glom.xml
	xmllint --xinclude --valid --noout --catalogs $<

validate: glom.xml
	xmllint --xinclude --valid --noout --catalogs $<



