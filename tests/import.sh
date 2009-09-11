#!/bin/sh

# hm, can someone please put me in the test makefile?

export G_DEBUG=gc-friendly
export G_SLICE=always-malloc

which valgrind && valgrind --tool=memcheck --leak-check=full --leak-resolution=high --trace-children=yes --num-callers=30 tests/import/test_parsing
