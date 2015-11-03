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

#include <glom/box_reports.h>
#include <glom/appwindow.h>
#include <libglom/utils.h> //For bold_message()).
#include <gtkmm/alignment.h>
#include <gtkmm/dialog.h>
#include <gtkmm/messagedialog.h>
#include <glibmm/i18n.h>

#include <iostream>

namespace Glom
{

const char* Box_Reports::glade_id("box_reports");
const bool Box_Reports::glade_developer(true);

Box_Reports::Box_Reports(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Box_DB_Table(cobject, builder),
  m_colReportName(0),
  m_colTitle(0)
{
  //Get the Glade-instantiated widgets, and connect signal handlers:
  Gtk::Button* pButtonCancel = nullptr;
  builder->get_widget("button_cancel", pButtonCancel);
  if(!pButtonCancel)
  {
    std::cerr << G_STRFUNC << "Missing widget from glade file." << std::endl;
    return;
  }
  set_button_cancel(*pButtonCancel);

  Gtk::Box* pAddDelParent = nullptr;
  builder->get_widget("vbox_adddel_parent", pAddDelParent);
  if(!pAddDelParent)
  {
    std::cerr << G_STRFUNC << "Missing widget from glade file." << std::endl;
    return;
  }
  pAddDelParent->pack_start(m_AddDel);

  m_AddDel.signal_user_added().connect(sigc::mem_fun(*this, &Box_Reports::on_adddel_Add));
  m_AddDel.signal_user_requested_delete().connect(sigc::mem_fun(*this, &Box_Reports::on_adddel_Delete));
  m_AddDel.signal_user_requested_edit().connect(sigc::mem_fun(*this, &Box_Reports::on_adddel_Edit));
  m_AddDel.signal_user_changed().connect(sigc::mem_fun(*this, &Box_Reports::on_adddel_changed));

  show_all_children();
}

Box_Reports::~Box_Reports()
{
}

void Box_Reports::fill_row(const Gtk::TreeModel::iterator& iter, const std::shared_ptr<const Report>& report)
{
  if(iter)
  {
    const auto report_name = report->get_name();
    m_AddDel.set_value_key(iter, report_name);
    m_AddDel.set_value(iter, m_colReportName, report_name);
    m_AddDel.set_value(iter, m_colTitle, item_get_title(report));
  }
}

bool Box_Reports::fill_from_database()
{
  BusyCursor busy_cursor(get_app_window());

  bool result = Box_DB_Table::fill_from_database();

  //Enable/Disable extra widgets:
  bool developer_mode = (get_userlevel() == AppState::userlevels::DEVELOPER);

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
  m_colReportName = m_AddDel.add_column(_("Name"), AddDelColumnInfo::enumStyles::Text, editable, visible_extras);
  m_AddDel.prevent_duplicates(m_colReportName); //Don't allow a relationship to be added twice.
  m_AddDel.set_prevent_duplicates_warning(_("This report already exists. Please choose a different report name"));

  m_colTitle = m_AddDel.add_column(_("Title"), AddDelColumnInfo::enumStyles::Text, editable, true);

  auto document = get_document();
  if(document)
  {
    for(const auto& item : document->get_report_names(m_table_name))
    {
      auto report = document->get_report(m_table_name, item);
      if(report)
      {
        auto row = m_AddDel.add_item(report->get_name());
        fill_row(row, report);
      }
    }
   }
  else
    std::cerr << G_STRFUNC << ": document is null" << std::endl;

  //TODO:

  m_AddDel.set_allow_add(developer_mode);
  m_AddDel.set_allow_delete(developer_mode);

  return result;
}

void Box_Reports::on_adddel_Add(const Gtk::TreeModel::iterator& row)
{
  std::shared_ptr<Report> report = std::make_shared<Report>();

  const auto report_name = m_AddDel.get_value(row, m_colReportName);
  if(!report_name.empty())
  {
    report->set_name(report_name);
    m_AddDel.set_value_key(row, report_name);

    //Set a suitable starting title, if there is none already:
    Glib::ustring title = m_AddDel.get_value(row, m_colTitle);
    if(title.empty())
    {
      title = Utils::title_from_string(report_name);
      m_AddDel.set_value(row, m_colTitle, title);
    }

    report->set_title(title, AppWindow::get_current_locale());

    get_document()->set_report(m_table_name, report);
  }
}

void Box_Reports::on_adddel_Delete(const Gtk::TreeModel::iterator& rowStart, const Gtk::TreeModel::iterator& /* TODO: rowEnd */)
{
  const auto name = m_AddDel.get_value_key(rowStart);
  if(!name.empty())
  {
    get_document()->remove_report(m_table_name, name);
    m_AddDel.remove_item_by_key(name);
  }
}

void Box_Reports::on_adddel_Edit(const Gtk::TreeModel::iterator& row)
{
  Glib::ustring report_name = m_AddDel.get_value_key(row);

  auto document = get_document();
  if(document)
  {
     save_to_document();

     //Emit the signal:
     signal_selected.emit(report_name);
  }
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void Box_Reports::save_to_document()
{
  if(get_userlevel() == AppState::userlevels::DEVELOPER)
  {
    //Add any reports that are not in the document:
    std::vector<Glib::ustring> listReports = get_document()->get_report_names(m_table_name);

    bool modified = false;
    for(const auto& item : m_AddDel.get_model()->children())
    {
      const auto report_name = m_AddDel.get_value(item, m_colReportName);

      if(!report_name.empty() && std::find(listReports.begin(), listReports.end(), report_name) == listReports.end())
      {
        auto report = std::make_shared<Report>();
        report->set_name(report_name);

        report->set_title( m_AddDel.get_value(item, m_colTitle) , AppWindow::get_current_locale()); //TODO: Translations: Store the original in the TreeView.

        get_document()->set_report(m_table_name, report);
        modified = true;
     }
    }

    if(modified)
     get_document()->set_modified(true);
  }
}
#endif //GLOM_ENABLE_CLIENT_ONLY

void Box_Reports::on_adddel_changed(const Gtk::TreeModel::iterator& row, guint column)
{
  if(get_userlevel() == AppState::userlevels::DEVELOPER)
  {
    const auto report_name = m_AddDel.get_value_key(row);
    auto document = get_document();

    std::shared_ptr<Report> report = document->get_report(m_table_name, report_name);
    if(report)
    {
      if(column == m_colTitle)
      {
        report->set_title( m_AddDel.get_value(row, m_colTitle) , AppWindow::get_current_locale());
        //TODO: Unnecessary:
        document->set_report(m_table_name, report);
      }
      else if(column == m_colReportName)
      {
        const auto report_name_new = m_AddDel.get_value(row, m_colReportName);
        if(!report_name.empty() && !report_name_new.empty())
        {
          const auto strMsg = _("Are you sure that you want to rename this report?");  //TODO: Show old and new names?
          Gtk::MessageDialog dialog(_("Rename Report"));
          dialog.set_secondary_text(strMsg);
          int iButtonClicked = dialog.run();

          //Rename the report:
          if(iButtonClicked == Gtk::RESPONSE_OK)
          {
            m_AddDel.set_value_key(row, report_name_new);

            document->remove_report(m_table_name, report_name);

            report->set_name(report_name_new);
            document->set_report(m_table_name, report);
          }
        }
      }
    }
  }
}

void Box_Reports::on_userlevel_changed(AppState::userlevels /* userlevel */)
{
  fill_from_database();
}

} //namespace Glom
