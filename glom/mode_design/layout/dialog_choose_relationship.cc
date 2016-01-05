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

#include "dialog_choose_relationship.h"
//#include <libgnome/gnome-i18n.h>
#include <glibmm/i18n.h>

namespace Glom
{

const char* Dialog_ChooseRelationship::glade_id("dialog_choose_relationship");
const bool Dialog_ChooseRelationship::glade_developer(true);

Dialog_ChooseRelationship::Dialog_ChooseRelationship()
: m_label_table_name(nullptr),
  m_button_select(nullptr),
  m_treeview(nullptr),
  m_document(nullptr)
{
}

Dialog_ChooseRelationship::Dialog_ChooseRelationship(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Dialog(cobject),
  m_label_table_name(nullptr),
  m_button_select(nullptr),
  m_treeview(nullptr),
  m_document(nullptr)
{
  builder->get_widget("button_select", m_button_select);
  builder->get_widget("label_table_name", m_label_table_name);
  builder->get_widget("treeview_relationships", m_treeview);

  if(m_treeview)
  {
    m_model = Gtk::ListStore::create(m_ColumnsRelationships);
    m_treeview->set_model(m_model);

    m_treeview->append_column( _("Name"), m_ColumnsRelationships.m_col_name );
    //m_treeview->append_column( _("Title"), m_ColumnsRelationships.m_col_title );
  }

  show_all_children();
}

void Dialog_ChooseRelationship::set_document(Document* document, const Glib::ustring& table_name)
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
    for(const auto& relationship : document->get_relationships(table_name))
    {
      auto iterRow = m_model->append();
      Gtk::TreeModel::Row row = *iterRow;

      if(relationship)
        row[m_ColumnsRelationships.m_col_name] = glom_get_sharedptr_name(relationship);

      //row[m_ColumnsRelationships.m_col_title] = item_get_title(iter);
      row[m_ColumnsRelationships.m_col_relationship] = relationship;
    }
  }
}

void Dialog_ChooseRelationship::select_item(const std::shared_ptr<const Relationship>& relationship)
{
  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_treeview->get_selection();
  if(!refTreeSelection)
    return; //Should never happen.

  if(!relationship)
    refTreeSelection->unselect_all();
  else
  {
    //Find any items with the same name:
    for(const auto& row : m_model->children())
    {
      const auto relationship_name = glom_get_sharedptr_name(relationship);

      std::shared_ptr<Relationship> relationship_item = row[m_ColumnsRelationships.m_col_relationship];
      if(glom_get_sharedptr_name(relationship_item) == relationship_name)
      {
        //Select the item:
        refTreeSelection->select(row);
      }
    }
  }
}

std::shared_ptr<Relationship> Dialog_ChooseRelationship::get_relationship_chosen() const
{
  std::shared_ptr<Relationship> result;

  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_treeview->get_selection();
  if(refTreeSelection)
  {
    auto iter = refTreeSelection->get_selected();
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;
      result =  row[m_ColumnsRelationships.m_col_relationship];
    }
  }

  return result;
}

} //namespace Glom
