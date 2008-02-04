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
#include "dialog_choose_relationship.h"
#include "../layout_item_dialogs/dialog_buttonscript.h"
#include "../layout_item_dialogs/dialog_notebook.h"
#include "../frame_glom.h" //For show_ok_dialog()
//#include <libgnome/gnome-i18n.h>
#include <bakery/App/App_Gtk.h> //For util_bold_message().
#include <glibmm/i18n.h>
#include <sstream> //For stringstream

namespace Glom
{

Dialog_Layout_Details::Dialog_Layout_Details(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Dialog_Layout(cobject, refGlade),
  m_treeview_fields(0),
  m_treeview_column_title(0),
  m_box_table_widgets(0),
  m_box_related_table_widgets(0),
  m_box_related_navigation(0),
  m_button_field_up(0),
  m_button_field_down(0),
  m_button_field_add(0),
  m_button_field_add_group(0),
  m_button_add_notebook(0),
  m_button_add_related(0),
  m_button_add_button(0),
  m_button_add_text(0),
  m_button_add_image(0),
  m_button_field_delete(0),
  m_button_field_formatting(0),
  m_button_edit(0),
  m_label_table_name(0)
{
  // Get the alternate sets of widgets, only one of which should be shown:
  // Derived classes will hide one and show the other:
  refGlade->get_widget("hbox_table_widgets", m_box_table_widgets);
  m_box_table_widgets->show();
  refGlade->get_widget("hbox_related_table_widgets", m_box_related_table_widgets);
  m_box_related_table_widgets->hide();
  refGlade->get_widget("frame_related_table_navigation", m_box_related_navigation); 
  m_box_related_navigation->hide();

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
    renderer_name->signal_edited().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_treeview_cell_edited_name) );


    //Title column:
    m_treeview_column_title = Gtk::manage( new Gtk::TreeView::Column(_("Title")) );
    m_treeview_fields->append_column(*m_treeview_column_title);

    Gtk::CellRendererText* renderer_title = Gtk::manage(new Gtk::CellRendererText);
    m_treeview_column_title->pack_start(*renderer_title);
    m_treeview_column_title->set_cell_data_func(*renderer_title, sigc::mem_fun(*this, &Dialog_Layout_Details::on_cell_data_title));

    //Connect to its signal:
    renderer_title->signal_edited().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_treeview_cell_edited_title) );


    //Columns-count column:
    Gtk::TreeView::Column* column_count = Gtk::manage( new Gtk::TreeView::Column(_("Columns Count")) );
    m_treeview_fields->append_column(*column_count);

    Gtk::CellRendererText* renderer_count = Gtk::manage(new Gtk::CellRendererText);
    column_count->pack_start(*renderer_count);
    column_count->set_cell_data_func(*renderer_count, sigc::mem_fun(*this, &Dialog_Layout_Details::on_cell_data_columns_count));

    //Connect to its signal:
    renderer_count->signal_edited().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_treeview_cell_edited_columns_count) );

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

  refGlade->get_widget("button_formatting", m_button_field_formatting);
  m_button_field_formatting->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_button_field_formatting) );

  refGlade->get_widget("button_field_add", m_button_field_add);
  m_button_field_add->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_button_field_add) );

  refGlade->get_widget("button_field_add_group", m_button_field_add_group);
  m_button_field_add_group->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_button_field_add_group) );

  refGlade->get_widget("button_add_notebook", m_button_add_notebook);
  m_button_add_notebook->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_button_add_notebook) );

  refGlade->get_widget("button_add_related", m_button_add_related);
  m_button_add_related->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_button_add_related) );

  refGlade->get_widget("button_add_button", m_button_add_button);
  m_button_add_button->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_button_add_button) );

  refGlade->get_widget("button_add_text", m_button_add_text);
  m_button_add_text->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_button_add_text) );

  refGlade->get_widget("button_add_image", m_button_add_image);
  m_button_add_image->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_button_add_image) );

  refGlade->get_widget("button_edit", m_button_edit);
  m_button_edit->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_button_edit) );

  //show_all_children();
}

Dialog_Layout_Details::~Dialog_Layout_Details()
{
}

void Dialog_Layout_Details::fill_group(const Gtk::TreeModel::iterator& iter, sharedptr<LayoutGroup>& group)
{
  sharedptr<LayoutItem_Portal> portal = sharedptr<LayoutItem_Portal>::cast_dynamic(group);
  if(portal)
    return; //This method is not for portals.

  if(iter)
  {
    Gtk::TreeModel::Row row = *iter;
    sharedptr<LayoutItem> layout_item_top = row[m_model_items->m_columns.m_col_layout_item];
    sharedptr<LayoutGroup> group_row = sharedptr<LayoutGroup>::cast_dynamic(layout_item_top);
    if(!group_row)
      return;

    sharedptr<LayoutItem_Portal> portal_row = sharedptr<LayoutItem_Portal>::cast_dynamic(group_row);
    if(portal_row) //This is only for groups.
      return;

    *group = *group_row; //Copy name, title, columns count, etc.
    group->remove_all_items(); //Remove the copied child items, if any, so we can add them.

    //Get child layout items:
    for(Gtk::TreeModel::iterator iterChild = row.children().begin(); iterChild != row.children().end(); ++iterChild)
    {
      Gtk::TreeModel::Row rowChild = *iterChild;

      sharedptr<LayoutItem> layout_item = rowChild[m_model_items->m_columns.m_col_layout_item];

      sharedptr<LayoutItem_Portal> layout_portal = sharedptr<LayoutItem_Portal>::cast_dynamic(layout_item);
      if(layout_portal)
      {
        //std::cout << "fill_group(): adding portal." << std::endl;
        group->add_item(glom_sharedptr_clone(layout_portal));
      }
      else
      {
        //std::cout << "fill_group(): adding group." << std::endl;
        sharedptr<LayoutGroup> layout_group = sharedptr<LayoutGroup>::cast_dynamic(layout_item);
        sharedptr<LayoutItem_Portal> layout_portal = sharedptr<LayoutItem_Portal>::cast_dynamic(layout_group);
        if(layout_group && !layout_portal)
        {
          //Recurse:
          sharedptr<LayoutGroup> group_child = glom_sharedptr_clone(layout_group);
          fill_group(iterChild, group_child);
          group->add_item(group_child);
        }
        else if(layout_item)
        {
          //std::cout << "fill_group(): adding item." << std::endl;

          //Add field or button:
          sharedptr<LayoutItem> item = glom_sharedptr_clone(layout_item);
          group->add_item(item);
        }
      }
    }
  }
}


void Dialog_Layout_Details::add_group(const Gtk::TreeModel::iterator& parent, const sharedptr<const LayoutGroup>& group)
{
  if(!group)
   return;

  sharedptr<const LayoutItem_Portal> portal = sharedptr<const LayoutItem_Portal>::cast_dynamic(group);
  if(portal)
    return; //This method is not for portals.

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
    Gtk::TreeModel::Row rowGroup = *iterNewGroup;

    sharedptr<LayoutGroup> group_inserted = glom_sharedptr_clone(group);
    group_inserted->remove_all_items();
    rowGroup[m_model_items->m_columns.m_col_layout_item] = group_inserted;

    //Add the child items: (TODO_Performance: The child items are ignored/wasted because we clone them again.)
    LayoutGroup::type_map_const_items items = group->get_items();
    for(LayoutGroup::type_map_const_items::const_iterator iter = items.begin(); iter != items.end(); ++iter)
    {
      sharedptr<const LayoutItem> item = iter->second;

      sharedptr<const LayoutItem_Portal> portal = sharedptr<const LayoutItem_Portal>::cast_dynamic(item);
      if(portal) //If it is a portal
      {
        //Handle this differently to regular groups, so we do not also add its children:
        Gtk::TreeModel::iterator iter = m_model_items->append(iterNewGroup->children());
        Gtk::TreeModel::Row row = *iter;
        row[m_model_items->m_columns.m_col_layout_item] = glom_sharedptr_clone(portal);
      }
      else
      {
        sharedptr<const LayoutGroup> child_group = sharedptr<const LayoutGroup>::cast_dynamic(item);
        if(child_group) //If it is a group:
          add_group(iterNewGroup, child_group); //recursive
        else
        {
          //Add the item to the treeview:
          Gtk::TreeModel::iterator iter = m_model_items->append(iterNewGroup->children());
          Gtk::TreeModel::Row row = *iter;
          row[m_model_items->m_columns.m_col_layout_item] = glom_sharedptr_clone(item);
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
    document->fill_layout_field_details(m_table_name, mapGroups); //Update with full field information.

    //If no information is stored in the document, then start with something:

    if(mapGroups.empty())
    {
      sharedptr<LayoutGroup> group = sharedptr<LayoutGroup>::create();
      group->set_name("main");
      group->m_columns_count = 2;

      guint field_sequence = 1; //0 means no sequence
      for(type_vecLayoutFields::const_iterator iter = table_fields.begin(); iter != table_fields.end(); ++iter)
      {
        sharedptr<LayoutItem_Field> item = *iter;
        item->m_sequence = field_sequence;

        group->add_item(item, field_sequence);

        ++field_sequence;
      }

      mapGroups[1] = group;
    }

    //Show the field layout
    //typedef std::list< Glib::ustring > type_listStrings;

    m_model_items->clear();

    //guint field_sequence = 1; //0 means no sequence
    //guint group_sequence = 1; //0 means no sequence
    for(Document_Glom::type_mapLayoutGroupSequence::const_iterator iter = mapGroups.begin(); iter != mapGroups.end(); ++iter)
    {
      sharedptr<const LayoutGroup> group = iter->second;
      //std::cout << "debug: Dialog_Layout_Details::set_document(): adding group name=" << group->get_name() << std::endl;
        
      sharedptr<const LayoutGroup> portal = sharedptr<const LayoutItem_Portal>::cast_dynamic(group);
      if(group && !portal)
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
      Gtk::TreeModel::iterator iterParent = iter->parent();
      if(iterParent)
      {
        //See whether it is the last child of its parent:
        if(iter == iterParent->children().begin())
          enable_up = false;
      }
      else
      {
        //It is at the top-level:
        if(iter == m_model_items->children().begin())
          enable_up = false;  //It can't go any higher.
      }

      m_button_field_up->set_sensitive(enable_up);


      //Disable Down if It can't go any lower.
      Gtk::TreeModel::iterator iterNext = iter;
      iterNext++;

      bool enable_down = true;
      if(iterParent)
      {
        //See whether it is the last child of its parent:
        if(iterNext == iterParent->children().end())
          enable_down = false;
      }
      else
      {
        //It is at the top-level:
        if(iterNext == m_model_items->children().end())
          enable_down = false;
      }

      m_button_field_down->set_sensitive(enable_down);

      m_button_field_delete->set_sensitive(true);

      //Only fields have formatting:
      sharedptr<LayoutItem> layout_item = (*iter)[m_model_items->m_columns.m_col_layout_item];
      sharedptr<LayoutItem_Field> layout_item_field = sharedptr<LayoutItem_Field>::cast_dynamic(layout_item);
      const bool is_field = layout_item_field;
      m_button_field_formatting->set_sensitive(is_field);
    }
    else
    {
      //Disable all buttons that act on a selection:
      m_button_field_down->set_sensitive(false);
      m_button_field_up->set_sensitive(false);
      m_button_field_delete->set_sensitive(false);
      m_button_field_formatting->set_sensitive(false);
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
  sharedptr<LayoutItem_Field> layout_item = offer_field_list(m_table_name, this);
  if(layout_item)
  {
    //Add the field details to the layout treeview:
    Gtk::TreeModel::iterator iter = append_appropriate_row();
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;
      row[m_model_items->m_columns.m_col_layout_item] = glom_sharedptr_clone(layout_item);

      //Scroll to, and select, the new row:
      Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_treeview_fields->get_selection();
      if(refTreeSelection)
        refTreeSelection->select(iter);

      m_treeview_fields->scroll_to_row( Gtk::TreeModel::Path(iter) );

      m_modified = true;
    }
  }

  enable_buttons();
}


sharedptr<LayoutItem_Button> Dialog_Layout_Details::offer_button_script_edit(const sharedptr<const LayoutItem_Button>& button)
{
  sharedptr<LayoutItem_Button> result;

  try
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "window_button_script");

    Dialog_ButtonScript* dialog = 0;
    refXml->get_widget_derived("window_button_script", dialog);

    if(dialog)
    {
      dialog->set_script(button, m_table_name);
      dialog->set_transient_for(*this);
      int response = Glom::Utils::dialog_run_with_help(dialog, "window_button_script");
      dialog->hide();
      if(response == Gtk::RESPONSE_OK)
      {
        //Get the chosen relationship:
        result = dialog->get_script();
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

/*
sharedptr<LayoutItem_Text> Dialog_Layout_Details::offer_textobject_edit(const sharedptr<const LayoutItem_Text>& textobject)
{
  sharedptr<LayoutItem_Text> result;

  try
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "window_textobject");

    Dialog_TextObject* dialog = 0;
    refXml->get_widget_derived("window_textobject", dialog);

    if(dialog)
    {
      dialog->set_textobject(textobject, m_table_name);
      dialog->set_transient_for(*this);
      const int response = dialog->run();
      dialog->hide();
      if(response == Gtk::RESPONSE_OK)
      {
        //Get the chosen relationship:
        result = dialog->get_textobject();
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
*/

sharedptr<Relationship> Dialog_Layout_Details::offer_relationship_list()
{
  return offer_relationship_list(sharedptr<Relationship>());
}

sharedptr<Relationship> Dialog_Layout_Details::offer_relationship_list(const sharedptr<const Relationship>& item)
{
  sharedptr<Relationship> result = glom_sharedptr_clone(item);

  try
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_choose_relationship");

    Dialog_ChooseRelationship* dialog = 0;
    refXml->get_widget_derived("dialog_choose_relationship", dialog);

    if(dialog)
    {
      dialog->set_document(get_document(), m_table_name);
      dialog->select_item(item);
      dialog->set_transient_for(*this);
      const int response = dialog->run();
      dialog->hide();
      if(response == Gtk::RESPONSE_OK)
      {
        //Get the chosen relationship:
        result = dialog->get_relationship_chosen();
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

Gtk::TreeModel::iterator Dialog_Layout_Details::append_appropriate_row()
{
  Gtk::TreeModel::iterator result;

  Gtk::TreeModel::iterator parent = get_selected_group_parent();

  //Add the field details to the layout treeview:
  Gtk::TreeModel::iterator iter;
  if(parent)
    result = m_model_items->append(parent->children());
  else
  {
    //Find the first group, and make the new row a child of that:
    Gtk::TreeModel::iterator iter_first = m_model_items->children().begin();
    if(iter_first)
    {
      Gtk::TreeModel::Row row = *iter_first;

      sharedptr<LayoutItem> layout_item = row[m_model_items->m_columns.m_col_layout_item];
      sharedptr<LayoutGroup> layout_group = sharedptr<LayoutGroup>::cast_dynamic(layout_item);
      sharedptr<LayoutItem_Portal> layout_portal = sharedptr<LayoutItem_Portal>::cast_dynamic(layout_item);

      if(layout_group && !layout_portal)
        result = m_model_items->append(iter_first->children());
    }
  }

  return result;
}

void Dialog_Layout_Details::on_button_add_button()
{
  Gtk::TreeModel::iterator iter = append_appropriate_row();
  if(iter)
  {
    Gtk::TreeModel::Row row = *iter;

    //Add a new button:
    sharedptr<LayoutItem_Button> button = sharedptr<LayoutItem_Button>::create();
    button->set_title(_("New Button")); //Give the button a default title, so it is big enough, and so people see that they should change it.
    row[m_model_items->m_columns.m_col_layout_item] = button;

    //Scroll to, and select, the new row:
    Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_treeview_fields->get_selection();
    if(refTreeSelection)
      refTreeSelection->select(iter);

    m_treeview_fields->scroll_to_row( Gtk::TreeModel::Path(iter) );

    m_modified = true;
  }

  enable_buttons();
}

void Dialog_Layout_Details::on_button_add_text()
{
  Gtk::TreeModel::iterator iter = append_appropriate_row();
  if(iter)
  {
    Gtk::TreeModel::Row row = *iter;

    //Add a new button:
    sharedptr<LayoutItem_Text> textobject = sharedptr<LayoutItem_Text>::create();
    textobject->set_title(_("Text Title")); //Give the button a default title, so it is big enough, and so people see that they should change it.
    row[m_model_items->m_columns.m_col_layout_item] = textobject;

    //Scroll to, and select, the new row:
    Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_treeview_fields->get_selection();
    if(refTreeSelection)
      refTreeSelection->select(iter);

    m_treeview_fields->scroll_to_row( Gtk::TreeModel::Path(iter) );

    m_modified = true;
  }

  enable_buttons();
}

void Dialog_Layout_Details::on_button_add_image()
{
  Gtk::TreeModel::iterator iter = append_appropriate_row();
  if(iter)
  {
    Gtk::TreeModel::Row row = *iter;

    //Add a new button:
    sharedptr<LayoutItem_Image> imageobject = sharedptr<LayoutItem_Image>::create();
    imageobject->set_title(_("Image Title")); //Give the item a default title, so it is big enough, and so people see that they should change it.
    row[m_model_items->m_columns.m_col_layout_item] = imageobject;

    //Scroll to, and select, the new row:
    Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_treeview_fields->get_selection();
    if(refTreeSelection)
      refTreeSelection->select(iter);

    m_treeview_fields->scroll_to_row( Gtk::TreeModel::Path(iter) );

    m_modified = true;
  }

  enable_buttons();
}

void Dialog_Layout_Details::on_button_add_notebook()
{
  Gtk::TreeModel::iterator parent = get_selected_group_parent();

  Gtk::TreeModel::iterator iter = append_appropriate_row();
  if(iter)
  {
    Gtk::TreeModel::Row row = *iter;

    sharedptr<LayoutItem_Notebook> notebook = sharedptr<LayoutItem_Notebook>::create();
    notebook->set_name(_("notebook"));
    row[m_model_items->m_columns.m_col_layout_item] = notebook;

    //Scroll to, and select, the new row:
    Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_treeview_fields->get_selection();
    if(refTreeSelection)
      refTreeSelection->select(iter);

    m_treeview_fields->scroll_to_row( Gtk::TreeModel::Path(iter) );

    m_modified = true;
  }

  enable_buttons();
}

void Dialog_Layout_Details::on_button_add_related()
{
  Gtk::TreeModel::iterator parent = get_selected_group_parent();

  sharedptr<Relationship> relationship = offer_relationship_list();
  if(relationship)
  {
    Gtk::TreeModel::iterator iter = append_appropriate_row();
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;

      sharedptr<LayoutItem_Portal> portal = sharedptr<LayoutItem_Portal>::create();
      portal->set_relationship(relationship);
      row[m_model_items->m_columns.m_col_layout_item] = portal;

      //Scroll to, and select, the new row:
      Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_treeview_fields->get_selection();
      if(refTreeSelection)
        refTreeSelection->select(iter);

      m_treeview_fields->scroll_to_row( Gtk::TreeModel::Path(iter) );

      m_modified = true;
    }
  }

  enable_buttons();
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

      sharedptr<LayoutItem> layout_item = row[m_model_items->m_columns.m_col_layout_item];
      sharedptr<LayoutGroup> layout_group = sharedptr<LayoutGroup>::cast_dynamic(layout_item);
      sharedptr<LayoutItem_Portal> layout_portal = sharedptr<LayoutItem_Portal>::cast_dynamic(layout_item);

      if(layout_group && !layout_portal)
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
    sharedptr<LayoutGroup> layout_item = sharedptr<LayoutGroup>::create();
    layout_item->set_name(_("group"));
    row[m_model_items->m_columns.m_col_layout_item] = layout_item;

    //Scroll to, and select, the new row:
    Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_treeview_fields->get_selection();
    if(refTreeSelection)
      refTreeSelection->select(iterNewGroup);

    m_treeview_fields->scroll_to_row( Gtk::TreeModel::Path(iterNewGroup) );

    m_modified = true;
  }

  enable_buttons();
}

void Dialog_Layout_Details::on_button_field_formatting()
{
  //TODO: Abstract this into the base class:

  //Get the selected item:
  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_treeview_fields->get_selection();
  if(refTreeSelection)
  {
    Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;

      sharedptr<LayoutItem> layout_item = row[m_model_items->m_columns.m_col_layout_item];
      sharedptr<LayoutItem_Field> field = sharedptr<LayoutItem_Field>::cast_dynamic(layout_item);
      if(field)
      {
        sharedptr<LayoutItem_Field> chosenitem = offer_field_formatting(field, m_table_name, this);
        if(chosenitem)
        {
          *field = *chosenitem; //TODO_Performance.
          m_modified = true;
          //m_model_parts->row_changed(Gtk::TreePath(iter), iter); //TODO: Add row_changed(iter) to gtkmm?
        }
      }
    }
  }
}

void Dialog_Layout_Details::on_button_edit()
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

      sharedptr<LayoutItem> layout_item = row[m_model_items->m_columns.m_col_layout_item];

      sharedptr<LayoutItem_Portal> layout_portal = sharedptr<LayoutItem_Portal>::cast_dynamic(layout_item);
      if(layout_portal)
      {
        sharedptr<Relationship> relationship = offer_relationship_list(layout_portal->get_relationship());
        if(relationship)
        {
          layout_portal->set_relationship(relationship);
 
          //This is unnecessary and seems to cause a crash.
          //row[m_model_items->m_columns.m_col_layout_item] = layout_portal;

          m_modified = true;
        }
      }
      else
      {
        sharedptr<LayoutItem_Notebook> layout_notebook = sharedptr<LayoutItem_Notebook>::cast_dynamic(layout_item);
        if(layout_notebook)
        {
          Frame_Glom::show_ok_dialog(_("Notebook Tabs"), _("Add child groups to the notebook to add tabs."), *this);
          /*
          sharedptr<LayoutItem_Notebook> chosen = offer_notebook(layout_notebook);
          if(chosen)
          {
            row[m_model_items->m_columns.m_col_layout_item] = chosen;
            m_modified = true;
          }
          */
        }
        else
        {
          sharedptr<LayoutGroup> layout_group = sharedptr<LayoutGroup>::cast_dynamic(layout_item);
          if(layout_group)
          {
              Gtk::TreeModel::Path path = m_model_items->get_path(iter);
              m_treeview_fields->set_cursor(path, *m_treeview_column_title, true /* start_editing */);
          }
          else
          {
            sharedptr<LayoutItem_Field> layout_item_field = sharedptr<LayoutItem_Field>::cast_dynamic(layout_item);
            if(layout_item_field)
            {
              sharedptr<LayoutItem_Field> chosenitem = offer_field_list(layout_item_field, m_table_name, this);
              if(chosenitem)
              {
                *layout_item_field = *chosenitem;
                //row[m_model_items->m_columns.m_col_layout_item] = chosenitem;
                m_modified = true;
              }
            }
            else
            {
              sharedptr<LayoutItem_Button> layout_item_button = sharedptr<LayoutItem_Button>::cast_dynamic(layout_item);
              if(layout_item_button)
              {
                sharedptr<LayoutItem_Button> chosen = offer_button_script_edit(layout_item_button);
                if(chosen)
                {
                  *layout_item_button = *chosen;
                  //std::cout << "script: " << layout_item_button->get_script() << std::endl;

                  m_modified = true;
                }
              }
              else
              {
                sharedptr<LayoutItem_Text> layout_item_text = sharedptr<LayoutItem_Text>::cast_dynamic(layout_item);
                if(layout_item_text)
                {
                  sharedptr<LayoutItem_Text> chosen = offer_textobject(layout_item_text);
                  if(chosen)
                  {
                    *layout_item_text = *chosen;
                    //std::cout << "script: " << layout_item_button->get_script() << std::endl;

                    m_modified = true;
                  }
                }
                else
                {
                  sharedptr<LayoutItem_Image> layout_item_image = sharedptr<LayoutItem_Image>::cast_dynamic(layout_item);
                  if(layout_item_image)
                  {
                    sharedptr<LayoutItem_Image> chosen = offer_imageobject(layout_item_image);
                    if(chosen)
                    {
                      *layout_item_image = *chosen;
                      //std::cout << "script: " << layout_item_button->get_script() << std::endl;

                      m_modified = true;
                    }
                  }
                }
              }
            }
          }
        }
      }

      if(m_modified)
      {
        Gtk::TreeModel::Path path = m_model_items->get_path(iter);
        m_model_items->row_changed(path, iter);
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
    Document_Glom* document = get_document();
    if(document)
      document->set_table_title( m_table_name, m_entry_table_title->get_text() );

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

      sharedptr<LayoutItem> layout_item = row[m_model_items->m_columns.m_col_layout_item];
      sharedptr<LayoutGroup> layout_group = sharedptr<LayoutGroup>::cast_dynamic(layout_item);
      sharedptr<LayoutItem_Portal> layout_portal = sharedptr<LayoutItem_Portal>::cast_dynamic(layout_item);
      if(layout_group && !layout_portal) //There may be top-level groups, but no top-level fields, because the fields must be in a group (so that they are in columns)
      {
        sharedptr<LayoutGroup> group = sharedptr<LayoutGroup>::create();
        fill_group(iterFields, group);

        group->m_sequence = group_sequence;
        mapGroups[group_sequence] = group;
        //std::cout << "debug: Dialog_Layout_Details::save_to_document(): adding group sequence=" << group_sequence << ", group->m_sequence=" << group->m_sequence << ". group name=" << group->get_name() << std::endl;
        
        ++group_sequence;
      }
    }

    if(document)
    {
      document->set_data_layout_groups(m_layout_name, m_table_name, mapGroups);
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

      sharedptr<LayoutItem> layout_item = row[m_model_items->m_columns.m_col_layout_item];

      //Use the markup property instead of the text property, so that we can give the text some style:
      Glib::ustring markup;

      bool is_group = false;

      sharedptr<LayoutItem_Portal> layout_portal = sharedptr<LayoutItem_Portal>::cast_dynamic(layout_item);
      if(layout_portal)
      {
        markup = _("Related: ") + layout_portal->get_relationship_name();
      }
      else
      {
        sharedptr<LayoutGroup> layout_group = sharedptr<LayoutGroup>::cast_dynamic(layout_item);
        if(layout_group)
        {
          is_group = true;

          //Make group names bold:
          markup = Bakery::App_Gtk::util_bold_message( layout_item->get_name() );
        }
        else
        {
          sharedptr<LayoutItem_Field> layout_item_field = sharedptr<LayoutItem_Field>::cast_dynamic(layout_item);
          if(layout_item_field)
          {
            markup = _("Field: ") + layout_item_field->get_layout_display_name(); //TODO: Put this all in a sprintf-style string so it can be translated properly.

            //Just for debugging:
            //if(!row[m_model_items->m_columns.m_col_editable])
            // markup += " *";
          }
          else
          {
            sharedptr<LayoutItem_Button> layout_item_button = sharedptr<LayoutItem_Button>::cast_dynamic(layout_item);
            if(layout_item_button)
            {
              markup = _("Button"); //Buttons don't have names - just titles. TODO: Would they be useful?
            }
            else
            {
              sharedptr<LayoutItem_Text> layout_item_text = sharedptr<LayoutItem_Text>::cast_dynamic(layout_item);
              if(layout_item_text)
              {
                markup = _("Text"); //Text objects don't have names - just titles. TODO: Would they be useful?
              }
              else
              {
                sharedptr<LayoutItem_Image> layout_item_image = sharedptr<LayoutItem_Image>::cast_dynamic(layout_item);
                if(layout_item_image)
                {
                  markup = _("Image"); //Image objects don't have names - just titles. TODO: Would they be useful?
                }
                else if(layout_item)
                  markup = layout_item->get_name();
              }
            }
          }
        }
      }

      renderer_text->property_markup() = markup;

      if(is_group)
        renderer_text->property_editable() = true; //Group names can be changed.
      else
        renderer_text->property_editable() = false; //Field names can never be edited.
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
      sharedptr<LayoutItem> layout_item = row[m_model_items->m_columns.m_col_layout_item];
      sharedptr<LayoutItem_Notebook> layout_notebook = sharedptr<LayoutItem_Notebook>::cast_dynamic(layout_item);
      if(layout_notebook)
        renderer_text->property_text() = _("(Notebook)");
      else
        renderer_text->property_text() = layout_item->get_title();

      sharedptr<LayoutGroup> layout_group = sharedptr<LayoutGroup>::cast_dynamic(layout_item);
      sharedptr<LayoutItem_Portal> layout_portal = sharedptr<LayoutItem_Portal>::cast_dynamic(layout_item);
      sharedptr<LayoutItem_Button> layout_button = sharedptr<LayoutItem_Button>::cast_dynamic(layout_item);
      sharedptr<LayoutItem_Text> layout_text = sharedptr<LayoutItem_Text>::cast_dynamic(layout_item);
      const bool editable = (layout_group && !layout_portal) || layout_button || layout_text; //Only groups, buttons, and text objects have titles that can be edited.
      renderer_text->property_editable() = editable;
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
      sharedptr<LayoutItem> layout_item = row[m_model_items->m_columns.m_col_layout_item];

      sharedptr<LayoutGroup> layout_group = sharedptr<LayoutGroup>::cast_dynamic(layout_item);
      sharedptr<LayoutItem_Portal> layout_portal = sharedptr<LayoutItem_Portal>::cast_dynamic(layout_item);

      const bool is_group = layout_group && !layout_portal; //Only groups have column_counts.

      //Get a text representation of the number:
      Glib::ustring text;
      if(is_group)
      {
        std::stringstream the_stream;
        //the_stream.imbue(std::locale::classic());
        const guint number = layout_group->m_columns_count;
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

void Dialog_Layout_Details::on_treeview_cell_edited_title(const Glib::ustring& path_string, const Glib::ustring& new_text)
{
  if(!path_string.empty())
  {
    Gtk::TreePath path(path_string);

    //Get the row from the path:
    Gtk::TreeModel::iterator iter = m_model_items->get_iter(path);
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;
      sharedptr<LayoutItem> layout_item = row[m_model_items->m_columns.m_col_layout_item];
      if(layout_item)
      {
        //Store the user's new text in the model:
        Gtk::TreeRow row = *iter;
        layout_item->set_title(new_text);

        m_modified = true;
      }
    }
  }
}

void Dialog_Layout_Details::on_treeview_cell_edited_name(const Glib::ustring& path_string, const Glib::ustring& new_text)
{
  if(!path_string.empty())
  {
    Gtk::TreePath path(path_string);

    //Get the row from the path:
    Gtk::TreeModel::iterator iter = m_model_items->get_iter(path);
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;
      sharedptr<LayoutItem> layout_item = row[m_model_items->m_columns.m_col_layout_item];
      if(layout_item)
      {
        //Store the user's new text in the model:
        Gtk::TreeRow row = *iter;
        layout_item->set_name(new_text);

        m_modified = true;
      }
    }
  }
}

void Dialog_Layout_Details::on_treeview_cell_edited_columns_count(const Glib::ustring& path_string, const Glib::ustring& new_text)
{
  //This is used on numerical model columns:
  if(path_string.empty())
    return;

  Gtk::TreePath path(path_string);

  //Get the row from the path:
  Gtk::TreeModel::iterator iter = m_model_items->get_iter(path);
  if(iter)
  {
    Gtk::TreeModel::Row row = *iter;
    sharedptr<LayoutItem> layout_item = row[m_model_items->m_columns.m_col_layout_item];
    sharedptr<LayoutGroup> layout_group = sharedptr<LayoutGroup>::cast_dynamic(layout_item);
    sharedptr<LayoutItem_Portal> layout_portal = sharedptr<LayoutItem_Portal>::cast_dynamic(layout_item);
    if(layout_group && !layout_portal)
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
      layout_group->m_columns_count = new_value;

      m_modified = true;
    }
  }
}

} //namespace Glom









