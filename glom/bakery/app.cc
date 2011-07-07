/*
 * Copyright 2000 Murray Cumming
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <glom/bakery/app.h>
#include <algorithm>


namespace GlomBakery
{

//Initialize static member data:
Glib::ustring App::m_strVersion;

bool App::m_bOperationCancelled = false;
Glib::ustring App::m_strCommandLine_0;
Glib::ustring App::m_strAppName;

App::App(const Glib::ustring& appname)
{
  init_app_name(appname);
}

App::~App()
{

}

void App::init_app_name(const Glib::ustring& appname) //static
{
  m_strAppName = appname;
}

void App::init()
{
  //set_wmclass(m_strAppName, m_strTitle); //The docs say "Don't use this".

  init_ui_manager();
  init_menus();
  init_toolbars();

  //on_document_load(); //Show the document (even if it is empty).
}

void App::init_ui_manager()
{
  
}

void App::init_menus()
{
  init_menus_file();
  init_menus_edit();

  //create_menus(m_menu_UI_Infos);
  //install_menu_hints();

  //Override this to add more menus.
}

void App::init_toolbars()
{
  
}

void App::on_menu_file_new()
{
  new_instance();
}

void App::on_menu_file_close()
{
  ui_hide();
}

void App::on_menu_edit_cut()
{
  on_menu_edit_copy();
  on_menu_edit_clear();
}


void App::on_menu_edit_copy()
{
  
}

void App::on_menu_edit_paste()
{
  
}

void App::on_menu_edit_clear()
{
  
}

Glib::ustring App::get_version() const
{
  return m_strVersion;
}

void App::set_operation_cancelled(bool bVal /* = true */)
{
  m_bOperationCancelled = bVal;
}

bool App::get_operation_cancelled()
{
  return m_bOperationCancelled;
}

void App::set_command_line_args(int argc, char **&argv)
{
  if( (argc > 0) && argv[0])
    m_strCommandLine_0 = (char*)argv[0];
}

App::type_signal_hide App::ui_signal_hide()
{
  return m_signal_hide;
}



} //namespace
