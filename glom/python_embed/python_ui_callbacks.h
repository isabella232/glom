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

#ifndef GLOM_PYTHON_EMBED_UI_CALLBACKS_H
#define GLOM_PYTHON_EMBED_UI_CALLBACKS_H

#include <libglom/python_embed/py_glom_ui_callbacks.h>

namespace Glom
{

/** UI code should connect to the signals to respond when Python code
 * request a change in the UI.
 */
class AppPythonUICallbacks : public PythonUICallbacks
{
public:
  AppPythonUICallbacks();

private:
  void on_show_table_details(const Glib::ustring& table_name, const Gnome::Gda::Value& primary_key_value);
  void on_show_table_list(const Glib::ustring& table_name);
  void on_print_report(const Glib::ustring& report_name);
  void on_print_layout();
  void on_start_new_record();
};

} //namespace Glom

#endif //GLOM_PYTHON_GLOM_UI_H
