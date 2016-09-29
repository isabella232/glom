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

#include <glom/navigation/box_tables.h>
#include <glom/utils_ui.h> //For bold_message()).
#include <libglom/db_utils.h>
#include <glom/appwindow.h>
#include <glibmm/i18n.h>

#include <iostream>

namespace Glom
{

const char* Box_Tables::glade_id("box_navigation_tables");
const bool Box_Tables::glade_developer(false);

Box_Tables::Box_Tables(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Box_WithButtons(cobject, builder),
  m_check_button_show_hidden(nullptr),
  m_col_table_name(0),
  m_col_hidden(0),
  m_col_title(0),
  m_col_default(0),
  m_col_title_singular(0)
{
  //Get the Glade-instantiated widgets, and connect signal handlers:
  Gtk::Button* pButtonCancel = nullptr;
  builder->get_widget("button_cancel_tables", pButtonCancel);
  set_button_cancel(*pButtonCancel);

  // Set a name for the AddDel TreeView, so it can be accessed by LDTP
  m_AddDel.set_treeview_accessible_name("Tables");

  Gtk::Box* pAddDelParent = nullptr;
  builder->get_widget("vbox_adddel_parent", pAddDelParent);
  pAddDelParent->pack_start(m_AddDel);

  builder->get_widget("checkbutton_show_hidden", m_check_button_show_hidden);
  m_check_button_show_hidden->signal_toggled().connect(sigc::mem_fun(*this, &Box_Tables::on_show_hidden_toggled));

#ifndef GLOM_ENABLE_CLIENT_ONLY
  m_AddDel.signal_user_added().connect(sigc::mem_fun(*this, &Box_Tables::on_adddel_Add));
  m_AddDel.signal_user_requested_delete().connect(sigc::mem_fun(*this, &Box_Tables::on_adddel_Delete));
  m_AddDel.signal_user_changed().connect(sigc::mem_fun(*this, &Box_Tables::on_adddel_changed));
#endif //GLOM_ENABLE_CLIENT_ONLY
  m_AddDel.signal_user_requested_edit().connect(sigc::mem_fun(*this, &Box_Tables::on_adddel_Edit));

  show_all_children();
}

void Box_Tables::fill_table_row(const Gtk::TreeModel::iterator& iter, const std::shared_ptr<const TableInfo>& table_info)
{
  if(!table_info)
  {
    std::cerr << G_STRFUNC << ": table_info was null.\n";
    return;
  }

  if(iter)
  {
    const auto developer_mode = (get_userlevel() == AppState::userlevels::DEVELOPER);

    m_AddDel.set_value_key(iter, table_info->get_name());
    m_AddDel.set_value(iter, m_col_table_name, table_info->get_name());
    m_AddDel.set_value(iter, m_col_hidden, table_info->get_hidden());

    if(developer_mode)
    {
      //std::cout << "debug: " << G_STRFUNC << ": dev title=" << table_info->get_title(AppWindow::get_current_locale()) << std::endl;
      m_AddDel.set_value(iter, m_col_title, item_get_title(table_info));
      m_AddDel.set_value(iter, m_col_title_singular, table_info->get_title_singular(AppWindow::get_current_locale()));
    }
    else
    {
      //std::cout << "debug: " << G_STRFUNC << ": op get_title_or_name=" << table_info->get_title_or_name(AppWindow::get_current_locale()) << std::endl;
      m_AddDel.set_value(iter, m_col_title, item_get_title_or_name(table_info));
    }

    m_AddDel.set_value(iter, m_col_default, table_info->get_default());
  }
}

bool Box_Tables::fill_from_database()
{
  BusyCursor busy_cursor(AppWindow::get_appwindow());

  bool result = Base_DB::fill_from_database();

  //Enable/Disable extra widgets:
  const auto developer_mode = (get_userlevel() == AppState::userlevels::DEVELOPER);

  //Developers see more columns, so make it bigger:
  if(developer_mode)
    set_size_request(400, -1);
  else
    set_size_request(-1, -1);

  m_check_button_show_hidden->set_sensitive(developer_mode); //Operators have no choice - they can't see hidden tables ever.
  if(!developer_mode)
    m_check_button_show_hidden->set_active(false); //Operators have no choice - they can't see hidden tables ever.

  m_AddDel.remove_all();

  //Add the columns:
  m_AddDel.remove_all_columns();

  const bool editable = developer_mode;
  const bool visible_extras = developer_mode;
  m_col_table_name = m_AddDel.add_column(_("Table"), AddDelColumnInfo::enumStyles::Text, editable, visible_extras);
  m_AddDel.prevent_duplicates(m_col_table_name); //Prevent two tables with the same name from being added.
  m_AddDel.set_prevent_duplicates_warning(_("This table already exists. Please choose a different table name"));

  m_col_hidden = m_AddDel.add_column(_("Hidden"), AddDelColumnInfo::enumStyles::Boolean, editable, visible_extras);
  m_col_title =  m_AddDel.add_column(_("Title"), AddDelColumnInfo::enumStyles::Text, editable, true);

  //TODO: This should really be a radio, but the use of AddDel makes it awkward to change that CellRenderer property.
  m_col_default = m_AddDel.add_column(_("Default"), AddDelColumnInfo::enumStyles::Boolean, editable, visible_extras);

  if(developer_mode)
    m_col_title_singular = m_AddDel.add_column(_("Title (Singular Form)"), AddDelColumnInfo::enumStyles::Text, editable, true);

  //Get the list of hidden tables:

  Document::type_listTableInfo listTablesDocument;
  auto document = get_document();
  if(document)
  {
    listTablesDocument = document->get_tables();
  }
  else
    std::cerr << G_STRFUNC << ": document is null\n";

  //Get the list of tables in the database, from the server:
  auto sharedconnection = connect_to_server(AppWindow::get_appwindow());

  if(sharedconnection)
  {
    m_AddDel.remove_all();
    auto connection = sharedconnection->get_gda_connection();

    const auto vecTables = DbUtils::get_table_names_from_database();

    for(const auto& strName : vecTables)
    {
      std::shared_ptr<TableInfo> table_info;

      //Check whether it should be hidden:
      const auto iterFind = find_if_same_name(listTablesDocument, strName);
      if(iterFind != listTablesDocument.end())
      {
        table_info = *iterFind;

        //std::cout << "debug: " << G_STRFUNC << ": name=" << (*iterFind)->get_name() << ", item_get_title(table_info)=" << item_get_title(*iterFind) << std::endl;
      }
      else
      {
        //This table is in the database, but not in the document.
        //Show it as hidden:
        table_info = std::make_shared<TableInfo>();
        table_info->set_name(strName);
        table_info->set_hidden(true);
      }

      const auto hidden = table_info->get_hidden();

      bool bAddIt = true;
      if(hidden && !developer_mode) //Don't add hidden tables unless we are in developer mode:
        bAddIt = false;

      if(hidden && !m_check_button_show_hidden->get_active()) //Don't add hidden tables if that checkbox is unset.
      {
        bAddIt = false;
      }

      //Check whether it's a system table, though they should never be in this list:
      const Glib::ustring prefix = "glom_system_";
      const auto table_prefix = strName.substr(0, prefix.size());
      if(table_prefix == prefix)
        bAddIt = false;

      if(bAddIt)
      {
        auto iter = m_AddDel.add_item(strName);
        fill_table_row(iter, table_info);
      }
    }
  }

  m_AddDel.set_allow_add(developer_mode);
  m_AddDel.set_allow_delete(developer_mode);

  return result;
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void Box_Tables::on_adddel_Add(const Gtk::TreeModel::iterator& row)
{
  //TODO: Handle cell renderer changes to prevent illegal table names (e.g. starting with numbers.).

  const auto table_name = m_AddDel.get_value(row, m_col_table_name);
  if(table_name.empty())
    return;

  bool created = false;

  // Validate the name:
  // TODO: Remove this check when the libgda bug has been fixed:
  // https://bugzilla.gnome.org/show_bug.cgi?id=763534
  if (std::find(std::begin(table_name), std::end(table_name), ' ') != std::end(table_name)) {
    Gtk::MessageDialog dialog(UiUtils::bold_message(_("Table Names Cannot Contain Spaces")), true, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
    dialog.set_secondary_text(_("Unfortunately, Glom tables cannot have names that contain spaces. Please enter a different name for the table."));
    dialog.set_transient_for(*AppWindow::get_appwindow());
    dialog.run();
    dialog.hide();
    return;
  }

  //Check whether it exists already. (Maybe it is somehow in the database but not in the document. That shouldn't happen.)
  const auto exists_in_db = DbUtils::get_table_exists_in_database(table_name);
  if(exists_in_db)
  {
    //Ask the user if they want us to try to cope with this:
    Gtk::MessageDialog dialog(UiUtils::bold_message(_("Table Already Exists")), true, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL);
    dialog.set_secondary_text(_("This table already exists on the database server, though it is not mentioned in the .glom file. This should not happen. Would you like Glom to attempt to use the existing table?"));
    dialog.set_transient_for(*AppWindow::get_appwindow());

    const auto response = dialog.run();
    dialog.hide();

    if(response == Gtk::RESPONSE_OK)
    {
      //Maybe Glom will cope with whatever fields are there. Let's see.
      created = true;
    }
  }
  else
  {
    created = DbUtils::create_table_with_default_fields(get_document(), table_name);
  }

  if(created)
  {
    //Show the new information for this whole row:

    auto document = get_document();
    if(document)
    {
      auto table_info = document->get_table(table_name);
      fill_table_row(row, table_info);

      //Save the field information directly into the database, because we cannot get all the correct information from the database.
      //Otherwise some information would be forgotten:

      const auto fields = document->get_table_fields(table_name);
      document->set_table_fields(table_name, fields);

      //TODO: Just let create_table_with_default_fields() update the document, and then reload the row.
    }

    save_to_document();
    //fill_from_database(); //We should not modify the model structure in a cellrenderer signal handler.
  }
}

void Box_Tables::on_adddel_Delete(const Gtk::TreeModel::iterator& rowStart, const Gtk::TreeModel::iterator& rowEnd)
{
  auto iterAfter = rowEnd;
  ++iterAfter;

  bool something_changed = false;
  for(auto iter = rowStart; iter != iterAfter; ++iter)
  {
    const auto table_name = m_AddDel.get_value_key(iter);

    if(!table_name.empty())
    {
      auto document = get_document();
      if(document)
      {
        //Don't delete a table that the document does not know about, because we need information from the document:
        if(!document->get_table_is_known(table_name))
        {
           //TODO: Do not show tables that are not in the document.
           Gtk::MessageDialog dialog(_("You cannot delete this table, because there is no information about this table in the document."));
           dialog.set_transient_for(*AppWindow::get_appwindow());
           dialog.run();
        }
        else
        {
          //Ask the user to confirm:
          const auto strMsg = Glib::ustring::compose(_("Are you sure that you want to delete this table?\nTable name: %1"), table_name);
          Gtk::MessageDialog dialog(UiUtils::bold_message(_("Delete Table")),
            true, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE,
            true /* modal */);
          dialog.set_secondary_text(strMsg);
          dialog.set_transient_for(*AppWindow::get_appwindow());
          dialog.add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
          dialog.add_button(_("Delete Table"), Gtk::RESPONSE_OK);
          const auto iButtonClicked = dialog.run();

          //Get a list of autoincrementing fields in the table:
          const auto fields = document->get_table_fields(table_name);

          //Delete the table:
          if(iButtonClicked == Gtk::RESPONSE_OK)
          {
            const auto test = DbUtils::drop_table(table_name);
            if(!test)
              std::cerr << G_STRFUNC << ": DROP TABLE failed.\n";
            else
            {
              //Forget about it in the document too.
              get_document()->remove_table(table_name);

              something_changed = true;
            }

            //Remove the auto-increment rows.
            //Otherwise it would not start at 0 if a table with the same name, and same field, is added again later.
            for(const auto& field : fields)
            {
              if(!field || !field->get_auto_increment())
                continue;

              const auto field_name = field->get_name();

              if(!field_name.empty())
                DbUtils::remove_auto_increment(table_name, field_name);
              else
              {
                std::cerr << G_STRFUNC << ": field_name is empty\n";
              }
            }
          }
        }
      }
    }
  }

  if(something_changed)
  {
    save_to_document();

    fill_from_database();
  }
}


void Box_Tables::on_adddel_changed(const Gtk::TreeModel::iterator& row, guint column)
{
  if(get_userlevel() == AppState::userlevels::DEVELOPER)
  {
    if(column == m_col_hidden)
    {
      save_to_document();
      //TODO: This causes a crash. fill_from_database(); //Hide/show the table.
    }
    else if(column == m_col_title)
    {
      save_to_document();
    }
    else if(column == m_col_title_singular)
    {
      save_to_document();
    }
    else if(column == m_col_default)
    {
      //Only one table can be the default, so ensure that:
      const auto is_default = m_AddDel.get_value_as_bool(row, m_col_default);
      if(is_default)
      {
        //Set all the other rows to false:
        auto model = m_AddDel.get_model();
        for(const auto& child_row : model->children())
        {
          m_AddDel.set_value(child_row, m_col_default, false);
        }
      }

      save_to_document();
    }
    else if(column == m_col_table_name)
    {
      Glib::ustring table_name = m_AddDel.get_value_key(row);
      Glib::ustring table_name_new = m_AddDel.get_value(row, m_col_table_name);
      if(!table_name.empty() && !table_name_new.empty())
      {
        Glib::ustring strMsg = _("Are you sure that you want to rename this table?");  //TODO: Show old and new names?
        Gtk::MessageDialog dialog(_("<b>Rename Table</b>"), true, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE );
        dialog.add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
        dialog.add_button(_("Rename"), Gtk::RESPONSE_OK);
        dialog.set_secondary_text(strMsg);
        int iButtonClicked = dialog.run();

        //Rename the table:
        if(iButtonClicked == Gtk::RESPONSE_OK)
        {
          const auto test = DbUtils::rename_table(table_name, table_name_new);
          if(test)
          {
            //Change the AddDel item's key:
            m_AddDel.set_value_key(row, table_name_new);

            set_modified();

            //Tell the document that this table's name has changed:
            auto document = get_document();
            if(document)
              document->change_table_name(table_name, table_name_new);

            //fill_from_database(); //We should not modify the model structure in a cellrenderer signal handler.
          }
        }
      }
    }
  }
}
#endif //GLOM_ENABLE_CLIENT_ONLY

void Box_Tables::on_adddel_Edit(const Gtk::TreeModel::iterator& row)
{
  Glib::ustring table_name = m_AddDel.get_value_key(row);

  auto document = get_document();
  if(document)
  {
    //Don't open a table that the document does not know about, because we need information from the document:
    //This should never happen, because we never show them in the list:
    if(false) //Let's see if we can adapt.  (!document->get_table_is_known(table_name))
    {
       Gtk::MessageDialog dialog(UiUtils::bold_message(_("Unknown Table")), true);
       dialog.set_secondary_text(_("You cannot open this table, because there is no information about this table in the document."));
       dialog.set_transient_for(*AppWindow::get_appwindow());
       dialog.run();
    }
    else
    {
       //Go ahead:

       save_to_document();

       //Emit the signal:
       signal_selected.emit(table_name);
    }
  }
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void Box_Tables::save_to_document()
{
  if(get_userlevel() == AppState::userlevels::DEVELOPER)
  {
    //Save the hidden tables. TODO_usermode: Only if we are in developer mode.
    Document::type_listTableInfo listTables;

    auto document = get_document();

    for(const auto& row : m_AddDel.get_model()->children())
    {
      const auto table_name = m_AddDel.get_value(row, m_col_table_name); //The name has already been changed in the document.
      auto table_info = document->get_table(table_name); //Start with the existing table_info, to preserve extra information, such as translations.
      if(table_info)
      {
        table_info->set_name( m_AddDel.get_value(row, m_col_table_name) );

        if(!table_info->get_name().empty())
        {
          table_info->set_hidden( m_AddDel.get_value_as_bool(row, m_col_hidden) );
          table_info->set_title( m_AddDel.get_value(row, m_col_title) , AppWindow::get_current_locale()); //TODO_Translations: Store the TableInfo in the TreeView.
          table_info->set_title_singular( m_AddDel.get_value(row, m_col_title_singular), AppWindow::get_current_locale()); //TODO_Translations: Store the TableInfo in the TreeView.
          //std::cout << "debug: " << G_STRFUNC << ": title=" << item_get_title(table_info) << std::endl;
          table_info->set_default( m_AddDel.get_value_as_bool(row, m_col_default) );

          listTables.emplace_back(table_info);
        }
      }
    }

    if(document)
      document->set_tables(listTables); //TODO: Don't save all new tables - just the ones already in the document.
  }
}
#endif //GLOM_ENABLE_CLIENT_ONLY

void Box_Tables::on_show_hidden_toggled()
{
  fill_from_database();
}

void Box_Tables::on_userlevel_changed(AppState::userlevels /* userlevel */)
{
  fill_from_database();
}

} //namespace Glom
