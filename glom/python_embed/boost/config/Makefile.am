INCLUDES = -I@top_srcdir@/ $(GLOM_CFLAGS) $(PYTHON_INCLUDES)

SUBDIRS = abi compiler platform stdlib

noinst_LIBRARIES = libpython_embed_boost_config.a
libpython_embed_boost_config_a_SOURCES = abi_prefix.hpp auto_link.hpp requires_threads.hpp \
  select_platform_config.hpp suffix.hpp abi_suffix.hpp posix_features.hpp select_compiler_config.hpp \
  select_stdlib_config.hpp user.hpp

#libpython_embed_boost_config_a_LIBADD = 
