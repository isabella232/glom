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

#include "dialog_choose_field.h"
//#include <libgnome/gnome-i18n.h>
#include <libintl.h>

Dialog_ChooseField::Dialog_ChooseField(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::Dialog(cobject),
  m_combo_relationship(0),
  m_button_select(0),
  m_treeview(0),
  m_document(0)
{
  refGlade->get_widget("button_select", m_button_select);
  refGlade->get_widget_derived("combobox_relationship", m_combo_relationship);
  m_combo_relationship->signal_changed().connect(sigc::mem_fun(*this, &Dialog_ChooseField::on_combo_relationship_changed));

  refGlade->get_widget("treeview_fields", m_treeview);

  if(m_treeview)
  {
    m_model = Gtk::ListStore::create(m_ColumnsFields);
    m_treeview->set_model(m_model);

    m_treeview->append_column( gettext("Name"), m_ColumnsFields.m_col_name );
    m_treeview->append_column( gettext("Title"), m_ColumnsFields.m_col_title );

    m_treeview->signal_row_activated().connect( sigc::mem_fun(*this, &Dialog_ChooseField::on_row_activated) );
  }

  show_all_children();
}

Dialog_ChooseField::~Dialog_ChooseField()
{
}

void Dialog_ChooseField::set_document(Document_Glom* document, const Glib::ustring& table_name,  const LayoutItem_Field& field)
{
  set_document(document, table_name);

  //Select the current relationship at the start:

  if(field.get_has_relationship_name())
  {
    m_combo_relationship->set_active_text( field.get_relationship_name() );
  }
  else
    m_combo_relationship->set_active_text(table_name);

  //Select the current field at the start:
  const Glib::ustring field_name = field.get_name();

  //Get the iterator for the row:
  Gtk::TreeModel::iterator iterFound = m_model->children().end();
  for(Gtk::TreeModel::iterator iter = m_model->children().begin(); iter != m_model->children().end(); ++iter)
  {
    if(field_name == (*iter)[m_ColumnsFields.m_col_name])
    {
      iterFound = iter;
      break;
    }
  }

  if(iterFound != m_model->children().end())
  {
    m_treeview->get_selection()->select(iterFound);
  }
}

void Dialog_ChooseField::set_document(Document_Glom* document, const Glib::ustring& table_name)
{
  m_document = document;
  m_table_name = table_name;
 
  if(!m_document)
  {
    g_warning("Dialog_ChooseField::set_document(): document is null");
  }

  if(table_name.empty())
  {
    g_warning("Dialog_ChooseField::set_document(): table_name is empty");
  }

  //Update the tree models from the document
  if(document)
  {
    //Fill the list of relationships:
    m_combo_relationship->clear_text();
    Document_Glom::type_vecRelationships vecRelationships = document->get_relationships(table_name);

    //Add a special option for the current table:
    m_combo_relationship->append_text(table_name);

    //Add the relationships for this table:
    if(!vecRelationships.empty())
      m_combo_relationship->append_separator();

    for(Document_Glom::type_vecRelationships::iterator iter = vecRelationships.begin(); iter != vecRelationships.end(); ++iter)
    {
      m_combo_relationship->append_text(iter->get_name());
    }

    //Set the table name and title:
    m_combo_relationship->set_active_text(table_name);

    //Fill the treeview:
    m_model->clear();
    Document_Glom::type_vecFields vecFields = document->get_table_fields(table_name);
    for(Document_Glom::type_vecFields::const_iterator iter = vecFields.begin(); iter != vecFields.end(); ++iter)
    {
      Gtk::TreeModel::iterator iterRow = m_model->append();
      Gtk::TreeModel::Row row = *iterRow;
      row[m_ColumnsFields.m_col_name] = iter->get_name();
      row[m_ColumnsFields.m_col_title] = iter->get_title();
      row[m_ColumnsFields.m_col_field] = *iter;
    }
  }
}

/*
void Dialog_ChooseField::select_item(const Field& field)
{
  //TODO: We do this in set_document() as well.

  //Find any items with the same name:
  for(Gtk::TreeModel::iterator iter = m_model->children().begin(); iter != m_model->children().end(); ++iter)
  {
    Gtk::TreeModel::Row row = *iter;
    const Field& field_item = row[m_ColumnsFields.m_col_field];
    if(field_item.get_name() == field.get_name())
    {
      //Select the item:
      Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_treeview->get_selection();
      if(refTreeSelection)
        refTreeSelection->select(iter);
    }
  }
}
*/

bool Dialog_ChooseField::get_field_chosen(LayoutItem_Field& field) const
{
  //Don't initialize output argument,
  //because it might contain other useful data.
  //TODO: Store a LayoutItem_Field as a member.


  //Relationship:
  Glib::ustring relationship_name = m_combo_relationship->get_active_text();
  if(relationship_name == m_table_name)
    relationship_name.clear();

  field.set_relationship_name(relationship_name);

  //Field:
  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_treeview->get_selection();
  if(refTreeSelection)
  {
    Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;
      field.set_name(row[m_ColumnsFields.m_col_name]);
      field.m_field = row[m_ColumnsFields.m_col_field];
      return true;
    }
  }

  return false;
}

void Dialog_ChooseField::on_row_activated(const Gtk::TreePath& /* path */, Gtk::TreeViewColumn* /* view_column */)
{
  response(Gtk::RESPONSE_OK);
}

void Dialog_ChooseField::on_combo_relationship_changed()
{
  Glib::ustring relationship_name = m_combo_relationship->get_active_text();

  Document_Glom* pDocument = m_document;
  if(pDocument)
  {
    //Show the list of fields from this relationship:

    Document_Glom::type_vecFields vecFields;
    if(relationship_name == m_table_name)
      vecFields = pDocument->get_table_fields(m_table_name);
    else
    {
      Relationship relationship;
      pDocument->get_relationship(m_table_name, relationship_name, relationship);

      vecFields = pDocument->get_table_fields(relationship.get_to_table());
    }

    m_model->clear();
    for(Document_Glom::type_vecFields::const_iterator iter = vecFields.begin(); iter != vecFields.end(); ++iter)
    {
      Gtk::TreeModel::iterator iterRow = m_model->append();
      Gtk::TreeModel::Row row = *iterRow;
      row[m_ColumnsFields.m_col_name] = iter->get_name();
      row[m_ColumnsFields.m_col_title] = iter->get_title();
      row[m_ColumnsFields.m_col_field] = *iter;
    }

  }
}
