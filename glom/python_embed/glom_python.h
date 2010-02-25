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

#include <libglom/data_structure/field.h>
#include <libglom/document/document.h>
#include <glibmm/ustring.h>

namespace Glom
{

/** Check that Python can really import the Glom module,
 * as a runtime sanity check.
 */
bool glom_python_module_is_available();

/** Check that Python can really import the gda module,
 * as a runtime sanity check.
 */
bool gda_python_module_is_available();

typedef std::map<Glib::ustring, Gnome::Gda::Value> type_map_fields;

void glom_execute_python_function_implementation(const Glib::ustring& func_impl,
  const type_map_fields& field_values,
  Document* pDocument,
  const Glib::ustring& table_name,
  const sharedptr<const Field>& key_field,
  const Gnome::Gda::Value& key_field_value,
  const Glib::RefPtr<Gnome::Gda::Connection>& opened_connection);

Gnome::Gda::Value glom_evaluate_python_function_implementation(Field::glom_field_type result_type,
  const Glib::ustring& func_impl,
  const type_map_fields& field_values,
  Document* pDocument,
  const Glib::ustring& table_name,
  const sharedptr<const Field>& key_field,
  const Gnome::Gda::Value& key_field_value,
  const Glib::RefPtr<Gnome::Gda::Connection>& opened_connection);

} //namespace Glom

#endif //GLOM_PYTHON_H
