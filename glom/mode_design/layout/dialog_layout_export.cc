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

#include <glom/mode_design/layout/dialog_layout_export.h>
#include <glom/mode_design/layout/dialog_choose_field.h>
#include <glom/mode_design/layout/layout_item_dialogs/dialog_field_layout.h>

//#include <libgnome/gnome-i18n.h>
#include <glibmm/i18n.h>

namespace Glom
{

const char* Dialog_Layout_Export::glade_id("window_data_layout_export");
const bool Dialog_Layout_Export::glade_developer(true);

Dialog_Layout_Export::Dialog_Layout_Export(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Dialog_Layout(cobject, builder, false /* no table title */),
  m_treeview_fields(nullptr),
  m_button_field_up(nullptr),
  m_button_field_down(nullptr),
  m_button_field_add(nullptr),
  m_button_field_delete(nullptr),
  m_button_field_edit(nullptr),
  m_label_table_name(nullptr)
{
  builder->get_widget("label_table_name", m_label_table_name);
  m_entry_table_title = nullptr; //Not in this glade file.

  builder->get_widget("treeview_fields", m_treeview_fields);
  if(m_treeview_fields)
  {
    m_model_fields = Gtk::ListStore::create(m_ColumnsFields);
    m_treeview_fields->set_model(m_model_fields);

    // Append the View columns:
    auto column_name = Gtk::manage( new Gtk::TreeView::Column(_("Fields")) );
    m_treeview_fields->append_column(*column_name);

    auto renderer_name = Gtk::manage(new Gtk::CellRendererText);
    column_name->pack_start(*renderer_name);
    column_name->set_cell_data_func(*renderer_name, sigc::mem_fun(*this, &Dialog_Layout_Export::on_cell_data_name));


    //Sort by sequence, so we can change the order by changing the values in the hidden sequence column.
    m_model_fields->set_sort_column(m_ColumnsFields.m_col_sequence, Gtk::SORT_ASCENDING);


    //Respond to changes of selection:
    auto refSelection = m_treeview_fields->get_selection();
    if(refSelection)
    {
      refSelection->signal_changed().connect( sigc::mem_fun(*this, &Dialog_Layout_Export::on_treeview_fields_selection_changed) );
    }

    m_model_fields->signal_row_changed().connect( sigc::mem_fun(*this, &Dialog_Layout_Export::on_treemodel_row_changed) );
  }


  builder->get_widget("button_field_up", m_button_field_up);
  m_button_field_up->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Export::on_button_up) );

  builder->get_widget("button_field_down", m_button_field_down);
  m_button_field_down->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Export::on_button_down) );

  builder->get_widget("button_field_delete", m_button_field_delete);
  m_button_field_delete->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Export::on_button_delete) );

  builder->get_widget("button_field_add", m_button_field_add);
  m_button_field_add->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Export::on_button_add_field) );

  builder->get_widget("button_field_edit", m_button_field_edit);
  m_button_field_edit->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Export::on_button_edit_field) );

  show_all_children();
}

void Dialog_Layout_Export::set_layout_groups(Document::type_list_const_layout_groups& mapGroups, const std::shared_ptr<Document>& document, const Glib::ustring& table_name)
{
  Base_DB::set_document(document);

  m_modified = false;
  //m_document = document

  m_table_name = table_name;

  //Update the tree models from the document
  if(document)
  {
    //Set the table name and title:
    m_label_table_name->set_text(table_name);

    //If we have not layout information, then start with something:
    /*
    if(mapGroups.empty())
    {
      LayoutGroup group;
      group->set_name("main");

      guint field_sequence = 1; //0 means no sequence
      for(const auto& field : table_fields)
      {
        LayoutItem_Field item = *(field);
        group->add_item(item);

        ++field_sequence;
      }

      mapGroups[1] = group;
    }
    */

    //Show the field layout:
    m_model_fields->clear();

    guint field_sequence = 1; //0 means no sequence
    for(const auto& group : mapGroups)
    {
      if(!group)
        continue;

      //Add the group's fields:
      for(const auto& base_item : group->get_items())
      {
        auto item = std::dynamic_pointer_cast<const LayoutItem_Field>(base_item); 
        if(item)
        {
          auto iterTree = m_model_fields->append();
          Gtk::TreeModel::Row row = *iterTree;

          auto layout_item = glom_sharedptr_clone(item);
          if(document)
            document->fill_layout_field_details_item(m_table_name, layout_item); //Update with full field information.

          row[m_ColumnsFields.m_col_layout_item] = layout_item;
          row[m_ColumnsFields.m_col_sequence] = field_sequence;
          ++field_sequence;
        }
      }
    }

    treeview_fill_sequences(m_model_fields, m_ColumnsFields.m_col_sequence); //The document should have checked this already, but it does not hurt to check again.
  }

  m_modified = false;
}

void Dialog_Layout_Export::enable_buttons()
{
  //Fields:
  auto refSelection = m_treeview_fields->get_selection();
  if(refSelection)
  {
    auto iter = refSelection->get_selected();
    if(iter)
    {
      //Disable Up if It can't go any higher.
      bool enable_up = true;
      if(iter == m_model_fields->children().begin())
        enable_up = false;  //It can't go any higher.

      m_button_field_up->set_sensitive(enable_up);


      //Disable Down if It can't go any lower.
      auto iterNext = iter;
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


void Dialog_Layout_Export::on_button_up()
{
  move_treeview_selection_up(m_treeview_fields, m_ColumnsFields.m_col_sequence);
}

void Dialog_Layout_Export::on_button_down()
{
  move_treeview_selection_down(m_treeview_fields, m_ColumnsFields.m_col_sequence);
}

void Dialog_Layout_Export::get_layout_groups(Document::type_list_const_layout_groups& layout_groups) const
{
  //Get the data from the TreeView and store it in the document:

  //Get the groups and their fields:
  Document::type_list_const_layout_groups groups;

  //Add the fields to the one group:
  auto others = std::make_shared<LayoutGroup>();
  others->set_name("main");

  guint field_sequence = 1; //0 means no sequence
  for(const auto& row : m_model_fields->children())
  {
    std::shared_ptr<LayoutItem_Field> item = row[m_ColumnsFields.m_col_layout_item];
    const auto field_name = item->get_name();
    if(!field_name.empty())
    {
      others->add_item(item); //Add it to the group:

      ++field_sequence;
    }
  }

  groups.emplace_back(others);
  layout_groups.swap(groups);
}

void Dialog_Layout_Export::on_treeview_fields_selection_changed()
{
  enable_buttons();
}

void Dialog_Layout_Export::on_button_add_field()
{
  //Get the chosen fields:
  type_list_field_items fields_list = offer_field_list(m_table_name, this);
  for(const auto& field : fields_list) 
  {
    if(!field)
      continue;

    //Add the field details to the layout treeview:
    auto iter =  m_model_fields->append();

    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;
      row[m_ColumnsFields.m_col_layout_item] = field;

      //Scroll to, and select, the new row:
      auto refTreeSelection = m_treeview_fields->get_selection();
      if(refTreeSelection)
        refTreeSelection->select(iter);

      m_treeview_fields->scroll_to_row( Gtk::TreeModel::Path(iter) );

      treeview_fill_sequences(m_model_fields, m_ColumnsFields.m_col_sequence); //The document should have checked this already, but it does not hurt to check again.
    }
  }

  enable_buttons();
}

void Dialog_Layout_Export::on_button_delete()
{
  auto refTreeSelection = m_treeview_fields->get_selection();
  if(refTreeSelection)
  {
    //TODO: Handle multiple-selection:
    auto iter = refTreeSelection->get_selected();
    if(iter)
    {
      m_model_fields->erase(iter);

      m_modified = true;
    }
  }

  enable_buttons();
}


void Dialog_Layout_Export::on_cell_data_name(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter)
{
  //Set the view's cell properties depending on the model's data:
  auto renderer_text = dynamic_cast<Gtk::CellRendererText*>(renderer);
  if(renderer_text)
  {
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;

      //Indicate that it's a field in another table.
      std::shared_ptr<LayoutItem_Field> item = row[m_ColumnsFields.m_col_layout_item];

      //Names can never be edited.
      renderer_text->property_markup() = item->get_layout_display_name();
    }
  }
}


void Dialog_Layout_Export::on_button_edit_field()
{
  auto refTreeSelection = m_treeview_fields->get_selection();
  if(refTreeSelection)
  {
    //TODO: Handle multiple-selection:
    auto iter = refTreeSelection->get_selected();
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;
      std::shared_ptr<LayoutItem_Field> field = row[m_ColumnsFields.m_col_layout_item];

      //Get the chosen field:
      auto field_chosen = offer_field_list_select_one_field(field, m_table_name, this);
      if(field_chosen)
      {
        //Set the field details in the layout treeview:

        row[m_ColumnsFields.m_col_layout_item] = field;

        //Scroll to, and select, the new row:
        /*
        auto refTreeSelection = m_treeview_fields->get_selection();
        if(refTreeSelection)
          refTreeSelection->select(iter);

        m_treeview_fields->scroll_to_row( Gtk::TreeModel::Path(iter) );

        treeview_fill_sequences(m_model_fields, m_ColumnsFields.m_col_sequence); //The document should have checked this already, but it does not hurt to check again.
        */
      }
    }
  }
}

} //namespace Glom
