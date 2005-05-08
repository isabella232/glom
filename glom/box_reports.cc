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

#include "box_reports.h"
#include <bakery/App/App_Gtk.h> //For util_bold_message().
#include <glibmm/i18n.h>

Box_Reports::Box_Reports(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Box_DB_Table(cobject, refGlade),
  m_pLabelFrameTitle(0),
  m_colReportName(0),
  m_colTitle(0)
{
  //Get the Glade-instantiated widgets, and connect signal handlers:
  Gtk::Button* pButtonCancel = 0;
  refGlade->get_widget("button_cancel", pButtonCancel);
  set_button_cancel(*pButtonCancel);

  Gtk::Alignment* pAligmentPlaceholder = 0;
  refGlade->get_widget("alignment_placeholder_adddel", pAligmentPlaceholder);
  pAligmentPlaceholder->add(m_AddDel);

  //refGlade->get_widget("label_frame_title", m_pLabelFrameTitle);

  m_AddDel.signal_user_added().connect(sigc::mem_fun(*this, &Box_Reports::on_adddel_Add));
  m_AddDel.signal_user_requested_delete().connect(sigc::mem_fun(*this, &Box_Reports::on_adddel_Delete));
  m_AddDel.signal_user_requested_edit().connect(sigc::mem_fun(*this, &Box_Reports::on_adddel_Edit));
  m_AddDel.signal_user_changed().connect(sigc::mem_fun(*this, &Box_Reports::on_adddel_changed));

  show_all_children();
}

Box_Reports::~Box_Reports()
{
}

void Box_Reports::fill_row(const Gtk::TreeModel::iterator& iter, const Report& report)
{
  if(iter)
  {
    m_AddDel.set_value_key(iter, report.m_name);
    m_AddDel.set_value(iter, m_colReportName, report.m_name);
    m_AddDel.set_value(iter, m_colTitle, report.m_title);
  }
}

bool Box_Reports::fill_from_database()
{
  Bakery::BusyCursor(*get_app_window());

  bool result = Box_DB::fill_from_database();

  //Enable/Disable extra widgets:
  bool developer_mode = (get_userlevel() == AppState::USERLEVEL_DEVELOPER);

  //Developers see more columns, so make it bigger:
  if(developer_mode)
    set_size_request(400, -1);
  else
    set_size_request(-1, -1);

  m_AddDel.remove_all();

  //Add the columns:
  m_AddDel.remove_all_columns();

  const bool editable = developer_mode;
  const bool visible_extras = developer_mode;
  m_colReportName = m_AddDel.add_column(_("Report"), AddDelColumnInfo::STYLE_Text, editable, visible_extras);
  m_colTitle = m_AddDel.add_column(_("Title"), AddDelColumnInfo::STYLE_Text, editable, true);

  //_("Server: ") +  m_strServerName + ", " + 
  //Glib::ustring strTitle = Glib::ustring("<b>") + _("Tables from Database: ") + get_database_name() + "");
  //m_pLabelFrameTitle->set_markup(strTitle);

  Document_Glom::type_listReports listTableReports;
  Document_Glom* document = get_document();
  if(document)
  {
    listTableReports = document->get_report_names(m_strTableName);
  }
  else
    g_warning("Box_Reports::fill_from_database(): document is null");

  //TODO:

  fill_end();

  m_AddDel.set_allow_add(developer_mode);
  m_AddDel.set_allow_delete(developer_mode);

  return result;
}

void Box_Reports::on_adddel_Add(const Gtk::TreeModel::iterator& row)
{
  Report report;
  report.m_name = m_AddDel.get_value(row, m_colReportName);

  get_document()->set_report(m_strTableName, report);
}

void Box_Reports::on_adddel_Delete(const Gtk::TreeModel::iterator& rowStart, const Gtk::TreeModel::iterator& /* TODO: rowEnd */)
{
  const Glib::ustring name = m_AddDel.get_value_key(rowStart);
  if(!name.empty())
    get_document()->remove_report(m_strTableName, name);
}

void Box_Reports::on_adddel_Edit(const Gtk::TreeModel::iterator& row)
{
  Glib::ustring report_name = m_AddDel.get_value_key(row);

  Document_Glom* document = get_document();
  if(document)
  {
     save_to_document();

     //Emit the signal:
     signal_selected.emit(report_name);
  }
}

void Box_Reports::save_to_document()
{
/*
  if(get_userlevel() == AppState::USERLEVEL_DEVELOPER)
  {
    //Save the hidden tables. TODO_usermode: Only if we are in developer mode.
    Document_Glom::type_listReports listReports;

    for(Gtk::TreeModel::iterator iter = m_AddDel.get_model()->children().begin(); iter != m_AddDel.get_model()->children().end(); ++iter)
    {
      Report report;
      report.m_name = m_AddDel.get_value(iter, m_colReportName);

      if(!report.m_name.empty())
      {
        report.m_title  = m_AddDel.get_value(iter, m_colTitle);

        listReports.push_back(report);
      }
    }

    Document_Glom* document = get_document();
    if(document)make 
      document->set_reports(m_strTableName, listReports); //TODO: Don't save all new tables - just the ones already in the document.
  }
*/
}

void Box_Reports::on_adddel_changed(const Gtk::TreeModel::iterator& row, guint column)
{
  if(get_userlevel() == AppState::USERLEVEL_DEVELOPER)
  {
    if(column == m_colTitle)
    {
      save_to_document();
    } 
    else if(column == m_colReportName)
    {
      Glib::ustring table_name = m_AddDel.get_value_key(row);
      Glib::ustring table_name_new = m_AddDel.get_value(row, m_colReportName);
      if(!table_name.empty() && !table_name_new.empty())
      {
        Glib::ustring strMsg = _("Are you sure that you want to rename this report?");  //TODO: Show old and new names?
        Gtk::MessageDialog dialog(_("Rename Report"));
        dialog.set_secondary_text(strMsg);
        int iButtonClicked = dialog.run();

        //Rename the table:
        if(iButtonClicked == Gtk::RESPONSE_OK)
        {
          //TODO
        }
      }
    }
  }
}

void Box_Reports::on_userlevel_changed(AppState::userlevels /* userlevel */)
{
  fill_from_database();
}
