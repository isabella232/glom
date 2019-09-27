## Copyright (c) 2010  Openismus GmbH  <http://www.openismus.com/>
##
## This file is part of Glom.
##
## Glom is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published
## by the Free Software Foundation, either version 2 of the License,
## or (at your option) any later version.
##
## Glom is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Glom.  If not, see <http://www.gnu.org/licenses/>.

include $(top_srcdir)/glom/libglom/filelist.am

book_name  = libglom-$(GLOM_ABI_VERSION)
doc_input  = $(addprefix $(top_srcdir)/,$(libglom_headers)) \
	     $(addprefix $(top_builddir)/,$(libglom_headers_generated))
doc_outdir = .

include $(top_srcdir)/doc-reference.am
