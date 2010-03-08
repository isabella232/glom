/* Glom
 *
 * Copyright (C) 2001-2005 Murray Cumming
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

#ifndef GLOM_PYTHON_GLOM_UI_H
#define GLOM_PYTHON_GLOM_UI_H

#include <boost/python.hpp>

#include <libglom/document/document.h>
#include <libglom/data_structure/field.h>
#include <glibmm/ustring.h>

namespace Glom
{

/** UI code should connect to the signals to respond when Python code
 * request a change in the UI.
 */
class PythonUICallbacks
{
public:
  /** For example,
   * void on_show_details(const Glib::ustring& table_name, const Gnome::Gda::Value& primary_key_value);
   */
  sigc::slot<void, const Glib::ustring&, const Gnome::Gda::Value&> m_slot_show_table_details;

   /** For example,
   * void on_show_list(const Glib::ustring& table_name);
   */
  sigc::slot<void, const Glib::ustring&> m_slot_show_table_list;

   /** For example,
   * void on_print_report(const Glib::ustring& table_name);
   */
  sigc::slot<void, const Glib::ustring&> m_slot_print_report;

   /** For example,
   * void on_print_layout();
   */
  sigc::slot<void> m_slot_print_layout;

  /** For example,
   * void on_start_new_record(const Gnome::Gda::Value& new_primary_key_value);
   * Use an empty Value for auto-created fields.
   */
  sigc::slot<void> m_slot_start_new_record;
};

class PyGlomUI
{
public:
  //A default constructor seems to be necessary for boost::python
  PyGlomUI();
  explicit PyGlomUI(const PythonUICallbacks& callbacks);
  ~PyGlomUI();

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
