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

#include "box_data_list_related.h"
#include "dialog_layout_list_related.h"
#include "../data_structure/glomconversions.h"
#include <bakery/App/App_Gtk.h> //For util_bold_message().
#include <glibmm/i18n.h>

Box_Data_List_Related::Box_Data_List_Related()
: m_pDialogLayoutRelated(0)
{
  set_size_request(400, -1); //An arbitrary default.

  //m_Frame.set_label_widget(m_Label_Related);
  m_Frame.set_shadow_type(Gtk::SHADOW_NONE);

  m_Frame.add(m_Alignment);
  m_Frame.show();

  m_Frame.set_label_widget(m_Label);
  m_Label.show();

  m_Alignment.set_padding(6 /* top */, 0, 12 /* left */, 0);
  m_Alignment.show();

  remove(m_AddDel);
  m_Alignment.add(m_AddDel);
  m_AddDel.show();
  add(m_Frame);

  m_layout_name = "list_related"; //TODO: We need a unique name when 2 portals use the same table.

  //Delete the dialog from the base class, because we don't use it.
  if(m_pDialogLayout)
    delete m_pDialogLayout;

  Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "window_data_layout_list_related");
  if(refXml)
  {
    Dialog_Layout_List_Related* dialog = 0;
    refXml->get_widget_derived("window_data_layout_list_related", dialog);
    if(dialog)
    {
      //Use the new dialog:
      m_pDialogLayoutRelated = dialog;
      m_pDialogLayoutRelated->signal_hide().connect( sigc::mem_fun(*this, &Box_Data::on_dialog_layout_hide) );
    }
  }

}

Box_Data_List_Related::~Box_Data_List_Related()
{
  if(m_pDialogLayoutRelated)
  {
    delete m_pDialogLayoutRelated;
    m_pDialogLayoutRelated = 0;
  }
}

void Box_Data_List_Related::init_db_details(const Relationship& relationship)
{
  m_relationship = relationship;

  m_Label.set_markup(Bakery::App_Gtk::util_bold_message( relationship.get_title_or_name() ));

  bool found = get_fields_for_table_one_field(relationship.get_to_table(), relationship.get_to_field(), m_key_field /* output parameter */);
  if(!found)
  {
    g_warning("Box_Data_List_Related::init_db_details(): key_field not found.");
  }

  Box_Data_List::init_db_details(relationship.get_to_table());
}

void Box_Data_List_Related::refresh_db_details(const Gnome::Gda::Value& foreign_key_value, const Gnome::Gda::Value& /* from_table_primary_key_value */)
{
  m_key_value = foreign_key_value;


  if(!GlomConversions::value_is_empty(m_key_value))
  {
    Glib::ustring strWhereClause = m_key_field.get_name() + " = " + m_key_field.sql(m_key_value);

    Box_Data_List::refresh_db_details(strWhereClause);
  }
  else
  {
    g_warning("Box_Data_List_Related::refresh_db_details(): m_key_value is NULL.");
  }
  //TODO: Clear the list if there is no key value?
}
 

void Box_Data_List_Related::fill_from_database()
{
  bool allow_add = true;

  if(!m_strWhereClause.empty())
  {
    Box_Data_List::fill_from_database();


    //Is there already one record here?
    if(m_has_one_or_more_records) //This was set by Box_Data_List::fill_from_database().
    {
      //Is the to_field unique? If so, there can not be more than one.
      if(m_key_field.get_field_info().get_unique_key()) //automatically true if it is a primary key
        allow_add = false;
    }

    //TODO: Disable add if the from_field already has a value and the to_field is auto-incrementing because
    //- we can not override the auto-increment in the to_field.
    //- we can not change the value in the from_field to the new auto_increment value in the to_field.
  }
  else
  {
    //No Foreign Key value, so just show the field names:

    Box_DB_Table::fill_from_database();

    fill_column_titles();

    fill_end();
  }

  //Prevent addition of new records if that is what the relationship specifies:
  if(allow_add)
    allow_add = m_relationship.get_auto_create();

  m_AddDel.set_allow_add(allow_add);
}

void Box_Data_List_Related::on_record_added(const Gnome::Gda::Value& primary_key_value)
{
  //primary_key_value is a new autogenerated or human-entered key for the row.
  //It has already been added to the database.

  //Get row of new record:
  Gtk::TreeModel::iterator iter = m_AddDel.get_row(primary_key_value);
  if(iter)
  {
    Gnome::Gda::Value key_value;
    //m_key_field is the field in this table that must match another field in the parent table.
    LayoutItem_Field layout_item;
    layout_item.m_field = m_key_field;
    key_value = m_AddDel.get_value(iter, layout_item);

    Box_Data_List::on_record_added(key_value); //adds blank row.

    //Make sure that the new related record is related,
    //by setting the foreign key:
    //If it's not auto-generated.
    if(!GlomConversions::value_is_empty(key_value)) //If there is already a value.
    {
      //It was auto-generated. Tell the parent about it, so it can make a link.
      signal_record_added.emit(key_value);
    }
    else if(GlomConversions::value_is_empty(m_key_value))
    {
      g_warning("Box_Data_List_Related::on_record_added(): m_key_value is NULL.");
    }
    else
    {
      Field field_primary_key = m_AddDel.get_key_field();

      //Create the link by setting the foreign key
      Glib::ustring strQuery = "UPDATE " + m_relationship.get_to_table();
      strQuery += " SET " +  /* get_table_name() + "." +*/ m_key_field.get_name() + " = " + m_key_field.sql(m_key_value);
      strQuery += " WHERE " + get_table_name() + "." + field_primary_key.get_name() + " = " + field_primary_key.sql(primary_key_value);
      bool test = Query_execute(strQuery);
      if(test)
      {
        //Show it on the view, if it's visible:
        LayoutItem_Field layout_item;
        layout_item.m_field = field_primary_key;

        m_AddDel.set_value(iter, layout_item, m_key_value);
      }

      //on_adddel_user_changed(iter, iKey); //Update the database.
    }
  }
}

Relationship Box_Data_List_Related::get_relationship() const
{
  return m_relationship;
}
 
Field Box_Data_List_Related::get_key_field() const
{
  return m_key_field;
}

void Box_Data_List_Related::on_adddel_user_added(const Gtk::TreeModel::iterator& row, guint col_with_first_value)
{
  //Like Box_Data_List::on_adddel_user_added(),
  //but it doesn't allow adding if the new record can not be a related record.
  //This would happen if there is already one related record and the relationship uses the primary key in the related record.

  bool bAllowAdd = true;

  const Gnome::Gda::FieldAttributes fieldInfo = m_key_field.get_field_info();
  if(fieldInfo.get_unique_key() || fieldInfo.get_primary_key())
  {
    if(m_AddDel.get_count() > 0) //If there is already 1 record
      bAllowAdd = false;
  }

  if(bAllowAdd)
  {
    Box_Data_List::on_adddel_user_added(row, col_with_first_value);
  }
  else
  {
    //Tell user that they can't do that:
    Gtk::MessageDialog dialog(Bakery::App_Gtk::util_bold_message(_("Extra related records not possible.")), true, Gtk::MESSAGE_WARNING);
    dialog.set_secondary_text(_("You attempted to add a new related record, but there can only be one related record, because the relationship uses a unique key.")),
    dialog.set_transient_for(*get_app_window());
    dialog.run();

    //Replace with correct values:
    fill_from_database();
  }
}

void Box_Data_List_Related::enable_buttons()
{
}

Box_Data_List_Related::type_vecLayoutFields Box_Data_List_Related::get_fields_to_show() const
{
  const Document_Glom* document = get_document();
  if(document)
  {
    Document_Glom::type_mapLayoutGroupSequence mapGroups = document->get_relationship_data_layout_groups_plus_new_fields(m_layout_name, m_relationship);
    return get_table_fields_to_show(m_relationship.get_to_table(), mapGroups);
  }

  return type_vecLayoutFields();
}

void Box_Data_List_Related::show_layout_dialog()
{
  if(m_pDialogLayoutRelated)
  {
    m_pDialogLayoutRelated->set_document(m_layout_name, get_document(), m_relationship);
    m_pDialogLayoutRelated->show();
  }
}

void Box_Data_List_Related::on_dialog_layout_hide()
{
  const Glib::ustring relationship_name = m_pDialogLayoutRelated->get_relationship_name();
  if(!relationship_name.empty())
  {
    //Get the new relationship information, in case it has changed:
    Document_Glom* document = get_document();
    if(document)
    {
      document->get_relationship(m_relationship.get_from_table(), m_relationship.get_name(), m_relationship);

      //Update the UI:
      init_db_details(m_relationship);
    }

    Box_Data::on_dialog_layout_hide();

    LayoutItem_Portal* pLayoutItem = dynamic_cast<LayoutItem_Portal*>(get_layout_item());
    if(pLayoutItem)
    {
      pLayoutItem->set_relationship(relationship_name);
      signal_layout_changed().emit(); //TODO: Check whether it has really changed.
    }
  }
}

