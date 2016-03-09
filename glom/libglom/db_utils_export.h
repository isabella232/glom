/* Glom
 *
 * Copyright (C) 2001-2016 Murray Cumming
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

#ifndef GLOM_DB_UTILS_EXPORT_H
#define GLOM_DB_UTILS_EXPORT_H

#include <libglom/document/document.h>
#include <libglom/data_structure/system_prefs.h>
#include <memory> //For shared_ptr<>.

namespace Glom
{

namespace DbUtils
{

void export_data_to_vector(const std::shared_ptr<Document>& document, Document::type_example_rows& the_vector, const FoundSet& found_set, const Document::type_list_layout_groups& sequence);

void export_data_to_stream(const std::shared_ptr<Document>& document, std::ostream& the_stream, const FoundSet& found_set, const Document::type_list_layout_groups& sequence);

} //namespace DbUtils

} //namespace Glom

#endif //GLOM_DB_UTILS_EXPORT_H
