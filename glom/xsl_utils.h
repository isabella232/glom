/* Glom
 *
 * Copyright (C) 2001-2004 Murray Cumming
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

#ifndef GLOM_XSL_UTILS_H
#define GLOM_XSL_UTILS_H

#include <glom/libglom/data_structure/field.h>
#include <glom/libglom/data_structure/numeric_format.h>

#include <glom/libglom/data_structure/layout/layoutitem_field.h>
#include <libxml++/libxml++.h>

///field, ascending
typedef std::pair< sharedptr<const LayoutItem_Field>, bool> type_pair_sort_field;
typedef std::list<type_pair_sort_field> type_sort_clause;

namespace Gtk
{
  class Window;
}

namespace GlomXslUtils
{

void transform_and_open(const xmlpp::Document& xml_document, const Glib::ustring& xsl_file_path, Gtk::Window* parent_window = 0);
Glib::ustring xslt_process(const xmlpp::Document& xml_document, const std::string& filepath_xslt);

} //namespace GlomXslUtils

#endif //GLOM_XSL_UTILS_H

