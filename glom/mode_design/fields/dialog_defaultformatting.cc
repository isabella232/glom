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

#include "dialog_defaultformatting.h"
#include <glom/glade_utils.h>
#include <glom/utils_ui.h>
#include <glom/appwindow.h>
#include "../../box_db_table.h"
#include <libglom/db_utils.h>
//#include <libgnome/gnome-i18n.h>
#include <glibmm/i18n.h>

namespace Glom
{

const char* Dialog_DefaultFormatting::glade_id("window_default_formatting");
const bool Dialog_DefaultFormatting::glade_developer(true);

Dialog_DefaultFormatting::Dialog_DefaultFormatting(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Dialog_Properties(cobject, builder),
  m_box_formatting_placeholder(0),
  m_box_formatting(0)
{
  //Get the place to put the Formatting stuff:
  builder->get_widget("box_formatting_placeholder", m_box_formatting_placeholder);

  //Get the formatting stuff:
  Utils::box_pack_start_glade_child_widget_derived_with_warning(m_box_formatting_placeholder, m_box_formatting);
  add_view(m_box_formatting);

  if(m_box_formatting)
  {
    connect_each_widget(m_box_formatting);

    //Plus an extra signal for the related extra show-also fields:
    m_box_formatting->signal_modified().connect(
     sigc::mem_fun(*this, &Dialog_DefaultFormatting::on_anything_changed));
  }

  Dialog_Properties::set_modified(false);

  show_all_children();
}

Dialog_DefaultFormatting::~Dialog_DefaultFormatting()
{
  remove_view(m_box_formatting);
}

void Dialog_DefaultFormatting::set_field(const sharedptr<const Field>& field, const Glib::ustring& table_name)
{
  set_blocked();

  m_Field = glom_sharedptr_clone(field); //Remember it so we save any details that are not in our UI.
  m_table_name = table_name;  //Used for lookup combo boxes.

  //Formatting:
  m_box_formatting->set_formatting_for_field(field->m_default_formatting, m_table_name, field);

  set_blocked(false);

  Dialog_Properties::set_modified(false);
}

sharedptr<Field> Dialog_DefaultFormatting::get_field() const
{
  sharedptr<Field> field = glom_sharedptr_clone(m_Field); //Start with the old details, to preserve anything that is not in our UI.
  // const_cast is necessary and save here for the window (jhs)
  sharedptr<SharedConnection> sharedcnc = connect_to_server(const_cast<Dialog_DefaultFormatting*>(this));
  Glib::RefPtr<Gnome::Gda::Connection> cnc = sharedcnc->get_gda_connection();

  //Get the field info from the widgets:

  //Formatting:
  m_box_formatting->get_formatting(field->m_default_formatting);

  return field;
}

} //namespace Glom
