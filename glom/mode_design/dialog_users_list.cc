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
//#include <libgnome/gnome-i18n.h>
#include <libintl.h>

Dialog_UsersList::Dialog_UsersList(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::Dialog(cobject),
  m_treeview_users(0),
  m_button_add(0),
  m_button_delete(0),
  m_button_edit(0)
{
  //refGlade->get_widget("label_table_name", m_label_table_name);

  refGlade->get_widget("treeview", m_treeview_users);
  if(m_treeview_users)
  {
    m_treeview_users->set_reorderable();
    m_treeview_users->enable_model_drag_source();
    m_treeview_users->enable_model_drag_dest();

    Glib::RefPtr<Gtk::TreeView::Selection> refSelection = m_treeview_users->get_selection();
    if(refSelection)
    {
      refSelection->signal_changed().connect( sigc::mem_fun(*this, &Dialog_UsersList::on_treeview_selection_changed) );
    }

    m_model_items = Gtk::ListStore::create(m_model_columns);

    fill_list();

    m_treeview_users->set_model(m_model_items);


    // Append the View columns:
    m_treeview_users->append_column(gettext("User"), m_model_columns.m_col_user);

    // Use set_cell_data_func() to give more control over the cell attributes depending on the row:

    //Name column:
    /*
    Gtk::TreeView::Column* column_name = Gtk::manage( new Gtk::TreeView::Column(gettext("Name")) );
    m_treeview_users->append_column(*column_name);

    Gtk::CellRendererText* renderer_name = Gtk::manage(new Gtk::CellRendererText);
    column_name->pack_start(*renderer_name);
    column_name->set_cell_data_func(*renderer_name, sigc::mem_fun(*this, &Dialog_UsersList::on_cell_data_name));

    //Connect to its signal:
    renderer_name->property_editable() = true;
    renderer_name->signal_edited().connect(
      sigc::bind( sigc::mem_fun(*this, &Dialog_UsersList::on_treeview_cell_edited_text), m_model_items->m_columns.m_col_name) );


    //Title column:
    Gtk::TreeView::Column* column_title = Gtk::manage( new Gtk::TreeView::Column(gettext("Title")) );
    m_treeview_users->append_column(*column_title);

    Gtk::CellRendererText* renderer_title = Gtk::manage(new Gtk::CellRendererText);
    column_title->pack_start(*renderer_title);
    column_title->set_cell_data_func(*renderer_title, sigc::mem_fun(*this, &Dialog_UsersList::on_cell_data_title));

    //Connect to its signal:
    renderer_title->signal_edited().connect(
      sigc::bind( sigc::mem_fun(*this, &Dialog_UsersList::on_treeview_cell_edited_text), m_model_items->m_columns.m_col_title) );


    //Columns-count column:
    Gtk::TreeView::Column* column_count = Gtk::manage( new Gtk::TreeView::Column(gettext("Columns Count")) );
    m_treeview_users->append_column(*column_count);

    Gtk::CellRendererText* renderer_count = Gtk::manage(new Gtk::CellRendererText);
    column_count->pack_start(*renderer_count);
    column_count->set_cell_data_func(*renderer_count, sigc::mem_fun(*this, &Dialog_UsersList::on_cell_data_columns_count));

    //Connect to its signal:
    renderer_count->signal_edited().connect(
      sigc::bind( sigc::mem_fun(*this, &Dialog_UsersList::on_treeview_cell_edited_numeric), m_model_items->m_columns.m_col_columns_count) );

    //Respond to changes of selection:
    Glib::RefPtr<Gtk::TreeView::Selection> refSelection = m_treeview_users->get_selection();
    if(refSelection)
    {
      refSelection->signal_changed().connect( sigc::mem_fun(*this, &Dialog_UsersList::on_treeview_fields_selection_changed) );
    }

    m_model_items->signal_row_changed().connect( sigc::mem_fun(*this, &Dialog_UsersList::on_treemodel_row_changed) );
    */
  }

  refGlade->get_widget("button_delete", m_button_delete);
  m_button_delete->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_UsersList::on_button_delete) );

  refGlade->get_widget("button_add", m_button_add);
  m_button_add->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_UsersList::on_button_add) );

  refGlade->get_widget("button_edit", m_button_edit);
  m_button_edit->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_UsersList::on_button_edit) );

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

    m_model_items->clear();

    //guint field_sequence = 1; //0 means no sequence
    //guint group_sequence = 1; //0 means no sequence
    for(Document_Glom::type_mapLayoutGroupSequence::const_iterator iter = mapGroups.begin(); iter != mapGroups.end(); ++iter)
    {
      const LayoutGroup& group = iter->second;

      add_group(Gtk::TreeModel::iterator(), group);
    }

    //treeview_fill_sequences(m_model_items, m_model_items->m_columns.m_col_sequence); //The document should have checked this already, but it does not hurt to check again.
  }

  //Open all the groups:
  m_treeview_users->expand_all();

  m_modified = false;

}
*/

void Dialog_UsersList::enable_buttons()
{
  //Fields:
  Glib::RefPtr<Gtk::TreeView::Selection> refSelection = m_treeview_users->get_selection();
  if(refSelection)
  {
    Gtk::TreeModel::iterator iter = refSelection->get_selected();
    if(iter)
    {
      m_button_edit->set_sensitive(true);
      m_button_delete->set_sensitive(true);
    }
    else
    {
      //Disable all buttons that act on a selection:
      m_button_edit->set_sensitive(false);
      m_button_delete->set_sensitive(false);
    }
  }

}



void Dialog_UsersList::on_button_delete()
{
  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_treeview_users->get_selection();
  if(refTreeSelection)
  {
    Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;

      const Glib::ustring user = row[m_model_columns.m_col_user];
      if(!user.empty())
      {
        Gtk::MessageDialog dialog(gettext("<b>Delete User</b>"), true, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL);
        dialog.set_secondary_text(gettext("Are your sure that you wish to delete this user?"));
        dialog.set_transient_for(*this);

        int response = dialog.run();
        dialog.hide();

        if(response == Gtk::RESPONSE_OK)
        {
          Glib::ustring strQuery = "DROP USER " + user;
          Query_execute(strQuery);

          fill_list();
        }
      }

      //m_modified = true;
    }
  }
}

void Dialog_UsersList::on_button_add()
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
  dialog->run();

  const Glib::ustring user = dialog->m_entry_user->get_text();
  const Glib::ustring password = dialog->m_entry_password->get_text();

  delete dialog;

  if(!user.empty() && !password.empty())
  {
    Glib::ustring strQuery = "CREATE USER " + user + " PASSWORD '" + password + "'" ;
    Glib::RefPtr<Gnome::Gda::DataModel> data_model = Query_execute(strQuery);

    fill_list();
  }
}

void Dialog_UsersList::on_button_edit()
{
  //Get the selected item:
  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_treeview_users->get_selection();
  if(refTreeSelection)
  {
    Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
    if(iter)
    {
      //Do something different for each type of item:
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

      dialog->m_entry_user->set_text( row[m_model_columns.m_col_user] );
      dialog->m_entry_user->set_sensitive(false); //They can edit the password, but not the name. TODO: Allow editing of name?

      dialog->run();

      const Glib::ustring user = dialog->m_entry_user->get_text();
      const Glib::ustring password = dialog->m_entry_password->get_text();

      delete dialog;

      if(!user.empty() && !password.empty())
      {
        Glib::ustring strQuery = "ALTER USER " + user + " PASSWORD '" + password + "'" ;
        Glib::RefPtr<Gnome::Gda::DataModel> data_model = Query_execute(strQuery);

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

void Dialog_UsersList::on_treeview_selection_changed()
{
  enable_buttons();
}

void Dialog_UsersList::fill_list()
{
  //Fill the model rows:
  m_model_items->clear();

  //pg_shadow contains the users. pg_users is a view of pg_shadow without the password.
  Glib::ustring strQuery = "SELECT pg_shadow.usename FROM pg_shadow";
  Glib::RefPtr<Gnome::Gda::DataModel> data_model = Query_execute(strQuery);
  if(data_model)
  {
    const int rows_count = data_model->get_n_rows();
    for(int row = 0; row < rows_count; ++row)
    {
        const Gnome::Gda::Value value = data_model->get_value_at(0, row);
        const Glib::ustring name = value.get_string();

        Gtk::TreeModel::iterator iter = m_model_items->append();
        Gtk::TreeModel::Row row = *iter;
        row[m_model_columns.m_col_user] = name;
    }
  }
  else
  {
    handle_error();
  }
}















