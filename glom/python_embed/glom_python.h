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

#ifndef GLOM_PYTHON_H
#define GLOM_PYTHON_H

#include <libglom/data_structure/field.h>
#include <libglom/document/document.h>
#include <libglom/python_embed/py_glom_ui.h>
#include <glibmm/ustring.h>

namespace Glom
{

/** Check that Python can really import the Glom module,
 * as a runtime sanity check.
 */
bool glom_python_module_is_available();

/** Check that Python can really import the gi.repository module,
 * as a runtime sanity check.
 */
bool gir_python_module_is_available();

/** Check that Python can really import the gda module,
 * as a runtime sanity check.
 */
bool gda_python_module_is_available();

typedef std::map<Glib::ustring, Gnome::Gda::Value> type_map_fields;

/** Run a script, ignoring the python return value.
 * The record object will be writable and the function will receive a ui 
 * parameter so it can control navigation in the UI.
 */
void glom_execute_python_function_implementation(const Glib::ustring& func_impl,
  const type_map_fields& field_values,
  const std::shared_ptr<const Document>& document,
  const Glib::ustring& table_name,
  const std::shared_ptr<const Field>& key_field,
  const Gnome::Gda::Value& key_field_value,
  const Glib::RefPtr<Gnome::Gda::Connection>& opened_connection,
  const PythonUICallbacks& callbacks,
  Glib::ustring& error_message);

/** Run a python calculation, returning the python return value.
 * @param for_script: If this is true then the record object will be writable, 
 * and the function will receive a ui parameter so it can control navigation in the UI.
 */
Gnome::Gda::Value glom_evaluate_python_function_implementation(Field::glom_field_type result_type,
  const Glib::ustring& func_impl,
  const type_map_fields& field_values,
  const std::shared_ptr<const Document>& document,
  const Glib::ustring& table_name,
  const std::shared_ptr<const Field>& key_field,
  const Gnome::Gda::Value& key_field_value,
  const Glib::RefPtr<Gnome::Gda::Connection>& opened_connection,
  Glib::ustring& error_message);

} //namespace Glom

#endif //GLOM_PYTHON_H
