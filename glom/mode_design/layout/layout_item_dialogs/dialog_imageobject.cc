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


#include "dialog_imageobject.h"
#include <glom/python_embed/glom_python.h>
#include <libglom/data_structure/glomconversions.h>
#include <glom/appwindow.h>

//#include <libgnome/gnome-i18n.h>
#include <glibmm/i18n.h>

namespace Glom
{

const char* Dialog_ImageObject::glade_id("window_imageobject");
const bool Dialog_ImageObject::glade_developer(true);

Dialog_ImageObject::Dialog_ImageObject(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Dialog(cobject),
  m_box_title(0),
  m_entry_title(0),
  m_image(0)
{
  builder->get_widget("vbox_title", m_box_title);
  builder->get_widget("entry_title", m_entry_title);
  builder->get_widget_derived("imageglom", m_image);

  builder->get_widget("button_choose_image", m_button_choose_image);
  m_button_choose_image->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_ImageObject::on_button_choose));

  //connect_each_widget(this);

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


void Dialog_ImageObject::set_imageobject(const std::shared_ptr<const LayoutItem_Image>& imageobject, const Glib::ustring& table_name, bool show_title)
{
  //set_blocked();

  m_imageobject = glom_sharedptr_clone(imageobject); //Remember it so we save any details that are not in our UI.
  m_table_name = table_name;  //Used for lookup combo boxes.

  m_entry_title->set_text(item_get_title(imageobject));
  m_image->set_value( imageobject->get_image() );

  if(show_title)
    m_box_title->show();
  else
    m_box_title->hide();

  //set_blocked(false);

  //Dialog_Properties::set_modified(false);
}

std::shared_ptr<LayoutItem_Image> Dialog_ImageObject::get_imageobject() const
{
  std::shared_ptr<LayoutItem_Image> result = glom_sharedptr_clone(m_imageobject); //Start with the old details, to preserve anything that is not in our UI.

  result->set_title(m_entry_title->get_text(), AppWindow::get_current_locale());
  result->set_image( m_image->get_value() );

  return result;
}

} //namespace Glom





