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

#include "dialog_layout_report.h"
#include "data_structure/layout/report_parts/layoutitem_groupby.h"
#include "data_structure/layout/report_parts/layoutitem_summary.h"
#include "data_structure/layout/report_parts/layoutitem_fieldsummary.h"
#include "data_structure/layout/layoutitem_field.h"
#include "data_structure/layout/layoutitem_text.h"
#include "mode_data/dialog_choose_field.h"
#include "layout_item_dialogs/dialog_field_layout.h"
#include "mode_design/dialog_textobject.h"
#include "layout_item_dialogs/dialog_group_by.h"
#include "layout_item_dialogs/dialog_field_summary.h"
#include "mode_data/dialog_choose_relationship.h"
//#include <libgnome/gnome-i18n.h>
#include <bakery/App/App_Gtk.h> //For util_bold_message().
#include <glibmm/i18n.h>
#include <sstream> //For stringstream

Dialog_Layout_Report::Dialog_Layout_Report(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Dialog_Layout(cobject, refGlade, false /* No table title */),
  m_treeview_parts(0),
  m_treeview_available_parts(0),
  m_button_up(0),
  m_button_down(0),
  m_button_add(0),
  m_button_delete(0),
  m_button_edit(0),
  m_button_formatting(0),
  m_label_table_name(0),
  m_entry_name(0),
  m_entry_title(0)
{
  refGlade->get_widget("label_table_name", m_label_table_name);
  refGlade->get_widget("entry_name", m_entry_name);
  refGlade->get_widget("entry_title", m_entry_title);

  //Available parts:
  refGlade->get_widget("treeview_available_parts", m_treeview_available_parts);
  if(m_treeview_available_parts)
  {
    //Add list of available parts:
    //These are deleted in the destructor:
    m_model_available_parts = Gtk::TreeStore::create(m_columns_available_parts);

    Gtk::TreeModel::iterator iter = m_model_available_parts->append();
    (*iter)[m_columns_available_parts.m_col_item] = sharedptr<LayoutItem>(static_cast<LayoutItem*>(new LayoutItem_GroupBy()));

    Gtk::TreeModel::iterator iterField = m_model_available_parts->append(iter->children()); //Place Field under GroupBy to indicate that that's where it belongs in the actual layout.
    (*iterField)[m_columns_available_parts.m_col_item] = sharedptr<LayoutItem>(static_cast<LayoutItem*>(new LayoutItem_Field()));

    Gtk::TreeModel::iterator  iterText = m_model_available_parts->append(iter->children());
    (*iterText)[m_columns_available_parts.m_col_item] = sharedptr<LayoutItem>(static_cast<LayoutItem*>(new LayoutItem_Text()));

    iter = m_model_available_parts->append();
    (*iter)[m_columns_available_parts.m_col_item] = sharedptr<LayoutItem>(static_cast<LayoutItem*>(new LayoutItem_Summary()));
    iter = m_model_available_parts->append(iter->children());
    (*iter)[m_columns_available_parts.m_col_item] = sharedptr<LayoutItem>(static_cast<LayoutItem*>(new LayoutItem_FieldSummary()));

    m_treeview_available_parts->set_model(m_model_available_parts);
    m_treeview_available_parts->expand_all();

    // Append the View columns:
    // Use set_cell_data_func() to give more control over the cell attributes depending on the row:

    //Name column:
    Gtk::TreeView::Column* column_part = Gtk::manage( new Gtk::TreeView::Column(_("Part")) );
    m_treeview_available_parts->append_column(*column_part);

    Gtk::CellRendererText* renderer_part = Gtk::manage(new Gtk::CellRendererText());
    column_part->pack_start(*renderer_part);
    column_part->set_cell_data_func(*renderer_part, sigc::mem_fun(*this, &Dialog_Layout_Report::on_cell_data_available_part));

    m_treeview_available_parts->set_headers_visible(false); //There's only one column, so this is not useful.

    //Respond to changes of selection:
    Glib::RefPtr<Gtk::TreeView::Selection> refSelection = m_treeview_available_parts->get_selection();
    if(refSelection)
    {
      refSelection->signal_changed().connect( sigc::mem_fun(*this, &Dialog_Layout_Report::on_treeview_available_parts_selection_changed) );
    }
  }

  refGlade->get_widget("treeview_parts", m_treeview_parts);
  if(m_treeview_parts)
  {
    //Allow drag-and-drop:
    m_treeview_parts->enable_model_drag_source();
    m_treeview_parts->enable_model_drag_dest();

    m_model_parts = Gtk::TreeStore::create(m_columns_parts);
    m_treeview_parts->set_model(m_model_parts);

    // Append the View columns:
    // Use set_cell_data_func() to give more control over the cell attributes depending on the row:

    //Name column:
    Gtk::TreeView::Column* column_part = Gtk::manage( new Gtk::TreeView::Column(_("Part")) );
    m_treeview_parts->append_column(*column_part);

    Gtk::CellRendererText* renderer_part = Gtk::manage(new Gtk::CellRendererText);
    column_part->pack_start(*renderer_part);
    column_part->set_cell_data_func(*renderer_part, sigc::mem_fun(*this, &Dialog_Layout_Report::on_cell_data_part));

    //Details column:
    Gtk::TreeView::Column* column_details = Gtk::manage( new Gtk::TreeView::Column(_("Details")) );
    m_treeview_parts->append_column(*column_details);

    Gtk::CellRendererText* renderer_details = Gtk::manage(new Gtk::CellRendererText);
    column_details->pack_start(*renderer_details);
    column_details->set_cell_data_func(*renderer_details, sigc::mem_fun(*this, &Dialog_Layout_Report::on_cell_data_details));


    //Connect to its signal:
    //renderer_count->signal_edited().connect(
    //  sigc::bind( sigc::mem_fun(*this, &Dialog_Layout_Report::on_treeview_cell_edited_numeric), m_model_parts->m_columns.m_col_columns_count) );

    //Respond to changes of selection:
    Glib::RefPtr<Gtk::TreeView::Selection> refSelection = m_treeview_parts->get_selection();
    if(refSelection)
    {
      refSelection->signal_changed().connect( sigc::mem_fun(*this, &Dialog_Layout_Report::on_treeview_parts_selection_changed) );
    }

    //m_model_parts->signal_row_changed().connect( sigc::mem_fun(*this, &Dialog_Layout_Report::on_treemodel_row_changed) );
  }

  refGlade->get_widget("button_up", m_button_up);
  m_button_up->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Report::on_button_up) );

  refGlade->get_widget("button_down", m_button_down);
  m_button_down->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Report::on_button_down) );

  refGlade->get_widget("button_delete", m_button_delete);
  m_button_delete->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Report::on_button_delete) );

  refGlade->get_widget("button_add", m_button_add);
  m_button_add->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Report::on_button_add) );

  refGlade->get_widget("button_edit", m_button_edit);
  m_button_edit->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Report::on_button_edit) );

  refGlade->get_widget("button_formatting", m_button_formatting);
  m_button_formatting->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Report::on_button_formatting) );

  show_all_children();
}

Dialog_Layout_Report::~Dialog_Layout_Report()
{
}

sharedptr<LayoutGroup> Dialog_Layout_Report::fill_group(const Gtk::TreeModel::iterator& iter)
{
  if(iter)
  {
    Gtk::TreeModel::Row row = *iter;

    sharedptr<LayoutItem> pItem = row[m_columns_parts.m_col_item];
    sharedptr<LayoutGroup> pGroup = sharedptr<LayoutGroup>::cast_dynamic(pItem);
    if(pGroup)
    {
      //Make sure that it contains the child items:
      fill_group_children(pGroup, iter);
      return glom_sharedptr_clone(pGroup);
    }

  }

  return  sharedptr<LayoutGroup>();
}

void Dialog_Layout_Report::fill_group_children(const sharedptr<LayoutGroup>& group, const Gtk::TreeModel::iterator& iter)
{
  if(iter)
  {
    Gtk::TreeModel::Row row = *iter;

    guint sequence = 0;
    group->remove_all_items();
    for(Gtk::TreeModel::iterator iterChild = row.children().begin(); iterChild != row.children().end(); ++iterChild)
    {
      Gtk::TreeModel::Row row = *iterChild;
      sharedptr<LayoutItem> item = row[m_columns_parts.m_col_item];

      //Recurse:
      sharedptr<LayoutGroup> child_group = sharedptr<LayoutGroup>::cast_dynamic(item);
      if(child_group)
        fill_group_children(child_group, iterChild);

      sharedptr<LayoutItem> item_copy = glom_sharedptr_clone(item);
      item_copy->m_sequence = sequence;
      group->m_map_items[sequence] = item_copy;
      ++sequence;
    }

  }
}

void Dialog_Layout_Report::add_group(const Gtk::TreeModel::iterator& parent, const sharedptr<const LayoutGroup>& group)
{
  Gtk::TreeModel::iterator iterNewItem;
  if(!parent)
  {
    //Add it at the top-level, because nothing was selected:
    iterNewItem = m_model_parts->append();
  }
  else
  {
    iterNewItem = m_model_parts->append(parent->children());
  }

  if(iterNewItem)
  {
    Gtk::TreeModel::Row row = *iterNewItem;

    row[m_columns_parts.m_col_item] = sharedptr<LayoutItem>(static_cast<LayoutItem*>(group->clone()));

    //Add child rows:
    for(LayoutGroup::type_map_items::const_iterator iter = group->m_map_items.begin(); iter != group->m_map_items.end(); ++iter)
    {
      sharedptr<const LayoutItem> pItem = iter->second;

      sharedptr<const LayoutGroup> pGroup = sharedptr<const LayoutGroup>::cast_dynamic(pItem);
      if(pGroup)
        add_group(iterNewItem /*parent of the new group */, pGroup); //recurse
      else
      {
        Gtk::TreeModel::iterator iterChildRow = m_model_parts->append(iterNewItem->children());

        Gtk::TreeModel::Row row = *iterChildRow;

        sharedptr<LayoutItem> pCloned = sharedptr<LayoutItem>(pItem->clone());
        row[m_columns_parts.m_col_item] = pCloned;

        //sharedptr<LayoutItem_Field> pField = sharedptr<LayoutItem_Field>::cast_dynamic(pCloned);
        //if(pField)
        //  fill_full_field_details(m_table_name, pField);
      }
    }

    m_modified = true;
  }
}

//void Dialog_Layout_Report::set_document(const Glib::ustring& layout, Document_Glom* document, const Glib::ustring& table_name, const type_vecLayoutFields& table_fields)
void Dialog_Layout_Report::set_report(const Glib::ustring& table_name, const sharedptr<const Report>& report)
{
  m_modified = false;

  m_name_original = report->get_name();
  m_report = sharedptr<Report>(new Report(*report)); //Copy it, so we only use the changes when we want to.
  m_table_name = table_name;

  //Dialog_Layout::set_document(layout, document, table_name, table_fields);

  //Set the table name and title:
  m_label_table_name->set_text(table_name);

  m_entry_name->set_text(report->get_name()); 
  m_entry_title->set_text(report->get_title());

  //Update the tree models from the document

  if(true) //document)
  {


    //m_entry_table_title->set_text( document->get_table_title(table_name) );

    //document->fill_layout_field_details(m_table_name, mapGroups); //Update with full field information.

    //Show the report items:
    m_model_parts->clear();

    //guint field_sequence = 1; //0 means no sequence
    //guint group_sequence = 1; //0 means no sequence
    for(LayoutGroup::type_map_items::const_iterator iter = report->m_layout_group->m_map_items.begin(); iter != report->m_layout_group->m_map_items.end(); ++iter)
    {
      sharedptr<const LayoutItem> item = iter->second;
      sharedptr<const LayoutGroup> group = sharedptr<const LayoutGroup>::cast_dynamic(item);
      if(group)
        add_group(Gtk::TreeModel::iterator() /* null == top-level */, group);
      else
      {
        Gtk::TreeModel::iterator iter = m_model_parts->append();
        Gtk::TreeModel::Row row = *iter;
        row[m_columns_parts.m_col_item] = glom_sharedptr_clone(item);
      }
    }

    //treeview_fill_sequences(m_model_parts, m_model_parts->m_columns.m_col_sequence); //The document should have checked this already, but it does not hurt to check again.
  }

  //Open all the groups:
  m_treeview_parts->expand_all();

  m_modified = false;
}

bool Dialog_Layout_Report::may_be_child_of(const sharedptr<const LayoutItem>& parent, const sharedptr<const LayoutItem>& suggested_child)
{
  if(!parent)
    return true; //Anything may be at the top-level.

  if(!(sharedptr<const LayoutGroup>::cast_dynamic(parent)))
    return false; //Only LayoutGroup (and derived types) may have children.

  const bool fieldsummary = sharedptr<const LayoutItem_FieldSummary>::cast_dynamic(suggested_child);

  sharedptr<const LayoutItem_Summary> summary =  sharedptr<const LayoutItem_Summary>::cast_dynamic(parent);

  //A Summary may only have FieldSummary children:
  if(summary && !fieldsummary)
      return false;

  //FieldSummary may only be a member of Summary:
  if(fieldsummary && !summary)
    return false;

  return true;
}

void Dialog_Layout_Report::enable_buttons()
{
  sharedptr<LayoutItem> layout_item_available;
  bool enable_add = false;

  //Available Parts:
  Glib::RefPtr<Gtk::TreeView::Selection> refSelectionAvailable = m_treeview_available_parts->get_selection();
  if(refSelectionAvailable)
  {
    Gtk::TreeModel::iterator iter = refSelectionAvailable->get_selected();
    if(iter)
    {
      layout_item_available = (*iter)[m_columns_available_parts.m_col_item];

      enable_add = true;
    }
    else
    {
      enable_add = false;
    }
  }

  sharedptr<LayoutItem> layout_item_parent;

  //Parts:
  Glib::RefPtr<Gtk::TreeView::Selection> refSelection = m_treeview_parts->get_selection();
  if(refSelection)
  {
    Gtk::TreeModel::iterator iter = refSelection->get_selected();
    if(iter)
    {
      layout_item_parent = (*iter)[m_columns_parts.m_col_item];

      //Disable Up if It can't go any higher.
      bool enable_up = true;
      if(iter == m_model_parts->children().begin())
        enable_up = false;  //It can't go any higher.

      m_button_up->set_sensitive(enable_up);


      //Disable Down if It can't go any lower.
      Gtk::TreeModel::iterator iterNext = iter;
      iterNext++;

      bool enable_down = true;
      if(iterNext == m_model_parts->children().end())
        enable_down = false;

      m_button_down->set_sensitive(enable_down);

      m_button_delete->set_sensitive(true);

      //The [Formatting] button:
      bool enable_formatting = false;
      sharedptr<LayoutItem_Field> item_field = sharedptr<LayoutItem_Field>::cast_dynamic(layout_item_parent);
      if(item_field)
        enable_formatting = true; 

      m_button_formatting->set_sensitive(enable_formatting);

      //m_button_formatting->set_sensitive( (*iter)[m_columns_parts->m_columns.m_col_type] == TreeStore_Layout::TYPE_FIELD);
    }
    else
    {
      //Disable all buttons that act on a selection:
      m_button_down->set_sensitive(false);
      m_button_up->set_sensitive(false);
      m_button_delete->set_sensitive(false);
      m_button_formatting->set_sensitive(false);
    }

     //Not all parts may be children of all other parts.
    if(layout_item_available && layout_item_parent)
    {
      const bool may_be_child_of_parent = may_be_child_of(layout_item_parent, layout_item_available);
      enable_add = may_be_child_of_parent;

      if(!may_be_child_of_parent)
      {
        //Maybe it can be a sibling of the parent instead (and that's what would happen if Add was clicked).
        sharedptr<LayoutItem> layout_item_parent_of_parent;

        Gtk::TreeModel::iterator iterParent = iter->parent();
        if(iterParent)
          layout_item_parent_of_parent = (*iterParent)[m_columns_parts.m_col_item];

        enable_add = may_be_child_of(layout_item_parent_of_parent, layout_item_available);
      }
    }
  }

  m_button_add->set_sensitive(enable_add);
}



void Dialog_Layout_Report::on_button_delete()
{
  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_treeview_parts->get_selection();
  if(refTreeSelection)
  {
    Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
    if(iter)
    {
      m_model_parts->erase(iter);

      m_modified = true;
    }
  }
}

void Dialog_Layout_Report::on_button_up()
{
  Glib::RefPtr<Gtk::TreeView::Selection> refSelection = m_treeview_parts->get_selection();
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
        is_first = (iter == m_model_parts->children().begin());

      if(!is_first)
      {
        Gtk::TreeModel::iterator iterBefore = iter;
        --iterBefore;

        m_model_parts->iter_swap(iter, iterBefore);

        m_modified = true;
      }
    }

  }

  enable_buttons();
}

void Dialog_Layout_Report::on_button_down()
{
  Glib::RefPtr<Gtk::TreeView::Selection> refSelection = m_treeview_parts->get_selection();
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
        is_last = (iterNext == m_model_parts->children().end());

      if(!is_last)
      {
        //Swap the sequence values, so that the one before will be after:
        m_model_parts->iter_swap(iter, iterNext);

        m_modified = true;
      }
    }

  }

  enable_buttons();
}

void Dialog_Layout_Report::on_button_add()
{
  Gtk::TreeModel::iterator parent = get_selected_group_parent();
  Gtk::TreeModel::iterator available = get_selected_available();

  //Copy the available part to the list of parts:
  if(available)
  {
    sharedptr<LayoutItem> pAvailable = (*available)[m_columns_available_parts.m_col_item];

    Gtk::TreeModel::iterator iter;
    if(parent)
    {
      m_treeview_parts->expand_row( Gtk::TreePath(parent), true);
      iter = m_model_parts->append(parent->children());
    }
    else
      iter = m_model_parts->append();

    (*iter)[m_columns_parts.m_col_item] = sharedptr<LayoutItem>(pAvailable->clone());
  }

  if(parent)
    m_treeview_parts->expand_row( Gtk::TreePath(parent), true);

  enable_buttons();
}


sharedptr<Relationship> Dialog_Layout_Report::offer_relationship_list()
{
  sharedptr<Relationship> result;

  try
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_choose_relationship");

    Dialog_ChooseRelationship* dialog = 0;
    refXml->get_widget_derived("dialog_choose_relationship", dialog);

    if(dialog)
    {
      dialog->set_document(get_document(), m_table_name);
      dialog->set_transient_for(*this);
      int response = dialog->run();
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

Gtk::TreeModel::iterator Dialog_Layout_Report::get_selected_group_parent() const
{
  //Get the selected group, or a suitable parent group, or the first group:

  Gtk::TreeModel::iterator parent;

  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_treeview_parts->get_selection();
  if(refTreeSelection)
  {
    Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;
      sharedptr<LayoutItem> layout_item = row[m_columns_parts.m_col_item];
      if(sharedptr<LayoutGroup>::cast_dynamic(layout_item))
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

Gtk::TreeModel::iterator Dialog_Layout_Report::get_selected_available() const
{
  //Get the selected group, or a suitable parent group, or the first group:

  Gtk::TreeModel::iterator iter;

  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_treeview_available_parts->get_selection();
  if(refTreeSelection)
  {
    iter = refTreeSelection->get_selected();
  }

  return iter;
}


void Dialog_Layout_Report::on_button_formatting()
{
  //Get the selected item:
  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_treeview_parts->get_selection();
  if(refTreeSelection)
  {
    Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;
      sharedptr<LayoutItem> item = row[m_columns_parts.m_col_item];

      sharedptr<LayoutItem_Field> field = sharedptr<LayoutItem_Field>::cast_dynamic(item);
      if(field)
      {
        sharedptr<LayoutItem_Field> field_chosen = offer_field_formatting(field, m_table_name, this);
        if(field_chosen)
        {
          *field = *field_chosen;
          m_model_parts->row_changed(Gtk::TreePath(iter), iter);
        }
      }
    }
  }
}

void Dialog_Layout_Report::on_button_edit()
{
  //Get the selected item:
  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_treeview_parts->get_selection();
  if(refTreeSelection)
  {
    Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
    if(iter)
    {
      //Do something different for each type of item:
      Gtk::TreeModel::Row row = *iter;
      sharedptr<LayoutItem> item = row[m_columns_parts.m_col_item];

      sharedptr<LayoutItem_FieldSummary> fieldsummary = sharedptr<LayoutItem_FieldSummary>::cast_dynamic(item);
      if(fieldsummary)
      {
        try
        {
          Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_field_summary");

          Dialog_FieldSummary* dialog = 0;
          refXml->get_widget_derived("dialog_field_summary", dialog);

          if(dialog)
          {
            add_view(dialog);
            dialog->set_item(fieldsummary, m_table_name);
            dialog->set_transient_for(*this);

            const int response = dialog->run();
            dialog->hide();

            if(response == Gtk::RESPONSE_OK)
            {
              //Get the chosen relationship:
              sharedptr<LayoutItem_FieldSummary> chosenitem = dialog->get_item();
              if(chosenitem)
              {
                *fieldsummary = *chosenitem; //TODO_Performance.

                m_model_parts->row_changed(Gtk::TreePath(iter), iter); //TODO: Add row_changed(iter) to gtkmm?
              }
            }

            remove_view(dialog);
            delete dialog;
          }
        }
        catch(const Gnome::Glade::XmlError& ex)
        {
          std::cerr << ex.what() << std::endl;
        }
      }
      else
      {
        sharedptr<LayoutItem_Field> field = sharedptr<LayoutItem_Field>::cast_dynamic(item);
        if(field)
        {
          sharedptr<LayoutItem_Field> chosenitem = offer_field_list(field, m_table_name, this);
          if(chosenitem)
          {
            *field = *chosenitem; //TODO_Performance.
            m_model_parts->row_changed(Gtk::TreePath(iter), iter); //TODO: Add row_changed(iter) to gtkmm?
            m_modified = true;
          }
        }
        else
        {
          sharedptr<LayoutItem_Text> layout_item_text = sharedptr<LayoutItem_Text>::cast_dynamic(item);
          if(layout_item_text)
          {
            sharedptr<LayoutItem_Text> chosen = offer_textobject(layout_item_text);
            if(chosen)
            {
              *layout_item_text = *chosen;
              m_model_parts->row_changed(Gtk::TreePath(iter), iter); //TODO: Add row_changed(iter) to gtkmm?
            }
          }
          else
          {
            sharedptr<LayoutItem_GroupBy> group_by = sharedptr<LayoutItem_GroupBy>::cast_dynamic(item);
            if(group_by)
            {
              try
              {
                Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_group_by");

                Dialog_GroupBy* dialog = 0;
                refXml->get_widget_derived("dialog_group_by", dialog);

                if(dialog)
                {
                  add_view(dialog);
                  dialog->set_item(group_by, m_table_name);
                  dialog->set_transient_for(*this);

                  const int response = dialog->run();
                  dialog->hide();

                  if(response == Gtk::RESPONSE_OK)
                  {
                    //Get the chosen relationship:
                    sharedptr<LayoutItem_GroupBy> chosenitem = dialog->get_item();
                    if(chosenitem)
                    {
                      *group_by = *chosenitem;
                      m_model_parts->row_changed(Gtk::TreePath(iter), iter); //TODO: Add row_changed(iter) to gtkmm?
                    }
                  }

                  remove_view(dialog);
                  delete dialog;
                }
              }
              catch(const Gnome::Glade::XmlError& ex)
              {
                std::cerr << ex.what() << std::endl;
              }
            }
          }
        }
      }
    }
  }
}

void Dialog_Layout_Report::save_to_document()
{
  Dialog_Layout::save_to_document();

/*
  if(m_modified)
  {
    //Set the table name and title:
    Document_Glom* document = get_document();
    //if(document)
    //  document->set_table_title( m_table_name, m_entry_table_title->get_text() );

    //Get the data from the TreeView and store it in the document:

    //Fill the sequences:
    //(This model is not sorted - we need to set the sequence numbers based on the order).
    //m_model_parts->fill_sequences();

    //Get the groups and their fields:
    Document_Glom::type_mapLayoutGroupSequence mapGroups;
    guint group_sequence = 1; //0 means no sequence

    //Add the layout items:
    //guint field_sequence = 1; //0 means no sequence
    for(Gtk::TreeModel::iterator iterFields = m_model_parts->children().begin(); iterFields != m_model_parts->children().end(); ++iterFields)
    {
      //Gtk::TreeModel::Row row = *iterFields;
      //if(row[m_columns_parts->m_columns.m_col_type] == TreeStore_Layout::TYPE_GROUP) //There may be top-level groups, but no top-level fields, because the fields must be in a group (so that they are in columns)
      {
        LayoutGroup group;
        group->m_sequence = group_sequence;
        fill_group(iterFields, group);

        mapGroups[group_sequence] = group;
        ++group_sequence;
      }
    }

    if(document)
    {
      document->set_data_layout_groups(m_layout_name, m_table_name, mapGroups);
      m_modified = false;
    }
  }
*/
}

void Dialog_Layout_Report::on_treeview_available_parts_selection_changed()
{
  enable_buttons();
}

void Dialog_Layout_Report::on_treeview_parts_selection_changed()
{
  enable_buttons();
}

void Dialog_Layout_Report::on_cell_data_part(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter)
{
  //TODO: If we ever use this as a real layout tree, then let's add icons for each type.

  //Set the view's cell properties depending on the model's data:
  Gtk::CellRendererText* renderer_text = dynamic_cast<Gtk::CellRendererText*>(renderer);
  if(renderer_text)
  {
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;
      sharedptr<LayoutItem> pItem = row[m_columns_parts.m_col_item];
      const Glib::ustring part = pItem->get_part_type_name();

      renderer_text->property_text() = part;
      renderer_text->property_editable() = false; //Part names can never be edited.
    }
  }
}

void Dialog_Layout_Report::on_cell_data_details(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter)
{
//Set the view's cell properties depending on the model's data:
  Gtk::CellRendererText* renderer_text = dynamic_cast<Gtk::CellRendererText*>(renderer);
  if(renderer_text)
  {
    if(iter)
    {
      Glib::ustring text;

      Gtk::TreeModel::Row row = *iter;
      sharedptr<LayoutItem> pItem = row[m_columns_parts.m_col_item];

      sharedptr<LayoutItem_GroupBy> pGroup = sharedptr<LayoutItem_GroupBy>::cast_dynamic(pItem);
      if(pGroup)
      {
        //TODO: Internationalize this properly:
        if(pGroup->get_has_field_group_by())
          text = pGroup->get_field_group_by()->get_layout_display_name();

        if(pGroup->get_has_field_sort_by())
          text += "(sort by: " + pGroup->get_field_sort_by()->get_layout_display_name() + ")";
      }
      else
      {
        /*
        const LayoutItem_FieldSummary* pFieldSummary = dynamic_cast<const LayoutItem_FieldSummary*>(pItem);
        if(pFieldSummary)
        {
           text = pFieldSummary.LayoutItem_Field::get_layout_display_name_field();
        }
        else
        {
        */
          sharedptr<LayoutItem_Field> pField = sharedptr<LayoutItem_Field>::cast_dynamic(pItem);
          if(pField)
          {
            text = pField->get_layout_display_name();
          }
        //}
      }

      renderer_text->property_text() = text;
      renderer_text->property_editable() = false;
    }
  }
}


void Dialog_Layout_Report::on_cell_data_available_part(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter)
{
  //TODO: If we ever use this as a real layout tree, then let's add icons for each type.

  //Set the view's cell properties depending on the model's data:
  Gtk::CellRendererText* renderer_text = dynamic_cast<Gtk::CellRendererText*>(renderer);
  if(renderer_text)
  {
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;
      sharedptr<LayoutItem> pItem = row[m_columns_available_parts.m_col_item];
      Glib::ustring part = pItem->get_part_type_name();

      renderer_text->property_text() = part;
      renderer_text->property_editable() = false; //Part names can never be edited.
    }
  }
}

Glib::ustring Dialog_Layout_Report::get_original_report_name() const
{
  return m_name_original;
}

sharedptr<Report> Dialog_Layout_Report::get_report()
{
  m_report->set_name( m_entry_name->get_text() );
  m_report->set_title( m_entry_title->get_text() );

  m_report->m_layout_group->remove_all_items();

  guint item_sequence = 0;
  m_report->m_layout_group->remove_all_items();

  for(Gtk::TreeModel::iterator iter = m_model_parts->children().begin(); iter != m_model_parts->children().end(); ++iter)
  {
    //Recurse into a group if necessary:
    sharedptr<LayoutGroup> group = fill_group(iter);
    if(group)
    {
      //Add the group:
      group->m_sequence = item_sequence;

      m_report->m_layout_group->m_map_items[item_sequence] = group;
      ++item_sequence;
    }
    else
    {
      sharedptr<LayoutItem> item = (*iter)[m_columns_parts.m_col_item];
      if(item)
      {
        item->m_sequence = item_sequence;

        m_report->m_layout_group->m_map_items[item_sequence] = item;
        ++item_sequence;
      }
    }
  }

  return m_report;
}










