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
#include <bakery/App/App_Gtk.h> //For util_bold_message().

//#include <libgnome/gnome-i18n.h>
#include <libintl.h>

Dialog_Layout_List::Dialog_Layout_List(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Dialog_Layout(cobject, refGlade),
  m_treeview_fields(0),
  m_button_field_up(0),
  m_button_field_down(0),
  m_button_field_add(0),
  m_button_field_delete(0),
  m_button_field_edit(0),
  m_label_table_name(0)
{
  refGlade->get_widget("label_table_name", m_label_table_name);

  refGlade->get_widget("treeview_fields", m_treeview_fields);
  if(m_treeview_fields)
  {
    m_model_fields = Gtk::ListStore::create(m_ColumnsFields);
    m_treeview_fields->set_model(m_model_fields);

    // Append the View columns:
    Gtk::TreeView::Column* column_name = Gtk::manage( new Gtk::TreeView::Column(gettext("Name")) );
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

    //If no information is stored in the document, then start with something:

    if(mapGroups.empty())
    {
      LayoutGroup group;
      group.set_name("main");

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

    m_model_fields->clear();

    guint field_sequence = 1; //0 means no sequence
    for(Document_Glom::type_mapLayoutGroupSequence::const_iterator iter = mapGroups.begin(); iter != mapGroups.end(); ++iter)
    {
      const LayoutGroup& group = iter->second;

      //Add the group's fields:
      LayoutGroup::type_map_const_items items = group.get_items();
      for(LayoutGroup::type_map_const_items::const_iterator iter = items.begin(); iter != items.end(); ++iter)
      {
        const LayoutItem_Field* item = dynamic_cast<const LayoutItem_Field*>(iter->second); 

        Gtk::TreeModel::iterator iterTree = m_model_fields->append();
        Gtk::TreeModel::Row row = *iterTree;

        row[m_ColumnsFields.m_col_layout_item] = *item;
        row[m_ColumnsFields.m_col_sequence] = field_sequence;
        ++field_sequence;
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
    if(m_document)
      m_document->set_table_title( m_table_name, m_entry_table_title->get_text() );

    //Get the data from the TreeView and store it in the document:

    //Get the groups and their fields:
    Document_Glom::type_mapLayoutGroupSequence mapGroups;

    //Add the fields to the one group:
    LayoutGroup others;
    others.set_name("main");
    others.m_sequence = 1;

    guint field_sequence = 1; //0 means no sequence
    for(Gtk::TreeModel::iterator iterFields = m_model_fields->children().begin(); iterFields != m_model_fields->children().end(); ++iterFields)
    {
      Gtk::TreeModel::Row row = *iterFields;

      LayoutItem_Field item = row[m_ColumnsFields.m_col_layout_item];
      const Glib::ustring field_name = item.get_name();
      if(!field_name.empty())
      {
        item.m_sequence = field_sequence;

        others.add_item(item, field_sequence); //Add it to the group:

        ++field_sequence;
      }
    }

    mapGroups[1] = others;

    if(m_document)
    {
      m_document->set_data_layout_groups(m_layout_name, m_table_name, mapGroups);
      m_modified = false;
    }
  }
}

void Dialog_Layout_List::on_treeview_fields_selection_changed()
{
  enable_buttons();
}

void Dialog_Layout_List::on_button_add_field()
{
  try
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_choose_field");

    Dialog_ChooseField* dialog = 0;
    refXml->get_widget_derived("dialog_choose_field", dialog);

    if(dialog)
    {
      dialog->set_document(m_document, m_table_name);
      dialog->set_transient_for(*this);
      int response = dialog->run();
      if(response == Gtk::RESPONSE_OK)
      {
        //Get the chosen field:
        LayoutItem_Field field;
        dialog->get_field_chosen(field);

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
        }
      }
    }

    delete dialog;
  }
  catch(const Gnome::Glade::XmlError& ex)
  {
    std::cerr << ex.what() << std::endl;
  }
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
      const LayoutItem_Field item = row[m_ColumnsFields.m_col_layout_item]; //TODO_performance: Reduce copying.

      Glib::ustring markup;

      if(item.get_has_relationship_name())
        markup = item.get_relationship_name() + "::";

      markup += item.get_name();

      renderer_text->property_markup() = markup;

      renderer_text->property_editable() = false; //Names can never be edited.
    }
  }
}


void Dialog_Layout_List::on_button_edit_field()
{
  try
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_choose_field");

    Dialog_ChooseField* dialog = 0;
    refXml->get_widget_derived("dialog_choose_field", dialog);

    if(dialog)
    {
      Glib::RefPtr<Gtk::TreeView::Selection> refTreeSelection = m_treeview_fields->get_selection();
      if(refTreeSelection)
      {
        //TODO: Handle multiple-selection:
        Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
        if(iter)
        {
          Gtk::TreeModel::Row row = *iter;
          const LayoutItem_Field& field = row[m_ColumnsFields.m_col_layout_item];
  
          dialog->set_document(m_document, m_table_name, field);
          dialog->set_transient_for(*this);
          int response = dialog->run();
          if(response == Gtk::RESPONSE_OK)
          {
            //Get the chosen field:
            LayoutItem_Field field;
            dialog->get_field_chosen(field);
  
            //Set the field details in the layout treeview:
  
            row[m_ColumnsFields.m_col_layout_item] = field;
  
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
    }

    delete dialog;
  }
  catch(const Gnome::Glade::XmlError& ex)
  {
    std::cerr << ex.what() << std::endl;
  }
}
