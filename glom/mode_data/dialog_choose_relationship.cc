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

#include "dialog_choose_relationship.h"
//#include <libgnome/gnome-i18n.h>
#include <glibmm/i18n.h>

Dialog_ChooseRelationship::Dialog_ChooseRelationship()
: m_label_table_name(0),
  m_button_select(0),
  m_treeview(0),
  m_document(0)
{
}

Dialog_ChooseRelationship::Dialog_ChooseRelationship(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::Dialog(cobject),
  m_label_table_name(0),
  m_button_select(0),
  m_treeview(0),
  m_document(0)
{
  refGlade->get_widget("button_select", m_button_select);
  refGlade->get_widget("label_table_name", m_label_table_name);
  refGlade->get_widget("treeview_relationships", m_treeview);

  if(m_treeview)
  {
    m_model = Gtk::ListStore::create(m_ColumnsRelationships);
    m_treeview->set_model(m_model);

    m_treeview->append_column( _("Name"), m_ColumnsRelationships.m_col_name );
    //m_treeview->append_column( _("Title"), m_ColumnsRelationships.m_col_title );
  }

  show_all_children();
}

Dialog_ChooseRelationship::~Dialog_ChooseRelationship()
{
}

void Dialog_ChooseRelationship::set_document(Document_Glom* document, const Glib::ustring& table_name)
{
  m_document = document;
  m_table_name = table_name;

  //Update the tree models from the document
  if(document)
  {
    //Set the table name and title:
    m_label_table_name->set_text(table_name); //TODO: Show table title here too.

    //Fill the treeview:
    m_model->clear();
    Document_Glom::type_vecRelationships vecRelationships = document->get_relationships(table_name);
    for(Document_Glom::type_vecRelationships::const_iterator iter = vecRelationships.begin(); iter != vecRelationships.end(); ++iter)
    {
      Gtk::TreeModel::iterator iterRow = m_model->append();
      Gtk::TreeModel::Row row = *iterRow;
      row[m_ColumnsRelationships.m_col_name] = iter->get_name();
      //row[m_ColumnsRelationships.m_col_title] = iter->get_title();
      row[m_ColumnsRelationships.m_col_relationship] = *iter;
    }
  }
}

void Dialog_ChooseRelationship::select_item(const Relationship& relationship)
{
  //Find any items with the same name:
  for(Gtk::TreeModel::iterator iter = m_model->children().begin(); iter != m_model->children().end(); ++iter)
  {
    Gtk::TreeModel::Row row = *iter;
    const Relationship& relationship_item = row[m_ColumnsRelationships.m_col_relationship];
    if(relationship_item.get_name() == relationship.get_name())
    {
      //Select the item:
      Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_treeview->get_selection();
      if(refTreeSelection)
        refTreeSelection->select(iter);
    }
  }
}

bool Dialog_ChooseRelationship::get_relationship_chosen(Relationship& relationship) const
{
  //Initialize output argument:
  relationship = Relationship();

  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_treeview->get_selection();
  if(refTreeSelection)
  {
    Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;
      relationship =  row[m_ColumnsRelationships.m_col_relationship];
      return true;
    }
  }
    
  return false;
}
