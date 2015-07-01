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

#include "box_data_manyrecords.h"
#include <libglom/data_structure/glomconversions.h>
#include <glom/glade_utils.h>
#include <glom/appwindow.h>
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
: m_read_only(false)
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

Box_Data_ManyRecords::type_signal_record_selection_changed Box_Data_ManyRecords::signal_record_selection_changed()
{
  return m_signal_record_selection_changed;
}

void Box_Data_ManyRecords::print_layout()
{
  const auto table_privs = Privs::get_current_privs(m_table_name);

  //Don't try to print tables that the user can't view.
  if(!table_privs.m_view)
  {
    //TODO: Warn the user.
  }
  else
  {
    //Create a simple report on the fly:
    Document* document = get_document();
    std::shared_ptr<Report> report_temp = ReportBuilder::create_standard_list_report(document, m_table_name);

    //TODO: Find a way to get a full locale name from the simplified locale name from AppWindow::get_current_locale():
    ReportBuilder report_builder(std::locale("") /* the user's current locale */);
    report_builder.set_document(document);
    const std::string filepath = 
      report_builder.report_build_and_save(m_found_set, report_temp);
    UiUtils::show_report_in_browser(filepath, get_app_window());
  }
}

void Box_Data_ManyRecords::print_layout_group(xmlpp::Element* /* node_parent */, const std::shared_ptr<const LayoutGroup>& /* group */)
{
}

void Box_Data_ManyRecords::set_primary_key_value_selected(const Gnome::Gda::Value& /* primary_key_value */)
{
  //Overridden in derived classes.
}

} //namespace Glom
