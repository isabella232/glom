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

#include <glom/python_embed/python_ui_callbacks.h>
#include <glom/appwindow.h>

namespace Glom
{

AppPythonUICallbacks::AppPythonUICallbacks()
{
  m_slot_show_table_details =
    std::bind(&AppPythonUICallbacks::on_show_table_details, this, std::placeholders::_1, std::placeholders::_2);
  m_slot_show_table_list =
    std::bind(&AppPythonUICallbacks::on_show_table_list, this, std::placeholders::_1);
  m_slot_print_report =
    std::bind(&AppPythonUICallbacks::on_print_report, this, std::placeholders::_1);
  m_slot_print_layout =
    std::bind(&AppPythonUICallbacks::on_print_layout, this);
  m_slot_start_new_record =
    std::bind(&AppPythonUICallbacks::on_start_new_record, this);
}

void AppPythonUICallbacks::on_show_table_details(const Glib::ustring& table_name, const Gnome::Gda::Value& primary_key_value)
{
  AppWindow* app = AppWindow::get_appwindow();
  if(app)
    app->show_table_details(table_name, primary_key_value);
}

void AppPythonUICallbacks::on_show_table_list(const Glib::ustring& table_name)
{
  AppWindow* app = AppWindow::get_appwindow();
  if(app)
    app->show_table_list(table_name);
}

void AppPythonUICallbacks::on_print_report(const Glib::ustring& report_name)
{
  AppWindow* app = AppWindow::get_appwindow();
  if(app)
    app->print_report(report_name);
}

void AppPythonUICallbacks::on_print_layout()
{
  AppWindow* app = AppWindow::get_appwindow();
  if(app)
    app->print_layout();
}

void AppPythonUICallbacks::on_start_new_record()
{
  AppWindow* app = AppWindow::get_appwindow();
  if(app)
    app->start_new_record();
}

} //namespace Glom
