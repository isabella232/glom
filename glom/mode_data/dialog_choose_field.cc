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
#include <glibmm/i18n.h>

namespace Glom
{

Dialog_ChooseField::Dialog_ChooseField(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Dialog(cobject),
  m_combo_relationship(0),
  m_button_select(0),
  m_checkbutton_show_related_relationships(0),
  m_treeview(0),
  m_document(0)
{
  builder->get_widget("checkbutton_show_related_relationships", m_checkbutton_show_related_relationships);
  m_checkbutton_show_related_relationships->set_active(false); //Start with the simpler list, to avoid confusing people.
  m_checkbutton_show_related_relationships->signal_toggled().connect(sigc::mem_fun(*this, &Dialog_ChooseField::on_checkbutton_related_relationships_toggled));

  builder->get_widget("button_select", m_button_select);
  builder->get_widget_derived("combobox_relationship", m_combo_relationship);
  m_combo_relationship->signal_changed().connect(sigc::mem_fun(*this, &Dialog_ChooseField::on_combo_relationship_changed));

  builder->get_widget("treeview_fields", m_treeview);

  if(m_treeview)
  {
    m_model = Gtk::ListStore::create(m_ColumnsFields);
    m_treeview->set_model(m_model);

    m_treeview->append_column( _("Name"), m_ColumnsFields.m_col_name );
    m_treeview->append_column( _("Title"), m_ColumnsFields.m_col_title );
    m_model->set_sort_column(m_ColumnsFields.m_col_name, Gtk::SORT_ASCENDING);

    m_treeview->signal_row_activated().connect( sigc::mem_fun(*this, &Dialog_ChooseField::on_row_activated) );

    Glib::RefPtr<Gtk::TreeView::Selection> refSelection = m_treeview->get_selection();
    if(refSelection)
    {
      refSelection->signal_changed().connect( sigc::mem_fun(*this, &Dialog_ChooseField::on_treeview_selection_changed) );
    }
  }

  show_all_children();
}

Dialog_ChooseField::~Dialog_ChooseField()
{
}

void Dialog_ChooseField::set_document(Document_Glom* document, const Glib::ustring& table_name, const sharedptr<const LayoutItem_Field>& field)
{
  set_document(document, table_name);

  m_start_field = glom_sharedptr_clone(field);

  //Select the current relationship at the start:
  if(field && field->get_has_relationship_name())
  {
    if(field->get_has_related_relationship_name())
      m_checkbutton_show_related_relationships->set_active(true); //We'll need the full list of relationships to show this.

    m_combo_relationship->set_selected_relationship( field->get_relationship(), field->get_related_relationship());
  }
  else
    m_combo_relationship->set_selected_parent_table(table_name, document->get_table_title(table_name)); 

  //Select the current field at the start:
  if(field)
  {
    const Glib::ustring field_name = field->get_name();

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

      //Make sure that it is scrolled into view:
      m_treeview->scroll_to_row(Gtk::TreeModel::Path(iterFound));
    }
  }
}

void Dialog_ChooseField::set_document(Document_Glom* document, const Glib::ustring& table_name)
{
  m_document = document;
  m_table_name = table_name;
  m_start_field.clear();

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

    //Add a special option for the current table:
    m_combo_relationship->set_display_parent_table(table_name, document->get_table_title(table_name));

    //Add the relationships for this table:
    const Glib::ustring table_title = document->get_table_title(table_name);
    m_combo_relationship->set_relationships(document, table_name, false /* show related relationships */);

    //Set the table name and title:
    m_combo_relationship->set_selected_parent_table(table_name, table_title);

    //Fill the treeview:
    m_model->clear();
    Document_Glom::type_vecFields vecFields = document->get_table_fields(table_name);
    for(Document_Glom::type_vecFields::const_iterator iter = vecFields.begin(); iter != vecFields.end(); ++iter)
    {
      Gtk::TreeModel::iterator iterRow = m_model->append();
      Gtk::TreeModel::Row row = *iterRow;

      sharedptr<Field> field = *iter;
      row[m_ColumnsFields.m_col_name] = field->get_name();
      row[m_ColumnsFields.m_col_title] = field->get_title();
      row[m_ColumnsFields.m_col_field] = field;
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

sharedptr<LayoutItem_Field> Dialog_ChooseField::get_field_chosen() const
{
  sharedptr<LayoutItem_Field> field;

  //Field:
  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_treeview->get_selection();
  if(refTreeSelection)
  {
    Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
    if(iter)
    {
      if(m_start_field)
        field = m_start_field; //Start with this, to preserve extra information such as Translations.
      else
        field = sharedptr<LayoutItem_Field>::create();

      //Relationship:
      //Note that a null relationship means that the parent table was selected instead.
      sharedptr<Relationship> related_relationship;
      sharedptr<Relationship> relationship = m_combo_relationship->get_selected_relationship(related_relationship);

      //field.set_relationship_name(relationship_name);

      field->set_relationship(relationship);
      field->set_related_relationship(related_relationship);


      Gtk::TreeModel::Row row = *iter;
      sharedptr<Field> field_details = row[m_ColumnsFields.m_col_field];
      field->set_full_field_details(field_details);
      field->set_name(row[m_ColumnsFields.m_col_name]);
    }
  }

  return field;
}

void Dialog_ChooseField::on_row_activated(const Gtk::TreeModel::Path& /* path */, Gtk::TreeViewColumn* /* view_column */)
{
  response(Gtk::RESPONSE_OK);
}

void Dialog_ChooseField::on_checkbutton_related_relationships_toggled()
{
  if(!m_document)
    return;

  const bool show_related_relationships = m_checkbutton_show_related_relationships->get_active();

  //Preserve the selection:
  sharedptr<Relationship> related_relationship;
  sharedptr<Relationship> relationship = m_combo_relationship->get_selected_relationship(related_relationship);

  //Refresh the list, hiding or showing the child relationships:
  m_combo_relationship->set_relationships(m_document, m_table_name, show_related_relationships);

  m_combo_relationship->set_selected_relationship(relationship, related_relationship);
}

void Dialog_ChooseField::on_combo_relationship_changed()
{
  sharedptr<Relationship> relationship = m_combo_relationship->get_selected_relationship();

  Document_Glom* pDocument = m_document;
  if(pDocument)
  {
    //Show the list of fields from this relationship:

    Document_Glom::type_vecFields vecFields;
    if(!relationship)
      vecFields = pDocument->get_table_fields(m_table_name);
    else
    {
      vecFields = pDocument->get_table_fields(relationship->get_to_table());
    }

    m_model->clear();
    for(Document_Glom::type_vecFields::const_iterator iter = vecFields.begin(); iter != vecFields.end(); ++iter)
    {
      Gtk::TreeModel::iterator iterRow = m_model->append();
      Gtk::TreeModel::Row row = *iterRow;

      sharedptr<Field> field = *iter;
      row[m_ColumnsFields.m_col_name] = field->get_name();
      row[m_ColumnsFields.m_col_title] = field->get_title();
      row[m_ColumnsFields.m_col_field] = field;
    }

  }
}

void Dialog_ChooseField::on_treeview_selection_changed()
{
  Glib::RefPtr<Gtk::TreeView::Selection> refSelection = m_treeview->get_selection();
  if(refSelection)
  {
    Gtk::TreeModel::iterator iter = refSelection->get_selected();
    if(iter)
    {
      /*
      Gtk::TreeModel::Row row = *iter;
      const Field& field = row[m_ColumnsFields.m_col_field];
      const bool is_numeric = (field.get_glom_type() == Field::TYPE_NUMERIC);
      if(is_numeric)
        m_vbox_numeric_format->show();
      else
        m_vbox_numeric_format->hide();
      */
    }
  }
}

} //namespace Glom
