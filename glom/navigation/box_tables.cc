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

#include "box_tables.h"
#include <bakery/App/App_Gtk.h> //For util_bold_message().
#include <glibmm/i18n.h>

namespace Glom
{

Box_Tables::Box_Tables(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Box_DB(cobject),
  m_pLabelFrameTitle(0),
  m_pCheckButtonShowHidden(0),
  m_colTableName(0),
  m_colHidden(0),
  m_colTitle(0),
  m_colDefault(0)
{
  //Get the Glade-instantiated widgets, and connect signal handlers:
  Gtk::Button* pButtonCancel = 0;
  refGlade->get_widget("button_cancel", pButtonCancel);
  set_button_cancel(*pButtonCancel);

  Gtk::Alignment* pAligmentPlaceholder = 0;
  refGlade->get_widget("alignment_placeholder_adddel", pAligmentPlaceholder);
  pAligmentPlaceholder->add(m_AddDel);

  refGlade->get_widget("label_frame_title", m_pLabelFrameTitle);

  refGlade->get_widget("checkbutton_show_hidden", m_pCheckButtonShowHidden);
  m_pCheckButtonShowHidden->signal_toggled().connect(sigc::mem_fun(*this, &Box_Tables::on_show_hidden_toggled));

  m_AddDel.signal_user_added().connect(sigc::mem_fun(*this, &Box_Tables::on_adddel_Add));
  m_AddDel.signal_user_requested_delete().connect(sigc::mem_fun(*this, &Box_Tables::on_adddel_Delete));
  m_AddDel.signal_user_requested_edit().connect(sigc::mem_fun(*this, &Box_Tables::on_adddel_Edit));
  m_AddDel.signal_user_changed().connect(sigc::mem_fun(*this, &Box_Tables::on_adddel_changed));

  show_all_children();
}

Box_Tables::~Box_Tables()
{
}

void Box_Tables::fill_table_row(const Gtk::TreeModel::iterator& iter, const sharedptr<const TableInfo>& table_info)
{
  if(iter)
  {
    const bool developer_mode = (get_userlevel() == AppState::USERLEVEL_DEVELOPER);

    m_AddDel.set_value_key(iter, table_info->get_name());
    m_AddDel.set_value(iter, m_colTableName, table_info->get_name());
    m_AddDel.set_value(iter, m_colHidden, table_info->m_hidden);

    if(developer_mode)
    {
      //std::cout << "Box_Tables::fill_table_row(): dev title=" << table_info->get_title() << std::endl;
      m_AddDel.set_value(iter, m_colTitle, table_info->get_title());
    }
    else
    {
      //std::cout << "Box_Tables::fill_table_row(): op get_title_or_name=" << table_info->get_title_or_name() << std::endl;
      m_AddDel.set_value(iter, m_colTitle, table_info->get_title_or_name());
    }

    m_AddDel.set_value(iter, m_colDefault, table_info->m_default);
  }
}

bool Box_Tables::fill_from_database()
{
  Bakery::BusyCursor busy_cursor(get_app_window());

  bool result = Box_DB::fill_from_database();

  //Enable/Disable extra widgets:
  const bool developer_mode = (get_userlevel() == AppState::USERLEVEL_DEVELOPER);

  //Developers see more columns, so make it bigger:
  if(developer_mode)
    set_size_request(400, -1);
  else
    set_size_request(-1, -1);

  m_pCheckButtonShowHidden->set_sensitive(developer_mode); //Operators have no choice - they can't see hidden tables ever.
  if(!developer_mode)
    m_pCheckButtonShowHidden->set_active(false); //Operators have no choice - they can't see hidden tables ever.

  m_AddDel.remove_all();

  //Add the columns:
  m_AddDel.remove_all_columns();

  const bool editable = developer_mode;
  const bool visible_extras = developer_mode;
  m_colTableName = m_AddDel.add_column(_("Tables"), AddDelColumnInfo::STYLE_Text, editable, visible_extras);
  m_AddDel.prevent_duplicates(m_colTableName); //Prevent two tables with the same name from being added.
  m_AddDel.set_prevent_duplicates_warning(_("This table already exists. Please choose a different table name"));

  m_colHidden = m_AddDel.add_column(_("Hidden"), AddDelColumnInfo::STYLE_Boolean, editable, visible_extras);
  m_colTitle =  m_AddDel.add_column(_("Title"), AddDelColumnInfo::STYLE_Text, editable, true);
  m_colDefault =  m_AddDel.add_column(_("Default"), AddDelColumnInfo::STYLE_Boolean,  editable, visible_extras);


  //_("Server: ") +  m_strServerName + ", " + 
  //Glib::ustring strTitle = Glib::ustring("<b>") + _("Tables from Database: ") + get_database_name() + "");
  //m_pLabelFrameTitle->set_markup(strTitle);

  //Get the list of hidden tables:

  Document_Glom::type_listTableInfo listTablesDocument;
  Document_Glom* document = get_document();
  if(document)
  {
    listTablesDocument = document->get_tables();
  }
  else
    g_warning("Box_Tables::fill_from_database(): document is null");

  //Get the list of tables in the database, from the server:
  sharedptr<SharedConnection> sharedconnection = connect_to_server(get_app_window());
  if(sharedconnection)
  {
    m_AddDel.remove_all();
    Glib::RefPtr<Gnome::Gda::Connection> connection = sharedconnection->get_gda_connection();

    type_vecStrings vecTables = document->get_table_names();

    for(type_vecStrings::iterator iter = vecTables.begin(); iter != vecTables.end(); iter++)
    {
      const Glib::ustring strName = *iter;

      sharedptr<TableInfo> table_info;

      //Check whether it should be hidden:
      Document_Glom::type_listTableInfo::iterator iterFind = std::find_if(listTablesDocument.begin(), listTablesDocument.end(), predicate_FieldHasName<TableInfo>(strName));
      if(iterFind != listTablesDocument.end())
      {
        table_info = *iterFind;

        //std::cout << "fill_from_database(): name=" << (*iterFind)->get_name() << ", table_info->get_title()=" << (*iterFind)->get_title() << std::endl;
      }
      else
      {
        //This table is in the database, but not in the document.
        //Show it as hidden:
        table_info = sharedptr<TableInfo>(new TableInfo());
        table_info->set_name(strName);
        table_info->m_hidden = true;
      }

      const bool hidden = table_info->m_hidden;

      bool bAddIt = true;
      if(hidden && !developer_mode)  //Don't add hidden tables unless we are in developer mode:
        bAddIt = false;

      if(hidden && !m_pCheckButtonShowHidden->get_active()) //Don't add hidden tables if that checkbox is unset.
      {
        bAddIt = false;
      }

      //Check whether it's a system table, though they should never be in this list:
      const Glib::ustring prefix = "glom_system_";
      const Glib::ustring table_prefix = strName.substr(0, prefix.size());
      if(table_prefix == prefix)
      {
        bAddIt = false;
      }

      if(bAddIt)
      {
        Gtk::TreeModel::iterator iter = m_AddDel.add_item(strName);
        fill_table_row(iter, table_info);
      }
    }
  }

  fill_end();

  m_AddDel.set_allow_add(developer_mode);
  m_AddDel.set_allow_delete(developer_mode);

  return result;
}

void Box_Tables::on_adddel_Add(const Gtk::TreeModel::iterator& row)
{
  //TODO: Handle cell renderer changes to prevent illegal table names (e.g. starting with numbers.)"

  Glib::ustring table_name = m_AddDel.get_value(row, m_colTableName);
  if(!table_name.empty())
  {
    //Primary key:
    sharedptr<Field> field_primary_key(new Field());
    field_primary_key->set_name(table_name + "_id");
    field_primary_key->set_title(table_name + " ID");
    field_primary_key->set_primary_key();
    field_primary_key->set_auto_increment();

    Gnome::Gda::FieldAttributes field_info = field_primary_key->get_field_info();
    field_info.set_allow_null(false);
    field_primary_key->set_field_info(field_info);

    field_primary_key->set_glom_type(Field::TYPE_NUMERIC);
    //std::cout << "field_primary_key->get_auto_increment():" << field_primary_key->get_auto_increment() << std::endl;

    type_vecFields fields;
    fields.push_back(field_primary_key);

    //Description:
    sharedptr<Field> field_description(new Field());
    field_description->set_name("description");
    field_description->set_title(_("Description")); //Use a translation, because the original locale will be marked as non-English if the current locale is non-English.
    field_description->set_glom_type(Field::TYPE_TEXT);
    fields.push_back(field_description);

    //Comments:
    sharedptr<Field> field_comments(new Field());
    field_comments->set_name("comments");
    field_comments->set_title(_("Comments"));
    field_comments->set_glom_type(Field::TYPE_TEXT);
    field_comments->m_default_formatting.set_text_format_multiline();
    fields.push_back(field_comments);

    sharedptr<TableInfo> table_info(new TableInfo());
    table_info->set_name(table_name);
    table_info->set_title( Utils::title_from_string( table_name ) ); //Start with a title that might be appropriate.

    //Check whether it exists already. (Maybe it is somehow in the database but not in the document. That shouldn't happen.)
    const bool exists_in_db = get_table_exists_in_database(table_name);
    bool created = false; 
    if(exists_in_db)
    {
      //Ask the user if they want us to try to cope with this:
      Gtk::MessageDialog dialog(Bakery::App_Gtk::util_bold_message(_("Table Already Exists")), true, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL);
      dialog.set_secondary_text(_("This table already exists on the database server, though it is not mentioned in the .glom file. This should not happen. Would you like Glom to attempt to use the existing table?"));
      dialog.set_transient_for(*get_app_window());

      const int response = dialog.run();
      dialog.hide();

      if(response == Gtk::RESPONSE_OK)
      {
        //Maybe Glom will cope with whatever fields are there. Let's see.
        created = true;
      }

      created = true;
    }
    else
    {
      created = create_table(table_info, fields);
    }

    //Create a table with 1 "ID" field:
   //MSYQL:
    //query_execute( "CREATE TABLE \"" + table_name + "\" (" + primary_key_name + " INT NOT NULL AUTO_INCREMENT PRIMARY KEY)" );
    //query_execute( "INSERT INTO \"" + table_name + "\" VALUES (0)" );

    //PostgresSQL:
    //query_execute( "CREATE TABLE " + table_name + " (" + primary_key_name + " serial NOT NULL  PRIMARY KEY)" );

    //query_execute( "CREATE TABLE \"" + table_name + "\" (" +
    //  field_primary_key->get_name() + " numeric NOT NULL  PRIMARY KEY," + 
    //  extra_field_description + "varchar, " +
    //  extra_field_comments + "varchar" +
    //  ")" );

    if(created)
    {
      //Show the new information for this whole row:
      fill_table_row(row, table_info);

      //Save the field information directly into the database, because we cannot get all the correct information from the database.
      //Otherwise some information would be forgotten:



      Document_Glom* document = get_document();
      if(document)
          document->set_table_fields(table_name, fields);

      save_to_document();
      //fill_from_database(); //We should not modify the model structure in a cellrenderer signal handler.
    }
  }
}

void Box_Tables::on_adddel_Delete(const Gtk::TreeModel::iterator& rowStart, const Gtk::TreeModel::iterator& rowEnd)
{
  Gtk::TreeModel::iterator iterAfter = rowEnd;
  ++iterAfter;

  bool something_changed = false;
  for(Gtk::TreeModel::iterator iter = rowStart; iter != iterAfter; ++iter)
  {
    Glib::ustring table_name = m_AddDel.get_value_key(iter);

    if(!table_name.empty())
    {
      Document_Glom* document = get_document();
      if(document)
      {
        //Don't open a table that the document does not know about, because we need information from the document:
        if(!document->get_table_is_known(table_name))
        {
           //TODO: Do not show tables that are not in the document.
           Gtk::MessageDialog dialog(_("You cannot delete this table, because there is no information about this table in the document."));
           dialog.set_transient_for(*get_app_window());
           dialog.run();
        }
        else
        {
          //Ask the user to confirm:
          Glib::ustring strMsg = _("Are you sure that you want to delete this table?\nTable name: ") + table_name;
          Gtk::MessageDialog dialog(Bakery::App_Gtk::util_bold_message(_("Delete Table")), true);
          dialog.set_secondary_text(strMsg);
          dialog.set_transient_for(*get_app_window());
          int iButtonClicked = dialog.run();

          //Delete the table:
          if(iButtonClicked == Gtk::RESPONSE_OK)
          {
            query_execute( "DROP TABLE " + table_name, get_app_window());
            get_document()->remove_table(table_name); //Forget about it in the document too.
            something_changed = true;
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

void Box_Tables::on_adddel_Edit(const Gtk::TreeModel::iterator& row)
{
  Glib::ustring table_name = m_AddDel.get_value_key(row);

  Document_Glom* document = get_document();
  if(document)
  {
    //Don't open a table that the document does not know about, because we need information from the document:
    //This should never happen, because we never show them in the list:
    if(!document->get_table_is_known(table_name))
    {
       //TODO: Do not show tables that are not in the document.
       Gtk::MessageDialog dialog(Bakery::App_Gtk::util_bold_message(_("Unknown Table")), true);
       dialog.set_secondary_text(_("You cannot open this table, because there is no information about this table in the document."));
       dialog.set_transient_for(*get_app_window());
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

void Box_Tables::save_to_document()
{
  if(get_userlevel() == AppState::USERLEVEL_DEVELOPER)
  {
    //Save the hidden tables. TODO_usermode: Only if we are in developer mode.
    Document_Glom::type_listTableInfo listTables;

    Document_Glom* document = get_document();

    for(Gtk::TreeModel::iterator iter = m_AddDel.get_model()->children().begin(); iter != m_AddDel.get_model()->children().end(); ++iter)
    {
      const Glib::ustring table_name = m_AddDel.get_value(iter, m_colTableName); //The name has already been changed in the document.
      sharedptr<TableInfo> table_info = document->get_table(table_name); //Start with the existing table_info, to preserve extra information, such as translations.
      if(table_info)
      {
        table_info->set_name( m_AddDel.get_value(iter, m_colTableName) );

        if(!table_info->get_name().empty())
        {
          table_info->m_hidden  = m_AddDel.get_value_as_bool(iter, m_colHidden);
          table_info->set_title( m_AddDel.get_value(iter, m_colTitle) ); //TODO_Translations: Store the TableInfo in the TreeView.
          //std::cout << "save_to_document(): title=" << table_info->get_title() << std::endl;
          table_info->m_default  = m_AddDel.get_value_as_bool(iter, m_colDefault);

          listTables.push_back(table_info);
        }
      }
    }

    if(document)
      document->set_tables( listTables); //TODO: Don't save all new tables - just the ones already in the document.
  }
}

void Box_Tables::on_show_hidden_toggled()
{
  fill_from_database();
}

void Box_Tables::on_adddel_changed(const Gtk::TreeModel::iterator& row, guint column)
{
  if(get_userlevel() == AppState::USERLEVEL_DEVELOPER)
  {
    if( (column == m_colHidden) )
    {
      save_to_document();
      //TODO: This causes a crash. fill_from_database(); //Hide/show the table.
    }
    else if( (column == m_colTitle) || (column == m_colDefault) )
    {
      save_to_document();
    }
    else if(column == m_colTableName)
    {
      Glib::ustring table_name = m_AddDel.get_value_key(row);
      Glib::ustring table_name_new = m_AddDel.get_value(row, m_colTableName);
      if(!table_name.empty() && !table_name_new.empty())
      {
        Glib::ustring strMsg = _("Are you sure that you want to rename this table?");  //TODO: Show old and new names?
        Gtk::MessageDialog dialog(_("Rename Table"));
        dialog.set_secondary_text(strMsg);
        int iButtonClicked = dialog.run();

        //Rename the table:
        if(iButtonClicked == Gtk::RESPONSE_OK)
        {
          const bool test = query_execute( "ALTER TABLE \"" + table_name + "\" RENAME TO \"" + table_name_new + "\"", get_app_window());
          if(test)
          {
            //Change the AddDel item's key:
            m_AddDel.set_value_key(row, table_name_new);

            set_modified();

            //Tell the document that this table's name has changed:
            Document_Glom* document = get_document();
            if(document)
              document->change_table_name(table_name, table_name_new);

            //fill_from_database(); //We should not modify the model structure in a cellrenderer signal handler.
          }
        }
      }
    }
  }
}

void Box_Tables::on_userlevel_changed(AppState::userlevels /* userlevel */)
{
  fill_from_database();
}

} //namespace Glom
