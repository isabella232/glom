INCLUDES = \
  $(EGG_CFLAGS) \
  -I@srcdir@/../util \
  -DEGG_COMPILATION \
  -DGTK_DISABLE_DEPRECATED \
  -DGDK_DISABLE_DEPRECATED \
  -DG_DISABLE_DEPRECATED

noinst_LTLIBRARIES = libeggtoolpalette.la
noinst_PROGRAMS = testtoolpalette

libeggtoolpalette_la_SOURCES = eggenumaction.c eggtoolpalette.c eggtoolitemgroup.c eggtoolpalette.h eggtoolitemgroup.h eggenumaction.h eggtoolpaletteprivate.h

testtoolpalette_SOURCES = testtoolpalette.c
testtoolpalette_LDFLAGS = libeggtoolpalette.la ../util/libeggutil.la $(EGG_LIBS)
testtoolpalette_DEPENDENCIES = libeggtoolpalette.la ../util/libeggutil.la

-include Makefile.local
