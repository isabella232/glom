/* Glom
 *
 * Copyright (C) 2011 Murray Cumming
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

#include "notebook_noframe.h"
#include <glom/utils_ui.h>
#include <gtkmm/label.h>
#include <gtkmm/togglebutton.h>
#include <glibmm/i18n.h>
//#include <sstream> //For stringstream

namespace Glom
{

NotebookNoFrame::NotebookNoFrame()
: m_current_page(0)
{
  set_orientation(Gtk::ORIENTATION_VERTICAL);
  set_spacing(Utils::DEFAULT_SPACING_SMALL);

  m_box_tabs.set_spacing(Utils::DEFAULT_SPACING_SMALL);
  m_box_tabs.pack_start(m_box_action_left, Gtk::PACK_SHRINK);
  m_box_tabs.pack_end(m_box_action_right, Gtk::PACK_SHRINK);

  pack_start(m_box_tabs, Gtk::PACK_SHRINK);
  m_box_tabs.show();


  pack_start(m_box_pages);
  m_box_pages.show();
}

NotebookNoFrame::~NotebookNoFrame()
{
}

NotebookNoFrame::type_signal_switch_page NotebookNoFrame::signal_switch_page()
{
  return m_signal_switch_page;
}

Gtk::Widget* NotebookNoFrame::get_nth_page(int page_num)
{
  if(page_num < 0)
    return 0;

  if(page_num >= (int)m_vec_page_widgets.size())
    return 0;

  Gtk::Box* box =  m_vec_page_widgets[page_num];
  if(!box)
    return 0;

  std::vector<Gtk::Widget*> children = box->get_children();
  if(children.empty())
    return 0;

  return children[0];
}

const Gtk::Widget* NotebookNoFrame::get_nth_page(int page_num) const
{
  NotebookNoFrame* unconstThis = const_cast<NotebookNoFrame*>(this);
  return unconstThis->get_nth_page(page_num);
}

int NotebookNoFrame::get_current_page() const
{
  return m_current_page;
}

void NotebookNoFrame::set_current_page(int page_num)
{
  if(page_num < 0)
    return;

  const int size = (int)m_vec_page_widgets.size();
  if(page_num >= size)
    return;

  m_current_page = page_num;

  //TODO: Enable the tab button too.

  //Show only the current page:
  for(int i = 0; i < size; ++i)
  {
    Gtk::ToggleButton* tab = m_vec_tab_widgets[i];
    if(!tab)
      continue;

    Gtk::Box* box = m_vec_page_widgets[i];
    if(!box)
      continue;

    if(i == page_num)
    {
      if(!tab->get_active())
        tab->set_active();

      box->show();
    }
    else
    {
      if(tab->get_active())
        tab->set_active(false);

      box->hide();
    }
  }
}

int NotebookNoFrame::append_page(Gtk::Widget& child, Gtk::Widget& tab_label)
{
  Gtk::ToggleButton* toggle = Gtk::manage(new Gtk::ToggleButton());
  toggle->set_active(false);
  toggle->add(tab_label);
  toggle->show();
  m_box_tabs.pack_start(*toggle, Gtk::PACK_SHRINK);
  m_vec_tab_widgets.push_back(toggle);

  //We put the child into a box so we can show or hide it regardless of
  //whether the callers calls show() or hide() on the child widget.
  //Note that this would make the public list of children incorrect, if we supported that anyway.
  Gtk::Box* box = Gtk::manage(new Gtk::Box());
  box->pack_start(child, Gtk::PACK_EXPAND_WIDGET);
  m_box_pages.pack_start(*box, Gtk::PACK_EXPAND_WIDGET);
  m_vec_page_widgets.push_back(box);

  const int index = m_vec_page_widgets.size() - 1;

  //Make sure that the first one is showing:
  if(index == 0)
  {
    m_current_page = 0;
    toggle->set_active();
    box->show();
  }
  else
  {
    toggle->set_active(false);
    box->hide();
  }

  set_current_page(index);

  toggle->signal_toggled().connect(
    sigc::bind(
      sigc::mem_fun(*this, &NotebookNoFrame::on_tab_toggled),
      index));

  return index;
}

int NotebookNoFrame::append_page(Widget& child, const Glib::ustring& tab_label, bool use_mnemonic)
{
  Gtk::Label* pLabel = Gtk::manage( new Gtk::Label(tab_label, use_mnemonic) );
  pLabel->show();
  return append_page(child, *pLabel);
}

void NotebookNoFrame::on_tab_toggled(int index)
{
  Gtk::ToggleButton* tab = m_vec_tab_widgets[index];
  if(!tab)
    return;

  int new_current_page = 0;
  if(tab->get_active())
  {
    //A different page was selected by clicking on its tab, pressing the button down:
    new_current_page = index;
  }
  else
  {
    //A different page was selected by clicking on another tab, raising it's button:
    //So let's choose another one:
    new_current_page = index + 1;
    if(new_current_page >= (int)m_vec_tab_widgets.size())
      new_current_page = 0;
  }

  set_current_page(new_current_page);
  Gtk::Widget* child = get_nth_page(new_current_page);
  m_signal_switch_page.emit(child, new_current_page);
}

void NotebookNoFrame::set_action_widget(Gtk::Widget* widget, Gtk::PackType pack_type)
{
  if(pack_type == Gtk::PACK_START)
  {
    m_box_action_left.pack_start(*widget, Gtk::PACK_SHRINK);
    m_box_action_left.show();
  }
  else
  {
    m_box_action_right.pack_end(*widget, Gtk::PACK_SHRINK);
    m_box_action_right.show();
  }
}


} //namespace Glom
