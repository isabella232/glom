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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#ifndef GLOM_XSL_UTILS_H
#define GLOM_XSL_UTILS_H

#include <libglom/data_structure/field.h>
#include <libglom/data_structure/numeric_format.h>

#include <libglom/data_structure/layout/layoutitem_field.h>
#include <libxml++/libxml++.h>


namespace Glom
{

namespace GlomXslUtils
{

/**
 * @result the generated HTML.
 */
Glib::ustring transform(const xmlpp::Document& xml_document, const std::string& xsl_file_path);

} //namespace GlomXslUtils

} //namespace Glom

#endif //GLOM_XSL_UTILS_H

