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

#include "dialog_fieldslist.h"
#include "dialog_field_layout.h"

//#include <libgnome/gnome-i18n.h>
#include <glibmm/i18n.h>

namespace Glom
{

const char* Dialog_FieldsList::glade_id("dialog_fieldslist");
const bool Dialog_FieldsList::glade_developer(true);

Dialog_FieldsList::Dialog_FieldsList(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Dialog_Layout(cobject, builder, false /* means no table title */),
  m_treeview_fields(0),
  m_button_field_up(0),
  m_button_field_down(0),
  m_button_field_add(0),
  m_button_field_delete(0),
  m_button_field_edit(0),
  m_button_field_formatting(0),
  m_label_table_name(0)
{
  builder->get_widget("label_table_name", m_label_table_name);

  builder->get_widget("treeview_fields", m_treeview_fields);
  if(m_treeview_fields)
  {
    m_model_fields = Gtk::ListStore::create(m_ColumnsFields);
    m_treeview_fields->set_model(m_model_fields);

    // Append the View columns:
    Gtk::TreeView::Column* column_name = Gtk::manage( new Gtk::TreeView::Column(_("Name")) );
    m_treeview_fields->append_column(*column_name);

    Gtk::CellRendererText* renderer_name = Gtk::manage(new Gtk::CellRendererText);
    column_name->pack_start(*renderer_name);
    column_name->set_cell_data_func(*renderer_name, sigc::mem_fun(*this, &Dialog_FieldsList::on_cell_data_name));


    //Sort by sequence, so we can change the order by changing the values in the hidden sequence column.
    m_model_fields->set_sort_column(m_ColumnsFields.m_col_sequence, Gtk::SORT_ASCENDING);


    //Respond to changes of selection:
    Glib::RefPtr<Gtk::TreeView::Selection> refSelection = m_treeview_fields->get_selection();
    if(refSelection)
    {
      refSelection->signal_changed().connect( sigc::mem_fun(*this, &Dialog_FieldsList::on_treeview_fields_selection_changed) );
    }

    m_model_fields->signal_row_changed().connect( sigc::mem_fun(*this, &Dialog_FieldsList::on_treemodel_row_changed) );
  }


  builder->get_widget("button_field_up", m_button_field_up);
  m_button_field_up->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_FieldsList::on_button_field_up) );

  builder->get_widget("button_field_down", m_button_field_down);
  m_button_field_down->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_FieldsList::on_button_field_down) );

  builder->get_widget("button_field_delete", m_button_field_delete);
  m_button_field_delete->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_FieldsList::on_button_delete) );

  builder->get_widget("button_field_add", m_button_field_add);
  m_button_field_add->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_FieldsList::on_button_add_field) );

  builder->get_widget("button_field_edit", m_button_field_edit);
  m_button_field_edit->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_FieldsList::on_button_edit_field) );

  builder->get_widget("button_field_formatting", m_button_field_formatting);
  m_button_field_formatting->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_FieldsList::on_button_formatting) );

  show_all_children();
}

Dialog_FieldsList::~Dialog_FieldsList()
{
}

void Dialog_FieldsList::set_fields(const Glib::ustring& table_name, const LayoutGroup::type_list_items& fields)
{
  m_modified = false;
  m_table_name = table_name;

  Document* document = get_document();

  //Update the tree models from the document
  if(document)
  {
    //Set the table name and title:
    m_label_table_name->set_text(table_name);


    //Show the field layout:
    m_model_fields->clear();
    guint field_sequence = 0;
    for(LayoutGroup::type_list_items::const_iterator iter = fields.begin(); iter != fields.end(); ++iter)
    {
      std::shared_ptr<const LayoutItem_Field> item = std::dynamic_pointer_cast<const LayoutItem_Field>(*iter);
      if(!item)
        continue;

      Gtk::TreeModel::iterator iterTree = m_model_fields->append();
      Gtk::TreeModel::Row row = *iterTree;

      row[m_ColumnsFields.m_col_layout_item] = item;
      row[m_ColumnsFields.m_col_sequence] = field_sequence;
      ++field_sequence;
    }

    //treeview_fill_sequences(m_model_fields, m_ColumnsFields.m_col_sequence); //The document should have checked this already, but it does not hurt to check again.
  }

  m_modified = false;
}

void Dialog_FieldsList::enable_buttons()
{
  //Fields:
  Glib::RefPtr<Gtk::TreeView::Selection> refSelection = m_treeview_fields->get_selection();
  if(refSelection)
  {
    Gtk::TreeModel::iterator iter = refSelection->get_selected();
    if(iter)
    {
      //Disable Up if It can't go any higher.
      bool enable_up = true;
      if(iter == m_model_fields->children().begin())
        enable_up = false;  //It can't go any higher.

      m_button_field_up->set_sensitive(enable_up);


      //Disable Down if It can't go any lower.
      Gtk::TreeModel::iterator iterNext = iter;
      iterNext++;

      bool enable_down = true;
      if(iterNext == m_model_fields->children().end())
        enable_down = false;

      m_button_field_down->set_sensitive(enable_down);

      m_button_field_delete->set_sensitive(true);
    }
    else
    {
      //Disable all buttons that act on a selection:
      m_button_field_down->set_sensitive(false);
      m_button_field_up->set_sensitive(false);
      m_button_field_delete->set_sensitive(false);
    }
  }


}


void Dialog_FieldsList::on_button_field_up()
{
  move_treeview_selection_up(m_treeview_fields, m_ColumnsFields.m_col_sequence);
}

void Dialog_FieldsList::on_button_field_down()
{
  move_treeview_selection_down(m_treeview_fields, m_ColumnsFields.m_col_sequence);
}

LayoutGroup::type_list_items Dialog_FieldsList::get_fields() const
{

  const auto children = m_model_fields->children();
  LayoutGroup::type_list_items result(children.size());

  guint field_sequence = 0;
  for(Gtk::TreeModel::iterator iterFields = m_model_fields->children().begin(); iterFields != children.end(); ++iterFields)
  {
    Gtk::TreeModel::Row row = *iterFields;

    std::shared_ptr<const LayoutItem_Field> item = row[m_ColumnsFields.m_col_layout_item];
    const auto field_name = item->get_name();
    if(!field_name.empty())
    {
      std::shared_ptr<LayoutItem_Field> field_copy = glom_sharedptr_clone(item);

      //TODO: This seems to overwrite the sequence set when the user reorders an item.
      result[field_sequence] = field_copy;

      ++field_sequence;
    }
  }

  return result;
}

void Dialog_FieldsList::on_treeview_fields_selection_changed()
{
  enable_buttons();
}

Gtk::TreeModel::iterator Dialog_FieldsList::append_appropriate_row()
{
  Gtk::TreeModel::iterator result;

  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_treeview_fields->get_selection();
  Gtk::TreeModel::iterator selected = refTreeSelection->get_selected();

  //Add the field details to the layout treeview:
  if(selected)
  {
    //TODO: This doesn't work because it's the sequence ID that really affects the order.
    result = m_model_fields->insert_after(selected);
  }
  else
  {
    result = m_model_fields->append(); 
  }
  
  return result;
}

void Dialog_FieldsList::on_button_add_field()
{
  //Get the chosen fields:
  type_list_field_items fields_list = offer_field_list(m_table_name, this);
  for(type_list_field_items::iterator iter_chosen = fields_list.begin(); iter_chosen != fields_list.end(); ++iter_chosen)
  {
    std::shared_ptr<LayoutItem_Field> field = *iter_chosen;
    if(!field)
      continue;

    //Add the field details to the layout treeview:
    Gtk::TreeModel::iterator iter = append_appropriate_row();

    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;
      row[m_ColumnsFields.m_col_layout_item] = field;

      //Scroll to, and select, the new row:
      Glib::RefPtr<Gtk::TreeView::Selection> refTreeSelection = m_treeview_fields->get_selection();
      if(refTreeSelection)
        refTreeSelection->select(iter);

      m_treeview_fields->scroll_to_row( Gtk::TreeModel::Path(iter) );

      treeview_fill_sequences(m_model_fields, m_ColumnsFields.m_col_sequence); //The document should have checked this already, but it does not hurt to check again.
    }
  }
}

void Dialog_FieldsList::on_button_delete()
{
  Glib::RefPtr<Gtk::TreeView::Selection> refTreeSelection = m_treeview_fields->get_selection();
  if(refTreeSelection)
  {
    //TODO: Handle multiple-selection:
    Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
    if(iter)
    {
      m_model_fields->erase(iter);

      m_modified = true;
    }
  }
}


void Dialog_FieldsList::on_cell_data_name(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter)
{
  //Set the view's cell properties depending on the model's data:
  Gtk::CellRendererText* renderer_text = dynamic_cast<Gtk::CellRendererText*>(renderer);
  if(renderer_text)
  {
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;

      std::shared_ptr<const LayoutItem_Field> item = row[m_ColumnsFields.m_col_layout_item]; //TODO_performance: Reduce copying.
      if(item)
      {
        renderer_text->property_markup() = item->get_layout_display_name();
      }
      else
      {
        //Though this really shouldn't even be in the model:
        renderer_text->property_markup() = Glib::ustring();
      }

      renderer_text->property_editable() = false; //Names can never be edited.
    }
  }
}


void Dialog_FieldsList::on_button_edit_field()
{
  Glib::RefPtr<Gtk::TreeView::Selection> refTreeSelection = m_treeview_fields->get_selection();
  if(refTreeSelection)
  {
    //TODO: Handle multiple-selection:
    Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;
      std::shared_ptr<const LayoutItem_Field> field = row[m_ColumnsFields.m_col_layout_item];

      //Get the chosen field:
      std::shared_ptr<LayoutItem_Field> field_chosen =
        offer_field_list_select_one_field(field, m_table_name, this);

      //Set the field details in the layout treeview:
      if(field_chosen)
        row[m_ColumnsFields.m_col_layout_item] = field_chosen;

      //Scroll to, and select, the new row:
      /*
      Glib::RefPtr<Gtk::TreeView::Selection> refTreeSelection = m_treeview_fields->get_selection();
      if(refTreeSelection)
        refTreeSelection->select(iter);

      m_treeview_fields->scroll_to_row( Gtk::TreeModel::Path(iter) );

      treeview_fill_sequences(m_model_fields, m_ColumnsFields.m_col_sequence); //The document should have checked this already, but it does not hurt to check again.
      */
    }
  }
}

void Dialog_FieldsList::on_button_formatting()
{
  Glib::RefPtr<Gtk::TreeView::Selection> refTreeSelection = m_treeview_fields->get_selection();
  if(refTreeSelection)
  {
    //TODO: Handle multiple-selection:
    Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;
      std::shared_ptr<const LayoutItem_Field> field = row[m_ColumnsFields.m_col_layout_item];
      if(field)
      {
        field = offer_field_formatting(field, m_table_name, this, false /* no editing options */);
        row[m_ColumnsFields.m_col_layout_item] = field;
      }
    }
  }
}

} //namespace Glom
