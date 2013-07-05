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

#ifndef GLOM_PYTHON_GLOM_UI_CALLBACKS_H
#define GLOM_PYTHON_GLOM_UI_CALLBACKS_H

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
   * void on_show_table_details(const Glib::ustring& table_name, const Gnome::Gda::Value& primary_key_value);
   */
  std::function<void(const Glib::ustring&, const Gnome::Gda::Value&)> m_slot_show_table_details;

   /** For example,
   * void on_show_table_list(const Glib::ustring& table_name);
   */
  std::function<void(const Glib::ustring&)> m_slot_show_table_list;

   /** For example,
   * void on_print_report(const Glib::ustring& report_name);
   */
  std::function<void(const Glib::ustring&)> m_slot_print_report;

   /** For example,
   * void on_print_layout();
   */
  std::function<void()> m_slot_print_layout;

  /** For example,
   * void on_start_new_record();
   * Use an empty Value for auto-created fields.
   */
  std::function<void()> m_slot_start_new_record;
};

} //namespace Glom

#endif //GLOM_PYTHON_GLOM_CALLBACKS_UI_H
