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

#include "dialog_choose_id.h"
#include <glom/utils_ui.h> //For bold_message()).
#include <glom/appwindow.h>
#include <libglom/sql_utils.h>
//#include <libgnome/gnome-i18n.h>
#include <glibmm/i18n.h>

namespace Glom
{

namespace DataWidgetChildren
{

const char* Dialog_ChooseID::glade_id("dialog_find_id");
const bool Dialog_ChooseID::glade_developer(false);

Dialog_ChooseID::Dialog_ChooseID()
: m_label_table_name(nullptr),
  m_pBox_QuickFind(nullptr),
  m_pEntry_QuickFind(nullptr),
  m_pButton_QuickFind(nullptr),
  m_vbox_parent(nullptr),
  m_document(nullptr),
  m_stage(enumStage::INVALID)
{
}

Dialog_ChooseID::Dialog_ChooseID(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Dialog(cobject),
  m_label_table_name(nullptr),
  m_pBox_QuickFind(nullptr),
  m_pEntry_QuickFind(nullptr),
  m_pButton_QuickFind(nullptr),
  m_vbox_parent(nullptr),
  m_document(nullptr),
  m_stage(enumStage::INVALID)
{
  builder->get_widget("label_table_name", m_label_table_name);
  builder->get_widget("vbox_parent", m_vbox_parent);

  builder->get_widget("hbox_quickfind", m_pBox_QuickFind);
  builder->get_widget("entry_quickfind", m_pEntry_QuickFind);
  builder->get_widget("button_quickfind", m_pButton_QuickFind);

  m_pButton_QuickFind->signal_clicked().connect(
    sigc::mem_fun(*this, &Dialog_ChooseID::on_button_quickfind) );

  setup();
}

Dialog_ChooseID::~Dialog_ChooseID()
{
  remove_view(&m_box_find);
  remove_view(&m_box_select);
}

void Dialog_ChooseID::setup()
{
  m_box_find.signal_find_criteria.connect(sigc::mem_fun(*this, &Dialog_ChooseID::on_box_find_criteria));
  m_box_select.signal_user_requested_details().connect(sigc::mem_fun(*this, &Dialog_ChooseID::on_box_select_selected));
  m_box_select.set_read_only();

  //Fill composite view:
  add_view(&m_box_find);
  //m_box_select.show_all_children();
  add_view(&m_box_select);
  m_box_select.set_open_button_title(_("Select"));

  m_stage = enumStage::FIND;
  update_ui_for_stage();
}

bool Dialog_ChooseID::get_id_chosen(Gnome::Gda::Value& chosen_id) const
{
  chosen_id = m_id_chosen;
  return true;
}

void Dialog_ChooseID::on_button_quickfind()
{
  const auto criteria = m_pEntry_QuickFind->get_text();
  if(criteria.empty())
  {
    Glib::ustring message = _("You have not entered any quick find criteria.");
    Gtk::MessageDialog dialog(UiUtils::bold_message(_("No Find Criteria")), true, Gtk::MESSAGE_WARNING );
    dialog.set_secondary_text(message);
    dialog.set_transient_for(*this);

    dialog.run();
  }
  else
  {
    const Gnome::Gda::SqlExpr where_clause =
      SqlUtils::get_find_where_clause_quick(get_document(), m_table_name,
        Gnome::Gda::Value(criteria));
    on_box_find_criteria(where_clause);
  }
}

void Dialog_ChooseID::on_box_find_criteria(const Gnome::Gda::SqlExpr& where_clause)
{
  //Use the find criteria to show a list of results:
  if(!where_clause.empty())
  {
    FoundSet found_set = m_box_select.get_found_set();
    found_set.m_table_name = m_table_name;
    found_set.m_where_clause = where_clause;
    const auto records_found = m_box_select.init_db_details(found_set, m_layout_platform);
    if(!records_found)
    {
      const auto find_again = UiUtils::show_warning_no_records_found(*this);

      if(!find_again)
        response(Gtk::RESPONSE_CANCEL);
    }
    else
    {
      m_stage = enumStage::SELECT;
      update_ui_for_stage();
    }
  }
}

void Dialog_ChooseID::on_box_select_selected(const Gnome::Gda::Value& primary_key)
{
  m_id_chosen = primary_key;
  response(Gtk::RESPONSE_OK); //Close the dialog.
}

void Dialog_ChooseID::update_ui_for_stage()
{
  UiUtils::container_remove_all(*m_vbox_parent);

  if(m_stage == enumStage::FIND)
  {
    m_pBox_QuickFind->show();

    m_box_find.show();
    m_vbox_parent->pack_start(m_box_find);
  }
  else if(m_stage == enumStage::SELECT)
  {
    m_pBox_QuickFind->hide();

    m_box_select.show();
    m_vbox_parent->pack_start(m_box_select);
  }
}

bool Dialog_ChooseID::init_db_details(const Glib::ustring& table_name, const Glib::ustring& layout_platform)
{
  m_table_name = table_name;
  m_layout_platform = layout_platform;

  m_label_table_name->set_text( get_document()->get_table_title(m_table_name, AppWindow::get_current_locale()) );

  //Start by asking for find criteria:
  m_stage = enumStage::FIND;
  update_ui_for_stage();

  bool result = m_box_find.init_db_details(table_name, layout_platform);

  m_table_name = table_name;

  return result;
}

} //namespace DataWidetChildren
} //namespace Glom
