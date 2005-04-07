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

#include "dialog_layout_details.h"
#include "dialog_choose_field.h"
#include "dialog_choose_relationship.h"
//#include <libgnome/gnome-i18n.h>
#include <bakery/App/App_Gtk.h> //For util_bold_message().
#include <glibmm/i18n.h>
#include <sstream> //For stringstream

Dialog_Layout_Details::Dialog_Layout_Details(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Dialog_Layout(cobject, refGlade),
  m_treeview_fields(0),
  m_button_field_up(0),
  m_button_field_down(0),
  m_button_field_add(0),
  m_button_field_add_group(0),
  m_button_add_related(0),
  m_button_field_delete(0),
  m_button_field_edit(0),
  m_label_table_name(0)
{
  refGlade->get_widget("label_table_name", m_label_table_name);

  refGlade->get_widget("treeview_fields", m_treeview_fields);
  if(m_treeview_fields)
  {
    m_treeview_fields->set_reorderable();
    m_treeview_fields->enable_model_drag_source();
    m_treeview_fields->enable_model_drag_dest();

    m_model_items = TreeStore_Layout::create();
    m_treeview_fields->set_model(m_model_items);

    // Append the View columns:
    // Use set_cell_data_func() to give more control over the cell attributes depending on the row:

    //Name column:
    Gtk::TreeView::Column* column_name = Gtk::manage( new Gtk::TreeView::Column(_("Name")) );
    m_treeview_fields->append_column(*column_name);

    Gtk::CellRendererText* renderer_name = Gtk::manage(new Gtk::CellRendererText);
    column_name->pack_start(*renderer_name);
    column_name->set_cell_data_func(*renderer_name, sigc::mem_fun(*this, &Dialog_Layout_Details::on_cell_data_name));

    //Connect to its signal:
    renderer_name->property_editable() = true;
    renderer_name->signal_edited().connect(
      sigc::bind( sigc::mem_fun(*this, &Dialog_Layout_Details::on_treeview_cell_edited_text), m_model_items->m_columns.m_col_name) );


    //Title column:
    Gtk::TreeView::Column* column_title = Gtk::manage( new Gtk::TreeView::Column(_("Title")) );
    m_treeview_fields->append_column(*column_title);

    Gtk::CellRendererText* renderer_title = Gtk::manage(new Gtk::CellRendererText);
    column_title->pack_start(*renderer_title);
    column_title->set_cell_data_func(*renderer_title, sigc::mem_fun(*this, &Dialog_Layout_Details::on_cell_data_title));

    //Connect to its signal:
    renderer_title->signal_edited().connect(
      sigc::bind( sigc::mem_fun(*this, &Dialog_Layout_Details::on_treeview_cell_edited_text), m_model_items->m_columns.m_col_title) );


    //Columns-count column:
    Gtk::TreeView::Column* column_count = Gtk::manage( new Gtk::TreeView::Column(_("Columns Count")) );
    m_treeview_fields->append_column(*column_count);

    Gtk::CellRendererText* renderer_count = Gtk::manage(new Gtk::CellRendererText);
    column_count->pack_start(*renderer_count);
    column_count->set_cell_data_func(*renderer_count, sigc::mem_fun(*this, &Dialog_Layout_Details::on_cell_data_columns_count));

    //Connect to its signal:
    renderer_count->signal_edited().connect(
      sigc::bind( sigc::mem_fun(*this, &Dialog_Layout_Details::on_treeview_cell_edited_numeric), m_model_items->m_columns.m_col_columns_count) );

    //Respond to changes of selection:
    Glib::RefPtr<Gtk::TreeView::Selection> refSelection = m_treeview_fields->get_selection();
    if(refSelection)
    {
      refSelection->signal_changed().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_treeview_fields_selection_changed) );
    }

    m_model_items->signal_row_changed().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_treemodel_row_changed) );
  }

  refGlade->get_widget("button_field_up", m_button_field_up);
  m_button_field_up->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_button_field_up) );

  refGlade->get_widget("button_field_down", m_button_field_down);
  m_button_field_down->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_button_field_down) );

  refGlade->get_widget("button_field_delete", m_button_field_delete);
  m_button_field_delete->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_button_field_delete) );

  refGlade->get_widget("button_field_add", m_button_field_add);
  m_button_field_add->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_button_field_add) );

  refGlade->get_widget("button_field_add_group", m_button_field_add_group);
  m_button_field_add_group->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_button_field_add_group) );

  refGlade->get_widget("button_add_related", m_button_add_related);
  m_button_add_related->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_button_add_related) );

  refGlade->get_widget("button_edit", m_button_field_edit);
  m_button_field_edit->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_button_field_edit) );

  show_all_children();
}

Dialog_Layout_Details::~Dialog_Layout_Details()
{
}

void Dialog_Layout_Details::fill_group(const Gtk::TreeModel::iterator& iter, LayoutGroup& group)
{
  if(iter)
  {
    Gtk::TreeModel::Row row = *iter;

    group.set_name( row[m_model_items->m_columns.m_col_name] );
    group.m_columns_count = row[m_model_items->m_columns.m_col_columns_count];
    group.m_title = row[m_model_items->m_columns.m_col_title];

    //Get child layout items:
    for(Gtk::TreeModel::iterator iterChild = row.children().begin(); iterChild != row.children().end(); ++iterChild)
    {
      Gtk::TreeModel::Row rowChild = *iterChild;

      if(rowChild[m_model_items->m_columns.m_col_type] == TreeStore_Layout::TYPE_GROUP)
      {
        //Recurse:
        LayoutGroup group_child;
        fill_group(iterChild, group_child);
        group.add_item(group_child);
      }
      else if(rowChild[m_model_items->m_columns.m_col_type] == TreeStore_Layout::TYPE_PORTAL)
      {
        LayoutItem_Portal portal;
        portal.set_relationship(  rowChild[m_model_items->m_columns.m_col_relationship] );
        group.add_item(portal);
      }
      else if(rowChild[m_model_items->m_columns.m_col_type] == TreeStore_Layout::TYPE_FIELD)
      {
        //Add field:
        LayoutItem_Field field;
        field.set_name( rowChild[m_model_items->m_columns.m_col_name] );

        field.m_relationship = rowChild[m_model_items->m_columns.m_col_relationship_name];

        //if(!relationship_name.empty())
        //{
        //  get_document()->get_table_relationship(m_table_name, field.m_relationship);
        //}

        field.set_editable( rowChild[m_model_items->m_columns.m_col_editable] );

        group.add_item(field);
      }
    }
  }
}



void Dialog_Layout_Details::add_group(const Gtk::TreeModel::iterator& parent, const LayoutGroup& group)
{
  Gtk::TreeModel::iterator iterNewGroup;
  if(!parent)
  {
    //Add it at the top-level, because nothing was selected:
    iterNewGroup = m_model_items->append();
  }
  else
  {
    iterNewGroup = m_model_items->append(parent->children());
  }

  if(iterNewGroup)
  {
    Gtk::TreeModel::Row row = *iterNewGroup;
    row[m_model_items->m_columns.m_col_type] = TreeStore_Layout::TYPE_GROUP;
    row[m_model_items->m_columns.m_col_name] = group.get_name();
    row[m_model_items->m_columns.m_col_columns_count] = group.m_columns_count;
    row[m_model_items->m_columns.m_col_title] = group.m_title;

    //Add the child items:
    LayoutGroup::type_map_const_items items = group.get_items();
    for(LayoutGroup::type_map_const_items::const_iterator iter = items.begin(); iter != items.end(); ++iter)
    {
      const LayoutItem* item = iter->second;
      const LayoutGroup* child_group = dynamic_cast<const LayoutGroup*>(item);
      if(child_group) //If it is a group:
        add_group(iterNewGroup, *child_group); //recursive
      else
      {
        const LayoutItem_Portal* portal = dynamic_cast<const LayoutItem_Portal*>(item);
        if(portal) //If it is a portal
        {
          Gtk::TreeModel::iterator iterField = m_model_items->append(iterNewGroup->children());
          Gtk::TreeModel::Row row = *iterField;
          row[m_model_items->m_columns.m_col_type] = TreeStore_Layout::TYPE_PORTAL;
          row[m_model_items->m_columns.m_col_relationship] = portal->get_relationship();
        }
        else
        {
          const LayoutItem_Field* field = dynamic_cast<const LayoutItem_Field*>(item);
          if(field) //If it is a field
          {
            //Add the field to the treeview:
            Gtk::TreeModel::iterator iterField = m_model_items->append(iterNewGroup->children());
            Gtk::TreeModel::Row row = *iterField;
            row[m_model_items->m_columns.m_col_type] = TreeStore_Layout::TYPE_FIELD;
            row[m_model_items->m_columns.m_col_name] = field->get_name();
            row[m_model_items->m_columns.m_col_relationship_name] = field->m_relationship;

            row[m_model_items->m_columns.m_col_editable] = field->get_editable();
          }
        }
      }
    }

    m_modified = true;
  }
}

void Dialog_Layout_Details::set_document(const Glib::ustring& layout, Document_Glom* document, const Glib::ustring& table_name, const type_vecLayoutFields& table_fields)
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
      group.m_columns_count = 2;

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

    m_model_items->clear();

    //guint field_sequence = 1; //0 means no sequence
    //guint group_sequence = 1; //0 means no sequence
    for(Document_Glom::type_mapLayoutGroupSequence::const_iterator iter = mapGroups.begin(); iter != mapGroups.end(); ++iter)
    {
      const LayoutGroup& group = iter->second;

      add_group(Gtk::TreeModel::iterator() /* null == top-level */, group);
    }

    //treeview_fill_sequences(m_model_items, m_model_items->m_columns.m_col_sequence); //The document should have checked this already, but it does not hurt to check again.
  }

  //Open all the groups:
  m_treeview_fields->expand_all();

  m_modified = false;
}

void Dialog_Layout_Details::enable_buttons()
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
      if(iter == m_model_items->children().begin())
        enable_up = false;  //It can't go any higher.

      m_button_field_up->set_sensitive(enable_up);


      //Disable Down if It can't go any lower.
      Gtk::TreeModel::iterator iterNext = iter;
      iterNext++;

      bool enable_down = true;
      if(iterNext == m_model_items->children().end())
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



void Dialog_Layout_Details::on_button_field_delete()
{
  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_treeview_fields->get_selection();
  if(refTreeSelection)
  {
    Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
    if(iter)
    {
      m_model_items->erase(iter);

      m_modified = true;
    }
  }
}

void Dialog_Layout_Details::on_button_field_up()
{
  Glib::RefPtr<Gtk::TreeView::Selection> refSelection = m_treeview_fields->get_selection();
  if(refSelection)
  {
    Gtk::TreeModel::iterator iter = refSelection->get_selected();
    if(iter)
    {
      Gtk::TreeModel::iterator parent = iter->parent();
      bool is_first = false;
      if(parent)
        is_first = (iter == parent->children().begin());
      else
        is_first = (iter == m_model_items->children().begin());

      if(!is_first)
      {
        Gtk::TreeModel::iterator iterBefore = iter;
        --iterBefore;

        m_model_items->iter_swap(iter, iterBefore);

        m_modified = true;
      }
    }

  }

  enable_buttons();
}

void Dialog_Layout_Details::on_button_field_down()
{
  Glib::RefPtr<Gtk::TreeView::Selection> refSelection = m_treeview_fields->get_selection();
  if(refSelection)
  {
    Gtk::TreeModel::iterator iter = refSelection->get_selected();
    if(iter)
    {
      Gtk::TreeModel::iterator iterNext = iter;
      iterNext++;

      Gtk::TreeModel::iterator parent = iter->parent();
      bool is_last = false;
      if(parent)
        is_last = (iterNext == parent->children().end());
      else
        is_last = (iterNext == m_model_items->children().end());

      if(!is_last)
      {
        //Swap the sequence values, so that the one before will be after:
        m_model_items->iter_swap(iter, iterNext);

        m_modified = true;
      }
    }

  }

  enable_buttons();
}

void Dialog_Layout_Details::on_button_field_add()
{
  Gtk::TreeModel::iterator parent = get_selected_group_parent();

  LayoutItem_Field layout_item;
  bool test = offer_field_list(layout_item);
  if(test)
  {
    //Add the field details to the layout treeview:
    Gtk::TreeModel::iterator iter;
    if(parent)
      iter = m_model_items->append(parent->children());
    else
    {
      //Find the first group, and make the new row a child of that:
      Gtk::TreeModel::iterator iter_first = m_model_items->children().begin();
      if(iter_first)
      {
        Gtk::TreeModel::Row row = *iter_first;
        if(row[m_model_items->m_columns.m_col_type] == TreeStore_Layout::TYPE_GROUP)
          iter = m_model_items->append(iter_first->children());
      }
    }

    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;
      row[m_model_items->m_columns.m_col_type] = TreeStore_Layout::TYPE_FIELD;
      row[m_model_items->m_columns.m_col_name] = layout_item.get_name();
      row[m_model_items->m_columns.m_col_relationship_name] = layout_item.m_relationship;
      //row[m_model_items->m_columns.m_col_title] = field.get_title();

      //Scroll to, and select, the new row:
      Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_treeview_fields->get_selection();
      if(refTreeSelection)
        refTreeSelection->select(iter);

      m_treeview_fields->scroll_to_row( Gtk::TreeModel::Path(iter) );
    }
  }
}


bool Dialog_Layout_Details::offer_relationship_list(Relationship& relationship)
{
  bool result = false;

  try
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_choose_relationship");

    Dialog_ChooseRelationship* dialog = 0;
    refXml->get_widget_derived("dialog_choose_relationship", dialog);

    if(dialog)
    {
      dialog->set_document(m_document, m_table_name);
      dialog->set_transient_for(*this);
      int response = dialog->run();
      dialog->hide();
      if(response == Gtk::RESPONSE_OK)
      {
        //Get the chosen relationship:
        result = dialog->get_relationship_chosen(relationship);
      }

      delete dialog;
    }
  }
  catch(const Gnome::Glade::XmlError& ex)
  {
    std::cerr << ex.what() << std::endl;
  }

  return result;
}

bool Dialog_Layout_Details::offer_field_list(LayoutItem_Field& field)
{
  bool result = false;

  try
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_choose_field");

    Dialog_ChooseField* dialog = 0;
    refXml->get_widget_derived("dialog_choose_field", dialog);

    if(dialog)
    {
      dialog->set_document(m_document, m_table_name, field);
      dialog->set_transient_for(*this);
      int response = dialog->run();
      if(response == Gtk::RESPONSE_OK)
      {
        //Get the chosen field:
        result = dialog->get_field_chosen(field);
      }

      delete dialog;
    }
  }
  catch(const Gnome::Glade::XmlError& ex)
  {
    std::cerr << ex.what() << std::endl;
  }

  return result;
}

void Dialog_Layout_Details::on_button_add_related()
{
  Gtk::TreeModel::iterator parent = get_selected_group_parent();

  Relationship relationship;
  bool test = offer_relationship_list(relationship);
  if(test)
  {
    //Add the field details to the layout treeview:
    Gtk::TreeModel::iterator iter;
    if(parent)
      iter = m_model_items->append(parent->children());
    else
    {
      //Find the first group, and make the new row a child of that:
      Gtk::TreeModel::iterator iter_first = m_model_items->children().begin();
      if(iter_first)
      {
        Gtk::TreeModel::Row row = *iter_first;
        if(row[m_model_items->m_columns.m_col_type] == TreeStore_Layout::TYPE_GROUP)
          iter = m_model_items->append(iter_first->children());
      }
    }

    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;
      row[m_model_items->m_columns.m_col_type] = TreeStore_Layout::TYPE_PORTAL;
      row[m_model_items->m_columns.m_col_relationship] = relationship.get_name();
      //row[m_model_items->m_columns.m_col_title] = field.get_title();

      //Scroll to, and select, the new row:
      Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_treeview_fields->get_selection();
      if(refTreeSelection)
        refTreeSelection->select(iter);

      m_treeview_fields->scroll_to_row( Gtk::TreeModel::Path(iter) );
    }
  }
}

Gtk::TreeModel::iterator Dialog_Layout_Details::get_selected_group_parent() const
{
  //Get the selected group, or a suitable parent group, or the first group:

  Gtk::TreeModel::iterator parent;

  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_treeview_fields->get_selection();
  if(refTreeSelection)
  {
    Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;
      if(row[m_model_items->m_columns.m_col_type] == TreeStore_Layout::TYPE_GROUP)
      {
        //Add a group under this group:
        parent = iter;
      }
      else
      {
        //Add a group under this item's group:
        parent = iter->parent();
      }
    }
  }

  return parent;
}

void Dialog_Layout_Details::on_button_field_add_group()
{
  Gtk::TreeModel::iterator parent = get_selected_group_parent();

  Gtk::TreeModel::iterator iterNewGroup;
  if(!parent)
  {
    //Add it at the top-level, because nothing was selected:
    iterNewGroup = m_model_items->append();
  }
  else
  {
    iterNewGroup = m_model_items->append(parent->children());
  }

  if(iterNewGroup)
  {
    Gtk::TreeModel::Row row = *iterNewGroup;
    row[m_model_items->m_columns.m_col_type] = TreeStore_Layout::TYPE_GROUP;
    row[m_model_items->m_columns.m_col_name] = "Untitled group";
    row[m_model_items->m_columns.m_col_columns_count] = 1;

    //Scroll to, and select, the new row:
    Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_treeview_fields->get_selection();
    if(refTreeSelection)
      refTreeSelection->select(iterNewGroup);

    m_treeview_fields->scroll_to_row( Gtk::TreeModel::Path(iterNewGroup) );

    m_modified = true;
  }
}

void Dialog_Layout_Details::on_button_field_edit()
{
  //Get the selected item:
  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_treeview_fields->get_selection();
  if(refTreeSelection)
  {
    Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
    if(iter)
    {
      //Do something different for each type of item:
      //This is unpleasant, but so is this whole dialog.
      //This whole dialog is just a temporary way to edit the layout before we have a visual DnD way.
      Gtk::TreeModel::Row row = *iter;
      switch( row[m_model_items->m_columns.m_col_type])
      {
        case TreeStore_Layout::TYPE_GROUP:
        {
          //TODO: Start editing the name.
          break;
        }
        case TreeStore_Layout::TYPE_FIELD:
        {
          LayoutItem_Field field;
          field.set_name( row[m_model_items->m_columns.m_col_name] ); //Start with this one selected.
          field.m_relationship = row[m_model_items->m_columns.m_col_relationship_name];

          //if(!relationship_name.empty())
          //{
          //  get_document()->get_table_relationship(m_table_name, field.m_relationship);
          //}

          field.set_editable( row[m_model_items->m_columns.m_col_editable] );
          bool test = offer_field_list(field);
          if(test)
          {
            row[m_model_items->m_columns.m_col_name] = field.get_name();
            row[m_model_items->m_columns.m_col_relationship_name] = field.m_relationship;

            row[m_model_items->m_columns.m_col_editable] = field.get_editable();
          }

          break;
        }
        case TreeStore_Layout::TYPE_PORTAL:
        {
          Relationship relationship;
          relationship.set_name( row[m_model_items->m_columns.m_col_relationship] ); //Start with this one selected.
          bool test = offer_relationship_list(relationship);
          if(test)
          {
            row[m_model_items->m_columns.m_col_relationship] = relationship.get_name();
          }

          break;
        }
      }
    }
  }
}

void Dialog_Layout_Details::save_to_document()
{
  Dialog_Layout::save_to_document();

  if(m_modified)
  {
    //Set the table name and title:
    if(m_document)
      m_document->set_table_title( m_table_name, m_entry_table_title->get_text() );

    //Get the data from the TreeView and store it in the document:

    //Fill the sequences:
    //(This model is not sorted - we need to set the sequence numbers based on the order).
    m_model_items->fill_sequences();

    //Get the groups and their fields:
    Document_Glom::type_mapLayoutGroupSequence mapGroups;
    guint group_sequence = 1; //0 means no sequence

    //Add the layout items:
    //guint field_sequence = 1; //0 means no sequence
    for(Gtk::TreeModel::iterator iterFields = m_model_items->children().begin(); iterFields != m_model_items->children().end(); ++iterFields)
    {
      Gtk::TreeModel::Row row = *iterFields;
      if(row[m_model_items->m_columns.m_col_type] == TreeStore_Layout::TYPE_GROUP) //There may be top-level groups, but no top-level fields, because the fields must be in a group (so that they are in columns)
      {
        LayoutGroup group;
        group.m_sequence = group_sequence;
        fill_group(iterFields, group);

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

void Dialog_Layout_Details::on_treeview_fields_selection_changed()
{
  enable_buttons();
}

void Dialog_Layout_Details::on_cell_data_name(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter)
{
  //TODO: If we ever use this a real layout tree, then let's add icons for each type.

  //Set the view's cell properties depending on the model's data:
  Gtk::CellRendererText* renderer_text = dynamic_cast<Gtk::CellRendererText*>(renderer);
  if(renderer_text)
  {
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;

      const bool is_group = row[m_model_items->m_columns.m_col_type] == TreeStore_Layout::TYPE_GROUP;
      const bool is_relationship = row[m_model_items->m_columns.m_col_type] == TreeStore_Layout::TYPE_PORTAL;

      //Use the markup property instead of the text property, so that we can give the text some style:
      Glib::ustring markup;
      if(is_group)
      {
        //Make group names bold:
        markup = Bakery::App_Gtk::util_bold_message( row[m_model_items->m_columns.m_col_name] );
      }
      else if(is_relationship)
      {
        markup = _("Related: ") + row[m_model_items->m_columns.m_col_relationship];
      }
      else
      {
        //It's a field:

        //Indicate that it's a field in another table.
        const Relationship& rel = row[m_model_items->m_columns.m_col_relationship_name];
        const Glib::ustring relationship = rel.get_name();
        if(!relationship.empty())
          markup = relationship + "::";

        markup += row[m_model_items->m_columns.m_col_name];

        //Just for debugging:
       // if(!row[m_model_items->m_columns.m_col_editable])
       //  markup += " *";
      }

      renderer_text->property_markup() = markup;

      renderer_text->property_editable() = false; //Names can never be edited.
    }
  }
}

void Dialog_Layout_Details::on_cell_data_title(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter)
{
  //Set the view's cell properties depending on the model's data:
  Gtk::CellRendererText* renderer_text = dynamic_cast<Gtk::CellRendererText*>(renderer);
  if(renderer_text)
  {
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;

      renderer_text->property_text() = row[m_model_items->m_columns.m_col_title];
      renderer_text->property_editable() = row[m_model_items->m_columns.m_col_type] == TreeStore_Layout::TYPE_GROUP; //Only groups have titles that can be edited.
    }
  }
}

void Dialog_Layout_Details::on_cell_data_columns_count(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter)
{
  //Set the view's cell properties depending on the model's data:
  Gtk::CellRendererText* renderer_text = dynamic_cast<Gtk::CellRendererText*>(renderer);
  if(renderer_text)
  {
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;

      const bool is_group = row[m_model_items->m_columns.m_col_type] == TreeStore_Layout::TYPE_GROUP; //Only groups have column_count that can be edited.

      //Get a text representation of the number:
      Glib::ustring text;
      if(is_group)
      {
        std::stringstream the_stream;
        //the_stream.imbue(std::locale::classic());
        guint number = row[m_model_items->m_columns.m_col_columns_count];
        the_stream << number;
        text = the_stream.str();
      }
      else
      {
        //Show nothing in the columns_count columns for fields.
      }
      renderer_text->property_text() = text;

      renderer_text->property_editable() = is_group;
    }
  }
}

void Dialog_Layout_Details::on_treeview_cell_edited_text(const Glib::ustring& path_string, const Glib::ustring& new_text, const Gtk::TreeModelColumn<Glib::ustring>& model_column)
{
  if(!path_string.empty())
  {
    Gtk::TreePath path(path_string);

    //Get the row from the path:
    Gtk::TreeModel::iterator iter = m_model_items->get_iter(path);
    if(iter)
    {
      //Store the user's new text in the model:
      Gtk::TreeRow row = *iter;
      row[model_column] = new_text;
    }
  }
}

void Dialog_Layout_Details::on_treeview_cell_edited_numeric(const Glib::ustring& path_string, const Glib::ustring& new_text, const Gtk::TreeModelColumn<guint>& model_column)
{
  //This is used on numerical model columns:
  if(path_string.empty())
    return;

  Gtk::TreePath path(path_string);

  //Get the row from the path:
  Gtk::TreeModel::iterator iter = m_model_items->get_iter(path);
  if(iter)
  {
    //std::istringstream astream(new_text); //Put it in a stream.
    //ColumnType new_value = ColumnType();
    //new_value << astream; //Get it out of the stream as the numerical type.

    //Convert the text to a number, using the same logic used by GtkCellRendererText when it stores numbers.
    char* pchEnd = 0;
    guint new_value = static_cast<guint>( strtod(new_text.c_str(), &pchEnd) );

    //Don't allow a 0 columns_count:
    if(new_value == 0)
      new_value = 1;
      
    //Store the user's new text in the model:
    Gtk::TreeRow row = *iter;
    row[model_column] = (guint)new_value;
  }
}









