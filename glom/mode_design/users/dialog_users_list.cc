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

#include "dialog_users_list.h"
#include "dialog_user.h"
#include "dialog_choose_user.h"
#include <bakery/App/App_Gtk.h> //For util_bold_message().
//#include <libgnome/gnome-i18n.h>
#include <glibmm/i18n.h>

Dialog_UsersList::Dialog_UsersList(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::Dialog(cobject),
  m_treeview_users(0),
  m_combo_group(0),
  m_button_user_add(0),
  m_button_user_remove(0),
  m_button_user_new(0),
  m_button_user_delete(0),
  m_button_user_edit(0)
{
  refGlade->get_widget_derived("combobox_group", m_combo_group);
  m_combo_group->signal_changed().connect(sigc::mem_fun(*this, &Dialog_UsersList::on_combo_group_changed));


  refGlade->get_widget("treeview_users", m_treeview_users);
  if(m_treeview_users)
  {
    m_treeview_users->set_reorderable();
    m_treeview_users->enable_model_drag_source();
    m_treeview_users->enable_model_drag_dest();

    Glib::RefPtr<Gtk::TreeView::Selection> refSelection = m_treeview_users->get_selection();
    if(refSelection)
    {
      refSelection->signal_changed().connect( sigc::mem_fun(*this, &Dialog_UsersList::on_treeview_users_selection_changed) );
    }

    m_model_users = Gtk::ListStore::create(m_model_columns_users);

    fill_list();

    m_treeview_users->set_model(m_model_users);


    // Append the View columns:
    m_treeview_users->append_column(_("User"), m_model_columns_users.m_col_name);
  }


  refGlade->get_widget("button_delete", m_button_user_delete);
  m_button_user_delete->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_UsersList::on_button_user_delete) );

  refGlade->get_widget("button_add", m_button_user_add);
  m_button_user_add->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_UsersList::on_button_user_add) );

  refGlade->get_widget("button_remove", m_button_user_remove);
  m_button_user_remove->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_UsersList::on_button_user_remove) );

  refGlade->get_widget("button_new", m_button_user_new);
  m_button_user_new->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_UsersList::on_button_user_new) );

  refGlade->get_widget("button_edit", m_button_user_edit);
  m_button_user_edit->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_UsersList::on_button_user_edit) );

  enable_buttons();

  show_all_children();
}

Dialog_UsersList::~Dialog_UsersList()
{
}

/*
void Dialog_UsersList::set_document(const Glib::ustring& layout, Document_Glom* document, const Glib::ustring& table_name, const type_vecLayoutFields& table_fields)
{
  m_modified = false;

  Dialog_Layout::set_document(layout, document, table_name, table_fields);


  //Update the tree models from the document
  if(document)
  {
    //Set the table name and title:
    m_label_table_name->set_text(table_name);
    m_entry_table_title->set_text( document->get_table_title(table_name) );

    Document_Glom::type_mapLayoutGroupSequence mapGroups = document->get_data_layout_groups_plus_new_fields(layout, m_table_name);

    //If no information is stored in the document, then start with something:

    if(mapGroups.empty())
    {
      LayoutGroup group;
      group.set_name("main");
      group.m_columns_count = 2;

      guint field_sequence = 1; //0 means no sequence
      for(type_vecLayoutFields::const_iterator iter = table_fields.begin(); iter != table_fields.end(); ++iter)
      {
        LayoutItem_Field item = *iter;
        item.m_sequence = field_sequence;

        group.add_item(item, field_sequence);

        ++field_sequence;
      }

      mapGroups[1] = group;
    }

    //Show the field layout
    typedef std::list< Glib::ustring > type_listStrings;

    m_model_users->clear();

    //guint field_sequence = 1; //0 means no sequence
    //guint group_sequence = 1; //0 means no sequence
    for(Document_Glom::type_mapLayoutGroupSequence::const_iterator iter = mapGroups.begin(); iter != mapGroups.end(); ++iter)
    {
      sharedptr<const LayoutGroup> group = iter->second;

      add_group(Gtk::TreeModel::iterator(), group);
    }

    //treeview_fill_sequences(m_model_users, m_model_users->m_columns.m_col_sequence); //The document should have checked this already, but it does not hurt to check again.
  }

  //Open all the groups:
  m_treeview_users->expand_all();

  m_modified = false;

}
*/

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
          Glib::ustring strQuery = "ALTER GROUP \"" + m_combo_group->get_active_text() + "\" DROP USER \"" + user + "\"";
          query_execute(strQuery, this);

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
          Gtk::MessageDialog dialog(Bakery::App_Gtk::util_bold_message(_("Delete User")), true, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL);
          dialog.set_secondary_text(_("Are your sure that you wish to delete this user?"));
          dialog.set_transient_for(*this);

          int response = dialog.run();
          dialog.hide();

          if(response == Gtk::RESPONSE_OK)
          {
            Glib::ustring strQuery = "DROP USER " + user;
            query_execute(strQuery, this);

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
  Dialog_ChooseUser* dialog = 0;
  try
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_choose_user");

    refXml->get_widget_derived("dialog_choose_user", dialog);
  }
  catch(const Gnome::Glade::XmlError& ex)
  {
    std::cerr << ex.what() << std::endl;
  }

  dialog->set_transient_for(*this);

  //Fill it with the list of users:
  dialog->set_user_list( get_database_users() );

  int response = dialog->run();

  const Glib::ustring user = dialog->get_user();

  delete dialog;

  if(response != Gtk::RESPONSE_OK)
    return;

  if(!user.empty())
  {
    //Add it to the group:
    Glib::ustring strQuery = "ALTER GROUP \"" + m_combo_group->get_active_text() + "\" ADD USER " + user;
    query_execute(strQuery, this);

    //Remove any user rights, so that all rights come from the user's presence in the group:
    Document_Glom::type_listTableInfo table_list = get_document()->get_tables();

    for(Document_Glom::type_listTableInfo::const_iterator iter = table_list.begin(); iter != table_list.end(); ++iter)
    {
      Glib::ustring strQuery = "REVOKE ALL PRIVILEGES ON \"" + (*iter)->get_name() + "\" FROM \"" + user + "\"";
      query_execute(strQuery, this);
    }

    fill_list();
  }
}

void Dialog_UsersList::on_button_user_new()
{
  Dialog_User* dialog = 0;
  try
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_user");

    refXml->get_widget_derived("dialog_user", dialog);
  }
  catch(const Gnome::Glade::XmlError& ex)
  {
    std::cerr << ex.what() << std::endl;
  }

  dialog->set_transient_for(*this);
  int response = dialog->run();

  const Glib::ustring user = dialog->m_entry_user->get_text();
  const Glib::ustring password = dialog->m_entry_password->get_text();

  delete dialog;

  if(response != Gtk::RESPONSE_OK)
    return;

  if(!user.empty() && !password.empty())
  {
    //Create the user:
    Glib::ustring strQuery = "CREATE USER \"" + user + "\" PASSWORD '" + password + "'" ;
    Glib::RefPtr<Gnome::Gda::DataModel> data_model = query_execute(strQuery, this);

    //Add it to the group:
    strQuery = "ALTER GROUP \"" + m_combo_group->get_active_text() + "\" ADD USER \"" + user + "\"";
    data_model = query_execute(strQuery, this);

    fill_list();
  }
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
      try
      {
        Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_user");

        refXml->get_widget_derived("dialog_user", dialog);
      }
      catch(const Gnome::Glade::XmlError& ex)
      {
        std::cerr << ex.what() << std::endl;
      }

      dialog->set_transient_for(*this);

      //TODO: Do this in the derived class:
      dialog->m_entry_user->set_text( row[m_model_columns_users.m_col_name] );
      dialog->m_entry_user->set_sensitive(false); //They can edit the password, but not the name. TODO: Allow editing of name?

      //Fill groups:
      dialog->m_combo_group->clear_text();

      type_vecStrings group_list = get_database_groups();
      for(type_vecStrings::const_iterator iter = group_list.begin(); iter != group_list.end(); ++iter)
      {
         dialog->m_combo_group->append_text(*iter);
      }

      dialog->m_combo_group->set_active_text(m_combo_group->get_active_text());
      dialog->m_combo_group->set_sensitive(false); //TODO: Allow, and handle, changes to this.


      int response = dialog->run();

      const Glib::ustring user = dialog->m_entry_user->get_text();
      const Glib::ustring password = dialog->m_entry_password->get_text(); //TODO: Check the confirmation.

      delete dialog;

      if(response != Gtk::RESPONSE_OK)
        return;

      if(!user.empty() && !password.empty())
      {
        Glib::ustring strQuery = "ALTER USER \"" + user + "\" PASSWORD '" + password + "'" ;
        Glib::RefPtr<Gnome::Gda::DataModel> data_model = query_execute(strQuery, this);

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
    const Glib::ustring group_name = m_combo_group->get_active_text();
    type_vecStrings user_list = get_database_users(group_name);
    for(type_vecStrings::const_iterator iter = user_list.begin(); iter != user_list.end(); ++iter)
    {
      Gtk::TreeModel::iterator iterTree = m_model_users->append();
      Gtk::TreeModel::Row row = *iterTree;

      row[m_model_columns_users.m_col_name] = get_user_visible_group_name(*iter);
    }
  }
}

void Dialog_UsersList::set_group(const Glib::ustring& group_name)
{
  //Fill the list of groups:
  m_combo_group->clear_text();

  type_vecStrings group_list = get_database_groups();
  for(type_vecStrings::const_iterator iter = group_list.begin(); iter != group_list.end(); ++iter)
  {
    m_combo_group->append_text(*iter);
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
      Gtk::MessageDialog dialog(Bakery::App_Gtk::util_bold_message(_("Developer group may not be empty.")), true, Gtk::MESSAGE_WARNING);
      dialog.set_secondary_text(_("The developer group must contain at least one user."));
      dialog.set_transient_for(*this);
      dialog.run();

      return false;
    }
  }

  return true;
}












