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

#include "notebook_glom.h"
#include "application.h"

Notebook_Glom::Notebook_Glom()
: m_special_menus_merge_id(0),
  m_special_menus_actiongroup_added(false)
{
  m_uiPreviousPage = 0;

  //Connect signals:
  signal_switch_page().connect(sigc::mem_fun(*this, &Notebook_Glom::on_switch_page_handler));
  //signal_leave_page().connect(sigc::mem_fun(*this, &Notebook_Glom::on_leave_page));

  m_destructor_in_progress = false;
}

Notebook_Glom::~Notebook_Glom()
{
}

/*
Notebook_Glom::type_signal_leave_page Notebook_Glom::signal_leave_page()
{
  return m_signal_leave_page;
}
*/

void Notebook_Glom::on_switch_page_handler(GtkNotebookPage* pPage, guint uiPageNumber)
{
  //Call base class:
  Gtk::Notebook::on_switch_page(pPage, uiPageNumber);

  //Remove the help hint for the previous page:
  {
    Gtk::Window* pApp = get_app_window();

    App_Glom* pAppGlom = dynamic_cast<App_Glom*>(pApp);
    if(pAppGlom)
    {
      pAppGlom->statusbar_clear();
    }
  }

  //m_signal_leave_page.emit(m_uiPreviousPage);
  m_uiPreviousPage = uiPageNumber; //Remember the current page for next time.

  //Load the page as we enter it:
  Gtk::Widget* pChild  = get_nth_page(uiPageNumber);
  if(pChild)
  {
    Box_DB* pBox = dynamic_cast<Box_DB*>(pChild);
    if(pBox)
    {
      pBox->load_from_document();
    }
  }
}

void Notebook_Glom::on_leave_page(guint uiPageNumber)
{
  //Call base class:
  //Gtk::Notebook::on_leave_page(uiPageNumber);

  //Tell the page to save itself:
  if(!m_destructor_in_progress)
  {
    Gtk::Widget* pChild  = get_nth_page(uiPageNumber);
    if(pChild)
    {
      Box_DB* pBox = dynamic_cast<Box_DB*>(pChild);
      if(pBox)
      {
        pBox->save_to_document();
      }
    }
  }
}

Gtk::Window* Notebook_Glom::get_app_window()
{
  return dynamic_cast<Gtk::Window*>(get_toplevel());
}

void Notebook_Glom::show_hint()
{
  gint iPageCurrent = get_current_page();

  Gtk::Widget* pChild  = get_nth_page(iPageCurrent);
  if(pChild)
  {
    Box_DB* pBox = dynamic_cast<Box_DB*>(pChild);
    if(pBox)
      pBox->show_hint();
  }
}

void Notebook_Glom::merge_special_menus(const Glib::RefPtr<Gtk::UIManager>& ui_manager)
{
  if(m_actiongroup_special_menus)
  {
    if(!m_special_menus_actiongroup_added)
    {
      //Now we know the ui_manager, so we can add the actiongroup.
      //It is not actually shown until the ui string is merged in.
      ui_manager->insert_action_group(m_actiongroup_special_menus);
      m_special_menus_actiongroup_added = true;
    }

     try
     {
       m_special_menus_merge_id = ui_manager->add_ui_from_string(m_special_menus_ui_string);
     }
     catch(const Glib::Error& ex)
     {
       std::cerr << "building menus failed: " <<  ex.what();
     }
  }
}

void Notebook_Glom::unmerge_special_menus(const Glib::RefPtr<Gtk::UIManager>& ui_manager)
{
  if(m_actiongroup_special_menus)
  {
    ui_manager->remove_ui(m_special_menus_merge_id);
  }
}

