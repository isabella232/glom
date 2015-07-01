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

#include "dialog_users_list.h"
#include "dialog_user.h"
#include "dialog_choose_user.h"
#include <libglom/privs.h>
#include <libglom/db_utils.h>
#include <glom/glade_utils.h>
#include <glom/utils_ui.h> //For bold_message()).
//#include <libgnome/gnome-i18n.h>
#include <glibmm/i18n.h>

namespace Glom
{

const char* Dialog_UsersList::glade_id("window_users");
const bool Dialog_UsersList::glade_developer(true);

Dialog_UsersList::Dialog_UsersList(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Dialog(cobject),
  m_treeview_users(0),
  m_combo_group(0),
  m_button_user_add(0),
  m_button_user_remove(0),
  m_button_user_new(0),
  m_button_user_delete(0),
  m_button_user_edit(0)
{
  builder->get_widget("combobox_group", m_combo_group);
  m_combo_group->signal_changed().connect(sigc::mem_fun(*this, &Dialog_UsersList::on_combo_group_changed));


  builder->get_widget("treeview_users", m_treeview_users);
  if(m_treeview_users)
  {
    Glib::RefPtr<Gtk::TreeView::Selection> refSelection = m_treeview_users->get_selection();
    if(refSelection)
    {
      refSelection->signal_changed().connect( sigc::mem_fun(*this, &Dialog_UsersList::on_treeview_users_selection_changed) );
    }

    m_model_users = Gtk::ListStore::create(m_model_columns_users);

    m_treeview_users->set_model(m_model_users);


    // Append the View columns:
    m_treeview_users->append_column(C_("Users List", "User"), m_model_columns_users.m_col_name);
  }


  builder->get_widget("button_delete", m_button_user_delete);
  m_button_user_delete->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_UsersList::on_button_user_delete) );

  builder->get_widget("button_add", m_button_user_add);
  m_button_user_add->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_UsersList::on_button_user_add) );

  builder->get_widget("button_remove", m_button_user_remove);
  m_button_user_remove->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_UsersList::on_button_user_remove) );

  builder->get_widget("button_new", m_button_user_new);
  m_button_user_new->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_UsersList::on_button_user_new) );

  builder->get_widget("button_edit", m_button_user_edit);
  m_button_user_edit->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_UsersList::on_button_user_edit) );

  enable_buttons();

  show_all_children();
}

Dialog_UsersList::~Dialog_UsersList()
{
}

void Dialog_UsersList::enable_buttons()
{
  if(!m_button_user_edit)
   return;

  //Fields:
  Glib::RefPtr<Gtk::TreeView::Selection> refSelection = m_treeview_users->get_selection();
  if(refSelection)
  {
    Gtk::TreeModel::iterator iter = refSelection->get_selected();
    if(iter)
    {
      m_button_user_edit->set_sensitive(true);
      m_button_user_delete->set_sensitive(true);
    }
    else
    {
      //Disable all buttons that act on a selection:
      m_button_user_edit->set_sensitive(false);
      m_button_user_delete->set_sensitive(false);
    }
  }

}

void Dialog_UsersList::on_button_user_remove()
{
  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_treeview_users->get_selection();
  if(refTreeSelection)
  {
    Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
    if(iter)
    {
      if(warn_about_empty_standard_group())
      {
        Gtk::TreeModel::Row row = *iter;

        const Glib::ustring user = row[m_model_columns_users.m_col_name];
        if(!user.empty())
        {
          DbUtils::remove_user_from_group(user, m_combo_group->get_active_text());

          fill_list();
        }

        //m_modified = true;
      }
    }
  }
}


void Dialog_UsersList::on_button_user_delete()
{
  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_treeview_users->get_selection();
  if(refTreeSelection)
  {
    Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
    if(iter)
    {
      if(warn_about_empty_standard_group())
      {
        Gtk::TreeModel::Row row = *iter;

        const Glib::ustring user = row[m_model_columns_users.m_col_name];
        if(!user.empty())
        {
          Gtk::MessageDialog dialog(UiUtils::bold_message(_("Delete User")), true, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL);
          dialog.set_secondary_text(_("Are your sure that you wish to delete this user?"));
          dialog.set_transient_for(*this);

          const auto response = dialog.run();
          dialog.hide();

          if(response == Gtk::RESPONSE_OK)
          {
            DbUtils::remove_user(user); //TODO: Warn about failure when this returns false?
            fill_list();
          }
        }
  
        //m_modified = true;
      }
    }
  }
}

void Dialog_UsersList::on_button_user_add()
{
  //Fill it with the list of users:
  const auto users = Privs::get_database_users();
  if(users.empty())
  {
    UiUtils::show_ok_dialog(_("Error Retrieving the List of Users"),
      _("Glom could not get the list of users from the database server. You probably do not have enough permissions. You should be a superuser."), *this, Gtk::MESSAGE_ERROR);
    return;
  }

  Dialog_ChooseUser* dialog = 0;
  Utils::get_glade_widget_derived_with_warning(dialog);
  if(!dialog) //Unlikely and it already warns on stderr.
    return;

  dialog->set_transient_for(*this);

  dialog->set_user_list(users);

  const auto response = Glom::UiUtils::dialog_run_with_help(dialog);

  const auto user = dialog->get_user();

  delete dialog;

  if(response != Gtk::RESPONSE_OK)
    return;

  if(!user.empty())
  {
    //Add it to the group:
    const auto strQuery = DbUtils::build_query_add_user_to_group(m_combo_group->get_active_text(), user);
    const auto test = DbUtils::query_execute_string(strQuery);
    if(!test)
      std::cerr << G_STRFUNC << ": ALTER GROUP failed." << std::endl;

    //Remove any user rights, so that all rights come from the user's presence in the group:
    Document::type_listTableInfo table_list = get_document()->get_tables();

    for(Document::type_listTableInfo::const_iterator iter = table_list.begin(); iter != table_list.end(); ++iter)
    {
      const auto table_name = (*iter)->get_name();
      const Glib::ustring strQuery = "REVOKE ALL PRIVILEGES ON " + DbUtils::escape_sql_id(table_name) + " FROM " + DbUtils::escape_sql_id(user);
      const auto test = DbUtils::query_execute_string(strQuery);
      if(!test)
        std::cerr << G_STRFUNC << ": REVOKE failed." << std::endl;
    }

    fill_list();
  }
}

void Dialog_UsersList::on_button_user_new()
{
  Dialog_User* dialog = 0;
  Utils::get_glade_widget_derived_with_warning(dialog);
  if(!dialog) //Unlikely and it already warns on stderr.
    return;

  dialog->set_transient_for(*this);
  dialog->m_combo_group->set_sensitive(false); //It is being added to the current group, so don't offer a different group.
 
  int response = Gtk::RESPONSE_OK;
  bool keep_trying = true;
  while(keep_trying)
  {
    response = Glom::UiUtils::dialog_run_with_help(dialog);

    //Check the password is acceptable:
    if(response == Gtk::RESPONSE_OK)
    {
      const auto password_ok = dialog->check_password();
      if(password_ok)
        keep_trying = false;
    }
    else
      keep_trying = false;
  }

  const auto user = dialog->m_entry_user->get_text();
  const auto password = dialog->m_entry_password->get_text();

  delete dialog;

  if(response != Gtk::RESPONSE_OK)
    return;

  if(!DbUtils::add_user(get_document(), user, password, m_combo_group->get_active_text() /* group */))
  {
    std::cerr << G_STRFUNC << ": add_user() failed." << std::endl;
  }

  fill_list();
}

void Dialog_UsersList::on_button_user_edit()
{
  //Get the selected item:
  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_treeview_users->get_selection();
  if(refTreeSelection)
  {
    Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;

      Dialog_User* dialog = 0;
      Utils::get_glade_widget_derived_with_warning(dialog);
      if(!dialog) //Unlikely and it already warns on stderr.
        return;

      dialog->set_transient_for(*this);

      //TODO: Do this in the derived class:
      dialog->m_entry_user->set_text( row[m_model_columns_users.m_col_name] );
      dialog->m_entry_user->set_sensitive(false); //They can edit the password, but not the name. TODO: Allow editing of name?

      //Fill groups:
      dialog->m_combo_group->remove_all();

      type_vec_strings group_list = Privs::get_database_groups();
      for(type_vec_strings::const_iterator iter = group_list.begin(); iter != group_list.end(); ++iter)
      {
         dialog->m_combo_group->append(*iter);
      }

      dialog->m_combo_group->set_active_text(m_combo_group->get_active_text());
      dialog->m_combo_group->set_sensitive(false); //TODO: Allow, and handle, changes to this.


      int response = Gtk::RESPONSE_OK;
      bool keep_trying = true;
      while(keep_trying)
      {
        response = Glom::UiUtils::dialog_run_with_help(dialog);

        //Check the password is acceptable:
        if(response == Gtk::RESPONSE_OK)
        {
          const auto password_ok = dialog->check_password();
          if(password_ok)
            keep_trying = false;
        }
        else
          keep_trying = false;
      }

      const auto user = dialog->m_entry_user->get_text();
      const auto password = dialog->m_entry_password->get_text();

      delete dialog;

      if(response != Gtk::RESPONSE_OK)
        return;

      if(!user.empty() && !password.empty())
      {
        //TODO: Can this change the username too?
        //Note: If using MySQL, we need MySQL 5.6.7 for ALTER USER:
        const Glib::ustring strQuery = "ALTER USER " + DbUtils::escape_sql_id(user) + " PASSWORD '" + password + "'" ; //TODO: Escape the password.
        const auto test = DbUtils::query_execute_string(strQuery);
        if(!test)
          std::cerr << G_STRFUNC << ": ALTER USER failed." << std::endl;

        //Change the password in the current connection, if this is the current user.
        ConnectionPool* connection_pool = ConnectionPool::get_instance();
        if(connection_pool->get_user() == user)
          connection_pool->set_password(password);

        fill_list();
      }
    }
  }
}

void Dialog_UsersList::save_to_document()
{


  //if(m_modified)
  //{

  //}
}


void Dialog_UsersList::on_treeview_users_selection_changed()
{
  enable_buttons();
}

void Dialog_UsersList::fill_list()
{
  //Fill the model rows:
  m_model_users->clear();

  if(m_combo_group)
  {
    const auto group_name = m_combo_group->get_active_text();
    const auto user_list = Privs::get_database_users(group_name);
    for(type_vec_strings::const_iterator iter = user_list.begin(); iter != user_list.end(); ++iter)
    {
      Gtk::TreeModel::iterator iterTree = m_model_users->append();
      Gtk::TreeModel::Row row = *iterTree;

      row[m_model_columns_users.m_col_name] = Privs::get_user_visible_group_name(*iter);
    }
  }
}

void Dialog_UsersList::set_group(const Glib::ustring& group_name)
{
  //Fill the list of groups:
  m_combo_group->remove_all();

  type_vec_strings group_list = Privs::get_database_groups();
  for(type_vec_strings::const_iterator iter = group_list.begin(); iter != group_list.end(); ++iter)
  {
    m_combo_group->append(*iter);
  }

  m_combo_group->set_active_text(group_name);
}

void Dialog_UsersList::on_combo_group_changed()
{
  fill_list();
}

bool Dialog_UsersList::warn_about_empty_standard_group()
{
  //Prevent removal of the last developer:
  if(m_combo_group->get_active_text() == GLOM_STANDARD_GROUP_NAME_DEVELOPER)
  {
    if(m_model_users->children().size() == 1)
    {
      Gtk::MessageDialog dialog(UiUtils::bold_message(_("Developer group may not be empty.")), true, Gtk::MESSAGE_WARNING);
      dialog.set_secondary_text(_("The developer group must contain at least one user."));
      dialog.set_transient_for(*this);
      dialog.run();

      return false;
    }
  }

  return true;
}

} //namespace Glom
