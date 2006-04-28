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


#include "dialog_textobject.h"
#include "../python_embed/glom_python.h"
#include <glom/libglom/data_structure/glomconversions.h>

//#include <libgnome/gnome-i18n.h>
#include <glibmm/i18n.h>

Dialog_TextObject::Dialog_TextObject(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::Dialog(cobject)
{
  refGlade->get_widget("entry_title",  m_entry_title);
  refGlade->get_widget("textview_text",  m_text_view);

  //on_foreach_connect(*this);

  //Dialog_Properties::set_modified(false);

  show_all_children();
}

Dialog_TextObject::~Dialog_TextObject()
{
}

void Dialog_TextObject::set_textobject(const sharedptr<const LayoutItem_Text>& textobject, const Glib::ustring& table_name)
{
  //set_blocked();

  m_textobject = glom_sharedptr_clone(textobject); //Remember it so we save any details that are not in our UI.
  m_table_name = table_name;  //Used for lookup combo boxes.

  m_entry_title->set_text(textobject->get_title());
  m_text_view->get_buffer()->set_text( textobject->get_text() );

  //set_blocked(false);

  //Dialog_Properties::set_modified(false);
}

sharedptr<LayoutItem_Text> Dialog_TextObject::get_textobject() const
{
  sharedptr<LayoutItem_Text> result = glom_sharedptr_clone(m_textobject); //Start with the old details, to preserve anything that is not in our UI.

  result->set_title(m_entry_title->get_text());
  result->set_text( m_text_view->get_buffer()->get_text() );

  return result;
}





