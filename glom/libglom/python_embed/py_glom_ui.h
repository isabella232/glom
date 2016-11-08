/* Glom
 *
 * Copyright (C) 2001-2010 Murray Cumming
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

#ifndef GLOM_PYTHON_GLOM_UI_H
#define GLOM_PYTHON_GLOM_UI_H

#include <boost/python/object_fwd.hpp>

#include <libglom/data_structure/field.h>
#include <libglom/python_embed/py_glom_ui_callbacks.h>
#include <glibmm/ustring.h>

namespace Glom
{

class PyGlomUI
{
public:
  //A default constructor seems to be necessary for boost::python
  PyGlomUI();
  explicit PyGlomUI(const PythonUICallbacks& callbacks);
  ~PyGlomUI() = default;

  /** Navigate to the named table, showing the details view for the specified record.
   */
  void show_table_details(const std::string& table_name, const boost::python::object& primary_key_value);

  /** Navigate to the named table, showing its list view.
   */
  void show_table_list(const std::string& table_name);

  /** Print the current view of the current table.
   */
  void print_layout();

  /** Print the named report from the current table.
   */
  void print_report(const std::string& report_name);

  /** Offer the user the UI to add a new record.
   */
  void start_new_record();

private:
  const PythonUICallbacks* m_callbacks;
};

} //namespace Glom

#endif //GLOM_PYTHON_GLOM_UI_H
