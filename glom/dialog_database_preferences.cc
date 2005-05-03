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
 
#include "dialog_database_preferences.h"
#include "box_db.h" //For Box_DB::connect_to_server().
#include "standard_table_prefs_fields.h"
#include "data_structure/glomconversions.h"
#include <glibmm/i18n.h>

Dialog_Database_Preferences::Dialog_Database_Preferences(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::Dialog(cobject),
  Base_DB(),
  m_glade_variables_map(refGlade)
{
  m_glade_variables_map.connect_widget("entry_name", m_system_prefs.m_name);
  m_glade_variables_map.connect_widget("entry_org_name", m_system_prefs.m_org_name);
  m_glade_variables_map.connect_widget("entry_org_address_street", m_system_prefs.m_org_address_street);
  m_glade_variables_map.connect_widget("entry_org_address_street2", m_system_prefs.m_org_address_street2);
  m_glade_variables_map.connect_widget("entry_org_address_town", m_system_prefs.m_org_address_town);
  m_glade_variables_map.connect_widget("entry_org_address_county", m_system_prefs.m_org_address_county);
  m_glade_variables_map.connect_widget("entry_org_address_country", m_system_prefs.m_org_address_country);
  m_glade_variables_map.connect_widget("entry_org_address_postcode", m_system_prefs.m_org_address_postcode);
}

Dialog_Database_Preferences::~Dialog_Database_Preferences()
{
}

void Dialog_Database_Preferences::load_from_document()
{
  m_system_prefs = get_database_preferences();

  //Show the data in the UI:
  m_glade_variables_map.transfer_variables_to_widgets();
}

void Dialog_Database_Preferences::save_to_document()
{
  m_glade_variables_map.transfer_widgets_to_variables();
  set_database_preferences(m_system_prefs);
}

void Dialog_Database_Preferences::on_response(int response_id)
{
  if(response_id == Gtk::RESPONSE_OK)
    save_to_document();
}

