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

#ifndef GLOM_PYTHON_H
#define GLOM_PYTHON_H

#include <glom/libglom/data_structure/field.h>
#include <glibmm/ustring.h>

typedef std::map<Glib::ustring, Gnome::Gda::Value> type_map_fields;

void glom_execute_python_function_implementation(const Glib::ustring& func_impl, const type_map_fields& field_values, Document_Glom* pDocument, const Glib::ustring& table_name);

Gnome::Gda::Value glom_evaluate_python_function_implementation(Field::glom_field_type result_type, const Glib::ustring& func_impl,
  const type_map_fields& field_values, Document_Glom* pDocument, const Glib::ustring& table_name);

#endif //GLOM_PYTHON_H
