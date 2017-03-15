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

#include "dialog_groups_list.h"
#include "dialog_user.h"
#include "dialog_users_list.h"
#include "dialog_new_group.h"
#include <libglom/standard_table_prefs_fields.h>
#include <glom/glade_utils.h>
#include <glom/appwindow.h>
#include <libglom/privs.h>
//#include <libgnome/gnome-i18n.h>
#include <glom/utils_ui.h> //For bold_message()).
#include <libglom/db_utils.h>
#include <glibmm/i18n.h>

namespace Glom
{

const char* Dialog_GroupsList::glade_id("window_groups");
const bool Dialog_GroupsList::glade_developer(true);

Dialog_GroupsList::Dialog_GroupsList(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Dialog(cobject),
  m_treeview_groups(nullptr),
  m_button_group_new(nullptr),
  m_button_group_delete(nullptr),
  m_button_group_users(nullptr)
{
  //set_default_size(600, -1);

  builder->get_widget("treeview_groups", m_treeview_groups);
  builder->get_widget("treeview_tables", m_treeview_tables);

  m_model_groups = Gtk::ListStore::create(m_model_columns_groups);
  m_model_tables = Gtk::ListStore::create(m_model_columns_tables);

  //Do this only in load_from_document(), to avoid ever doing it too early, before there is a connection:
  //fill_group_list();
  //fill_table_list();

  m_treeview_groups->set_model(m_model_groups);
  m_treeview_tables->set_model(m_model_tables);

  // Append the View columns:

  //Groups:
  auto pCell = Gtk::manage(new Gtk::CellRendererText);
  auto pViewColumn = Gtk::manage(new Gtk::TreeView::Column(_("Name"), *pCell) );
  pViewColumn->set_cell_data_func(*pCell, sigc::mem_fun(*this, &Dialog_GroupsList::on_cell_data_group_name));
  m_treeview_groups->append_column(*pViewColumn);

  m_treeview_groups->append_column(_("Description"), m_model_columns_groups.m_col_description);

  //Tables:
  m_treeview_tables->append_column(_("Table"), m_model_columns_tables.m_col_title);


  treeview_append_bool_column(*m_treeview_tables, _("View"), m_model_columns_tables.m_col_view,
    sigc::mem_fun( *this, &Dialog_GroupsList::on_treeview_tables_toggled_view) );

  treeview_append_bool_column(*m_treeview_tables, _("_Edit"), m_model_columns_tables.m_col_edit,
    sigc::mem_fun( *this, &Dialog_GroupsList::on_treeview_tables_toggled_edit) );

  treeview_append_bool_column(*m_treeview_tables, _("Create"), m_model_columns_tables.m_col_create,
    sigc::mem_fun( *this, &Dialog_GroupsList::on_treeview_tables_toggled_create) );

  treeview_append_bool_column(*m_treeview_tables, _("_Delete"), m_model_columns_tables.m_col_delete,
    sigc::mem_fun( *this, &Dialog_GroupsList::on_treeview_tables_toggled_delete) );


  auto refSelection = m_treeview_groups->get_selection();
  if(refSelection)
  {
    refSelection->signal_changed().connect( sigc::mem_fun(*this, &Dialog_GroupsList::on_treeview_groups_selection_changed) );
  }

  refSelection = m_treeview_tables->get_selection();
  if(refSelection)
  {
    refSelection->signal_changed().connect( sigc::mem_fun(*this, &Dialog_GroupsList::on_treeview_tables_selection_changed) );
  }

  builder->get_widget("button_group_new", m_button_group_new);
  m_button_group_new->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_GroupsList::on_button_group_new) );

  builder->get_widget("button_group_delete", m_button_group_delete);
  m_button_group_delete->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_GroupsList::on_button_group_delete) );

  builder->get_widget("button_group_users", m_button_group_users);
  m_button_group_users->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_GroupsList::on_button_group_users) );

  enable_buttons();
}

/*
void Dialog_GroupsList::set_document(const Glib::ustring& layout, const std::shared_ptr<Document>& document, const Glib::ustring& table_name, const type_vecLayoutFields& table_fields)
{
  m_modified = false;

  Dialog_Layout::set_document(layout, document, table_name, table_fields);


  //Update the tree models from the document
  if(document)
  {
    //Set the table name and title:
    m_label_table_name->set_text(table_name);
    m_entry_table_title->set_text( document->get_table_title(table_name, AppWindow::get_current_locale()) );

    Document::type_list_layout_groups mapGroups = document->get_data_layout_groups_plus_new_fields(layout, m_table_name, m_layout_platform);

    //If no information is stored in the document, then start with something:

    if(mapGroups.empty())
    {
      LayoutGroup group;
      group.set_name("main");
      group.m_columns_count = 2;

      guint field_sequence = 1; //0 means no sequence
      for(const auto& item : table_fields)
      {
        group.add_item(item);

        ++field_sequence;
      }

      mapGroups[1] = group;
    }

    //Show the field layout
    typedef std::vector< Glib::ustring > type_listStrings;

    m_model_groups->clear();

    for(const auto& the_pair : mapGroups)
    {
      auto group = the_pairsecond;

      add_group(Gtk::TreeModel::iterator(), group);
    }

    //treeview_fill_sequences(m_model_groups, m_model_groups->m_columns.m_col_sequence); //The document should have checked this already, but it does not hurt to check again.
  }

  //Open all the groups:
  m_treeview_groups->expand_all();

  m_modified = false;

}
*/

void Dialog_GroupsList::enable_buttons()
{
  if(!m_button_group_users)
    return;

  auto refSelection = m_treeview_groups->get_selection();
  if(refSelection)
  {
    auto iter = refSelection->get_selected();
    if(iter)
    {
      m_button_group_users->set_sensitive(true);
      m_button_group_delete->set_sensitive(true);
    }
    else
    {
      //Disable all buttons that act on a selection:
      m_button_group_users->set_sensitive(false);
      m_button_group_delete->set_sensitive(false);
    }
  }

}

void Dialog_GroupsList::on_button_group_delete()
{
  auto refTreeSelection = m_treeview_groups->get_selection();
  if(refTreeSelection)
  {
    auto iter = refTreeSelection->get_selected();
    if(iter)
    {
      const auto row = *iter;

      const Glib::ustring group = row[m_model_columns_groups.m_col_name];
      if(!group.empty())
      {
        //TODO: Prevent deletion of standard groups
        Gtk::MessageDialog dialog(UiUtils::bold_message(_("Delete Group")), true, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL);
        dialog.set_secondary_text(_("Are your sure that you wish to delete this group?"));
        dialog.set_transient_for(*this);

        int dialog_response = dialog.run();
        dialog.hide();

        if(dialog_response == Gtk::RESPONSE_OK)
        {
          const Glib::ustring strQuery = "DROP GROUP " + DbUtils::escape_sql_id(group);
          const auto test = DbUtils::query_execute_string(strQuery);
          if(!test)
            std::cerr << G_STRFUNC << ": DROP GROUP failed.\n";

          fill_group_list();
        }
      }

      //m_modified = true;
    }
  }
}

void Dialog_GroupsList::on_button_group_new()
{
  Dialog_NewGroup* dialog = nullptr;
  Utils::get_glade_widget_derived_with_warning(dialog);
  if(!dialog) //Unlikely and it already warns on stderr.
    return;

  dialog->set_transient_for(*this);
  const auto dialog_response = Glom::UiUtils::dialog_run_with_help(dialog);

  const auto group_name = dialog->m_entry_name->get_text();

  delete dialog;

  if(dialog_response != Gtk::RESPONSE_OK)
    return;

  if(group_name.empty())
  {
    std::cerr << G_STRFUNC << ": : group_name is empty\n";
    return;
  }

  if(!DbUtils::add_group(get_document(), group_name))
  {
    std::cerr << G_STRFUNC << ": : DbUtils::add_group() failed.\n";
  }

  fill_group_list();
}

void Dialog_GroupsList::on_button_group_users()
{

  //Get the selected item:
  auto refTreeSelection = m_treeview_groups->get_selection();
  if(refTreeSelection)
  {
    auto iter = refTreeSelection->get_selected();
    if(iter)
    {
      const auto row = *iter;
      const Glib::ustring group_name = row[m_model_columns_groups.m_col_name];

      Dialog_UsersList* dialog = nullptr;
      Utils::get_glade_widget_derived_with_warning(dialog);
      if(!dialog) //Unlikely and it already warns on stderr.
        return;

      dialog->set_transient_for(*this);
      add_view(dialog); //Give it access to the document.

      dialog->fill_list();
      dialog->set_group(group_name);

      Glom::UiUtils::dialog_run_with_help(dialog);

      remove_view(dialog);
      delete dialog;

      fill_group_list();
    }
  }
}

void Dialog_GroupsList::save_to_document()
{


  //if(m_modified)
  //{

  //}
}

void Dialog_GroupsList::on_treeview_tables_selection_changed()
{
  enable_buttons();
}

Glib::ustring Dialog_GroupsList::get_selected_group() const
{
  auto refSelection = m_treeview_groups->get_selection();
  if(refSelection)
  {
    auto iter = refSelection->get_selected();
    if(iter)
    {
      const auto row = *iter;
      return row[m_model_columns_groups.m_col_name];
    }
  }

  return Glib::ustring();
}

void Dialog_GroupsList::on_treeview_groups_selection_changed()
{
  //Update the tables list for the currently-selected group:
  const auto group_name = get_selected_group();
  if(!group_name.empty()) //This can happen when clearing the list, just before filling it, so something real can be selected.
    fill_table_list(group_name);

  enable_buttons();
}

void Dialog_GroupsList::fill_group_list()
{
  //Fill the model rows:
  m_model_groups->clear();

  for(const auto& group : Privs::get_database_groups())
  {
    auto iterTree = m_model_groups->append();
    auto row = *iterTree;

    row[m_model_columns_groups.m_col_name] = group;

    if(group == GLOM_STANDARD_GROUP_NAME_DEVELOPER)
      row[m_model_columns_groups.m_col_description] = _("Full access.");
  }

  if(m_treeview_groups && m_treeview_groups->get_model()) //Avoid a warning.
  {
    //Select the first item, so that there is always something in the tables TreeView:
    auto refSelection = m_treeview_groups->get_selection();
    if(refSelection)
    {
      auto iterFirst = m_model_groups->children().begin();
      if(iterFirst)
      {
        refSelection->select(iterFirst);
      }
    }
  }
}

void Dialog_GroupsList::fill_table_list(const Glib::ustring& group_name)
{
  if(group_name.empty())
  {
    std::cerr << G_STRFUNC << ": group_name is empty.\n";
  }

  //Fill the model rows:
  m_model_tables->clear();

 auto document = get_document();
  if(document)
  {
    // Make sure that these are in the document,
    // so that the correct groups will be created if we recreate the database from the document:
    GroupInfo group_info;
    group_info.set_name(group_name);

    Document::type_listTableInfo table_list = document->get_tables(true /* plus system prefs */);

    for(const auto& table : table_list)
    {
      auto iterTree = m_model_tables->append();
      auto row = *iterTree;

      const auto table_name = table->get_name();

      row[m_model_columns_tables.m_col_name] = table_name;
      row[m_model_columns_tables.m_col_title] = item_get_title_or_name(table);

      const auto privs = Privs::get_table_privileges(group_name, table_name);
      row[m_model_columns_tables.m_col_view] = privs.m_view;
      row[m_model_columns_tables.m_col_edit] = privs.m_edit;
      row[m_model_columns_tables.m_col_create] = privs.m_create;
      row[m_model_columns_tables.m_col_delete] = privs.m_delete;


      group_info.m_map_privileges[table_name] = privs;
    }

    document->set_group(group_info);
  }
}

void Dialog_GroupsList::load_from_document()
{
  //Ensure that the glom_developer group exists.
  DbUtils::add_standard_groups(get_document());

  fill_group_list();
  //fill_table_list();
}

void Dialog_GroupsList::treeview_append_bool_column(Gtk::TreeView& treeview, const Glib::ustring& title, Gtk::TreeModelColumn<bool>& model_column, const sigc::slot<void(const Glib::ustring&)>& slot_toggled)
{
  auto pCellRenderer = Gtk::manage( new Gtk::CellRendererToggle() );

  //GTK+'s "activatable" really means "editable":
  pCellRenderer->property_activatable() = true;

  auto pViewColumn = Gtk::manage( new Gtk::TreeView::Column(title, *pCellRenderer) );
  pViewColumn->set_renderer(*pCellRenderer, model_column); //render it via the default "text" property.

  treeview.append_column(*pViewColumn);

  pCellRenderer->signal_toggled().connect( slot_toggled );
}

//TODO: Use Privs::set_table_privileges() instead?
bool Dialog_GroupsList::set_table_privilege(const Glib::ustring& table_name, const Glib::ustring& group_name, bool grant, enumPriv priv)
{
  if(group_name.empty() || table_name.empty())
    return false;

  if(group_name == GLOM_STANDARD_GROUP_NAME_DEVELOPER)
    return false;

  //Change the permission in the database:

  //Build the SQL statement:

  //Grant or revoke:
  Glib::ustring strQuery;
  if(grant)
    strQuery = "GRANT";
  else
    strQuery = "REVOKE";

  //What to grant or revoke:
  Glib::ustring strPrivilege;
  if(priv == enumPriv::VIEW)
    strPrivilege = "SELECT";
  else if(priv == enumPriv::EDIT)
    strPrivilege = "UPDATE";
  else if(priv == enumPriv::CREATE)
    strPrivilege = "INSERT";
  else if(priv == enumPriv::DELETE)
    strPrivilege = "DELETE";

  strQuery += " " + strPrivilege + " ON " + DbUtils::escape_sql_id(table_name) + " ";

  //This must match the Grant or Revoke:
  if(grant)
    strQuery += "TO";
  else
    strQuery += "FROM";

  strQuery += " GROUP " + DbUtils::escape_sql_id(group_name);

  const auto test = DbUtils::query_execute_string(strQuery); //TODO: Handle errors.
  if(!test)
    std::cerr << G_STRFUNC << ": GRANT/REVOKE failed.\n";

  return test;
}

void Dialog_GroupsList::on_treeview_tables_toggled_view(const Glib::ustring& path_string)
{
  Gtk::TreeModel::Path path(path_string);

  //Get the row from the path:
  auto iter = m_model_tables->get_iter(path);
  if(iter)
  {
    Gtk::TreeRow row = *iter;

    //Toggle the value in the model:
    bool bActive = row[m_model_columns_tables.m_col_view];
    bActive = !bActive;

    const auto group_name = get_selected_group();
    const Glib::ustring table_name = row[m_model_columns_tables.m_col_name];

    if(set_table_privilege(table_name, group_name, bActive, enumPriv::VIEW))
      row[m_model_columns_tables.m_col_view] = bActive;

    //If the group cannot view, then it should not do anything else either:
    if(!bActive)
    {
     if(set_table_privilege(table_name, group_name, bActive, enumPriv::EDIT))
        row[m_model_columns_tables.m_col_edit] = false;

      if(set_table_privilege(table_name, group_name, bActive, enumPriv::CREATE))
        row[m_model_columns_tables.m_col_create] = false;

      if(set_table_privilege(table_name, group_name, bActive, enumPriv::DELETE))
        row[m_model_columns_tables.m_col_delete] = false;
    }
  }
}

void Dialog_GroupsList::on_treeview_tables_toggled_edit(const Glib::ustring& path_string)
{
  Gtk::TreeModel::Path path(path_string);

  //Get the row from the path:
  auto iter = m_model_tables->get_iter(path);
  if(iter)
  {
    Gtk::TreeRow row = *iter;

    //Toggle the value in the model:
    bool bActive = row[m_model_columns_tables.m_col_edit];
    bActive = !bActive;

    const auto group_name = get_selected_group();
    const Glib::ustring table_name = row[m_model_columns_tables.m_col_name];

    bool test = set_table_privilege(table_name, group_name, bActive, enumPriv::EDIT);

    if(test)
      row[m_model_columns_tables.m_col_edit] = bActive;
  }
}

void Dialog_GroupsList::on_treeview_tables_toggled_create(const Glib::ustring& path_string)
{
  Gtk::TreeModel::Path path(path_string);

  //Get the row from the path:
  auto iter = m_model_tables->get_iter(path);
  if(iter)
  {
    Gtk::TreeRow row = *iter;

    //Toggle the value in the model:
    bool bActive = row[m_model_columns_tables.m_col_create];
    bActive = !bActive;

    const auto group_name = get_selected_group();
    const Glib::ustring table_name = row[m_model_columns_tables.m_col_name];

    const auto test = set_table_privilege(table_name, group_name, bActive, enumPriv::CREATE);

    if(test)
      row[m_model_columns_tables.m_col_create] = bActive;
  }
}

void Dialog_GroupsList::on_treeview_tables_toggled_delete(const Glib::ustring& path_string)
{
  Gtk::TreeModel::Path path(path_string);

  //Get the row from the path:
  auto iter = m_model_tables->get_iter(path);
  if(iter)
  {
    Gtk::TreeRow row = *iter;

    //Toggle the value in the model:
    bool bActive = row[m_model_columns_tables.m_col_delete];
    bActive = !bActive;

    const auto group_name = get_selected_group();
    const Glib::ustring table_name = row[m_model_columns_tables.m_col_name];

    const auto test = set_table_privilege(table_name, group_name, bActive, enumPriv::DELETE);

    if(test)
      row[m_model_columns_tables.m_col_delete] = bActive;
  }
}

void Dialog_GroupsList::on_cell_data_group_name(Gtk::CellRenderer* renderer, const Gtk::TreeModel::const_iterator& iter)
{
 //Set the view's cell properties depending on the model's data:
  auto renderer_text = dynamic_cast<Gtk::CellRendererText*>(renderer);
  if(!renderer_text)
    return;

  if(!iter)
    return;

  const auto row = *iter;

  Glib::ustring name = row[m_model_columns_groups.m_col_name];

  renderer_text->property_text() = Privs::get_user_visible_group_name(name);
}

} //namespace Glom
