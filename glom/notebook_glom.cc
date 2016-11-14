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

#include "notebook_glom.h"
#include "appwindow.h"

namespace Glom
{

Notebook_Glom::Notebook_Glom()
{
  //Connect signals:
  //We do this on on_show() instead, because otherwise GtkNotebook emits the signal (and we catch it) during show:
  //signal_switch_page().connect(sigc::mem_fun(*this, &Notebook_Glom::on_switch_page_handler));

  //signal_leave_page().connect(sigc::mem_fun(*this, &Notebook_Glom::on_leave_page));

  m_destructor_in_progress = false;
}

void Notebook_Glom::on_show()
{
  NotebookNoFrame::on_show();

  //TODO: Check this for GtkStack.
  //We do this only in on_show() because otherwise GtkNotebook emits the signal (and we catch it) during show:
  if(!m_connection_switch_page)
    m_connection_switch_page = signal_switch_page().connect(sigc::mem_fun(*this, &Notebook_Glom::on_switch_page_handler));
}

/*
Notebook_Glom::type_signal_leave_page Notebook_Glom::signal_leave_page()
{
  return m_signal_leave_page;
}
*/

void Notebook_Glom::on_switch_page_handler(Gtk::Widget* /* page */)
{
  //Remove the help hint for the previous page:
  auto pApp = get_app_window();

  //if(pAppGlom)
  //{
  //  pAppGlom->statusbar_clear();
 // }

  //Load the page as we enter it:
  auto pChild = get_visible_child();
  if(!pChild)
    return;

  auto pBox = dynamic_cast<Box_WithButtons*>(pChild);
  if(!pBox)
    return;

  //pBox->load_from_document();

  //Set the default button, if there is one:
  auto pAppGlom = dynamic_cast<AppWindow*>(pApp);
  if(!pAppGlom)
    return;

  if(auto default_button = pBox->get_default_button())
  {
    default_button->grab_default();
    pAppGlom->set_default(*default_button);
  }
  else
    pAppGlom->unset_default();
}

/* TODO: What this ever necessary?
void Notebook_Glom::on_leave_page(guint uiPageNumber)
{
  //Call base class:
  //NotebookNoFrame::on_leave_page(uiPageNumber);

  //Tell the page to save itself:
  if(!m_destructor_in_progress)
  {
    for(const auto& pChild : get_page_children())
    {
      auto pBox = dynamic_cast<Base_DB*>(pChild);
      if(!pBox)
        continue;

      pBox->save_to_document();
    }
  }
}
*/

Gtk::Window* Notebook_Glom::get_app_window()
{
  return dynamic_cast<Gtk::Window*>(get_toplevel());
}

/*
void Notebook_Glom::show_hint()
{
  int iPageCurrent = get_current_page();

  Gtk::Widget* pChild  = get_nth_page(iPageCurrent);
  if(pChild)
  {
    auto pBox = dynamic_cast<Box_WithButtons*>(pChild);
    if(!pBox)
      continue;

    pBox->show_hint();
  }
}
*/

void Notebook_Glom::do_menu_developer_layout()
{
  //Override this.
}

void Notebook_Glom::do_menu_file_print()
{
  //Override this.
}

} //namespace Glom
