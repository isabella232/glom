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

#include "dialog_layout_list.h"
#include "dialog_choose_field.h"
#include "../layout_item_dialogs/dialog_field_layout.h"
#include "../frame_glom.h"
#include <bakery/App/App_Gtk.h> //For util_bold_message().

//#include <libgnome/gnome-i18n.h>
#include <glibmm/i18n.h>

namespace Glom
{

Dialog_Layout_List::Dialog_Layout_List(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Dialog_Layout(cobject, refGlade),
  m_treeview_fields(0),
  m_button_field_up(0),
  m_button_field_down(0),
  m_button_field_add(0),
  m_button_field_delete(0),
  m_button_field_edit(0),
  m_button_field_formatting(0),
  m_label_table_name(0)
{
  refGlade->get_widget("label_table_name", m_label_table_name);

  refGlade->get_widget("treeview_fields", m_treeview_fields);
  if(m_treeview_fields)
  {
    m_model_fields = Gtk::ListStore::create(m_ColumnsFields);
    m_treeview_fields->set_model(m_model_fields);

    // Append the View columns:
    Gtk::TreeView::Column* column_name = Gtk::manage( new Gtk::TreeView::Column(_("Name")) );
    m_treeview_fields->append_column(*column_name);

    Gtk::CellRendererText* renderer_name = Gtk::manage(new Gtk::CellRendererText);
    column_name->pack_start(*renderer_name);
    column_name->set_cell_data_func(*renderer_name, sigc::mem_fun(*this, &Dialog_Layout_List::on_cell_data_name));


    //Sort by sequence, so we can change the order by changing the values in the hidden sequence column.
    m_model_fields->set_sort_column(m_ColumnsFields.m_col_sequence, Gtk::SORT_ASCENDING);


    //Respond to changes of selection:
    Glib::RefPtr<Gtk::TreeView::Selection> refSelection = m_treeview_fields->get_selection();
    if(refSelection)
    {
      refSelection->signal_changed().connect( sigc::mem_fun(*this, &Dialog_Layout_List::on_treeview_fields_selection_changed) );
    }

    m_model_fields->signal_row_changed().connect( sigc::mem_fun(*this, &Dialog_Layout_List::on_treemodel_row_changed) );
  }


  refGlade->get_widget("button_field_up", m_button_field_up);
  m_button_field_up->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_List::on_button_field_up) );

  refGlade->get_widget("button_field_down", m_button_field_down);
  m_button_field_down->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_List::on_button_field_down) );

  refGlade->get_widget("button_field_delete", m_button_field_delete);
  m_button_field_delete->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_List::on_button_delete) );

  refGlade->get_widget("button_field_add", m_button_field_add);
  m_button_field_add->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_List::on_button_add_field) );

  refGlade->get_widget("button_field_edit", m_button_field_edit);
  m_button_field_edit->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_List::on_button_edit_field) );

  refGlade->get_widget("button_field_formatting", m_button_field_formatting);
  m_button_field_formatting->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_List::on_button_field_formatting) );

  show_all_children();
}

Dialog_Layout_List::~Dialog_Layout_List()
{
}

void Dialog_Layout_List::set_document(const Glib::ustring& layout, Document_Glom* document, const Glib::ustring& table_name, const type_vecLayoutFields& table_fields)
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
    document->fill_layout_field_details(m_table_name, mapGroups); //Update with full field information.

    //If no information is stored in the document, then start with something:

    if(mapGroups.empty())
    {
      sharedptr<LayoutGroup> group = sharedptr<LayoutGroup>::create();
      group->set_name("main");

      guint field_sequence = 1; //0 means no sequence
      for(type_vecLayoutFields::const_iterator iter = table_fields.begin(); iter != table_fields.end(); ++iter)
      {
        sharedptr<LayoutItem_Field> item = glom_sharedptr_clone(*iter);
        item->m_sequence = field_sequence;

        group->add_item(item, field_sequence);

        ++field_sequence;
      }

      mapGroups[1] = group;
    }

    //Show the field layout
    typedef std::list< Glib::ustring > type_listStrings;

    m_model_fields->clear();

    guint field_sequence = 1; //0 means no sequence
    for(Document_Glom::type_mapLayoutGroupSequence::const_iterator iter = mapGroups.begin(); iter != mapGroups.end(); ++iter)
    {
      sharedptr<const LayoutGroup> group = iter->second;

      //Add the group's fields:
      LayoutGroup::type_map_const_items items = group->get_items();
      for(LayoutGroup::type_map_const_items::const_iterator iter = items.begin(); iter != items.end(); ++iter)
      {
        sharedptr<const LayoutItem_Field> item = sharedptr<const LayoutItem_Field>::cast_dynamic(iter->second);
        if(item)
        {
          Gtk::TreeModel::iterator iterTree = m_model_fields->append();
          Gtk::TreeModel::Row row = *iterTree;

          sharedptr<LayoutItem_Field> newitem = glom_sharedptr_clone(item);
          row[m_ColumnsFields.m_col_layout_item] = newitem;
          row[m_ColumnsFields.m_col_sequence] = field_sequence;
          ++field_sequence;
        }
      }
    }

    treeview_fill_sequences(m_model_fields, m_ColumnsFields.m_col_sequence); //The document should have checked this already, but it does not hurt to check again.
  }

  m_modified = false;
}

void Dialog_Layout_List::enable_buttons()
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


void Dialog_Layout_List::on_button_field_up()
{
  move_treeview_selection_up(m_treeview_fields, m_ColumnsFields.m_col_sequence);
}

void Dialog_Layout_List::on_button_field_down()
{
  move_treeview_selection_down(m_treeview_fields, m_ColumnsFields.m_col_sequence);
}

void Dialog_Layout_List::save_to_document()
{
  Dialog_Layout::save_to_document();

  if(m_modified)
  {
    //Set the table name and title:
    Document_Glom* document = get_document();
    if(document)
      document->set_table_title( m_table_name, m_entry_table_title->get_text() );

    //Get the data from the TreeView and store it in the document:

    //Get the groups and their fields:
    Document_Glom::type_mapLayoutGroupSequence mapGroups;

    //Add the fields to the one group:
    sharedptr<LayoutGroup> others = sharedptr<LayoutGroup>::create();
    others->set_name("main");
    others->m_sequence = 1;

    guint field_sequence = 1; //0 means no sequence
    for(Gtk::TreeModel::iterator iterFields = m_model_fields->children().begin(); iterFields != m_model_fields->children().end(); ++iterFields)
    {
      Gtk::TreeModel::Row row = *iterFields;

      sharedptr<LayoutItem_Field> item = row[m_ColumnsFields.m_col_layout_item];
      const Glib::ustring field_name = item->get_name();
      if(!field_name.empty())
      {
        item->m_sequence = field_sequence;

        others->add_item(item, field_sequence); //Add it to the group:

        ++field_sequence;
      }
    }

    mapGroups[1] = others;

    if(document)
    {
      document->set_data_layout_groups(m_layout_name, m_table_name, mapGroups);
      m_modified = false;
    }
  }
}

void Dialog_Layout_List::on_treeview_fields_selection_changed()
{
  enable_buttons();
}

void Dialog_Layout_List::warn_about_images()
{
  Frame_Glom::show_ok_dialog(_("Images Not Allowed On List View"), _("The list view cannot display image fields."), *this, Gtk::MESSAGE_WARNING); //TODO: Scale them down to thumbnails in a GtkCellRenderPixbuf?
}

void Dialog_Layout_List::on_button_add_field()
{
  //Get the chosen field:
  sharedptr<LayoutItem_Field> field = offer_field_list(m_table_name, this);
  if(field)
  {
    if(field->get_glom_type() == Field::TYPE_IMAGE)
    {
      warn_about_images();
    }
    else
    {
      //Add the field details to the layout treeview:
      Gtk::TreeModel::iterator iter =  m_model_fields->append();

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

        m_modified = true;
      }
    }
  }

  enable_buttons();
}

void Dialog_Layout_List::on_button_delete()
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

  enable_buttons();
}


void Dialog_Layout_List::on_cell_data_name(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter)
{
  //Set the view's cell properties depending on the model's data:
  Gtk::CellRendererText* renderer_text = dynamic_cast<Gtk::CellRendererText*>(renderer);
  if(renderer_text)
  {
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;

      //Indicate that it's a field in another table.
      sharedptr<LayoutItem_Field> item = row[m_ColumnsFields.m_col_layout_item]; //TODO_performance: Reduce copying.
      renderer_text->property_markup() = item->get_layout_display_name();

      renderer_text->property_editable() = false; //Names can never be edited.
    }
  }
}


void Dialog_Layout_List::on_button_edit_field()
{
  Glib::RefPtr<Gtk::TreeView::Selection> refTreeSelection = m_treeview_fields->get_selection();
  if(refTreeSelection)
  {
    //TODO: Handle multiple-selection:
    Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;
      sharedptr<LayoutItem_Field> field = row[m_ColumnsFields.m_col_layout_item];

      sharedptr<LayoutItem_Field> field_chosen = offer_field_list(field, m_table_name, this);
      if(field_chosen)
      {
        //Set the field details in the layout treeview:

        if(field_chosen->get_glom_type() == Field::TYPE_IMAGE)
        {
          warn_about_images();
        }
        else
        {
          row[m_ColumnsFields.m_col_layout_item] = field_chosen;
          m_modified = true;
        }
      }
    }
  }
}


void Dialog_Layout_List::on_button_field_formatting()
{
  Glib::RefPtr<Gtk::TreeView::Selection> refTreeSelection = m_treeview_fields->get_selection();
  if(refTreeSelection)
  {
    //TODO: Handle multiple-selection:
    Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();

    Gtk::TreeModel::Row row = *iter;
    sharedptr<LayoutItem_Field> field = row[m_ColumnsFields.m_col_layout_item];

    if(field)
    {
      sharedptr<LayoutItem_Field> chosenitem = offer_field_formatting(field, m_table_name, this);
      if(chosenitem)
      {
        *field = *chosenitem; //TODO_Performance.

        row[m_ColumnsFields.m_col_layout_item] = field;
        m_model_fields->row_changed(Gtk::TreePath(iter), iter); //TODO: Add row_changed(iter) to gtkmm?
        m_modified = true;
      }
    }
  }
}

} //namespace Glom
