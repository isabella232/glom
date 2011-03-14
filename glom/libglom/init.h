/* Glom
 *
 * Copyright (C) 2001-2009 Murray Cumming
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef GLOM_LIBGLOM_INIT_H
#define GLOM_LIBGLOM_INIT_H

/** @mainpage libglom Reference Manual
 *
 * @section description Description
 *
 * libglom provides API to access Glom's XML-based Glom::Document structure.
 * Applications may use it to load a .glom document and then call methods
 * on the document to discover the connection details and the data structure, including:
 * - The list of tables
 * - The list of fields in each table
 * - The details of each field, such as field type, title, default value, formatting, etc.
 * - The relationships between tables.
 * - The layout of fields on list and details views.
 * - The layout of print layouts.
 * - The layout of reports.
 *
 * libglom also contains utility functions, such as
 * Glom::Utils::build_sql_select_with_where_clause(), to build the complicated SQL queries
 * used by Glom to retrieve information from the database.
 *
 * See http://git.gnome.org/browse/glom/tree/glom/libglom/example_document_load.cc
 * for a small working example.
 *
 * @section warning Warning
 *
 * libglom is not yet API stable, not properly documented, not thoroughly tested,
 * and not yet really intended for serious use by applications other than
 * <a href="http://www.glom.org/">Glom</a> and
 * <a href="http://gitorious.org/qlom">Qlom</a>.
 *
 * @section basics Basic Usage
 *
 * Include the individual libglom headers. For instance:
 * @code
 * #include <libglom/document/document.h>
 * @endcode
 *
 * If your source file is @c program.cc, you can compile it with:
 * @code
 * g++ program.cc -o program  `pkg-config --cflags --libs glom-1.20`
 * @endcode
 *
 * Alternatively, if using autoconf, use the following in @c configure.ac:
 * @code
 * PKG_CHECK_MODULES([DEPS], [glom-1.20])
 * @endcode
 * Then use the generated @c DEPS_CFLAGS and @c DEPS_LIBS variables in the
 * project @c Makefile.am files. For example:
 * @code
 * program_CPPFLAGS = $(DEPS_CFLAGS)
 * program_LDADD = $(DEPS_LIBS)
 * @endcode
 */

namespace Glom
{

/** This must be used by applications other than Glom,
 * which are unlikely to otherwise initialize the libraries used by libglom.
 * Glom uses it too, just to avoid duplicating code.
 */
void libglom_init();

void libglom_deinit();

/** For some reason, we must call this repeatedly.
 * Maybe it is happening in different processes somehow.
 */
void libglom_pydatetime_import();

/** Whether libglom_init() successfully imported the python DateTime API.
 * This lets other code work around it when it is broken.
 */
bool libglom_pydatetime_imported();

} //namespace Glom

#endif //GLOM_LIBGLOM_INIT_H
