# pydoc can write docs for all modules in a directory (or a module name),
# but doesn't like being given a path to any actual file.


# Using pydoc instead:
# html:
#	pydoc -w $(top_builddir)/glom/python_embed/python_module/.libs/
#	mkdir html
#	mv glom_1_30.html html/index.html

book_name = pyglom_$(GLOM_ABI_VERSION_UNDERLINED)

# The name of the sub-directory where the generated documentation
# will be placed.
html_outdir = html

# A list of wildcard patterns matching the documentation files to distribute.
htmlref_patterns = *.css *.gif *.html *.png *.js *.inv *.txt

# Installation directories.
libdocdir    = $(datarootdir)/doc/$(book_name)
referencedir = $(libdocdir)/reference
htmlrefdir   = $(referencedir)/html
htmlref_staticdir = $(htmlrefdir)/_static
htmlref_sourcesdir = $(htmlrefdir)/_sources

# Function: $(call vpath_listall,PATTERN ...)
# Get all filenames which match a PATTERN from the list.  Look for files
# relative to either the current directory or $(srcdir).  Strip $(srcdir)/
# again before returning and remove any duplicates.
vpath_srclist = $(patsubst $(srcdir)/%,%,$(wildcard $(addprefix $(srcdir)/,$(1))))
vpath_listall = $(sort $(wildcard $(1)) $(if $(srcdir:.=),$(vpath_srclist)))

dist_htmlref_DATA = $(call vpath_listall,$(addprefix $(html_outdir)/,$(htmlref_patterns)))
dist_htmlref_static_DATA = $(call vpath_listall,$(addprefix $(html_outdir)/_static/,$(htmlref_patterns)))
dist_htmlref_sources_DATA = $(call vpath_listall,$(addprefix $(html_outdir)/_sources/,$(htmlref_patterns)))

MAINTAINERCLEANFILES = $(html_outdir)/_sources/* $(html_outdir)/_static/* $(html_outdir)/*

html/index.html:
	-$(AM_V_at)rm -fr $(html_outdir)
	$(AM_V_GEN)$(SPHINX_BUILD) -b html . $(html_outdir)

# Explicitly depend on the files to be distributed or installed.
if ENABLE_DOCUMENTATION
all-local: html/index.html
else
all-local:
endif

# Instruct GNU make to delete the targets of a rule after it failed, in
# order to avoid the complication of handling that situation manually.
.DELETE_ON_ERROR:
