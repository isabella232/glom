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

#include "box_print_layouts.h"
#include <glom/appwindow.h>
#include <gtkmm/alignment.h>
#include <gtkmm/messagedialog.h>
#include <libglom/algorithms_utils.h>
#include <libglom/utils.h> //For bold_message()).
#include <libglom/string_utils.h>
#include <glibmm/i18n.h>

#include <iostream>

namespace Glom
{

const char* Box_Print_Layouts::glade_id("box_print_layouts");
const bool Box_Print_Layouts::glade_developer(true);

Box_Print_Layouts::Box_Print_Layouts(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Box_DB_Table(cobject, builder),
  m_colName(0),
  m_colTitle(0)
{
  //Get the Glade-instantiated widgets, and connect signal handlers:
  Gtk::Button* pButtonCancel = nullptr;
  builder->get_widget("button_cancel", pButtonCancel);
  set_button_cancel(*pButtonCancel);

  Gtk::Box* pAddDelParent = nullptr;
  builder->get_widget("vbox_adddel_parent", pAddDelParent);
  pAddDelParent->pack_start(m_AddDel);

  m_AddDel.signal_user_added().connect(sigc::mem_fun(*this, &Box_Print_Layouts::on_adddel_user_added));
  m_AddDel.signal_user_requested_delete().connect(sigc::mem_fun(*this, &Box_Print_Layouts::on_adddel_user_requested_delete));
  m_AddDel.signal_user_requested_edit().connect(sigc::mem_fun(*this, &Box_Print_Layouts::on_adddel_user_requested_edit));
  m_AddDel.signal_user_changed().connect(sigc::mem_fun(*this, &Box_Print_Layouts::on_adddel_user_changed));

  show_all_children();
}

void Box_Print_Layouts::fill_row(const Gtk::TreeModel::iterator& iter, const std::shared_ptr<const PrintLayout>& item)
{
  if(iter)
  {
    const auto name = item->get_name();
    m_AddDel.set_value_key(iter, name);
    m_AddDel.set_value(iter, m_colName, name);
    m_AddDel.set_value(iter, m_colTitle, item_get_title(item));
  }
}

bool Box_Print_Layouts::fill_from_database()
{
  BusyCursor busy_cursor(get_app_window());

  bool result = Base_DB::fill_from_database();

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
  m_colName = m_AddDel.add_column(_("Name"), AddDelColumnInfo::enumStyles::Text, editable, visible_extras);
  m_AddDel.prevent_duplicates(m_colName); //Don't allow a relationship to be added twice.
  m_AddDel.set_prevent_duplicates_warning(_("This item already exists. Please choose a different item name"));

  m_colTitle = m_AddDel.add_column(_("Title"), AddDelColumnInfo::enumStyles::Text, editable, true);

  auto document = get_document();
  if(document)
  {
    for(const auto& print_layout_name : document->get_print_layout_names(m_table_name))
    {
      auto item = document->get_print_layout(m_table_name, print_layout_name);
      if(item)
      {
        auto row = m_AddDel.add_item(item->get_name());
        fill_row(row, item);
      }
    }
   }
  else
    std::cerr << G_STRFUNC << ": document is null\n";

  //TODO:

  m_AddDel.set_allow_add(developer_mode);
  m_AddDel.set_allow_delete(developer_mode);

  return result;
}

void Box_Print_Layouts::on_adddel_user_added(const Gtk::TreeModel::iterator& row)
{
  auto item = std::make_shared<PrintLayout>();

  const auto name = m_AddDel.get_value(row, m_colName);
  if(!name.empty())
  {
    item->set_name(name);
    m_AddDel.set_value_key(row, name);

    //Set a suitable starting title, if there is none already:
    Glib::ustring title = m_AddDel.get_value(row, m_colTitle);
    if(title.empty())
    {
      title = Utils::title_from_string(name);
      m_AddDel.set_value(row, m_colTitle, title);
    }

    item->set_title(title, AppWindow::get_current_locale());

    get_document()->set_print_layout(m_table_name, item);
  }
}

void Box_Print_Layouts::on_adddel_user_requested_delete(const Gtk::TreeModel::iterator& rowStart, const Gtk::TreeModel::iterator& /* TODO: rowEnd */)
{
  const auto name = m_AddDel.get_value_key(rowStart);
  if(!name.empty())
  {
    get_document()->remove_print_layout(m_table_name, name);
    m_AddDel.remove_item_by_key(name);
  }
}

void Box_Print_Layouts::on_adddel_user_requested_edit(const Gtk::TreeModel::iterator& row)
{
  Glib::ustring name = m_AddDel.get_value_key(row);

  auto document = get_document();
  if(document)
  {
     save_to_document();

     //Emit the signal:
     signal_selected.emit(name);
  }
}

void Box_Print_Layouts::save_to_document()
{
  if(get_userlevel() == AppState::userlevels::DEVELOPER)
  {
    //Add any that are not in the document:
    std::vector<Glib::ustring> listItems = get_document()->get_print_layout_names(m_table_name);

    bool modified = false;
    for(const auto& row : m_AddDel.get_model()->children())
    {
      const auto name = m_AddDel.get_value(row, m_colName);

      if(!name.empty() && !Utils::find_exists(listItems, name))
      {
        auto item = std::make_shared<PrintLayout>();
        item->set_name(name);

        item->set_title( m_AddDel.get_value(row, m_colTitle) , AppWindow::get_current_locale()); //TODO: Translations: Store the original in the TreeView.

        get_document()->set_print_layout(m_table_name, item);
        modified = true;
     }
    }

    if(modified)
     get_document()->set_modified(true);
  }
}

void Box_Print_Layouts::on_adddel_user_changed(const Gtk::TreeModel::iterator& row, guint column)
{
  if(get_userlevel() == AppState::userlevels::DEVELOPER)
  {
    const auto name = m_AddDel.get_value_key(row);
    auto document = get_document();

    auto item = document->get_print_layout(m_table_name, name);
    if(item)
    {
      if(column == m_colTitle)
      {
        item->set_title( m_AddDel.get_value(row, m_colTitle) , AppWindow::get_current_locale());
        //TODO: Unnecessary:
        document->set_print_layout(m_table_name, item);
      }
      else if(column == m_colName)
      {
        const auto name_new = m_AddDel.get_value(row, m_colName);
        if(!name.empty() && !name_new.empty())
        {
          Glib::ustring strMsg = _("Are you sure that you want to rename this print layout?");  //TODO: Show old and new names?
          Gtk::MessageDialog dialog(_("<b>Rename Print Layout</b>"), true, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE );
          dialog.add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
          dialog.add_button(_("Rename"), Gtk::RESPONSE_OK);
          dialog.set_secondary_text(strMsg);
          const auto iButtonClicked = dialog.run();

          //Rename the item:
          if(iButtonClicked == Gtk::RESPONSE_OK)
          {
            m_AddDel.set_value_key(row, name_new);

            document->remove_print_layout(m_table_name, name);

            item->set_name(name_new);
            document->set_print_layout(m_table_name, item);
          }
        }
      }
    }
  }
}

void Box_Print_Layouts::on_userlevel_changed(AppState::userlevels /* userlevel */)
{
  fill_from_database();
}

} //namespace Glom
