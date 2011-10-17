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

#include "box_data_manyrecords.h"
#include <libglom/data_structure/glomconversions.h>
#include <glom/glade_utils.h>
#include <libglom/report_builder.h>
#include <glom/mode_design/layout/dialog_layout_list.h>
#include <glom/utils_ui.h>
#include <libglom/privs.h>
#include <libglom/utils.h> //For bold_message()).
#include <sstream> //For stringstream
#include <glibmm/i18n.h>

namespace Glom
{

Box_Data_ManyRecords::Box_Data_ManyRecords()
: m_has_one_or_more_records(false),
  m_read_only(false)
{
  //We do not actually use this,
  //so it is a bug if this appears in the .glom file:
  m_layout_name = "manyrecords"; //Set by derived classes.

  //Groups are not very helpful for a list view:
  //m_pDialogLayout->set_show_groups(false);

}

Box_Data_ManyRecords::~Box_Data_ManyRecords()
{
}

void Box_Data_ManyRecords::refresh_data_from_database_blank()
{
  //Overridden by derived classes.
}

/*
Document::type_list_layout_groups Box_Data_ManyRecords::create_layout_get_layout()
{
  //Overriden in Box_Data_ManyRecords_Related:
  return get_data_layout_groups(m_layout_name);
}
*/

Box_Data_ManyRecords::type_signal_user_requested_details Box_Data_ManyRecords::signal_user_requested_details()
{
  return m_signal_user_requested_details;
}

void Box_Data_ManyRecords::print_layout()
{
  const Privileges table_privs = Privs::get_current_privs(m_table_name);

  //Don't try to print tables that the user can't view.
  if(!table_privs.m_view)
  {
    //TODO: Warn the user.
  }
  else
  {
    //Create a simple report on the fly:
    Document* document = get_document();
    sharedptr<Report> report_temp = ReportBuilder::create_standard_list_report(document, m_table_name);

    ReportBuilder report_builder;
    report_builder.set_document(document);
    const std::string filepath = 
      report_builder.report_build_and_save(m_found_set, report_temp);
    Utils::show_report_in_browser(filepath, get_app_window());
  }
}

void Box_Data_ManyRecords::print_layout_group(xmlpp::Element* /* node_parent */, const sharedptr<const LayoutGroup>& /* group */)
{
}

void Box_Data_ManyRecords::set_primary_key_value_selected(const Gnome::Gda::Value& /* primary_key_value */)
{
  //Overridden in derived classes.
}

} //namespace Glom
