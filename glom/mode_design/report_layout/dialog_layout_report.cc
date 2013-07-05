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

#include <glom/mode_design/report_layout/dialog_layout_report.h>
#include <libglom/data_structure/layout/report_parts/layoutitem_groupby.h>
#include <libglom/data_structure/layout/report_parts/layoutitem_summary.h>
#include <libglom/data_structure/layout/report_parts/layoutitem_fieldsummary.h>
#include <libglom/data_structure/layout/report_parts/layoutitem_verticalgroup.h>
#include <libglom/data_structure/layout/report_parts/layoutitem_header.h>
#include <libglom/data_structure/layout/report_parts/layoutitem_footer.h>
#include <libglom/data_structure/layout/layoutitem_field.h>
#include <libglom/data_structure/layout/layoutitem_text.h>
#include <libglom/data_structure/layout/layoutitem_image.h>
#include <glom/glade_utils.h>
#include <glom/mode_design/layout/dialog_choose_field.h>
#include <glom/mode_design/layout/layout_item_dialogs/dialog_field_layout.h>
#include <glom/mode_design/layout/layout_item_dialogs/dialog_group_by.h>
#include <glom/mode_design/layout/layout_item_dialogs/dialog_field_summary.h>
#include <glom/mode_design/layout/dialog_choose_relationship.h>
#include <glom/appwindow.h>
//#include <libgnome/gnome-i18n.h>
#include <libglom/utils.h> //For bold_message()).
#include <glibmm/i18n.h>
#include <sstream> //For stringstream

namespace Glom
{

const char* Dialog_Layout_Report::glade_id("window_report_layout");
const bool Dialog_Layout_Report::glade_developer(true);

Dialog_Layout_Report::Dialog_Layout_Report(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Dialog_Layout(cobject, builder, false /* No table title */),
  m_notebook_parts(0),
  m_treeview_parts_header(0),
  m_treeview_parts_footer(0),
  m_treeview_parts_main(0),
  m_treeview_available_parts(0),
  m_button_up(0),
  m_button_down(0),
  m_button_add(0),
  m_button_delete(0),
  m_button_edit(0),
  m_button_formatting(0),
  m_label_table_name(0),
  m_entry_name(0),
  m_entry_title(0),
  m_checkbutton_table_title(0)
{
  builder->get_widget("label_table_name", m_label_table_name);
  builder->get_widget("entry_name", m_entry_name);
  builder->get_widget("entry_title", m_entry_title);
  builder->get_widget("checkbutton_table_title", m_checkbutton_table_title);

  builder->get_widget("button_up", m_button_up);
  m_button_up->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Report::on_button_up) );

  builder->get_widget("button_down", m_button_down);
  m_button_down->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Report::on_button_down) );

  builder->get_widget("button_delete", m_button_delete);
  m_button_delete->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Report::on_button_delete) );

  builder->get_widget("button_add", m_button_add);
  m_button_add->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Report::on_button_add) );

  builder->get_widget("button_edit", m_button_edit);
  m_button_edit->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Report::on_button_edit) );

  builder->get_widget("button_formatting", m_button_formatting);
  m_button_formatting->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout_Report::on_button_formatting) );

  builder->get_widget("notebook_parts", m_notebook_parts);
  m_notebook_parts->set_current_page(1); //The main part, because it is used most often.

  //Available parts:
  builder->get_widget("treeview_available_parts", m_treeview_available_parts);
  if(m_treeview_available_parts)
  {
    //Add list of available parts:
    //These are deleted in the destructor:

    //Main parts:
    {
      m_model_available_parts_main = type_model::create();

  //     Gtk::TreeModel::iterator iterHeader = m_model_available_parts_main->append();
  //     (*iterHeader)[m_model_available_parts_main->m_columns.m_col_item] = std::make_shared<LayoutItem_Header>();

      Gtk::TreeModel::iterator iter = m_model_available_parts_main->append();
      (*iter)[m_model_available_parts_main->m_columns.m_col_item] = std::make_shared<LayoutItem_GroupBy>();

      Gtk::TreeModel::iterator iterField = m_model_available_parts_main->append(iter->children()); //Place Field under GroupBy to indicate that that's where it belongs in the actual layout.
      (*iterField)[m_model_available_parts_main->m_columns.m_col_item] = std::make_shared<LayoutItem_Field>();

      Gtk::TreeModel::iterator iterText = m_model_available_parts_main->append(iter->children());
      (*iterText)[m_model_available_parts_main->m_columns.m_col_item] = std::make_shared<LayoutItem_Text>();

      Gtk::TreeModel::iterator iterImage = m_model_available_parts_main->append(iter->children());
      (*iterImage)[m_model_available_parts_main->m_columns.m_col_item] = std::make_shared<LayoutItem_Image>();

      Gtk::TreeModel::iterator iterVerticalGroup = m_model_available_parts_main->append(iter->children());
      (*iterVerticalGroup)[m_model_available_parts_main->m_columns.m_col_item] = std::make_shared<LayoutItem_VerticalGroup>();

      iter = m_model_available_parts_main->append();
      (*iter)[m_model_available_parts_main->m_columns.m_col_item] = std::make_shared<LayoutItem_Summary>();
      iter = m_model_available_parts_main->append(iter->children());
      (*iter)[m_model_available_parts_main->m_columns.m_col_item] = std::make_shared<LayoutItem_FieldSummary>();

//     Gtk::TreeModel::iterator iterFooter = m_model_available_parts_main->append();
//     (*iterFooter)[m_model_available_parts_main->m_columns.m_col_item] = std::make_shared<LayoutItem_Footer>();
    }

    //Header/Footer parts:
    {
      m_model_available_parts_headerfooter = type_model::create();

      Gtk::TreeModel::iterator iterVerticalGroup = m_model_available_parts_headerfooter->append();
      (*iterVerticalGroup)[m_model_available_parts_headerfooter->m_columns.m_col_item] = std::make_shared<LayoutItem_VerticalGroup>();

      Gtk::TreeModel::iterator iterField = m_model_available_parts_headerfooter->append(iterVerticalGroup->children());
      (*iterField)[m_model_available_parts_headerfooter->m_columns.m_col_item] = std::make_shared<LayoutItem_Field>();

      Gtk::TreeModel::iterator iterText = m_model_available_parts_headerfooter->append(iterVerticalGroup->children());
      (*iterText)[m_model_available_parts_headerfooter->m_columns.m_col_item] = std::make_shared<LayoutItem_Text>();

      Gtk::TreeModel::iterator iterImage = m_model_available_parts_headerfooter->append(iterVerticalGroup->children());
      (*iterImage)[m_model_available_parts_headerfooter->m_columns.m_col_item] = std::make_shared<LayoutItem_Image>();
    }

    m_treeview_available_parts->set_model(m_model_available_parts_main);
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

  builder->get_widget("treeview_parts_header", m_treeview_parts_header);
  setup_model(*m_treeview_parts_header, m_model_parts_header);
  builder->get_widget("treeview_parts_footer", m_treeview_parts_footer);
  setup_model(*m_treeview_parts_footer, m_model_parts_footer);
  builder->get_widget("treeview_parts_main", m_treeview_parts_main);
  setup_model(*m_treeview_parts_main, m_model_parts_main);


  show_all_children();

  //We save the connection, so we can disconnect it in the destructor,
  //because, for some reason, this signal handler is still called _after_ the destructor.
  //TODO: Fix that problem in GTK+ or gtkmm?
  m_signal_connection = m_notebook_parts->signal_switch_page().connect(sigc::mem_fun(*this, &Dialog_Layout_Report::on_notebook_switch_page));
}

Dialog_Layout_Report::~Dialog_Layout_Report()
{
  m_signal_connection.disconnect();
}

void Dialog_Layout_Report::setup_model(Gtk::TreeView& treeview, Glib::RefPtr<type_model>& model)
{
  //Allow drag-and-drop:
  treeview.enable_model_drag_source();
  treeview.enable_model_drag_dest();

  model = type_model::create();
  treeview.set_model(model);

  // Append the View columns:
  // Use set_cell_data_func() to give more control over the cell attributes depending on the row:

  //Name column:
  Gtk::TreeView::Column* column_part = Gtk::manage( new Gtk::TreeView::Column(_("Part")) );
  treeview.append_column(*column_part);

  Gtk::CellRendererText* renderer_part = Gtk::manage(new Gtk::CellRendererText);
  column_part->pack_start(*renderer_part);
  column_part->set_cell_data_func(*renderer_part,
    sigc::bind( sigc::mem_fun(*this, &Dialog_Layout_Report::on_cell_data_part), model));

  //Details column:
  Gtk::TreeView::Column* column_details = Gtk::manage( new Gtk::TreeView::Column(_("Details")) );
  treeview.append_column(*column_details);

  Gtk::CellRendererText* renderer_details = Gtk::manage(new Gtk::CellRendererText);
  column_details->pack_start(*renderer_details);
  column_details->set_cell_data_func(*renderer_details,
    sigc::bind( sigc::mem_fun(*this, &Dialog_Layout_Report::on_cell_data_details), model));


  //Connect to its signal:
  //renderer_count->signal_edited().connect(
  //  sigc::bind( sigc::mem_fun(*this, &Dialog_Layout_Report::on_treeview_cell_edited_numeric), m_model_parts_main->m_columns.m_col_columns_count) );

  //Respond to changes of selection:
  Glib::RefPtr<Gtk::TreeView::Selection> refSelection = treeview.get_selection();
  if(refSelection)
  {
    refSelection->signal_changed().connect( sigc::mem_fun(*this, &Dialog_Layout_Report::on_treeview_parts_selection_changed) );
  }

  //m_model_parts_main->signal_row_changed().connect( sigc::mem_fun(*this, &Dialog_Layout_Report::on_treemodel_row_changed) );
}

std::shared_ptr<LayoutGroup> Dialog_Layout_Report::fill_group(const Gtk::TreeModel::iterator& iter, const Glib::RefPtr<const type_model> model)
{
  if(iter)
  {
    Gtk::TreeModel::Row row = *iter;

    std::shared_ptr<LayoutItem> pItem = row[model->m_columns.m_col_item];
    std::shared_ptr<LayoutGroup> pGroup = std::dynamic_pointer_cast<LayoutGroup>(pItem);
    if(pGroup)
    {
      //Make sure that it contains the child items:
      fill_group_children(pGroup, iter, model);
      return glom_sharedptr_clone(pGroup);
    }

  }

  return  std::shared_ptr<LayoutGroup>();
}

void Dialog_Layout_Report::fill_group_children(const std::shared_ptr<LayoutGroup>& group, const Gtk::TreeModel::iterator& iter, const Glib::RefPtr<const type_model> model)
{
  if(iter)
  {
    Gtk::TreeModel::Row row = *iter;

    group->remove_all_items();
    for(Gtk::TreeModel::iterator iterChild = row.children().begin(); iterChild != row.children().end(); ++iterChild)
    {
      Gtk::TreeModel::Row row = *iterChild;
      std::shared_ptr<LayoutItem> item = row[model->m_columns.m_col_item];

      //Recurse:
      std::shared_ptr<LayoutGroup> child_group = std::dynamic_pointer_cast<LayoutGroup>(item);
      if(child_group)
        fill_group_children(child_group, iterChild, model);

      //std::cout << "debug: " << G_STRFUNC << ": Adding group child: parent part type=" << group->get_part_type_name() << ", child part type=" << item->get_part_type_name() << std::endl;
      group->add_item(item);
    }

  }
}

void Dialog_Layout_Report::add_group_children(const Glib::RefPtr<type_model>& model_parts, const Gtk::TreeModel::iterator& parent, const std::shared_ptr<const LayoutGroup>& group)
{
  for(LayoutGroup::type_list_items::const_iterator iter = group->m_list_items.begin(); iter != group->m_list_items.end(); ++iter)
  {
    std::shared_ptr<const LayoutItem> item = *iter;
    std::shared_ptr<const LayoutGroup> group = std::dynamic_pointer_cast<const LayoutGroup>(item);
    if(group)
    {
      std::shared_ptr<const LayoutItem_Header> header = std::dynamic_pointer_cast<const LayoutItem_Header>(item);
      std::shared_ptr<const LayoutItem_Footer> footer = std::dynamic_pointer_cast<const LayoutItem_Footer>(item);

      //Special-case the header and footer so that their items go into the separate treeviews:
      if(header)
        add_group_children(m_model_parts_header, parent, group); //Without the Header group being explicitly shown.
      else if(footer)
        add_group_children(m_model_parts_footer, parent, group);  //Without the Footer group being explicitly shown.
      else
      {
        add_group(model_parts, parent, group);
      }
    }
    else
    {
      Gtk::TreeModel::iterator iter = model_parts->append(parent->children());
      Gtk::TreeModel::Row row = *iter;
      row[model_parts->m_columns.m_col_item] = glom_sharedptr_clone(item);
    }
  }

  m_modified = true;
}

void Dialog_Layout_Report::add_group(const Glib::RefPtr<type_model>& model_parts, const Gtk::TreeModel::iterator& parent, const std::shared_ptr<const LayoutGroup>& group)
{
  Gtk::TreeModel::iterator iterNewItem;
  if(!parent)
  {
    //Add it at the top-level, because nothing was selected:
    iterNewItem = model_parts->append();
  }
  else
  {
    iterNewItem = model_parts->append(parent->children());
  }

  if(iterNewItem)
  {
    Gtk::TreeModel::Row row = *iterNewItem;

    row[model_parts->m_columns.m_col_item] = glom_sharedptr_clone(group);

    add_group_children(model_parts, iterNewItem /* parent */, group);

    m_modified = true;
  }
}

//void Dialog_Layout_Report::set_document(const Glib::ustring& layout, Document* document, const Glib::ustring& table_name, const type_vecLayoutFields& table_fields)
void Dialog_Layout_Report::set_report(const Glib::ustring& table_name, const std::shared_ptr<const Report>& report)
{
  m_modified = false;

  m_name_original = report->get_name();
  m_report = std::make_shared<Report>(*report); //Copy it, so we only use the changes when we want to.
  m_table_name = table_name;

  //Dialog_Layout::set_document(layout, document, table_name, table_fields);

  //Set the table name and title:
  m_label_table_name->set_text(table_name);

  m_entry_name->set_text(report->get_name());
  m_entry_title->set_text(item_get_title(report));
  m_checkbutton_table_title->set_active(report->get_show_table_title());

  //Update the tree models from the document

  if(true) //document)
  {


    //m_entry_table_title->set_text( document->get_table_title(table_name, AppWindow::get_current_locale()) );

    //document->fill_layout_field_details(m_table_name, mapGroups); //Update with full field information.

    //Show the report items:
    m_model_parts_header->clear();
    m_model_parts_main->clear();
    m_model_parts_footer->clear();

    //Add most parts to main, adding any found header or footer chidlren to those other models:
    add_group_children(m_model_parts_main, Gtk::TreeModel::iterator() /* null == top-level */, report->get_layout_group());

    //treeview_fill_sequences(m_model_parts_main, m_model_parts_main->m_columns.m_col_sequence); //The document should have checked this already, but it does not hurt to check again.
  }

  //Open all the groups:
  m_treeview_parts_header->expand_all();
  m_treeview_parts_main->expand_all();
  m_treeview_parts_footer->expand_all();

  m_notebook_parts->set_current_page(1); //The main part, because it is used most often.

  m_modified = false;
}



void Dialog_Layout_Report::enable_buttons()
{
  std::shared_ptr<LayoutItem> layout_item_available;
  bool enable_add = false;

  //Available Parts:
  Glib::RefPtr<Gtk::TreeView::Selection> refSelectionAvailable = m_treeview_available_parts->get_selection();
  if(refSelectionAvailable)
  {
    Gtk::TreeModel::iterator iter = refSelectionAvailable->get_selected();
    if(iter)
    {
      layout_item_available = (*iter)[m_model_available_parts_main->m_columns.m_col_item];

      enable_add = true;
    }
    else
    {
      enable_add = false;
    }
  }

  std::shared_ptr<LayoutItem> layout_item_parent;

  Gtk::TreeView* treeview = get_selected_treeview();
  if(!treeview)
    return;

  Glib::RefPtr<type_model> model = get_selected_model();

  //Parts:
  Glib::RefPtr<Gtk::TreeView::Selection> refSelection = treeview->get_selection();
  if(refSelection)
  {
    Gtk::TreeModel::iterator iter = refSelection->get_selected();
    if(iter)
    {
      layout_item_parent = (*iter)[model->m_columns.m_col_item];

      //Disable Up if It can't go any higher.
      bool enable_up = true;
      if(iter == model->children().begin())
        enable_up = false;  //It can't go any higher.

      m_button_up->set_sensitive(enable_up);


      //Disable Down if It can't go any lower.
      Gtk::TreeModel::iterator iterNext = iter;
      iterNext++;

      bool enable_down = true;
      if(iterNext == model->children().end())
        enable_down = false;

      m_button_down->set_sensitive(enable_down);

      m_button_delete->set_sensitive(true);

      //The [Formatting] button:
      bool enable_formatting = false;
      std::shared_ptr<LayoutItem_Field> item_field = std::dynamic_pointer_cast<LayoutItem_Field>(layout_item_parent);
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
      const bool may_be_child_of_parent = TreeStore_ReportLayout::may_be_child_of(layout_item_parent, layout_item_available);
      enable_add = may_be_child_of_parent;

      if(!may_be_child_of_parent)
      {
        //Maybe it can be a sibling of the parent instead (and that's what would happen if Add was clicked).
        std::shared_ptr<LayoutItem> layout_item_parent_of_parent;

        Gtk::TreeModel::iterator iterParent = iter->parent();
        if(iterParent)
          layout_item_parent_of_parent = (*iterParent)[model->m_columns.m_col_item];

        enable_add = TreeStore_ReportLayout::may_be_child_of(layout_item_parent_of_parent, layout_item_available);
      }
    }
  }

  m_button_add->set_sensitive(enable_add);
}

Glib::RefPtr<Dialog_Layout_Report::type_model> Dialog_Layout_Report::get_selected_model()
{
  Glib::RefPtr <type_model> model;

  Gtk::TreeView* treeview = get_selected_treeview();
  if(treeview)
    model = Glib::RefPtr<type_model>::cast_dynamic(treeview->get_model());

  return model;
}

Glib::RefPtr<const Dialog_Layout_Report::type_model> Dialog_Layout_Report::get_selected_model() const
{
  Dialog_Layout_Report* this_nonconst = const_cast<Dialog_Layout_Report*>(this);
  return this_nonconst->get_selected_model();
}

Gtk::TreeView* Dialog_Layout_Report::get_selected_treeview()
{
  switch(m_notebook_parts->get_current_page())
  {
    //These numbers depende on the page position as defined by our .glade file.
    //We could just get the page widget, but I'd like to allow other widgets in the page if we decide to do that. It's not ideal. murrayc.
    case(0): //Header
      return m_treeview_parts_header;
    case(1): //Main
      return m_treeview_parts_main;
    case(2): //Footer
      return m_treeview_parts_footer;
    default:
    {
      std::cerr << G_STRFUNC << ": Dialog_Layout_Report::get_selected_treeview(): Unrecognised current notebook page:"  << m_notebook_parts->get_current_page() << std::endl;
      return 0;
    }
  }
}

const Gtk::TreeView* Dialog_Layout_Report::get_selected_treeview() const
{
  Dialog_Layout_Report* this_nonconst = const_cast<Dialog_Layout_Report*>(this);
  return this_nonconst->get_selected_treeview();
}



void Dialog_Layout_Report::on_button_delete()
{
  Gtk::TreeView* treeview = get_selected_treeview();
  if(!treeview)
    return;

  Glib::RefPtr<type_model> model = get_selected_model();

  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = treeview->get_selection();
  if(refTreeSelection)
  {
    Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
    if(iter)
    {
      model->erase(iter);

      m_modified = true;
    }
  }
}

void Dialog_Layout_Report::on_button_up()
{
  Gtk::TreeView* treeview = get_selected_treeview();
  if(!treeview)
    return;

  Glib::RefPtr<type_model> model = get_selected_model();

  Glib::RefPtr<Gtk::TreeView::Selection> refSelection = treeview->get_selection();
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
        is_first = (iter == model->children().begin());

      if(!is_first)
      {
        Gtk::TreeModel::iterator iterBefore = iter;
        --iterBefore;

        model->iter_swap(iter, iterBefore);

        m_modified = true;
      }
    }

  }

  enable_buttons();
}

void Dialog_Layout_Report::on_button_down()
{
  Gtk::TreeView* treeview = get_selected_treeview();
  if(!treeview)
    return;

  Glib::RefPtr<type_model> model = get_selected_model();

  Glib::RefPtr<Gtk::TreeView::Selection> refSelection = treeview->get_selection();
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
        is_last = (iterNext == model->children().end());

      if(!is_last)
      {
        //Swap the sequence values, so that the one before will be after:
        model->iter_swap(iter, iterNext);

        m_modified = true;
      }
    }

  }

  enable_buttons();
}

void Dialog_Layout_Report::on_button_add()
{
  Gtk::TreeView* treeview = get_selected_treeview();
  if(!treeview)
    return;

  Glib::RefPtr<type_model> model = get_selected_model();
  Glib::RefPtr<type_model> model_available = Glib::RefPtr<type_model>::cast_dynamic(m_treeview_available_parts->get_model());

  Gtk::TreeModel::iterator parent = get_selected_group_parent();
  std::shared_ptr<const LayoutItem> pParentPart;
  if(parent)
  {
    std::shared_ptr<LayoutItem> temp = (*parent)[m_model_available_parts_main->m_columns.m_col_item];
    pParentPart = temp;
  }

  Gtk::TreeModel::iterator available = get_selected_available();
  std::shared_ptr<const LayoutItem> pAvailablePart;
  if(available)
  {
    std::shared_ptr<LayoutItem> temp = (*available)[model_available->m_columns.m_col_item];
    pAvailablePart = temp;
  }


  //Check whether the available part may be a child of the selected parent:
  if(pParentPart && !TreeStore_ReportLayout::may_be_child_of(pParentPart, pAvailablePart))
  {
    //Maybe it may be a child of the items' parent instead,
    //to become a sibling of the selected item.
    parent = (*parent).parent();
    if(parent)
    {
      std::shared_ptr<LayoutItem> temp = (*parent)[model_available->m_columns.m_col_item];
      pParentPart = temp;

      if(!TreeStore_ReportLayout::may_be_child_of(pParentPart, pAvailablePart))
        return; //Not allowed either.
    }
  }

  //Copy the available part to the list of parts:
  if(available)
  {
    Gtk::TreeModel::iterator iter;
    if(parent)
    {
      m_treeview_parts_main->expand_row( Gtk::TreeModel::Path(parent), true);
      iter = model->append(parent->children());
    }
    else
      iter = model->append();

    (*iter)[model->m_columns.m_col_item] = glom_sharedptr_clone(pAvailablePart);
  }

  if(parent)
    treeview->expand_row( Gtk::TreeModel::Path(parent), true);

  enable_buttons();
}


std::shared_ptr<Relationship> Dialog_Layout_Report::offer_relationship_list()
{
  std::shared_ptr<Relationship> result;

  Dialog_ChooseRelationship* dialog = 0;
  Utils::get_glade_widget_derived_with_warning(dialog);
  if(!dialog) //Unlikely and it already warns on stderr.
    return result;

  dialog->set_document(get_document(), m_table_name);
  dialog->set_transient_for(*this);
  const int response = dialog->run();
  dialog->hide();
  if(response == Gtk::RESPONSE_OK)
  {
    //Get the chosen relationship:
    result = dialog->get_relationship_chosen();
  }

  delete dialog;

  return result;
}

Gtk::TreeModel::iterator Dialog_Layout_Report::get_selected_group_parent() const
{
  //Get the selected group, or a suitable parent group, or the first group:

  Gtk::TreeModel::iterator parent;

  Gtk::TreeView* treeview = const_cast<Gtk::TreeView*>(get_selected_treeview());
  if(!treeview)
    return parent;

  Glib::RefPtr<const type_model> model = get_selected_model();


  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = treeview->get_selection();
  if(refTreeSelection)
  {
    Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;
      std::shared_ptr<LayoutItem> layout_item = row[model->m_columns.m_col_item];
      if(std::dynamic_pointer_cast<LayoutGroup>(layout_item))
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
  Gtk::TreeView* treeview = get_selected_treeview();
  if(!treeview)
    return;

  Glib::RefPtr<type_model> model = get_selected_model();

  //Get the selected item:
  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = treeview->get_selection();
  if(refTreeSelection)
  {
    Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;
      std::shared_ptr<LayoutItem> item = row[model->m_columns.m_col_item];

      std::shared_ptr<LayoutItem_Field> field = std::dynamic_pointer_cast<LayoutItem_Field>(item);
      if(field)
      {
        std::shared_ptr<LayoutItem_Field> field_chosen = offer_field_formatting(field, m_table_name, this, false /* no editing options */);
        if(field_chosen)
        {
          *field = *field_chosen;
          model->row_changed(Gtk::TreeModel::Path(iter), iter);
        }
      }
    }
  }
}

void Dialog_Layout_Report::on_button_edit()
{
  Gtk::TreeView* treeview = get_selected_treeview();
  if(!treeview)
    return;

  Glib::RefPtr<type_model> model = get_selected_model();

  //Get the selected item:
  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = treeview->get_selection();
  if(refTreeSelection)
  {
    Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
    if(iter)
    {
      //Do something different for each type of item:
      Gtk::TreeModel::Row row = *iter;
      std::shared_ptr<LayoutItem> item = row[model->m_columns.m_col_item];

      std::shared_ptr<LayoutItem_FieldSummary> fieldsummary = std::dynamic_pointer_cast<LayoutItem_FieldSummary>(item);
      if(fieldsummary)
      {
        Dialog_FieldSummary* dialog = 0;
        Utils::get_glade_widget_derived_with_warning(dialog);
        add_view(dialog);
        dialog->set_item(fieldsummary, m_table_name);
        dialog->set_transient_for(*this);

        const int response = dialog->run();
        dialog->hide();

        if(response == Gtk::RESPONSE_OK)
        {
          //Get the chosen relationship:
          std::shared_ptr<LayoutItem_FieldSummary> chosenitem = dialog->get_item();
          if(chosenitem)
          {
            *fieldsummary = *chosenitem; //TODO_Performance.

            model->row_changed(Gtk::TreeModel::Path(iter), iter); //TODO: Add row_changed(iter) to gtkmm?
            m_modified = true;
          }
        }

        remove_view(dialog);
        delete dialog;
      }
      else
      {
        std::shared_ptr<LayoutItem_Field> field = std::dynamic_pointer_cast<LayoutItem_Field>(item);
        if(field)
        {
          std::shared_ptr<LayoutItem_Field> chosenitem = offer_field_list_select_one_field(field, m_table_name, this);
          if(chosenitem)
          {
            *field = *chosenitem; //TODO_Performance.
            model->row_changed(Gtk::TreeModel::Path(iter), iter); //TODO: Add row_changed(iter) to gtkmm?
            m_modified = true;
          }
        }
        else
        {
          std::shared_ptr<LayoutItem_Text> layout_item_text = std::dynamic_pointer_cast<LayoutItem_Text>(item);
          if(layout_item_text)
          {
            std::shared_ptr<LayoutItem_Text> chosen = offer_textobject(layout_item_text);
            if(chosen)
            {
              *layout_item_text = *chosen;
              model->row_changed(Gtk::TreeModel::Path(iter), iter); //TODO: Add row_changed(iter) to gtkmm?
              m_modified = true;
            }
          }
          else
          {
            std::shared_ptr<LayoutItem_Image> layout_item_image = std::dynamic_pointer_cast<LayoutItem_Image>(item);
            if(layout_item_image)
            {
              std::shared_ptr<LayoutItem_Image> chosen = offer_imageobject(layout_item_image);
              if(chosen)
              {
                *layout_item_image = *chosen;
                model->row_changed(Gtk::TreeModel::Path(iter), iter); //TODO: Add row_changed(iter) to gtkmm?
                m_modified = true;
              }
            }
            else
            {
              std::shared_ptr<LayoutItem_GroupBy> group_by = std::dynamic_pointer_cast<LayoutItem_GroupBy>(item);
              if(group_by)
              {
                Dialog_GroupBy* dialog = 0;
                Utils::get_glade_widget_derived_with_warning(dialog);
                if(!dialog)
                  return;

                add_view(dialog);
                dialog->set_item(group_by, m_table_name);
                dialog->set_transient_for(*this);

                const int response = dialog->run();
                dialog->hide();

                if(response == Gtk::RESPONSE_OK)
                {
                  //Get the chosen relationship:
                  std::shared_ptr<LayoutItem_GroupBy> chosenitem = dialog->get_item();
                  if(chosenitem)
                  {
                    *group_by = *chosenitem;
                    model->row_changed(Gtk::TreeModel::Path(iter), iter); //TODO: Add row_changed(iter) to gtkmm?
                    m_modified = true;
                  }
                }

                remove_view(dialog);
                delete dialog;
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
}

void Dialog_Layout_Report::on_treeview_available_parts_selection_changed()
{
  enable_buttons();
}

void Dialog_Layout_Report::on_treeview_parts_selection_changed()
{
  enable_buttons();
}

void Dialog_Layout_Report::on_cell_data_part(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter, const Glib::RefPtr<type_model>& model)
{
  //TODO: If we ever use this as a real layout tree, then let's add icons for each type.

  //Set the view's cell properties depending on the model's data:
  Gtk::CellRendererText* renderer_text = dynamic_cast<Gtk::CellRendererText*>(renderer);
  if(renderer_text)
  {
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;

      std::shared_ptr<LayoutItem> pItem = row[model->m_columns.m_col_item];
      const Glib::ustring part = pItem->get_part_type_name();

      renderer_text->property_text() = part;
      renderer_text->property_editable() = false; //Part names can never be edited.
    }
  }
}

void Dialog_Layout_Report::on_cell_data_details(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter, const Glib::RefPtr<type_model>& model)
{
//Set the view's cell properties depending on the model's data:
  Gtk::CellRendererText* renderer_text = dynamic_cast<Gtk::CellRendererText*>(renderer);
  if(renderer_text)
  {
    if(iter)
    {
      Glib::ustring text;

      Gtk::TreeModel::Row row = *iter;
      std::shared_ptr<LayoutItem> pItem = row[model->m_columns.m_col_item];
      renderer_text->property_text() = pItem->get_layout_display_name();
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
      Glib::RefPtr<type_model> model = Glib::RefPtr<type_model>::cast_dynamic(m_treeview_available_parts->get_model());
      std::shared_ptr<LayoutItem> pItem = row[model->m_columns.m_col_item];
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

std::shared_ptr<Report> Dialog_Layout_Report::get_report()
{
  m_report->set_name( m_entry_name->get_text() );
  m_report->set_title( m_entry_title->get_text() , AppWindow::get_current_locale());
  m_report->set_show_table_title( m_checkbutton_table_title->get_active() );

  std::shared_ptr<LayoutGroup> group = m_report->get_layout_group();
  group->remove_all_items();

  //The Header and Footer parts are implicit (they are the whole header or footer treeview)
  std::shared_ptr<LayoutItem_Header> header = std::make_shared<LayoutItem_Header>();
  std::shared_ptr<LayoutGroup> group_temp = header;
  fill_report_parts(group_temp, m_model_parts_header);
  if(header->get_items_count())
    group->add_item(header);

  fill_report_parts(group, m_model_parts_main);

  std::shared_ptr<LayoutItem_Footer> footer = std::make_shared<LayoutItem_Footer>();
  group_temp = footer;
  fill_report_parts(group_temp, m_model_parts_footer);
  if(footer->get_items_count())
    group->add_item(footer);

  return m_report;
}

void Dialog_Layout_Report::fill_report_parts(std::shared_ptr<LayoutGroup>& group, const Glib::RefPtr<const type_model> parts_model)
{
  for(Gtk::TreeModel::iterator iter = parts_model->children().begin(); iter != parts_model->children().end(); ++iter)
  {
    //Recurse into a group if necessary:
    std::shared_ptr<LayoutGroup> group_child = fill_group(iter, parts_model);
    if(group_child)
    {
      //Add the group:
      group->add_item(group_child);
    }
    else
    {
      std::shared_ptr<LayoutItem> item = (*iter)[parts_model->m_columns.m_col_item];
      if(item)
      {
        group->add_item(item);
      }
    }
  }
}

void Dialog_Layout_Report::on_notebook_switch_page(Gtk::Widget*, guint page_number)
{
  //Change the list of available parts, depending on the destination treeview:
  Glib::RefPtr<type_model> model_available_parts;

  switch(page_number)
  {
    case(0): //Header
    case(2): //Footer
      model_available_parts = m_model_available_parts_headerfooter;
      break;
    case(1): //Main
    default:
      model_available_parts = m_model_available_parts_main;
      break;
  }

  m_treeview_available_parts->set_model(model_available_parts);
  m_treeview_available_parts->expand_all();

  //Enable the correct buttons for the now-visible TreeView:
  enable_buttons();
}


} //namespace Glom
