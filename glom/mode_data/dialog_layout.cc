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

#include "dialog_layout.h"
//#include <libgnome/gnome-i18n.h>
#include <libintl.h>

Dialog_Layout::Dialog_Layout(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::Dialog(cobject),
  m_treeview_fields(0),
  m_treeview_groups(0),
  m_button_field_up(0),
  m_button_field_down(0),
  m_button_field_add(0),
  m_button_field_delete(0),        
  m_button_field_edit(0),
  m_button_group_up(0),
  m_button_group_down(0),
  m_button_group_add(0),
  m_button_group_delete(0),
  m_frame_groups(0),
  m_treeviewcolumn_field_groups(0),
  m_label_table_name(0),
  m_entry_table_title(0),
  m_document(0),
  m_modified(false)
{
  refGlade->get_widget("treeview_fields", m_treeview_fields);
  if(m_treeview_fields)
  {
    m_model_fields = Gtk::ListStore::create(m_ColumnsFields);
    m_treeview_fields->set_model(m_model_fields);

    // Append the View columns:
    m_treeview_fields->append_column( gettext("Field Name"), m_ColumnsFields.m_col_name );

    // The popup group column:
    m_treeviewcolumn_field_groups = Gtk::manage( new Gtk::TreeViewColumn(gettext("Group"), m_cellrenderer_field_group) ); //We save it in a member variable so that we can enable/disable it later.
    m_treeviewcolumn_field_groups->set_renderer(m_cellrenderer_field_group, m_ColumnsFields.m_col_group); //render it via the default "text" property.
    m_treeview_fields->append_column( *m_treeviewcolumn_field_groups  );

    //Make it editable:
    m_cellrenderer_field_group.property_editable() = true;

    //Connect to its signal:
    m_cellrenderer_field_group.signal_edited().connect( sigc::mem_fun(*this, &Dialog_Layout::on_treeview_fields_group_cell_edited) );
            

    m_treeview_fields->append_column_editable( gettext("Hidden"), m_ColumnsFields.m_col_hidden );

    //Sort by sequence, so we can change the order by changing the values in the hidden sequence column.
    m_model_fields->set_sort_column(m_ColumnsFields.m_col_sequence, Gtk::SORT_ASCENDING);


    //Respond to changes of selection:
    Glib::RefPtr<Gtk::TreeView::Selection> refSelection = m_treeview_fields->get_selection();
    if(refSelection)
    {
      refSelection->signal_changed().connect( sigc::mem_fun(*this, &Dialog_Layout::on_treeview_fields_selection_changed) );
    }

    m_model_fields->signal_row_changed().connect( sigc::mem_fun(*this, &Dialog_Layout::on_treemodel_row_changed) );
  }

  refGlade->get_widget("frame_groups", m_frame_groups);
  refGlade->get_widget("treeview_groups", m_treeview_groups);
  if(m_treeview_groups)
  {
    m_model_groups = Gtk::ListStore::create(m_ColumnsGroups);
    m_treeview_groups->set_model(m_model_groups);

    //We don't use append_column_editable() here because we want to validate the input, and respond to it;
    int columns_count = m_treeview_groups->append_column( gettext("Group Name"), m_ColumnsGroups.m_col_name );    
    Gtk::CellRendererText* cell_renderer = dynamic_cast<Gtk::CellRendererText*>(m_treeview_groups->get_column_cell_renderer(columns_count-1));
    if(cell_renderer)
    {
       //Make it editable:
       cell_renderer->property_editable() = true;

       //Connect to its signal:
       cell_renderer->signal_edited().connect( sigc::mem_fun(*this, &Dialog_Layout::on_treeview_groups_name_cell_edited) );
    }

    m_treeview_groups->append_column_editable( gettext("Title"), m_ColumnsGroups.m_col_title );
        
    m_treeview_groups->append_column( gettext("Fields"), m_ColumnsGroups.m_col_fields );
    
    //Sort by sequence, so we can change the order by changing the values in the hidden sequence column.
    m_model_groups->set_sort_column(m_ColumnsGroups.m_col_sequence, Gtk::SORT_ASCENDING);

    //Respond to changes of selection:
    Glib::RefPtr<Gtk::TreeView::Selection> refSelection = m_treeview_groups->get_selection();
    if(refSelection)
    {
      refSelection->signal_changed().connect( sigc::mem_fun(*this, &Dialog_Layout::on_treeview_groups_selection_changed) );
    }

    m_model_groups->signal_row_changed().connect( sigc::mem_fun(*this, &Dialog_Layout::on_treemodel_row_changed) );
  }

  refGlade->get_widget("button_field_up", m_button_field_up);
  m_button_field_up->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout::on_button_field_up) );

  refGlade->get_widget("button_field_down", m_button_field_down);
  m_button_field_down->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout::on_button_field_down) );

  refGlade->get_widget("button_field_delete", m_button_field_delete);

  refGlade->get_widget("button_field_add", m_button_field_add);

  refGlade->get_widget("button_field_edit", m_button_field_edit);
          
  refGlade->get_widget("button_group_up", m_button_group_up);
  m_button_group_up->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout::on_button_group_up) );

  refGlade->get_widget("button_group_down", m_button_group_down);
  m_button_group_down->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout::on_button_group_down) );

  refGlade->get_widget("button_group_add", m_button_group_add);
  m_button_group_add->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout::on_button_group_add) );

  refGlade->get_widget("button_group_delete", m_button_group_delete);
  m_button_group_delete->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout::on_button_group_delete) );
  
  Gtk::Button* button = 0;
  refGlade->get_widget("button_close", button);
  button->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout::on_button_close) );

  refGlade->get_widget("label_table_name", m_label_table_name);
  refGlade->get_widget("entry_table_title", m_entry_table_title);
  m_entry_table_title->signal_changed().connect( sigc::mem_fun(*this, &Dialog_Layout::on_entry_table_title_changed) );

   
  show_all_children();
}

Dialog_Layout::~Dialog_Layout()
{
}

void Dialog_Layout::set_document(const Glib::ustring& layout, Document_Glom* document, const Glib::ustring& table_name, const type_vecFields& table_fields)
{
  m_modified = false;

  m_layout_name = layout;
  set_title(layout + gettext(" Layout") );
  
  m_document = document;
  m_table_name = table_name;

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
      group.m_group_name = "others";

      guint field_sequence = 1; //0 means no sequence
      for(type_vecFields::const_iterator iter = table_fields.begin(); iter != table_fields.end(); ++iter)
      {
        LayoutItem item;
        item.m_field_name = iter->get_name();
        item.m_sequence = field_sequence;

        group.m_map_items[field_sequence] = item;

        ++field_sequence;
      }

      mapGroups[0] = group;
    }

    //Show the field layout
    typedef std::list< Glib::ustring > type_listStrings;

    m_model_fields->clear();
    m_model_groups->clear();

    guint field_sequence = 1; //0 means no sequence
    guint group_sequence = 1; //0 means no sequence
    for(Document_Glom::type_mapLayoutGroupSequence::const_iterator iter = mapGroups.begin(); iter != mapGroups.end(); ++iter)
    {
      const LayoutGroup& group = iter->second;

      //Add the group:
      Gtk::TreeModel::iterator iterTreeGroups = m_model_groups->append();
      Gtk::TreeModel::Row rowGroups = *iterTreeGroups;
      rowGroups[m_ColumnsGroups.m_col_name] = group.m_group_name;
      rowGroups[m_ColumnsGroups.m_col_title] = group.m_title;
      rowGroups[m_ColumnsGroups.m_col_sequence] = group_sequence;
      ++group_sequence;
         
      //Add the group's fields:
      for(LayoutGroup::type_map_items::const_iterator iter = group.m_map_items.begin(); iter != group.m_map_items.end(); ++iter)
      {
        const LayoutItem& item = iter->second;
        
        Gtk::TreeModel::iterator iterTree = m_model_fields->append();
        Gtk::TreeModel::Row row = *iterTree;

        row[m_ColumnsFields.m_col_name] = item.m_field_name;
        row[m_ColumnsFields.m_col_sequence] = field_sequence;
        ++field_sequence;

        row[m_ColumnsFields.m_col_group] = group.m_group_name; //item.m_group;
        row[m_ColumnsFields.m_col_hidden] = item.m_hidden; 
      }

        //Add a new group if one is somehow not in the list of groups.
        //if( mapGroups.find(item.m_group) == listGroups.end() )
        //  mapGroups.push_back(item.m_group);
    }

    treeview_fill_sequences(m_model_groups, m_ColumnsGroups.m_col_sequence); //The document should have checked this already, but it does not hurt to check again.
    treeview_fill_sequences(m_model_fields, m_ColumnsFields.m_col_sequence); //The document should have checked this already, but it does not hurt to check again.
      
    //Show the groups:
    fill_groups_fields();

    fill_cellrenderer_groups();  
  }
  
  m_modified = false;  
}

void Dialog_Layout::fill_groups_fields()
{
  for(Gtk::TreeModel::iterator iter = m_model_groups->children().begin(); iter != m_model_groups->children().end(); ++iter)
  {
    Gtk::TreeModel::Row row = *iter;

    Glib::ustring group_name = row[m_ColumnsGroups.m_col_name];

    //Get the list of fields that are in this group:
    Glib::ustring fields_in_group;
    for(Gtk::TreeModel::iterator iterFields = m_model_fields->children().begin(); iterFields != m_model_fields->children().end(); ++iterFields)
    {
      Gtk::TreeModel::Row rowFields = *iterFields;

      if( rowFields[m_ColumnsFields.m_col_group] == group_name )
      {
        Glib::ustring field_name = rowFields[m_ColumnsFields.m_col_name];

        if(fields_in_group.empty())
          fields_in_group = field_name;
        else
          fields_in_group += (", " + field_name);
      }
    }

    //Put the list of fields in the group's row:
    row[m_ColumnsGroups.m_col_fields] = fields_in_group;
  }
  
}

/*
void Dialog_Layout::fill_groups()
{
  //Get the groups:
  Document_Glom::type_mapLayoutGroupSequence mapGroups = m_document->get_data_layout_groups(m_layout_name, m_table_name);
 
  //Show the groups:
  m_model_groups->clear();
  for(Document_Glom::type_mapLayoutGroupSequence::const_iterator iter = mapGroups.begin(); iter != mapGroups.end(); ++iter)
  {
    Gtk::TreeModel::iterator iterTree = m_model_groups->append();
    Gtk::TreeModel::Row row = *iterTree;

    Glib::ustring group_name = iter->second.m_group_name;
    row[m_ColumnsGroups.m_col_name] = group_name;
    row[m_ColumnsGroups.m_col_title] = iter->second.m_title;
    row[m_ColumnsGroups.m_col_sequence] = iter->second.m_sequence;
  }

  fill_groups_fields();
  
  treeview_fill_sequences(m_model_groups, m_ColumnsGroups.m_col_sequence); //The document should have checked this already, but it does not hurt to check again.
}
*/

void Dialog_Layout::enable_buttons()
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
  

  //Groups:
  refSelection = m_treeview_groups->get_selection();
  if(refSelection)
  {
    Gtk::TreeModel::iterator iter = refSelection->get_selected();
    if(iter)
    {
      //Disable Up if It can't go any higher.
      bool enable_up = true;
      if(iter == m_model_groups->children().begin())
        enable_up = false;  //It can't go any higher.

      m_button_group_up->set_sensitive(enable_up);


      //Disable Down if It can't go any lower.
      Gtk::TreeModel::iterator iterNext = iter;
      iterNext++;

      bool enable_down = true;
      if(iterNext == m_model_groups->children().end())
        enable_down = false;

      m_button_group_down->set_sensitive(enable_down);

      m_button_group_delete->set_sensitive(true);
    }
    else
    {
      //Disable all buttons that act on a selection:
      m_button_group_down->set_sensitive(false);
      m_button_group_up->set_sensitive(false);
      m_button_group_delete->set_sensitive(false);
    }       
  }
      
}

void Dialog_Layout::on_button_group_add()
{
   Gtk::TreeModel::iterator iterTree = m_model_groups->append();
   Gtk::TreeModel::Row row = *iterTree;
   row[m_ColumnsFields.m_col_name] = "untitled";

   //Add sequences to any that don't have a sequence:
   treeview_fill_sequences(m_model_groups, m_ColumnsGroups.m_col_sequence);

   fill_cellrenderer_groups();

   //Put the cursor in the new row, because the default value is useless - the user must enter a group name:
   Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_treeview_groups->get_selection();
   if(refTreeSelection)
   {
     refTreeSelection->select(iterTree);

     Gtk::TreeView::Column* pColumn = m_treeview_groups->get_column(m_ColumnsGroups.m_col_name.index());
     if(pColumn)
     {
       Gtk::TreeModel::Path path = m_model_groups->get_path(iterTree);
       m_treeview_groups->set_cursor(path, *pColumn, true /* start editing */);
     }
   }
}

void Dialog_Layout::on_button_group_delete()
{
  fill_cellrenderer_groups();
}
  
void Dialog_Layout::on_button_group_up()
{
   move_treeview_selection_up(m_treeview_groups, m_ColumnsGroups.m_col_sequence);
}

void Dialog_Layout::on_button_field_up()
{
  move_treeview_selection_up(m_treeview_fields, m_ColumnsFields.m_col_sequence);
}

void Dialog_Layout::move_treeview_selection_up(Gtk::TreeView* treeview, const Gtk::TreeModelColumn<guint>& sequence_column)
{
  Glib::RefPtr<Gtk::TreeView::Selection> refSelection = treeview->get_selection();
  if(refSelection)
  {
    Gtk::TreeModel::iterator iter = refSelection->get_selected();
    if(iter)
    {
      Glib::RefPtr<Gtk::TreeModel> model = treeview->get_model();
      if(iter != model->children().begin()) //If it is not the first one.
      {
        Gtk::TreeModel::iterator iterBefore = iter;
        --iterBefore;

        Gtk::TreeModel::Row row = *iter;
        Gtk::TreeModel::Row rowBefore = **iterBefore;
                                                     
        //Swap the sequence values, so that the one before will be after:
        guint tempBefore = rowBefore[sequence_column];
        guint tempRow = row[sequence_column];
        rowBefore[sequence_column] = tempRow;
        row[sequence_column] = tempBefore;

        //Because the model is sorted, the visual order should now be swapped.

        m_modified = true;
      }   
    }

  }

  enable_buttons();
}

void Dialog_Layout::on_button_field_down()
{
  move_treeview_selection_down(m_treeview_fields, m_ColumnsFields.m_col_sequence);
}

void Dialog_Layout::on_button_group_down()
{
  move_treeview_selection_down(m_treeview_groups, m_ColumnsGroups.m_col_sequence);
}

void Dialog_Layout::move_treeview_selection_down(Gtk::TreeView* treeview, const Gtk::TreeModelColumn<guint>& sequence_column)
{
  Glib::RefPtr<Gtk::TreeView::Selection> refSelection = treeview->get_selection();
  if(refSelection)
  {
    Gtk::TreeModel::iterator iter = refSelection->get_selected();
    if(iter)
    {
      Gtk::TreeModel::iterator iterNext = iter;
      iterNext++;

      Glib::RefPtr<Gtk::TreeModel> model = treeview->get_model();
      if(iterNext != model->children().end()) //If it is not the last one.
      {
        Gtk::TreeModel::Row row = *iter;
        Gtk::TreeModel::Row rowNext = *iterNext;

        //Swap the sequence values, so that the one before will be after:
        guint tempNext = rowNext[sequence_column];
        guint tempRow = row[sequence_column];
        rowNext[sequence_column] = tempRow;
        row[sequence_column] = tempNext;

        //Because the model is sorted, the visual order should now be swapped.

        m_modified = true;
      }
    }

  }

  enable_buttons();
}

void Dialog_Layout::on_button_close()
{
  save_to_document();
  
  hide();
}

void Dialog_Layout::save_to_document()
{
  if(!m_modified)
    g_warning("save_to_document: not modified");
  else
  {
    g_warning("save_to_document: modified");
    //Set the table name and title:
    if(m_document)
      m_document->set_table_title( m_table_name, m_entry_table_title->get_text() );
    
    //Get the data from the TreeView and store it in the document:

    //Get the groups and their fields:
    Document_Glom::type_mapLayoutGroupSequence mapGroups;
    guint group_sequence = 1; //0 means no sequence
    
    for(Gtk::TreeModel::iterator iterGroups = m_model_groups->children().begin(); iterGroups != m_model_groups->children().end(); ++iterGroups)
    {
      Gtk::TreeModel::Row row = *iterGroups;

      Glib::ustring name = row[m_ColumnsGroups.m_col_name];
      if(!name.empty())
      {   
        LayoutGroup group;
        group.m_group_name = name;
        group.m_title = row[m_ColumnsGroups.m_col_title];

        //Add the fields for this group:

        guint field_sequence = 1; //0 means no sequence
        for(Gtk::TreeModel::iterator iter = m_model_fields->children().begin(); iter != m_model_fields->children().end(); ++iter)
        {
          Gtk::TreeModel::Row row = *iter;

          const Glib::ustring group_name = row[m_ColumnsFields.m_col_group];
          if(group_name == group.m_group_name)
          {
            LayoutItem item;
            item.m_field_name = row[m_ColumnsFields.m_col_name];  
            item.m_group = group_name;
            item.m_sequence = field_sequence;
            item.m_hidden = row[m_ColumnsFields.m_col_hidden];
            group.m_map_items[field_sequence] = item; //Add it to the group:
            
            ++field_sequence;
          }
        }

        //Add the group:
        group.m_sequence = group_sequence;
        mapGroups[group_sequence] = group;
        ++group_sequence;
      }
    }

    if(m_document)
    {
      m_document->set_data_layout_groups(m_layout_name, m_table_name, mapGroups);
      m_modified = false;
    }
  }
}

void Dialog_Layout::on_treeview_fields_selection_changed()
{
  enable_buttons(); 
}

void Dialog_Layout::on_treeview_groups_selection_changed()
{
  enable_buttons();
}

void Dialog_Layout::fill_cellrenderer_groups()
{
    m_cellrenderer_field_group.remove_all_list_items();

    //Get the groups from the groups treeview and put them in the popup in the fields treeview:        
    for(Gtk::TreeModel::iterator iter = m_model_groups->children().begin(); iter != m_model_groups->children().end(); ++iter)
    {
      Gtk::TreeModel::Row row = *iter;

      LayoutGroup item;
      Glib::ustring name = row[m_ColumnsGroups.m_col_name];
      if(!name.empty())
      {
         m_cellrenderer_field_group.append_list_item(name);
      }

    }

    m_modified = true;
}

void Dialog_Layout::treeview_fill_sequences(const Glib::RefPtr<Gtk::TreeModel> model, const Gtk::TreeModelColumn<guint>& sequence_column)
{
   //Get the highest sequence number:
  guint max_sequence = 1; //0 means no sequence.
  for(Gtk::TreeModel::iterator iter = model->children().begin(); iter != model->children().end(); ++iter)
  {
    Gtk::TreeModel::Row row = *iter;

    guint sequence = row[sequence_column];
    max_sequence = MAX(max_sequence, sequence);
  }

  //Add sequences to any that don't have a sequence:
  //(0 means no sequence)
  guint next_sequence = max_sequence+1; //This could leave holes, of course. But we want new groups to be after the old groups. We can compact it later.
  for(Gtk::TreeModel::iterator iter = model->children().begin(); iter != model->children().end(); ++iter)
  {
    Gtk::TreeModel::Row row = *iter;

    guint sequence = row[sequence_column];
    if(sequence == 0)
    {
      row[sequence_column] = next_sequence;
      ++next_sequence;

    }
  }
      
}

void Dialog_Layout::on_treeview_fields_group_cell_edited(const Glib::ustring& path_string, const Glib::ustring& new_text)
{
  Gtk::TreePath path(path_string);

  //Get the row from the path:
  Gtk::TreeModel::iterator iter = m_model_fields->get_iter(path);
  if(iter)
  {
      //Store the user's new text in the model:
      Gtk::TreeRow row = *iter;
      row[m_ColumnsFields.m_col_group] = new_text;
  }

  //Update the groups treeview, to show what fields are now in each group:
  fill_groups_fields(); 
}

void Dialog_Layout::on_treeview_groups_name_cell_edited(const Glib::ustring& path_string, const Glib::ustring& new_text)
{                                           
  Gtk::TreePath path(path_string);

  //Get the row from the path:
  Gtk::TreeModel::iterator iter = m_model_groups->get_iter(path);
  if(iter)
  {
      //Check whether it is the same name that has been used before:
      bool found = false;
      for(Gtk::TreeModel::iterator iterEach = m_model_groups->children().begin(); iterEach != m_model_groups->children().end(); ++iterEach)
      {
        if(iterEach != iter) //Don't compare it to itself
        {
          Gtk::TreeModel::Row row = *iterEach;
          
          Glib::ustring name = row[m_ColumnsGroups.m_col_name];
          if(name == new_text)
          {
            found = true;
            break;
          }
        }
      }

      if(found)
      {
        //Tell the user to choose a different name:
        Gtk::MessageDialog dialog(Glib::ustring("This group name already exists.") );
        dialog.run();

        //Make the item active again:
        //Put the cursor in the new row, because the default value is useless - the user must enter a group name:
        Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_treeview_groups->get_selection();
        if(refTreeSelection)
        {
          refTreeSelection->select(iter);

          Gtk::TreeView::Column* pColumn = m_treeview_groups->get_column(m_ColumnsGroups.m_col_name.index());
          if(pColumn)
          {
            m_treeview_groups->set_cursor(path, *pColumn, true /* start editing */);
          }
        }

      }
      else
      {
        //Store the user's new text in the model:
        Gtk::TreeRow row = *iter;
        row[m_ColumnsGroups.m_col_name] = new_text;

        //Add an appropriate title to begin with:
        row[m_ColumnsGroups.m_col_title] = Base_DB::util_title_from_string(new_text);

        fill_cellrenderer_groups();
      }
  }
}

void Dialog_Layout::set_show_groups(bool val)
{
  if(m_frame_groups)
  {
    if(val)
      m_frame_groups->show();
    else
      m_frame_groups->hide();
  }

  if(m_treeviewcolumn_field_groups)
    m_treeviewcolumn_field_groups->set_visible(val);  
}

void Dialog_Layout::on_treemodel_row_changed(const Gtk::TreeModel::Path& /* path */, const Gtk::TreeModel::iterator& /* iter */)
{
  m_modified = true;
}

void Dialog_Layout::on_entry_table_title_changed()
{
g_warning("on_entry_table_title_changed");
  m_modified = true;
}




