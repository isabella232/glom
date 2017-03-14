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


#include "dialog_textobject.h"
#include <glom/appwindow.h>

//#include <libgnome/gnome-i18n.h>
#include <glibmm/i18n.h>

namespace Glom
{

const char* Dialog_TextObject::glade_id("window_textobject");
const bool Dialog_TextObject::glade_developer(true);

Dialog_TextObject::Dialog_TextObject(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Dialog(cobject),
  m_box_title(nullptr),
  m_entry_title(nullptr),
  m_text_view(nullptr)
{
  builder->get_widget("vbox_title",  m_box_title);
  builder->get_widget("entry_title",  m_entry_title);
  builder->get_widget("textview_text",  m_text_view);

  //connect_each_widget(this);

  //Dialog_Properties::set_modified(false);
}

void Dialog_TextObject::set_textobject(const std::shared_ptr<const LayoutItem_Text>& textobject, const Glib::ustring& table_name, bool show_title)
{
  //set_blocked();

  m_textobject = glom_sharedptr_clone(textobject); //Remember it so we save any details that are not in our UI.
  m_table_name = table_name;  //Used for lookup combo boxes.

  m_entry_title->set_text(item_get_title(textobject));
  m_text_view->get_buffer()->set_text( textobject->get_text(AppWindow::get_current_locale()) );

  if(show_title)
    m_box_title->show();
  else
    m_box_title->hide();

  //set_blocked(false);

  //Dialog_Properties::set_modified(false);
}

std::shared_ptr<LayoutItem_Text> Dialog_TextObject::get_textobject() const
{
  auto result = glom_sharedptr_clone(m_textobject); //Start with the old details, to preserve anything that is not in our UI.

  get_textobject(result);

  return result;
}

void Dialog_TextObject::get_textobject(std::shared_ptr<LayoutItem_Text>& textobject) const
{
  textobject->set_title(m_entry_title->get_text(), AppWindow::get_current_locale());
  textobject->set_text( m_text_view->get_buffer()->get_text(), AppWindow::get_current_locale());
}

} //namespace Glom





