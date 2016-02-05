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

#include <glom/mode_design/layout/dialog_layout_details.h>
#include <glom/mode_design/layout/dialog_choose_relationship.h>
#include <glom/mode_design/layout/layout_item_dialogs/dialog_buttonscript.h>
#include <glom/glade_utils.h>
#include <glom/frame_glom.h> //For show_ok_dialog()
//#include <libgnome/gnome-i18n.h>
#include <glom/utils_ui.h> //For bold_message()).
#include <glom/appwindow.h>
#include <glibmm/i18n.h>
#include <sstream> //For stringstream

namespace Glom
{

const char* Dialog_Layout_Details::glade_id("window_data_layout");
const bool Dialog_Layout_Details::glade_developer(true);

Dialog_Layout_Details::Dialog_Layout_Details(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Dialog_Layout(cobject, builder),
  m_treeview_fields(nullptr),
  m_treeview_column_title(nullptr),
  m_treeview_column_group_columns(nullptr),
  m_treeview_column_column_width(nullptr),
  m_box_table_widgets(nullptr),
  m_box_related_table_widgets(nullptr),
  m_box_related_navigation(nullptr),
  m_button_up(nullptr),
  m_button_down(nullptr),
  m_button_add_field(nullptr),
  m_button_add_group(nullptr),
  m_button_add_notebook(nullptr),
  m_button_add_related(nullptr),
  m_button_add_related_calendar(nullptr),
  m_button_add_button(nullptr),
  m_button_add_text(nullptr),
  m_button_add_image(nullptr),
  m_button_field_delete(nullptr),
  m_button_formatting(nullptr),
  m_button_edit(nullptr),
  m_label_table_name(nullptr),
  m_hbox_rows_count(nullptr),
  m_spinbutton_rows_count_min(nullptr),
  m_spinbutton_rows_count_max(nullptr)
{
  // Get the alternate sets of widgets, only one of which should be shown:
  // Derived classes will hide one and show the other:
  builder->get_widget("hbox_table_widgets", m_box_table_widgets);
  m_box_table_widgets->show();
  builder->get_widget("hbox_related_table_widgets", m_box_related_table_widgets);
  m_box_related_table_widgets->hide();
  builder->get_widget("frame_related_table_navigation", m_box_related_navigation);
  m_box_related_navigation->hide();
  builder->get_widget("frame_lines", m_box_frame_lines);
  m_box_frame_lines->hide();

  Gtk::Frame* box_calendar = nullptr;
  builder->get_widget("frame_calendar", box_calendar);
  box_calendar->hide();

  builder->get_widget("label_table_name", m_label_table_name);
  
  //This is only shown in Dialog_Layout_List_Related:
  builder->get_widget("hbox_rows_count", m_hbox_rows_count);
  builder->get_widget("spinbutton_rows_count_min", m_spinbutton_rows_count_min);
  builder->get_widget("spinbutton_rows_count_max", m_spinbutton_rows_count_max);
  m_hbox_rows_count->hide();
  
  builder->get_widget("treeview_fields", m_treeview_fields);
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
    auto column_name = Gtk::manage( new Gtk::TreeView::Column(_("Name")) );
    m_treeview_fields->append_column(*column_name);

    auto renderer_name = Gtk::manage(new Gtk::CellRendererText);
    column_name->pack_start(*renderer_name);
    column_name->set_cell_data_func(*renderer_name, sigc::mem_fun(*this, &Dialog_Layout_Details::on_cell_data_name));

    //Connect to its signal:
    renderer_name->property_editable() = true;
    renderer_name->signal_edited().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_treeview_cell_edited_name) );


    //Title column:
    m_treeview_column_title = Gtk::manage( new Gtk::TreeView::Column(_("Title")) );
    m_treeview_fields->append_column(*m_treeview_column_title);

    auto renderer_title = Gtk::manage(new Gtk::CellRendererText);
    m_treeview_column_title->pack_start(*renderer_title);
    m_treeview_column_title->set_cell_data_func(*renderer_title, sigc::mem_fun(*this, &Dialog_Layout_Details::on_cell_data_title));

    //Connect to its signal:
    renderer_title->signal_edited().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_treeview_cell_edited_title) );


    //Columns-count column:
    //Note to translators: This is the number of columns in a group (group being a noun)
    m_treeview_column_group_columns = Gtk::manage( new Gtk::TreeView::Column(_("Group Columns")) );
    m_treeview_fields->append_column(*m_treeview_column_group_columns);

    auto renderer_count = Gtk::manage(new Gtk::CellRendererText);
    m_treeview_column_group_columns->pack_start(*renderer_count);
    m_treeview_column_group_columns->set_cell_data_func(*renderer_count, sigc::mem_fun(*this, &Dialog_Layout_Details::on_cell_data_group_columns));

    //Connect to its signal:
    renderer_count->signal_edited().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_treeview_cell_edited_group_columns) );


    //Column-Width column: (only for list views)
    //Note to translators: This is a name (the width of a UI element in the display), not an action.
    m_treeview_column_column_width = Gtk::manage( new Gtk::TreeView::Column(_("Display Width")) );
    m_treeview_fields->append_column(*m_treeview_column_column_width);

    auto renderer_column_width = Gtk::manage(new Gtk::CellRendererText);
    m_treeview_column_column_width->pack_start(*renderer_column_width);
    m_treeview_column_column_width->set_cell_data_func(*renderer_column_width, sigc::mem_fun(*this, &Dialog_Layout_Details::on_cell_data_column_width));

    //Connect to its signal:
    renderer_column_width->signal_edited().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_treeview_cell_edited_column_width) );

    //Hide this column because we don't need it for the details layout.
    //It is made visible by the (derived) list layout class.
    m_treeview_column_column_width->set_visible(false);


    //Respond to changes of selection:
    auto refSelection = m_treeview_fields->get_selection();
    if(refSelection)
    {
      refSelection->signal_changed().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_treeview_fields_selection_changed) );
    }

    m_model_items->signal_row_changed().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_treemodel_row_changed) );
  }

  builder->get_widget("button_up", m_button_up);
  m_button_up->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_button_up) );

  builder->get_widget("button_down", m_button_down);
  m_button_down->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_button_down) );

  builder->get_widget("button_field_delete", m_button_field_delete);
  m_button_field_delete->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_button_field_delete) );

  builder->get_widget("button_formatting", m_button_formatting);
  m_button_formatting->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_button_formatting) );

  builder->get_widget("button_add_field", m_button_add_field);
  m_button_add_field->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_button_add_field) );

  builder->get_widget("button_add_group", m_button_add_group);
  m_button_add_group->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_button_add_group) );

  builder->get_widget("button_add_notebook", m_button_add_notebook);
  m_button_add_notebook->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_button_add_notebook) );

  builder->get_widget("button_add_related", m_button_add_related);
  m_button_add_related->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_button_add_related) );

  builder->get_widget("button_add_related_calendar", m_button_add_related_calendar);
  m_button_add_related_calendar->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_button_add_related_calendar) );

  builder->get_widget("button_add_button", m_button_add_button);
  m_button_add_button->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_button_add_button) );

  builder->get_widget("button_add_text", m_button_add_text);
  m_button_add_text->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_button_add_text) );

  builder->get_widget("button_add_image", m_button_add_image);
  m_button_add_image->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_button_add_image) );

  builder->get_widget("button_edit", m_button_edit);
  m_button_edit->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Details::on_button_edit) );

  //show_all_children();
}

void Dialog_Layout_Details::fill_group(const Gtk::TreeModel::iterator& iter, std::shared_ptr<LayoutGroup>& group)
{
  auto portal = std::dynamic_pointer_cast<LayoutItem_Portal>(group);
  if(portal)
    return; //This method is not for portals.

  if(iter)
  {
    Gtk::TreeModel::Row row = *iter;
    std::shared_ptr<LayoutItem> layout_item_top = row[m_model_items->m_columns.m_col_layout_item];
    auto group_row = std::dynamic_pointer_cast<LayoutGroup>(layout_item_top);
    if(!group_row)
      return;

    auto portal_row = std::dynamic_pointer_cast<LayoutItem_Portal>(group_row);
    if(portal_row) //This is only for groups.
      return;

    *group = *group_row; //Copy name, title, columns count, etc.
    group->remove_all_items(); //Remove the copied child items, if any, so we can add them.

    //Get child layout items:
    for(const auto& rowChild : row.children())
    {
      std::shared_ptr<LayoutItem> layout_item = rowChild[m_model_items->m_columns.m_col_layout_item];

      auto layout_portal = std::dynamic_pointer_cast<LayoutItem_Portal>(layout_item);
      if(layout_portal)
      {
        //std::cout << "debug: " << G_STRFUNC << ": adding portal." << std::endl;
        group->add_item(glom_sharedptr_clone(layout_portal));
      }
      else
      {
        //std::cout << "debug: " << G_STRFUNC << ": adding group." << std::endl;
        auto layout_group = std::dynamic_pointer_cast<LayoutGroup>(layout_item);
        if(layout_group && !layout_portal)
        {
          //Recurse:
          auto group_child = glom_sharedptr_clone(layout_group);
          fill_group(rowChild, group_child);
          group->add_item(group_child);
        }
        else if(layout_item)
        {
          //std::cout << "debug: " << G_STRFUNC << ": adding item." << std::endl;

          //Add field or button:
          auto item = glom_sharedptr_clone(layout_item);
          group->add_item(item);
        }
      }
    }
  }
}


void Dialog_Layout_Details::add_group(const Gtk::TreeModel::iterator& parent, const std::shared_ptr<const LayoutGroup>& group)
{
  if(!group)
   return;

  auto parent_portal = std::dynamic_pointer_cast<const LayoutItem_Portal>(group);
  if(parent_portal)
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

    auto group_inserted = glom_sharedptr_clone(group);
    group_inserted->remove_all_items();
    rowGroup[m_model_items->m_columns.m_col_layout_item] = group_inserted;

    //Add the child items: (TODO_Performance: The child items are ignored/wasted because we clone them again.)
    LayoutGroup::type_list_const_items items = group->get_items();
    for(const auto& item : items)
    {
      auto portal = std::dynamic_pointer_cast<const LayoutItem_Portal>(item);
      if(portal) //If it is a portal
      {
        //Handle this differently to regular groups, so we do not also add its children:
        auto iter = m_model_items->append(iterNewGroup->children());
        Gtk::TreeModel::Row row = *iter;
        row[m_model_items->m_columns.m_col_layout_item] = glom_sharedptr_clone(portal);
      }
      else
      {
        auto child_group = std::dynamic_pointer_cast<const LayoutGroup>(item);
        if(child_group) //If it is a group:
          add_group(iterNewGroup, child_group); //recursive
        else
        {
          //Add the item to the treeview:
          auto iter = m_model_items->append(iterNewGroup->children());
          Gtk::TreeModel::Row row = *iter;
          row[m_model_items->m_columns.m_col_layout_item] = glom_sharedptr_clone(item);
        }
      }
    }

    m_modified = true;
  }
}

void Dialog_Layout_Details::init(const Glib::ustring& layout_name, const Glib::ustring& layout_platform, const std::shared_ptr<Document>& document, const Glib::ustring& table_name, const type_vecConstLayoutFields& table_fields)
{
  m_modified = false;

  Dialog_Layout::init(layout_name, layout_platform, document, table_name, table_fields);

  //Update the tree models from the document
  if(document)
  {
    //Set the table name and title:
    m_label_table_name->set_text(table_name);
    m_entry_table_title->set_text( document->get_table_title(table_name, AppWindow::get_current_locale()) );

    Document::type_list_layout_groups list_groups = document->get_data_layout_groups_plus_new_fields(m_layout_name, m_table_name, m_layout_platform);
    document->fill_layout_field_details(m_table_name, list_groups); //Update with full field information.

    //If no information is stored in the document, add a default group

    if(list_groups.empty())
    {
      auto group = std::make_shared<LayoutGroup>();
      group->set_name("main");
      group->set_columns_count(1);

      list_groups.emplace_back(group);
    }

    //Show the field layout
    //typedef std::list< Glib::ustring > type_listStrings;

    m_model_items->clear();

    for(const auto& group : list_groups)
    {
      auto portal = std::dynamic_pointer_cast<const LayoutItem_Portal>(group);
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
  auto refSelection = m_treeview_fields->get_selection();
  if(refSelection)
  {
    auto iter = refSelection->get_selected();
    if(iter)
    {
      //Disable Up if It can't go any higher.
      bool enable_up = true;
      auto iterParent = iter->parent();
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

      m_button_up->set_sensitive(enable_up);


      //Disable Down if It can't go any lower.
      auto iterNext = iter;
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

      m_button_down->set_sensitive(enable_down);

      m_button_field_delete->set_sensitive(true);

      //Only some items have formatting:
      std::shared_ptr<LayoutItem> layout_item = (*iter)[m_model_items->m_columns.m_col_layout_item];
      auto layoutitem_withformatting = std::dynamic_pointer_cast<LayoutItem_WithFormatting>(layout_item);
      const bool is_field = (bool)layoutitem_withformatting;
      m_button_formatting->set_sensitive(is_field);
    }
    else
    {
      //Disable all buttons that act on a selection:
      m_button_down->set_sensitive(false);
      m_button_up->set_sensitive(false);
      m_button_field_delete->set_sensitive(false);
      m_button_formatting->set_sensitive(false);
    }
  }

}



void Dialog_Layout_Details::on_button_field_delete()
{
  auto refTreeSelection = m_treeview_fields->get_selection();
  if(refTreeSelection)
  {
    auto iter = refTreeSelection->get_selected();
    if(iter)
    {
      m_model_items->erase(iter);

      m_modified = true;
    }
  }
}

void Dialog_Layout_Details::on_button_up()
{
  auto refSelection = m_treeview_fields->get_selection();
  if(refSelection)
  {
    auto iter = refSelection->get_selected();
    if(iter)
    {
      auto parent = iter->parent();
      bool is_first = false;
      if(parent)
        is_first = (iter == parent->children().begin());
      else
        is_first = (iter == m_model_items->children().begin());

      if(!is_first)
      {
        auto iterBefore = iter;
        --iterBefore;

        m_model_items->iter_swap(iter, iterBefore);

        m_modified = true;
      }
    }

  }

  enable_buttons();
}

void Dialog_Layout_Details::on_button_down()
{
  auto refSelection = m_treeview_fields->get_selection();
  if(refSelection)
  {
    auto iter = refSelection->get_selected();
    if(iter)
    {
      auto iterNext = iter;
      iterNext++;

      auto parent = iter->parent();
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

void Dialog_Layout_Details::on_button_add_field()
{
  type_list_field_items fields_list = offer_field_list(m_table_name, this);
  for(const auto& layout_item : fields_list)
  {
    if(!layout_item)
      continue;

    //Add the field details to the layout treeview:
    auto iter = append_appropriate_row();
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;
      row[m_model_items->m_columns.m_col_layout_item] = glom_sharedptr_clone(layout_item);

      //Scroll to, and select, the new row:
      auto refTreeSelection = m_treeview_fields->get_selection();
      if(refTreeSelection)
        refTreeSelection->select(iter);

      m_treeview_fields->scroll_to_row( Gtk::TreeModel::Path(iter) );

      m_modified = true;
    }
  }

  enable_buttons();
}


std::shared_ptr<LayoutItem_Button> Dialog_Layout_Details::offer_button_script_edit(const std::shared_ptr<const LayoutItem_Button>& button)
{
  std::shared_ptr<LayoutItem_Button> result;

  Dialog_ButtonScript* dialog = nullptr;
  Glom::Utils::get_glade_widget_derived_with_warning(dialog);
  if(!dialog) //Unlikely and it already warns on stderr.
    return result;

  dialog->set_script(button, m_table_name);
  dialog->set_transient_for(*this);
  const auto dialog_response = Glom::UiUtils::dialog_run_with_help(dialog);
  dialog->hide();
  if(dialog_response == Gtk::RESPONSE_OK)
  {
    //Get the chosen relationship:
     result = dialog->get_script();
  }

  delete dialog;

  return result;
}

std::shared_ptr<Relationship> Dialog_Layout_Details::offer_relationship_list()
{
  return offer_relationship_list(std::shared_ptr<Relationship>());
}

std::shared_ptr<Relationship> Dialog_Layout_Details::offer_relationship_list(const std::shared_ptr<const Relationship>& item)
{
  auto result = glom_sharedptr_clone(item);

  Dialog_ChooseRelationship* dialog = nullptr;
  Utils::get_glade_widget_derived_with_warning(dialog);
  if(!dialog) //Unlikely and it already warns on stderr.
    return result;

  dialog->set_document(get_document(), m_table_name);
  dialog->select_item(item);
  dialog->set_transient_for(*this);
  const auto dialog_response = dialog->run();
  dialog->hide();
  if(dialog_response == Gtk::RESPONSE_OK)
  {
    //Get the chosen relationship:
    result = dialog->get_relationship_chosen();
  }

  delete dialog;

  return result;
}

Gtk::TreeModel::iterator Dialog_Layout_Details::append_appropriate_row()
{
  Gtk::TreeModel::iterator result;

  auto parent = get_selected_group_parent();

  //Add the field details to the layout treeview:
  if(parent)
  {
    result = m_model_items->append(parent->children());
  }
  else
  {
    //Find the first group, and make the new row a child of that:
    auto iter_first = m_model_items->children().begin();
    if(iter_first)
    {
      Gtk::TreeModel::Row row = *iter_first;

      std::shared_ptr<LayoutItem> layout_item = row[m_model_items->m_columns.m_col_layout_item];
      auto layout_group = std::dynamic_pointer_cast<LayoutGroup>(layout_item);
      auto layout_portal = std::dynamic_pointer_cast<LayoutItem_Portal>(layout_item);

      if(layout_group && !layout_portal)
        result = m_model_items->append(iter_first->children());
    }

    if(!result)
    {
      //For instance, for a Dialog_Layout_List_Related derived class, with a portal as the top-level group:
      result = m_model_items->append();
    }
  }

  return result;
}

void Dialog_Layout_Details::on_button_add_button()
{
  auto iter = append_appropriate_row();
  if(iter)
  {
    Gtk::TreeModel::Row row = *iter;

    //Add a new button:
    auto button = std::make_shared<LayoutItem_Button>();
    button->set_title_original(_("New Button")); //Give the button a default title, so it is big enough, and so people see that they should change it.
    row[m_model_items->m_columns.m_col_layout_item] = button;

    //Scroll to, and select, the new row:
    auto refTreeSelection = m_treeview_fields->get_selection();
    if(refTreeSelection)
      refTreeSelection->select(iter);

    m_treeview_fields->scroll_to_row( Gtk::TreeModel::Path(iter) );

    m_modified = true;
  }

  enable_buttons();
}

void Dialog_Layout_Details::on_button_add_text()
{
  auto iter = append_appropriate_row();
  if(iter)
  {
    Gtk::TreeModel::Row row = *iter;

    //Add a new button:
    auto textobject = std::make_shared<LayoutItem_Text>();
    textobject->set_title_original(_("Text Title")); //Give the button a default title, so it is big enough, and so people see that they should change it.
    row[m_model_items->m_columns.m_col_layout_item] = textobject;

    //Scroll to, and select, the new row:
    auto refTreeSelection = m_treeview_fields->get_selection();
    if(refTreeSelection)
      refTreeSelection->select(iter);

    m_treeview_fields->scroll_to_row( Gtk::TreeModel::Path(iter) );

    m_modified = true;
  }

  enable_buttons();
}

void Dialog_Layout_Details::on_button_add_image()
{
  auto iter = append_appropriate_row();
  if(iter)
  {
    Gtk::TreeModel::Row row = *iter;

    //Add a new button:
    auto imageobject = std::make_shared<LayoutItem_Image>();
    imageobject->set_title_original(_("Image Title")); //Give the item a default title, so it is big enough, and so people see that they should change it.
    row[m_model_items->m_columns.m_col_layout_item] = imageobject;

    //Scroll to, and select, the new row:
    auto refTreeSelection = m_treeview_fields->get_selection();
    if(refTreeSelection)
      refTreeSelection->select(iter);

    m_treeview_fields->scroll_to_row( Gtk::TreeModel::Path(iter) );

    m_modified = true;
  }

  enable_buttons();
}

void Dialog_Layout_Details::on_button_add_notebook()
{
  auto iter = append_appropriate_row();
  if(iter)
  {
    Gtk::TreeModel::Row row = *iter;

    auto notebook = std::make_shared<LayoutItem_Notebook>();
    notebook->set_name(_("notebook"));
    row[m_model_items->m_columns.m_col_layout_item] = notebook;

    //Scroll to, and select, the new row:
    auto refTreeSelection = m_treeview_fields->get_selection();
    if(refTreeSelection)
      refTreeSelection->select(iter);

    m_treeview_fields->scroll_to_row( Gtk::TreeModel::Path(iter) );

    m_modified = true;
  }

  enable_buttons();
}

void Dialog_Layout_Details::on_button_add_related()
{
  /* We don't need to ask this because the portal layout dialog can now handle an empty portal:
  auto relationship = offer_relationship_list();
  if(relationship)
  {
  */
    auto iter = append_appropriate_row();
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;

      auto portal = std::make_shared<LayoutItem_Portal>();
      //portal->set_relationship(relationship);
      row[m_model_items->m_columns.m_col_layout_item] = portal;

      //Scroll to, and select, the new row:
      auto refTreeSelection = m_treeview_fields->get_selection();
      if(refTreeSelection)
        refTreeSelection->select(iter);

      m_treeview_fields->scroll_to_row( Gtk::TreeModel::Path(iter) );

      m_modified = true;
    }
  /*
  }
  */

  enable_buttons();
}

void Dialog_Layout_Details::on_button_add_related_calendar()
{
  /* We don't need to ask this because the portal layout dialog can now handle an empty portal:
  auto relationship = offer_relationship_list();
  if(relationship)
  {
  */
    auto iter = append_appropriate_row();
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;

      auto portal = std::make_shared<LayoutItem_CalendarPortal>();
      //portal->set_relationship(relationship);
      row[m_model_items->m_columns.m_col_layout_item] = portal;

      //Scroll to, and select, the new row:
      auto refTreeSelection = m_treeview_fields->get_selection();
      if(refTreeSelection)
        refTreeSelection->select(iter);

      m_treeview_fields->scroll_to_row( Gtk::TreeModel::Path(iter) );

      m_modified = true;
    }
  //}

  enable_buttons();
}

Gtk::TreeModel::iterator Dialog_Layout_Details::get_selected_group_parent() const
{
  //Get the selected group, or a suitable parent group, or the first group:

  Gtk::TreeModel::iterator parent;

  auto refTreeSelection = m_treeview_fields->get_selection();
  if(refTreeSelection)
  {
    auto iter = refTreeSelection->get_selected();
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;

      std::shared_ptr<LayoutItem> layout_item = row[m_model_items->m_columns.m_col_layout_item];
      auto layout_group = std::dynamic_pointer_cast<LayoutGroup>(layout_item);
      auto layout_portal = std::dynamic_pointer_cast<LayoutItem_Portal>(layout_item);

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

void Dialog_Layout_Details::on_button_add_group()
{
  auto parent = get_selected_group_parent();

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
    auto layout_item = std::make_shared<LayoutGroup>();
    layout_item->set_name(_("group"));
    row[m_model_items->m_columns.m_col_layout_item] = layout_item;

    //Scroll to, and select, the new row:
    auto refTreeSelection = m_treeview_fields->get_selection();
    if(refTreeSelection)
      refTreeSelection->select(iterNewGroup);

    m_treeview_fields->scroll_to_row( Gtk::TreeModel::Path(iterNewGroup) );

    m_modified = true;
  }

  enable_buttons();
}

void Dialog_Layout_Details::on_button_formatting()
{
  //TODO: Abstract this into the base class:

  //Get the selected item:
  auto refTreeSelection = m_treeview_fields->get_selection();
  if(refTreeSelection)
  {
    auto iter = refTreeSelection->get_selected();
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;

      std::shared_ptr<LayoutItem> layout_item = row[m_model_items->m_columns.m_col_layout_item];
      auto field = std::dynamic_pointer_cast<LayoutItem_Field>(layout_item);
      if(field)
      {
        //Handle field formatting, which includes more than the generic formatting stuff:
        auto chosenitem = offer_field_formatting(field, get_fields_table(), this, m_editable_layout);
        if(chosenitem)
        {
          *field = *chosenitem; //TODO_Performance.
          m_modified = true;
          //m_model_parts->row_changed(Gtk::TreeModel::Path(iter), iter); //TODO: Add row_changed(iter) to gtkmm?
        }
      }
      else
      {
        //Handle any other items that can have formatting:
        auto withformatting = std::dynamic_pointer_cast<LayoutItem_WithFormatting>(layout_item);
        if(withformatting)
        {
          const auto changed = offer_non_field_item_formatting(withformatting, this);
          if(changed)
            m_modified = true;
        }
      }
    }
  }
}

void Dialog_Layout_Details::on_button_edit()
{
  //Get the selected item:
  auto refTreeSelection = m_treeview_fields->get_selection();
  if(refTreeSelection)
  {
    auto iter = refTreeSelection->get_selected();
    if(iter)
    {
      //Do something different for each type of item:
      //This is unpleasant, but so is this whole dialog.
      //This whole dialog is just a temporary way to edit the layout before we have a visual DnD way.
      Gtk::TreeModel::Row row = *iter;

      std::shared_ptr<LayoutItem> layout_item = row[m_model_items->m_columns.m_col_layout_item];

      auto layout_portal = std::dynamic_pointer_cast<LayoutItem_Portal>(layout_item);
      if(layout_portal)
      {
        auto relationship = offer_relationship_list(layout_portal->get_relationship());
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
        auto layout_notebook = std::dynamic_pointer_cast<LayoutItem_Notebook>(layout_item);
        if(layout_notebook)
        {
          Frame_Glom::show_ok_dialog(_("Notebook Tabs"), _("Add child groups to the notebook to add tabs."), *this);
        }
        else
        {
          auto layout_group = std::dynamic_pointer_cast<LayoutGroup>(layout_item);
          if(layout_group)
          {
            Gtk::TreeModel::Path path = m_model_items->get_path(iter);
            m_treeview_fields->set_cursor(path, *m_treeview_column_title, true /* start_editing */);
          }
          else
          {
            auto layout_item_field = std::dynamic_pointer_cast<LayoutItem_Field>(layout_item);
            if(layout_item_field)
            {
              auto chosenitem = offer_field_list_select_one_field(layout_item_field, m_table_name, this);
              if(chosenitem)
              {
                *layout_item_field = *chosenitem;
                //row[m_model_items->m_columns.m_col_layout_item] = chosenitem;
                m_modified = true;
              }
            }
            else
            {
              auto layout_item_button = std::dynamic_pointer_cast<LayoutItem_Button>(layout_item);
              if(layout_item_button)
              {
                auto chosen = offer_button_script_edit(layout_item_button);
                if(chosen)
                {
                  *layout_item_button = *chosen;
                  //std::cout << "script: " << layout_item_button->get_script() << std::endl;

                  m_modified = true;
                }
              }
              else
              {
                auto layout_item_text = std::dynamic_pointer_cast<LayoutItem_Text>(layout_item);
                if(layout_item_text)
                {
                  auto chosen = offer_textobject(layout_item_text);
                  if(chosen)
                  {
                    *layout_item_text = *chosen;
                    //std::cout << "script: " << layout_item_button->get_script() << std::endl;

                    m_modified = true;
                  }
                }
                else
                {
                  auto layout_item_image = std::dynamic_pointer_cast<LayoutItem_Image>(layout_item);
                  if(layout_item_image)
                  {
                    auto chosen = offer_imageobject(layout_item_image);
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
    auto document = get_document();
    if(document)
      document->set_table_title( m_table_name, m_entry_table_title->get_text(), AppWindow::get_current_locale());

    //Get the data from the TreeView and store it in the document:

    //Fill the sequences:
    //(This model is not sorted - we need to set the sequence numbers based on the order).
    m_model_items->fill_sequences();

    //Get the groups and their fields:
    Document::type_list_layout_groups list_groups;

    //Add the layout items:
    for(const auto& row : m_model_items->children())
    {
      std::shared_ptr<LayoutItem> layout_item = row[m_model_items->m_columns.m_col_layout_item];

      //There may be top-level groups, but no top-level fields, because the fields must be in a group (so that they are in columns)
      auto layout_group = std::dynamic_pointer_cast<LayoutGroup>(layout_item);
      if(!layout_group)
        continue;

      //TODO: This is very ugly:
      auto layout_portal = std::dynamic_pointer_cast<LayoutItem_Portal>(layout_item);
      auto layout_calendar_portal = std::dynamic_pointer_cast<LayoutItem_CalendarPortal>(layout_item);
      auto layout_notebook = std::dynamic_pointer_cast<LayoutItem_Notebook>(layout_item);
      std::shared_ptr<LayoutGroup> group;
      if(layout_portal) {
        group = std::make_shared<LayoutItem_Portal>();
      } else if(layout_calendar_portal) {
        group = std::make_shared<LayoutItem_CalendarPortal>();
      } else if(layout_notebook) {
        group = std::make_shared<LayoutItem_Notebook>();
      } else {
        group = std::make_shared<LayoutGroup>();
      }

      fill_group(row, group);

      list_groups.emplace_back(group);
    }

    if(document)
    {
      document->set_data_layout_groups(m_layout_name, m_table_name, m_layout_platform, list_groups);
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
  auto renderer_text = dynamic_cast<Gtk::CellRendererText*>(renderer);
  if(renderer_text)
  {
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;

      std::shared_ptr<LayoutItem> layout_item = row[m_model_items->m_columns.m_col_layout_item];

      //Use the markup property instead of the text property, so that we can give the text some style:
      Glib::ustring markup;

      bool is_group = false;

      auto layout_portal = std::dynamic_pointer_cast<LayoutItem_Portal>(layout_item);
      if(layout_portal)
      {
        auto layout_calendar = std::dynamic_pointer_cast<LayoutItem_CalendarPortal>(layout_portal);
        if(layout_calendar)
          markup = Glib::ustring::compose(_("Related Calendar: %1"), layout_portal->get_relationship_name());
        else
          markup = Glib::ustring::compose(_("Related List: %1"), layout_portal->get_relationship_name());
      }
      else
      {
        auto layout_group = std::dynamic_pointer_cast<LayoutGroup>(layout_item);
        if(layout_group)
        {
          is_group = true;

          //Make group names bold:
          markup = UiUtils::bold_message( layout_item->get_name() );
        }
        else
        {
          auto layout_item_field = std::dynamic_pointer_cast<LayoutItem_Field>(layout_item);
          if(layout_item_field)
          {
            markup = Glib::ustring::compose(_("Field: %1"), layout_item_field->get_layout_display_name());

            //Just for debugging:
            //if(!row[m_model_items->m_columns.m_col_editable])
            // markup += " *";
          }
          else
          {
            auto layout_item_button = std::dynamic_pointer_cast<LayoutItem_Button>(layout_item);
            if(layout_item_button)
            {
              markup = _("Button"); //Buttons don't have names - just titles. TODO: Would they be useful?
            }
            else
            {
              auto layout_item_text = std::dynamic_pointer_cast<LayoutItem_Text>(layout_item);
              if(layout_item_text)
              {
                markup = _("Text"); //Text objects don't have names - just titles. TODO: Would they be useful?
              }
              else
              {
                auto layout_item_image = std::dynamic_pointer_cast<LayoutItem_Image>(layout_item);
                if(layout_item_image)
                {
                  markup = _("Image"); //Image objects don't have names - just titles. TODO: Would they be useful?
                }
                else if(layout_item)
                  markup = layout_item->get_name();
                else
                  markup = Glib::ustring();
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
  auto renderer_text = dynamic_cast<Gtk::CellRendererText*>(renderer);
  if(renderer_text)
  {
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;
      std::shared_ptr<LayoutItem> layout_item = row[m_model_items->m_columns.m_col_layout_item];
      auto layout_notebook = std::dynamic_pointer_cast<LayoutItem_Notebook>(layout_item);
      if(layout_notebook)
        renderer_text->property_text() = _("(Notebook)");
      else if(layout_item)
        renderer_text->property_text() = item_get_title(layout_item);
      else
        renderer_text->property_text() = Glib::ustring();

      auto layout_group = std::dynamic_pointer_cast<LayoutGroup>(layout_item);
      auto layout_portal = std::dynamic_pointer_cast<LayoutItem_Portal>(layout_item);
      auto layout_button = std::dynamic_pointer_cast<LayoutItem_Button>(layout_item);
      auto layout_text = std::dynamic_pointer_cast<LayoutItem_Text>(layout_item);
      const bool editable = (layout_group && !layout_portal) || layout_button || layout_text; //Only groups, buttons, and text objects have titles that can be edited.
      renderer_text->property_editable() = editable;
    }
  }
}

void Dialog_Layout_Details::on_cell_data_column_width(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter)
{
  //Set the view's cell properties depending on the model's data:
  auto renderer_text = dynamic_cast<Gtk::CellRendererText*>(renderer);
  if(renderer_text)
  {
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;
      std::shared_ptr<LayoutItem> layout_item = row[m_model_items->m_columns.m_col_layout_item];

      guint column_width = 0;
      if(layout_item)
      {
        auto layout_button = std::dynamic_pointer_cast<LayoutItem_Button>(layout_item);
        auto layout_text = std::dynamic_pointer_cast<LayoutItem_Text>(layout_item);
        auto layout_field = std::dynamic_pointer_cast<LayoutItem_Field>(layout_item);
        const bool editable = (layout_field || layout_button || layout_text); //Only these have column widths that can be edited.
        renderer_text->property_editable() = editable;

        column_width = layout_item->get_display_width();
      }

      Glib::ustring text;
      if(column_width) //Show nothing if no width has been specified, meaning that it's automatic.
        text = Utils::string_from_decimal(column_width);

      renderer_text->property_text() = text;
    }
  }
}

void Dialog_Layout_Details::on_cell_data_group_columns(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter)
{
  //Set the view's cell properties depending on the model's data:
  auto renderer_text = dynamic_cast<Gtk::CellRendererText*>(renderer);
  if(renderer_text)
  {
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;
      std::shared_ptr<LayoutItem> layout_item = row[m_model_items->m_columns.m_col_layout_item];

      auto layout_group = std::dynamic_pointer_cast<LayoutGroup>(layout_item);
      auto layout_portal = std::dynamic_pointer_cast<LayoutItem_Portal>(layout_item);

      const bool is_group = layout_group && !layout_portal; //Only groups have column_counts.

      //Get a text representation of the number:
      Glib::ustring text;
      if(is_group)
      {
        text = Utils::string_from_decimal(layout_group->get_columns_count());
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
    Gtk::TreeModel::Path path(path_string);

    //Get the row from the path:
    auto iter = m_model_items->get_iter(path);
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;
      std::shared_ptr<LayoutItem> layout_item = row[m_model_items->m_columns.m_col_layout_item];
      if(layout_item)
      {
        //Store the user's new text in the model:
        layout_item->set_title(new_text, AppWindow::get_current_locale());

        m_modified = true;
      }
    }
  }
}


void Dialog_Layout_Details::on_treeview_cell_edited_name(const Glib::ustring& path_string, const Glib::ustring& new_text)
{
  if(!path_string.empty())
  {
    Gtk::TreeModel::Path path(path_string);

    //Get the row from the path:
    auto iter = m_model_items->get_iter(path);
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;
      std::shared_ptr<LayoutItem> layout_item = row[m_model_items->m_columns.m_col_layout_item];
      if(layout_item)
      {
        //Store the user's new text in the model:
        layout_item->set_name(new_text);

        m_modified = true;
      }
    }
  }
}

void Dialog_Layout_Details::on_treeview_cell_edited_column_width(const Glib::ustring& path_string, const Glib::ustring& new_text)
{
  if(!path_string.empty())
  {
    Gtk::TreeModel::Path path(path_string);

    //Get the row from the path:
    auto iter = m_model_items->get_iter(path);
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;
      std::shared_ptr<LayoutItem> layout_item = row[m_model_items->m_columns.m_col_layout_item];
      if(layout_item)
      {
        //Convert the text to a number, using the same logic used by GtkCellRendererText when it stores numbers.
        const auto new_value = static_cast<guint>( std::stod(new_text) );

        //Store the user's new value in the model:
        layout_item->set_display_width(new_value);

        m_modified = true;
      }
    }
  }
}

void Dialog_Layout_Details::on_treeview_cell_edited_group_columns(const Glib::ustring& path_string, const Glib::ustring& new_text)
{
  //This is used on numerical model columns:
  if(path_string.empty())
    return;

  Gtk::TreeModel::Path path(path_string);

  //Get the row from the path:
  auto iter = m_model_items->get_iter(path);
  if(iter)
  {
    Gtk::TreeModel::Row row = *iter;
    std::shared_ptr<LayoutItem> layout_item = row[m_model_items->m_columns.m_col_layout_item];
    auto layout_group = std::dynamic_pointer_cast<LayoutGroup>(layout_item);
    auto layout_portal = std::dynamic_pointer_cast<LayoutItem_Portal>(layout_item);
    if(layout_group && !layout_portal)
    {
      //std::istringstream astream(new_text); //Put it in a stream.
      //ColumnType new_value = ColumnType();
      //new_value << astream; //Get it out of the stream as the numerical type.

      //Convert the text to a number, using the same logic used by GtkCellRendererText when it stores numbers.
      auto new_value = static_cast<guint>( std::stod(new_text) );

      //Don't allow a 0 columns_count:
      if(new_value == 0)
        new_value = 1;

      //Store the user's new text in the model:
      layout_group->set_columns_count(new_value);

      m_modified = true;
    }
  }
}


Glib::ustring Dialog_Layout_Details::get_fields_table() const
{
  return m_table_name;
}

} //namespace Glom
