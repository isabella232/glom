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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <glom/python_embed/python_ui_callbacks.h>
#include <glom/application.h>

namespace Glom
{

AppPythonUICallbacks::AppPythonUICallbacks()
{
  m_slot_show_table_details =
    sigc::mem_fun(*this, &AppPythonUICallbacks::on_show_table_details);
  m_slot_show_table_list =
    sigc::mem_fun(*this, &AppPythonUICallbacks::on_show_table_list);
  m_slot_print_report =
    sigc::mem_fun(*this, &AppPythonUICallbacks::on_print_report);
  m_slot_print_layout =
    sigc::mem_fun(*this, &AppPythonUICallbacks::on_print_layout);
  m_slot_start_new_record =
    sigc::mem_fun(*this, &AppPythonUICallbacks::on_start_new_record);
}

void AppPythonUICallbacks::on_show_table_details(const Glib::ustring& table_name, const Gnome::Gda::Value& primary_key_value)
{
  Application* app = Application::get_application();
  if(app)
    app->show_table_details(table_name, primary_key_value);
}

void AppPythonUICallbacks::on_show_table_list(const Glib::ustring& table_name)
{
  Application* app = Application::get_application();
  if(app)
    app->show_table_list(table_name);
}

void AppPythonUICallbacks::on_print_report(const Glib::ustring& report_name)
{
  Application* app = Application::get_application();
  if(app)
    app->print_report(report_name);
}

void AppPythonUICallbacks::on_print_layout()
{
  Application* app = Application::get_application();
  if(app)
    app->print_layout();
}

void AppPythonUICallbacks::on_start_new_record()
{
  Application* app = Application::get_application();
  if(app)
    app->start_new_record();
}

} //namespace Glom
