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

#include "dialog_new_record.h"
#include <glom/utils_ui.h> //For bold_message()).
#include <glom/appwindow.h>
//#include <libgnome/gnome-i18n.h>
#include <glibmm/i18n.h>

namespace Glom
{

namespace DataWidgetChildren
{

const char* Dialog_NewRecord::glade_id("dialog_new_record");
const bool Dialog_NewRecord::glade_developer(false);

Dialog_NewRecord::Dialog_NewRecord()
: m_label_table_name(0),
  m_vbox_parent(0)
{
}

Dialog_NewRecord::Dialog_NewRecord(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Dialog(cobject),
  m_label_table_name(0),
  m_vbox_parent(0)
{
  builder->get_widget("label_table_name", m_label_table_name);
  builder->get_widget("vbox_parent", m_vbox_parent);

  setup();
}

Dialog_NewRecord::~Dialog_NewRecord()
{
  remove_view(&m_box_details);
}

void Dialog_NewRecord::setup()
{
  m_box_details.show_layout_toolbar(false);
  m_vbox_parent->pack_start(m_box_details);

  //Fill composite view:
  add_view(&m_box_details);
}

bool Dialog_NewRecord::get_id_chosen(Gnome::Gda::Value& chosen_id) const
{
  chosen_id = m_box_details.get_primary_key_value_selected();
  return true;
}

bool Dialog_NewRecord::init_db_details(const Glib::ustring& table_name, const Glib::ustring& layout_platform)
{
  m_table_name = table_name;
  m_layout_platform = layout_platform;

  m_label_table_name->set_text( get_document()->get_table_title(m_table_name, AppWindow::get_current_locale()) );

  FoundSet found_set;
  found_set.m_table_name = m_table_name;
  const Gnome::Gda::Value primary_key_for_details;
  const auto result = m_box_details.init_db_details(found_set, layout_platform, primary_key_for_details);
  m_box_details.do_new_record();

  m_table_name = table_name;

  return result;
}

} //namespace DataWidetChildren
} //namespace Glom
