# pydoc can write docs for all modules in a directory (or a module name), 
# but doesn't like being given a path to any actual file.

html:
	pydoc -w $(top_builddir)/glom/python_embed/python_module/.libs/
	mkdir html
	mv glom_1_14.html html/index.html

MAINTAINERCLEANFILES = html/*
