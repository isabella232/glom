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


#include "dialog_imageobject.h"
#include "../python_embed/glom_python.h"
#include <glom/libglom/data_structure/glomconversions.h>

//#include <libgnome/gnome-i18n.h>
#include <glibmm/i18n.h>

Dialog_ImageObject::Dialog_ImageObject(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::Dialog(cobject)
{
  refGlade->get_widget("entry_title",  m_entry_title);
  refGlade->get_widget_derived("imageglom",  m_image);

  refGlade->get_widget("button_choose_image",  m_button_choose_image);
  m_button_choose_image->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_ImageObject::on_button_choose));

  //on_foreach_connect(*this);

  //Dialog_Properties::set_modified(false);

  show_all_children();
}

Dialog_ImageObject::~Dialog_ImageObject()
{
}

void Dialog_ImageObject::on_button_choose()
{
  m_image->do_choose_image();
}


void Dialog_ImageObject::set_imageobject(const sharedptr<const LayoutItem_Image>& imageobject, const Glib::ustring& table_name)
{
  //set_blocked();

  m_imageobject = glom_sharedptr_clone(imageobject); //Remember it so we save any details that are not in our UI.
  m_table_name = table_name;  //Used for lookup combo boxes.

  m_entry_title->set_text(imageobject->get_title());
  m_image->set_value( imageobject->get_image() );

  //set_blocked(false);

  //Dialog_Properties::set_modified(false);
}

sharedptr<LayoutItem_Image> Dialog_ImageObject::get_imageobject() const
{
  sharedptr<LayoutItem_Image> result = glom_sharedptr_clone(m_imageobject); //Start with the old details, to preserve anything that is not in our UI.

  result->set_title(m_entry_title->get_text());
  result->set_image( m_image->get_value() );

  return result;
}





