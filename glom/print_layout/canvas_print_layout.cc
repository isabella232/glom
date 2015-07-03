
/* Glom
 *
 * Copyright (C) 2001-2013 Murray Cumming
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

#include "canvas_print_layout.h"
#include <glom/utils_ui.h> //For bold_message()).
#include <glom/mode_design/print_layouts/dialog_text_formatting.h>
#include <glom/mode_design/layout/dialog_layout_list_related.h>
#include <glom/mode_design/layout/layout_item_dialogs/dialog_line.h>

//TODO: Remove these when we can just use a CanvasLayoutItem in a GooCanvasTable:
#include <glom/utility_widgets/canvas/canvas_table_movable.h>
#include <glom/utility_widgets/canvas/canvas_image_movable.h>
#include <glom/utility_widgets/canvas/canvas_text_movable.h>
#include <glom/print_layout/print_layout_utils.h>
#include <glom/appwindow.h>
#include <libglom/data_structure/glomconversions.h>
#include <libglom/db_utils.h>

#include <glom/glade_utils.h>
#include <gtkmm/builder.h>
#include <giomm/menu.h>
#include <glibmm/i18n.h>

namespace Glom
{

Canvas_PrintLayout::Canvas_PrintLayout()
: m_modified(false),
  m_dialog_format(nullptr),
  m_outline_visibility(false),
  m_page_count(1) //Sensible default
{
  #ifndef GLOM_ENABLE_CLIENT_ONLY
  setup_context_menu();
  #endif

  //Use millimeters, because that's something that is meaningful to the user,
  //and we can use it with Gtk::PageSetup too:
  property_units() = Gtk::UNIT_MM;

  m_items_group = Goocanvas::Group::create();
  //m_items_group->signal_button_press_event().connect( sigc::ptr_fun(&on_group_button_press_event), false );
  //TODO: How does this have any effect?: m_items_group->property_pointer_events() = Goocanvas::EVENTS_NONE;
  Glib::RefPtr<Goocanvas::Item> root = get_root_item();
  if(root)
    root->add_child(m_items_group);

  Glib::RefPtr<Gtk::PageSetup> page_setup = Gtk::PageSetup::create(); //start with something sensible.
  set_page_setup(page_setup);
}

Canvas_PrintLayout::~Canvas_PrintLayout()
{
}

void Canvas_PrintLayout::set_print_layout(const Glib::ustring& table_name, const std::shared_ptr<PrintLayout>& print_layout)
{
  m_table_name = table_name;
  m_modified = false;

  remove_all_items(m_items_group);
  add_layout_group(print_layout->get_layout_group(), true /* is top-level */);

  //Use the page setup:
  const Glib::ustring key_file_text = print_layout->get_page_setup();
  if(!key_file_text.empty())
  {
    Glib::KeyFile key_file;

    //TODO: Catch an exception
    key_file.load_from_data(key_file_text);

    Glib::RefPtr<Gtk::PageSetup> page_setup = 
      Gtk::PageSetup::create_from_key_file(key_file);
    set_page_setup(page_setup);
  }

  set_page_count(print_layout->get_page_count());

  //Add the rule lines:
  remove_rules();

  const PrintLayout::type_vec_doubles h_rules = print_layout->get_horizontal_rules();
  for(PrintLayout::type_vec_doubles::const_iterator iter = h_rules.begin();
    iter != h_rules.end(); ++iter)
  {
    add_horizontal_rule(*iter);
  }

  const PrintLayout::type_vec_doubles v_rules = print_layout->get_vertical_rules();
  for(PrintLayout::type_vec_doubles::const_iterator iter = v_rules.begin();
    iter != v_rules.end(); ++iter)
  {
    add_vertical_rule(*iter);
  }

  //TODO: This needs a number, but that is decided in WindowPrintLayoutEdit: set_grid_gap( print_layout->get_show_grid() );
  set_rules_visibility( print_layout->get_show_rules() );
  set_outlines_visibility( print_layout->get_show_outlines() );

  m_modified = false;
}

std::shared_ptr<PrintLayout> Canvas_PrintLayout::get_print_layout()
{
  std::shared_ptr<PrintLayout> result = std::make_shared<PrintLayout>();
  fill_layout_group(result->get_layout_group());

  //Page Setup:
  Glib::KeyFile key_file;
  if(m_page_setup)
    m_page_setup->save_to_key_file(key_file);

  Glib::ustring data;
  //TODO: Catch an exception
  data = key_file.to_data();

  result->set_page_setup(data);
  result->set_page_count(get_page_count());

  result->set_horizontal_rules( get_horizontal_rules() );
  result->set_vertical_rules( get_horizontal_rules() );

  return result;
}

/*
Glib::RefPtr<CanvasLayoutItem> Canvas_PrintLayout::create_canvas_item(const std::shared_ptr<LayoutItem>& item)
{
  Glib::RefPtr<CanvasLayoutItem> result = CanvasLayoutItem::create();
  //TODO: Add to the canvas.
  result->set_layout_item(item);

  return result;
}
*/


void Canvas_PrintLayout::add_layout_group_children(const std::shared_ptr<LayoutGroup>& group)
{
  //TODO: Add them inside the group item (when we actually use this code):
  for(const auto& item : group->m_list_items)
  {
    std::shared_ptr<LayoutItem_Portal> portal = std::dynamic_pointer_cast<LayoutItem_Portal>(item);
    std::shared_ptr<LayoutGroup> group = std::dynamic_pointer_cast<LayoutGroup>(item);
    if(group && !portal)
    {
      add_layout_group(group);
      continue;
    }
    else
    {
      create_canvas_layout_item_and_add(item);
    }
  }

  m_modified = true;
}

void Canvas_PrintLayout::create_canvas_layout_item_and_add(const std::shared_ptr<LayoutItem>& layout_item)
{
  Glib::RefPtr<CanvasLayoutItem> canvas_item = CanvasLayoutItem::create();
  add_canvas_layout_item(canvas_item);
  canvas_item->set_layout_item(layout_item);
  
  canvas_item->set_outline_visible(m_outline_visibility);
}

void Canvas_PrintLayout::add_canvas_layout_item(const Glib::RefPtr<CanvasLayoutItem>& item)
{
  if(!item)
    return;

  CanvasEditable::add_item(item, m_items_group);
  item->raise();

#ifndef GLOM_ENABLE_CLIENT_ONLY
  //Connect signals handlers:
  //TODO: Avoid the bind of a RefPtr. It has been known to cause memory/ref-counting problems:
  item->signal_show_context().connect(
    sigc::bind(
      sigc::mem_fun(*this, &Canvas_PrintLayout::on_item_show_context_menu),
      item) );
#endif //GLOM_ENABLE_CLIENT_ONLY

   if(item->get_child())
     item->set_outline_visible(m_outline_visibility);

   fill_with_data_system_preferences(item, get_document());
}

void Canvas_PrintLayout::remove_canvas_layout_item(const Glib::RefPtr<CanvasLayoutItem>& item)
{
  if(!item)
    return;

  CanvasEditable::remove_item(item, m_items_group);
}

void Canvas_PrintLayout::add_layout_group(const std::shared_ptr<LayoutGroup>& group, bool is_top_level)
{
  //row[model_parts->m_columns.m_col_item] = group->clone();

  //Add the group item:
  if(!is_top_level)
  {
    create_canvas_layout_item_and_add(group);
  }

  //Add the group's children.
  add_layout_group_children(group);

  m_modified = true;
}


void Canvas_PrintLayout::fill_layout_group(const std::shared_ptr<LayoutGroup>& group)
{
  const int count = m_items_group->get_n_children();
  for(int i = 0; i < count; ++i)
  {
    Glib::RefPtr<Goocanvas::Item> base_canvas_item = m_items_group->get_child(i);
    Glib::RefPtr<CanvasLayoutItem> canvas_item = Glib::RefPtr<CanvasLayoutItem>::cast_dynamic(base_canvas_item);
    if(canvas_item)
    {
      //Get the actual position:
      /*
      double x = 0;
      double y = 0;
      canvas_item->get_xy(x, y);
      double width = 0;
      double height = 0;
      canvas_item->get_width_height(width, height);
      if((width != 0)) //Allow height to be 0, because text items currently have no height. TODO: && (height != 0)) //Avoid bogus items.
      {
      */
        canvas_item->update_layout_position_from_canvas();

        group->add_item(canvas_item->get_layout_item());
      //}
    }

    //TODO: Recurse.
  }
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void Canvas_PrintLayout::setup_context_menu()
{
  m_context_menu_action_group = Gio::SimpleActionGroup::create();

/*
  Glib::RefPtr<Gtk::Action> action =  Gtk::Action::create("ContextInsertField", _("Field"));
  m_context_menu_action_group->add(action,
    sigc::mem_fun(*this, &Canvas_PrintLayout::on_context_menu_insert_field) );

  action = Gtk::Action::create("ContextInsertText", _("Text"));
  m_context_menu_action_group->add(action,
    sigc::mem_fun(*this, &Canvas_PrintLayout::on_context_menu_insert_text) );
*/

  m_action_edit = m_context_menu_action_group->add_action("edit",
    sigc::mem_fun(*this, &Canvas_PrintLayout::on_context_menu_edit));
  m_action_formatting = m_context_menu_action_group->add_action("formatting",
    sigc::mem_fun(*this, &Canvas_PrintLayout::on_context_menu_formatting));
  m_action_delete = m_context_menu_action_group->add_action("delete",
    sigc::mem_fun(*this, &Canvas_PrintLayout::on_context_menu_delete));

  insert_action_group("context", m_context_menu_action_group);

  Glib::RefPtr<Gio::Menu> menu = Gio::Menu::create();
  menu->append(_("_Edit"), "context.edit");
  menu->append(_("_Formatting"), "context.formatting");
  menu->append(_("_Delete"), "context.delete");
  m_context_menu = new Gtk::Menu(menu);
  m_context_menu->attach_to_widget(*this);
}


void Canvas_PrintLayout::on_item_show_context_menu(guint button, guint32 activate_time, Glib::RefPtr<CanvasLayoutItem> item)
{
  if(!m_context_menu || !item)
    return;

  m_context_item = item;
  
  //Do not enable the Formatting menu item for all types of items:
  std::shared_ptr<LayoutItem> layout_item = m_context_item->get_layout_item();
  bool enable_formatting = false;
  if(std::dynamic_pointer_cast<LayoutItem_WithFormatting>(layout_item))
  {
    enable_formatting = true;
  }

  m_action_formatting->set_enabled(enable_formatting);

  m_context_menu->popup(button, activate_time);
}

bool Canvas_PrintLayout::on_background_button_press_event(const Glib::RefPtr<Goocanvas::Item>& /* target */, GdkEventButton* /* event */)
{
  //A click on empty space should deselect any selected items:
  select_all(false);
  return false;
}

std::shared_ptr<LayoutItem_Portal> Canvas_PrintLayout::offer_related_records(const std::shared_ptr<LayoutItem_Portal>& portal, Gtk::Window* parent)
{
  std::shared_ptr<LayoutItem_Portal> result = portal;

  Dialog_Layout_List_Related* dialog = nullptr;
  Utils::get_glade_widget_derived_with_warning(dialog);
  if(!dialog) //Unlikely and it already warns on stderr.
    return result;

  add_view(dialog); //Give it access to the document.

  dialog->init_with_portal("layout_name_unused_for_portals", "", /* layout_platform */
    get_document(), portal, m_table_name,
    true /* for print layout */);

  if(parent)
    dialog->set_transient_for(*parent);

  UiUtils::show_window_until_hide(dialog);

  result = dialog->get_portal_layout();

  delete dialog;
  dialog = nullptr;

  return result;
}

std::shared_ptr<LayoutItem_Line> Canvas_PrintLayout::offer_line(const std::shared_ptr<LayoutItem_Line>& line, Gtk::Window* parent)
{
  std::shared_ptr<LayoutItem_Line> result = line;

  Dialog_Line* dialog = nullptr;
  Utils::get_glade_widget_derived_with_warning(dialog);
  if(!dialog) //Unlikely and it already warns on stderr.
    return result;

  if(parent)
    dialog->set_transient_for(*parent);
    
  dialog->set_line(line);

  const int response = Glom::UiUtils::dialog_run_with_help(dialog);
  dialog->hide();
  if(response == Gtk::RESPONSE_OK)
  {
    //Get the chosen relationship:
    result = dialog->get_line();
  }

  delete dialog;

  return result;
}

void Canvas_PrintLayout::on_context_menu_edit()
{
  Gtk::Window* parent = dynamic_cast<Gtk::Window*>(get_toplevel());

  m_context_item->update_layout_position_from_canvas();

  std::shared_ptr<LayoutItem> layout_item = m_context_item->get_layout_item();
  std::shared_ptr<LayoutItem_Field> field = 
    std::dynamic_pointer_cast<LayoutItem_Field>(layout_item);
  if(field)
  {
    std::shared_ptr<LayoutItem_Field> field_chosen = offer_field_list_select_one_field(field, m_table_name, parent);
    if(field_chosen)
    {
      //Never use the default formatting for print layouts:
      field_chosen->set_formatting_use_default(false);

      m_context_item->set_layout_item(field_chosen);
      fill_with_data_system_preferences(m_context_item, get_document());
    }
  }
  else
  {
    std::shared_ptr<LayoutItem_Text> text = std::dynamic_pointer_cast<LayoutItem_Text>(layout_item);
    if(text)
    {
      text = Base_DB::offer_textobject(text, parent, false /* don't show title */);
      m_context_item->set_layout_item(text);
    }
    else
    {
      std::shared_ptr<LayoutItem_Image> image = std::dynamic_pointer_cast<LayoutItem_Image>(layout_item);
      if(image)
      {
        image = Base_DB::offer_imageobject(image, parent, false /* don't show title */);
        m_context_item->set_layout_item(image);
      }
      else
      {
        std::shared_ptr<LayoutItem_Portal> portal = std::dynamic_pointer_cast<LayoutItem_Portal>(layout_item);
        if(portal)
        {
          portal = offer_related_records(portal, parent);
          m_context_item->set_layout_item(portal);
        }
        else
        {
         std::shared_ptr<LayoutItem_Line> line = std::dynamic_pointer_cast<LayoutItem_Line>(layout_item);
         if(line)
         {
           line = offer_line(line, parent);
           m_context_item->set_layout_item(line);
         }
        }
      }
    }
  }

  m_modified = true;

  m_context_item.reset();
}

void Canvas_PrintLayout::on_context_menu_formatting()
{
  if(!m_context_item)
    return;

  m_context_item->update_layout_position_from_canvas();

  std::shared_ptr<LayoutItem> layout_item = m_context_item->get_layout_item();
  std::shared_ptr<LayoutItem_Field> layout_item_field = std::dynamic_pointer_cast<LayoutItem_Field>(layout_item);
  std::shared_ptr<LayoutItem_Text> layout_item_text = std::dynamic_pointer_cast<LayoutItem_Text>(layout_item);
  if(!layout_item_field && !layout_item_text)
    return;

  if(m_dialog_format)
  {
     remove_view(m_dialog_format);
     delete m_dialog_format;
     m_dialog_format = nullptr;
  }

  Utils::get_glade_widget_derived_with_warning(m_dialog_format);
  add_view(m_dialog_format);

  Gtk::Window* window = dynamic_cast<Gtk::Window*>(get_toplevel());
  if(window)
    m_dialog_format->set_transient_for(*window);

  m_dialog_format->signal_hide().connect( sigc::mem_fun(*this, &Canvas_PrintLayout::on_dialog_format_hide) );

  //We need an if here, because they have no common base class.
  //TODO: Maybe they should. TODO: Maybe they already do.
  if(layout_item_field)
  {
    const Formatting& formatting = layout_item_field->m_formatting;
    m_dialog_format->m_box_formatting->set_formatting_for_field(formatting, m_table_name, layout_item_field->get_full_field_details());
  }
  else
  {
    const Formatting& formatting = layout_item_text->m_formatting;
    m_dialog_format->m_box_formatting->set_formatting_for_non_field(
      formatting, false /* don't show numeric options */);
  }

  m_dialog_format->m_box_formatting->set_is_for_non_editable();

  m_dialog_format->show();
}

void Canvas_PrintLayout::on_context_menu_delete()
{
  //If the item to be deleted was not selected then just delete it:
  if(!m_context_item->get_selected())
  {
    m_context_item->remove();
    m_context_item.reset();
    return;
  }

  //Requesting deletion of a selected item should delete all selected items:
  //TODO: If there are multiple items, ask the user for confirmation?
  m_context_item.reset();
  const type_vec_items items = get_selected_items();
  for(const auto& selected_item : items)
  {
    if(!selected_item)
      continue;
      
    const Glib::RefPtr<CanvasLayoutItem> canvas_layout_item = 
      Glib::RefPtr<CanvasLayoutItem>::cast_dynamic(selected_item);
    if(canvas_layout_item)
      remove_canvas_layout_item(canvas_layout_item);
  }
  
  signal_selection_changed().emit();
}

void Canvas_PrintLayout::on_dialog_format_hide()
{
  if(!m_dialog_format || !m_context_item)
    return;

  std::shared_ptr<LayoutItem> layout_item = m_context_item->get_layout_item();
  std::shared_ptr<LayoutItem_Field> layout_item_field = std::dynamic_pointer_cast<LayoutItem_Field>(layout_item);
  std::shared_ptr<LayoutItem_Text> layout_item_text = std::dynamic_pointer_cast<LayoutItem_Text>(layout_item);
  if(!layout_item_field && !layout_item_text)
    return;

  if(layout_item_field)
  {
    m_dialog_format->m_box_formatting->get_formatting(layout_item_field->m_formatting);

    //Never use the default formatting for print layouts:
    layout_item_field->set_formatting_use_default(false);
  }
  else if(layout_item_text)
    m_dialog_format->m_box_formatting->get_formatting(layout_item_text->m_formatting);

  
  m_context_item->set_layout_item(layout_item); //Redraw the child item with the new formatting.

  delete m_dialog_format;
  m_dialog_format = nullptr;
}

#endif //GLOM_ENABLE_CLIENT_ONLY

Glib::RefPtr<Goocanvas::Polyline> Canvas_PrintLayout::create_margin_line(double x1, double y1, double x2, double y2)
{
  Glib::RefPtr<Goocanvas::Polyline> line =
    Goocanvas::Polyline::create(x1, y1, x2, y2);
  line->property_line_width() = 0.5;
  line->property_stroke_color() = "light gray";

  //Interpret a click on the line like a click on the background behind it:
  //TODO: Do this for grid lines and rules too:
  line->signal_button_press_event().connect(
    sigc::mem_fun(*this, &Canvas_PrintLayout::on_background_button_press_event));

  m_bounds_group->add_child(line);
  return line;
}

void Canvas_PrintLayout::update_page_bounds()
{
  //Change the scroll extents to match the page size:
  const Gtk::PaperSize paper_size = m_page_setup->get_paper_size();
  Goocanvas::Bounds bounds;
  bounds.set_x1(0);
  bounds.set_y1(0);

  const Gtk::Unit units = property_units();

  //std::cout << "debug: " << G_STRFUNC << ": width=" << paper_size.get_width(units) << ", height=" paper_size.get_height(units) << std::endl;

  double page_width = 0;
  double page_height = 0;
  if(m_page_setup->get_orientation() == Gtk::PAGE_ORIENTATION_PORTRAIT) //TODO: Handle the reverse orientations too?
  {
    page_width = paper_size.get_width(units);
    page_height = paper_size.get_height(units);
  }
  else
  {
    page_width = paper_size.get_height(units);
    page_height = paper_size.get_width(units);
  }
  
  bounds.set_x2( page_width );
  bounds.set_y2( page_height * m_page_count );
  set_bounds(bounds);

  //Show the bounds with a rectangle, because the scrolled window might contain extra empty space.
  property_background_color() = "light gray";
  if(m_bounds_group)
  {
    m_bounds_group->remove();
    m_bounds_group.reset();
  }

  Glib::RefPtr<Goocanvas::Item> root = get_root_item();
  m_bounds_group = Goocanvas::Group::create();
  root->add_child(m_bounds_group);


  m_bounds_rect = Goocanvas::Rect::create(bounds.get_x1(), bounds.get_y1(), bounds.get_x2(), bounds.get_y2());
  m_bounds_rect->property_fill_color() = "white";
  m_bounds_rect->property_line_width() = 0;
  m_bounds_rect->signal_button_press_event().connect(
    sigc::mem_fun(*this, &Canvas_PrintLayout::on_background_button_press_event));
  m_bounds_group->add_child(m_bounds_rect);

  //Make sure that the bounds rect is at the bottom,
  //and that the grid and margins are just above it:
  if(m_grid)
    m_grid->lower();

  m_margin_left = create_margin_line(m_page_setup->get_left_margin(units), bounds.get_y1(), m_page_setup->get_left_margin(units), bounds.get_y2());
  m_margin_right = create_margin_line(bounds.get_x2() - m_page_setup->get_right_margin(units), bounds.get_y1(), bounds.get_x2() - m_page_setup->get_right_margin(units), bounds.get_y2());
 
  m_vec_margin_tops.clear();  
  m_vec_margin_bottoms.clear();
  for(guint page = 0; page < m_page_count; ++page)
  {
    const double top_y = paper_size.get_height(units) * page + m_page_setup->get_top_margin(units);
    Glib::RefPtr<Goocanvas::Polyline> margin_top = 
      create_margin_line(
        bounds.get_x1(), top_y, bounds.get_x2(), top_y);
    m_vec_margin_tops.push_back(margin_top);
      
    const double bottom_y = paper_size.get_height(units) * (page + 1) - m_page_setup->get_bottom_margin(units);
    Glib::RefPtr<Goocanvas::Polyline> margin_bottom = 
      create_margin_line(
        bounds.get_x1(), bottom_y, bounds.get_x2(), bottom_y);
    m_vec_margin_bottoms.push_back(margin_bottom);
  }
  
  m_bounds_group->lower();

  //Try to show the whole thing, by (indirectly) making the parent window big enough:
  /* TODO: This doesn't seem to work
  double width_pixels = bounds.get_x2() - bounds.get_x1();
  double height_pixels = bounds.get_y2() - bounds.get_y1();
  convert_to_pixels(width_pixels, height_pixels);
  std::cout << "DEBUG: width_pixels=" << width_pixels << ", height_pixels=" << height_pixels << std::endl;
  set_size_request(width_pixels, height_pixels);
  */
  
  //Update the grid lines:
  m_grid->update_grid_for_new_size();
}

void Canvas_PrintLayout::set_page_setup(const Glib::RefPtr<Gtk::PageSetup>& page_setup)
{
  m_page_setup = page_setup;
  if(!m_page_setup)
    return;

  update_page_bounds();
  m_modified = true;
}

Glib::RefPtr<Gtk::PageSetup> Canvas_PrintLayout::get_page_setup()
{
  return m_page_setup;
}

Glib::RefPtr<const Gtk::PageSetup> Canvas_PrintLayout::get_page_setup() const
{
  return m_page_setup;
}

void Canvas_PrintLayout::set_page_count(guint count)
{
  if(count < 1)
  {
    std::cerr << G_STRFUNC << ": count was less than 1" << std::endl;
    return;
  }

  m_page_count = count;
  
  update_page_bounds();
  m_modified = true;
}

guint Canvas_PrintLayout::get_page_count() const
{
  return m_page_count;
}

void Canvas_PrintLayout::fill_with_data(const FoundSet& found_set, bool avoid_page_margins)
{
  if(found_set.m_where_clause.empty())
  {
    //This might help a developer/debugger:
    //This is not an error.
    std::cout << G_STRFUNC << ": Not attempting to show real data because the where_clause is empty, maybe because there are no records in the database yet." << std::endl;
    return;
  }
  
  fill_with_data(m_items_group, found_set, avoid_page_margins);
}

void Canvas_PrintLayout::fill_with_data_system_preferences(const Glib::RefPtr<CanvasLayoutItem>& canvas_item, Document* document)
{
  std::shared_ptr<LayoutItem_Field> layoutitem_field = 
    std::dynamic_pointer_cast<LayoutItem_Field>(canvas_item->get_layout_item());
  if(!layoutitem_field)
    return;
  
  bool empty = true;
  if(!layoutitem_field->get_name().empty())
  {
    const std::shared_ptr<const Relationship> relationship = 
      layoutitem_field->get_relationship();

    if(!document)
    {
      std::cerr << G_STRFUNC << ": document is null" << std::endl;
      return;
    }

    if(document->get_relationship_is_system_properties(relationship))
      empty = false;
  }

  if(!empty)
  {
    LayoutFieldInRecord field_in_record;
    field_in_record.m_field = layoutitem_field;
    field_in_record.m_table_name = m_table_name;
    const Gnome::Gda::Value value = get_field_value_in_database(
     field_in_record, 0 /* TODO: parent window */);
    if(!Glom::Conversions::value_is_empty(value))
    {
      canvas_item->set_db_data(value);
      //TODO: canvas_item->expand_text_vertically();
    }
  }
}
 

void Canvas_PrintLayout::fill_with_data(const Glib::RefPtr<Goocanvas::Group>& canvas_group, const FoundSet& found_set, bool avoid_page_margins)
{
  //A map of the text representation (e.g. field_name or relationship::field_name) to the index:
  typedef std::map<Glib::ustring, guint> type_map_layout_fields_index;
  type_map_layout_fields_index map_fields_index;

  typedef std::list< std::shared_ptr<LayoutItem_Portal> > type_list_portals;
  type_list_portals list_portals;

  //Get list of fields to get from the database.
  Utils::type_vecLayoutFields fieldsToGet;
  const int count = canvas_group->get_n_children();
  guint field_i = 0;
  for(int i = 0; i < count; ++i)
  {
    Glib::RefPtr<Goocanvas::Item> base_canvas_item = canvas_group->get_child(i);
    Glib::RefPtr<CanvasLayoutItem> canvas_item = Glib::RefPtr<CanvasLayoutItem>::cast_dynamic(base_canvas_item);
    if(!canvas_item)
      continue;

    std::shared_ptr<LayoutItem> layout_item = canvas_item->get_layout_item();
    if(!layout_item)
      continue;

    std::shared_ptr<LayoutItem_Field> layoutitem_field = std::dynamic_pointer_cast<LayoutItem_Field>(layout_item);
    if(layoutitem_field && !(layoutitem_field->get_name().empty()))
    {
      fieldsToGet.push_back(layoutitem_field);

      //Remember the index so we can use it later,
      //to get the data for this column from the query results:
      map_fields_index[ layoutitem_field->get_layout_display_name() ] = field_i;
      ++field_i;
    }
    else
    {
      std::shared_ptr<LayoutItem_Portal> layoutitem_portal = std::dynamic_pointer_cast<LayoutItem_Portal>(layout_item);
      if(layoutitem_portal)
      {
        //Fill the related records table:
        std::shared_ptr<const Relationship> relationship = layoutitem_portal->get_relationship();
        if(relationship)
        {
          const Document* document = get_document();
          const std::shared_ptr<Field> from_field = DbUtils::get_fields_for_table_one_field(document,
            relationship->get_from_table(), relationship->get_from_field());
          const Gnome::Gda::Value from_key_value = get_field_value_in_database(from_field, found_set, 0 /* TODO: window */);
          fill_with_data_portal(canvas_item, from_key_value);
        }
      }
    }
  }

  if(fieldsToGet.empty())
    return;

  const Glib::RefPtr<const Gnome::Gda::SqlBuilder> sql_query = Utils::build_sql_select_with_where_clause(found_set.m_table_name,
    fieldsToGet,
    found_set.m_where_clause, std::shared_ptr<const Relationship>() /* extra_join */, found_set.m_sort_clause,
    1);

  bool records_found = false;
  Glib::RefPtr<Gnome::Gda::DataModel> datamodel;
  try
  {
    datamodel = DbUtils::query_execute_select(sql_query);
  }
  catch(const Glib::Exception& ex)
  {
    std::cout << "Canvas_PrintLayout::fill_with_data: exception: " << ex.what() << std::endl;
  }
  catch(const std::exception& ex)
  {
    std::cout << "Canvas_PrintLayout::fill_with_data: exception: " << ex.what() << std::endl;
  }

  if(datamodel)
  {
    const guint rows_count = datamodel->get_n_rows();
    if(rows_count > 0)
      records_found = true;
  }

  if(!records_found)
    return;

  //Set all the data for the fields in the canvas
  //(and clear the no-image pixbuf from images):
  for(int i = 0; i < count; ++i)
  {
    Glib::RefPtr<Goocanvas::Item> base_canvas_item = canvas_group->get_child(i);
    Glib::RefPtr<CanvasLayoutItem> canvas_item = Glib::RefPtr<CanvasLayoutItem>::cast_dynamic(base_canvas_item);
    if(!canvas_item)
      continue;

    std::shared_ptr<LayoutItem> layout_item = canvas_item->get_layout_item();
    if(!layout_item)
      continue;

    std::shared_ptr<LayoutItem_Field> layoutitem_field = std::dynamic_pointer_cast<LayoutItem_Field>(layout_item);
    if(layoutitem_field)
    {
      type_map_layout_fields_index::const_iterator iterFind = map_fields_index.find( layoutitem_field->get_layout_display_name() );
      if(iterFind != map_fields_index.end())
      {
        //Set the data from the database:
        const guint col_index = iterFind->second;

        //TODO: Actually catch exception:
        const Gnome::Gda::Value value = datamodel->get_value_at(col_index, 0);
        canvas_item->set_db_data(value);
      }
    }
    else
    {
      //Clear the no-image pixbuf from images:
      canvas_item->remove_empty_indicators();
    }
    
    if(avoid_page_margins)
    {
      const Glib::RefPtr<Gtk::PageSetup> page_setup = get_page_setup();
      const Gtk::Unit units = property_units();
      const bool needs_moving = PrintLayoutUtils::needs_move_fully_to_page(page_setup, units, canvas_item);
      if(needs_moving)
      {
        double x = 0;
        double y = 0;
        canvas_item->get_xy(x, y);
        double width = 0;
        double height = 0;
        canvas_item->get_width_height(width, height);
               
        const double offset = PrintLayoutUtils::get_offset_to_move_fully_to_next_page(page_setup, units, y, height);
        move_items_down(y, offset);
      }
    }
  }
}


void Canvas_PrintLayout::fill_with_data_portal(const Glib::RefPtr<CanvasLayoutItem>& canvas_item, const Gnome::Gda::Value& foreign_key_value)
{
  if(!canvas_item)
    return;

  std::shared_ptr<LayoutItem> layout_item = canvas_item->get_layout_item();
  std::shared_ptr<LayoutItem_Portal> portal = std::dynamic_pointer_cast<LayoutItem_Portal>(layout_item);
  if(!portal)
    return;

  Glib::RefPtr<CanvasTableMovable> canvas_table = Glib::RefPtr<CanvasTableMovable>::cast_dynamic(canvas_item->get_child());
  if(!canvas_table)
    return;

  //TODO: When goocanvas supports groups (and therefore Glom::CanvasLayoutItem)
  //in table cells, simplify this code by just setting the values of those
  //existing items:
  //
  //Remove all existing cells, so we can recreate them:
  //TODO: Add a function to goocanvas for this.
  //TODO: Move the child stuff into group in goocanvas.

  const LayoutGroup::type_list_items child_layout_items = portal->get_items();

  //Build and run the SQL query for this portal:
  const type_vecConstLayoutFields fields_shown = get_portal_fields_to_show(portal);

  FoundSet found_set;
  found_set.m_table_name = portal->get_table_used(Glib::ustring() /* parent table_name, not used. */);
  set_found_set_where_clause_for_portal(found_set, portal, foreign_key_value);

  const Glib::RefPtr<Gnome::Gda::SqlBuilder> sql_query = Utils::build_sql_select_with_where_clause(found_set.m_table_name, fields_shown, found_set.m_where_clause, found_set.m_extra_join, found_set.m_sort_clause);
  //std::cout << "DEBUG: sql_query=" << sql_query << std::endl;
  const Glib::RefPtr<const Gnome::Gda::DataModel> datamodel = DbUtils::query_execute_select(sql_query);
  if(!(datamodel))
    return;

  const int db_rows_count = datamodel->get_n_rows(); //TODO: Performance? COUNT them instead?
  const int db_columns_count = datamodel->get_n_columns();

  //Show as many rows as needed, but not more than the maximum:
  gulong rows_count_min = 0;
  gulong rows_count_max = 0;
  portal->get_rows_count(rows_count_min, rows_count_max);
  int rows_count = std::min((int)rows_count_max, db_rows_count);
  //Do not use less than the minimum:
  rows_count = std::max(rows_count, (int)rows_count_min);

  // TODO: Remove this method and/or change it to recalc the min when changing the height:
  //CanvasLayoutItem::get_rows_count_for_portal(portal, row_height_ignored);
  const double portal_height = rows_count * portal->get_print_layout_row_height();

  double old_width = 0;
  double old_height = 0;
  canvas_item->get_width_height(old_width, old_height);
  canvas_item->set_width_height(old_width, portal_height);

  //Move other items down if the portal table became bigger:
  double x = 0;
  double y = 0;
  canvas_item->get_xy(x, y);
  const double offset = portal_height - old_height;
  move_items_down(y + old_height, offset);

  canvas_item->add_portal_rows_if_necessary(rows_count);
  //TODO: Move everything else down.

  const int cols_count = child_layout_items.size();

  //Set the DB value for each cell:
  for(int row = 0; row < rows_count; ++row)
  {
    int db_col = 0;
    LayoutGroup::type_list_items::const_iterator iter_child_layout_items = child_layout_items.begin();
    for(int col = 0; col < cols_count; ++col)
    {
      //Glib::RefPtr<Goocanvas::Item> canvas_child = base_item->get_cell_child(row, col); //TODO: Add this to Goocanvas::Table.
      Glib::RefPtr<Goocanvas::Item> canvas_child = 
        CanvasLayoutItem::get_canvas_table_cell_child(canvas_table, row, col); //TODO: Add this to Goocanvas::Table.
      if(!canvas_child)
      {
        std::cerr << G_STRFUNC << ": canvas_child is NULL for row=" << row << ", col=" << col << std::endl;
        continue;
      }

      if(iter_child_layout_items == child_layout_items.end())
        continue;

      std::shared_ptr<LayoutItem> child_layout_item = *iter_child_layout_items;
      std::shared_ptr<LayoutItem_Field> field = std::dynamic_pointer_cast<LayoutItem_Field>(child_layout_item);
      if(field)
      {
        Gnome::Gda::Value db_value; //This default also wipes the placeholder field name text of empty rows.
        if( (row < db_rows_count) && (db_col < db_columns_count) )
        {
          //TODO: Actually catch exception.
          db_value = datamodel->get_value_at(db_col, row);
        }

        set_canvas_item_field_value(canvas_child, field, db_value);
        ++db_col;
      }

      ++iter_child_layout_items;
    }
  }
}

void Canvas_PrintLayout::set_canvas_item_field_value(const Glib::RefPtr<Goocanvas::Item>& canvas_item, const std::shared_ptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value)
{
  if(!field)
    return;

  //Expect the appropriate canvas item, depending on the field type:
  if(field->get_glom_type() == Field::TYPE_IMAGE)
  {
    Glib::RefPtr<CanvasImageMovable> canvas_image = Glib::RefPtr<CanvasImageMovable>::cast_dynamic(canvas_item);
    if(!canvas_image)
      return;

    Glib::RefPtr<Gdk::Pixbuf> pixbuf = UiUtils::get_pixbuf_for_gda_value(value);
    canvas_image->property_pixbuf() = pixbuf;
  }
  else //text, numbers, date, time, boolean:
  {
    Glib::RefPtr<CanvasTextMovable> canvas_text = Glib::RefPtr<CanvasTextMovable>::cast_dynamic(canvas_item);
    if(!canvas_text)
    {
      std::cerr << G_STRFUNC << ": The canvas item is not of the expected type. Instead it is of type." << std::endl;
      return;
    }

    Glib::ustring text;

    std::shared_ptr<const LayoutItem_WithFormatting> with_formatting = 
      std::dynamic_pointer_cast<const LayoutItem_WithFormatting>(field);
    if(with_formatting)
    {
      const Formatting& formatting = with_formatting->get_formatting_used();
      text = Conversions::get_text_for_gda_value(field->get_glom_type(), 
        value, formatting.m_numeric_format);
    }
    else
    {
      text = Conversions::get_text_for_gda_value(field->get_glom_type(), 
        value);
    }

    canvas_text->set_text(text);
  }
}

void Canvas_PrintLayout::set_zoom_percent(guint percent)
{
  if(percent == 0)
    return;

  const double scale = (double)percent / 100;
  if(scale == 0)
    return;

  set_scale(scale);
}

void Canvas_PrintLayout::hide_page_bounds()
{
  m_bounds_group->property_visibility() = Goocanvas::ITEM_HIDDEN;
}

void Canvas_PrintLayout::set_grid_gap(double gap)
{
  CanvasEditable::set_grid_gap(gap);

  //Make sure that the bounds rect is at the bottom,
  //and that the grid and margins are just above it:
  if(m_grid)
    m_grid->lower();

  if(m_bounds_group)
    m_bounds_group->lower();
}

Base_DB::type_vecConstLayoutFields Canvas_PrintLayout::get_portal_fields_to_show(const std::shared_ptr<LayoutItem_Portal>& portal)
{
  const Document* document = get_document();
  if(!document)
    std::cerr << G_STRFUNC << ": document is NULL." << std::endl;

  if(document && portal)
  {
    Document::type_list_layout_groups mapGroups;
    mapGroups.push_back(portal);

    std::shared_ptr<const Relationship> relationship = portal->get_relationship();
    if(relationship)
    {
      type_vecConstLayoutFields result = get_table_fields_to_show_for_sequence(portal->get_table_used(Glib::ustring() /* not relevant */), mapGroups);

      //If the relationship does not allow editing, then mark all these fields as non-editable:
      /* TODO: Find a better way to do this.
      if(!(portal->get_relationship_used_allows_edit()))
      {
        for(const auto& item : result)
        {
          if(item)
            item->set_editable(false);
        }
      }
      */

      return result;
    }
  }

  return type_vecConstLayoutFields();
}


Canvas_PrintLayout::type_vec_items Canvas_PrintLayout::get_selected_items()
{
  type_vec_items result;

  //TODO: Do this recursively.
  Glib::RefPtr<Goocanvas::Item> root = m_items_group;
  if(!root)
    return result;

  const int count = root->get_n_children();
  for(int i = 0; i < count; ++i)
  {
    Glib::RefPtr<Goocanvas::Item> child = root->get_child(i);
    Glib::RefPtr<CanvasLayoutItem> derived =
      Glib::RefPtr<CanvasLayoutItem>::cast_dynamic(child);
    if(!derived)
      continue;

    if(derived->get_selected())
      result.push_back(derived);
  }

  return result;
}

void Canvas_PrintLayout::set_outlines_visibility(bool visible)
{
  //Remember this so we can apply it to items added later:
  m_outline_visibility = visible;

  //TODO: Do this recursively.
  Glib::RefPtr<Goocanvas::Item> root = m_items_group;
  if(!root)
    return;

  const int count = root->get_n_children();
  for(int i = 0; i < count; ++i)
  {
    Glib::RefPtr<Goocanvas::Item> child = root->get_child(i);
    Glib::RefPtr<CanvasLayoutItem> derived =
      Glib::RefPtr<CanvasLayoutItem>::cast_dynamic(child);
    if(!derived)
      continue;

    derived->set_outline_visible(visible);
  }
}

void Canvas_PrintLayout::select_all(bool selected)
{
  Glib::RefPtr<Goocanvas::Item> root = m_items_group;
  if(!root)
    return;

  const int count = root->get_n_children();
  for(int i = 0; i < count; ++i)
  {
    Glib::RefPtr<Goocanvas::Item> child = root->get_child(i);
    Glib::RefPtr<CanvasLayoutItem> derived =
      Glib::RefPtr<CanvasLayoutItem>::cast_dynamic(child);
    if(!derived)
      continue;

    derived->set_selected(selected);
  }

  signal_selection_changed().emit();
}

Goocanvas::Bounds Canvas_PrintLayout::get_page_bounds(guint page_num) const
{
  Goocanvas::Bounds bounds;
  
   //Change the scroll extents to match the page size:
  const Gtk::PaperSize paper_size = m_page_setup->get_paper_size();
  const Gtk::Unit units = property_units();

  //std::cout << "debug: " << G_STRFUNC << ": width=" << paper_size.get_width(units) << ", height=" paper_size.get_height(units) << std::endl;

  double page_width = 0;
  double page_height = 0;
  if(m_page_setup->get_orientation() == Gtk::PAGE_ORIENTATION_PORTRAIT) //TODO: Handle the reverse orientations too?
  {
    page_width = paper_size.get_width(units);
    page_height = paper_size.get_height(units);
  }
  else
  {
    page_width = paper_size.get_height(units);
    page_height = paper_size.get_width(units);
  }
  
  bounds.set_x1(0);
  bounds.set_y1( page_height * page_num);
  bounds.set_x2( page_width );
  bounds.set_y2( page_height * (page_num + 1) );

  return bounds;
}

Glib::RefPtr<CanvasLayoutItem> Canvas_PrintLayout::move_items_down(double y_start, double offset)
{
  //Keep track of the top-most item that needs to be moved all the way on to a new page:
  Glib::RefPtr<CanvasLayoutItem> needs_moving_top;
  double y_needs_moving_top = 0;
  double needs_moving_top_height = 0;

  Glib::RefPtr<Goocanvas::Item> root = m_items_group;
  if(!root)
    return needs_moving_top;

  double bottom_max = 0;
  
  const Glib::RefPtr<Gtk::PageSetup> page_Setup = get_page_setup();

  const int count = root->get_n_children();
  for(int i = 0; i < count; ++i)
  {
    Glib::RefPtr<Goocanvas::Item> child = root->get_child(i);
    Glib::RefPtr<CanvasLayoutItem> derived = 
      Glib::RefPtr<CanvasLayoutItem>::cast_dynamic(child);
    if(!derived)
    {
      std::cout << "debug: not derived" << std::endl;
      continue;
    }

    //Ignore items above y_start:
    double x = 0;
    double y = 0;
    derived->get_xy(x, y);
    if(y < y_start)
      continue;

    //Ignore items that would not overlap even if they had the same y:
    double width = 0;
    double height = 0;
    derived->get_width_height(width, height);
    //if( (x + width) < item_x)
    //  continue;

    //if( x > (item_x + item_width))
    //  continue;

    //Move it down:
    y += offset;
    derived->set_xy(x, y);

    //Move it some more if necessary:
    //See if it should be moved down (but do that later):
    const auto needs_moving = PrintLayoutUtils::needs_move_fully_to_page(page_Setup, property_units(), derived);
    if(needs_moving)
    {
      if(!needs_moving_top || (y <= y_needs_moving_top))
      {
        y_needs_moving_top = y;

        needs_moving_top = derived;
        needs_moving_top_height = height;
      }
    }

    //Check where the bottom is:
    const double bottom = y + height;
    bottom_max = std::max(bottom_max, bottom);
  }

  //Add extra pages if necessary:
  const auto last_page_needed = PrintLayoutUtils::get_page_for_y(page_Setup, 
    property_units(), bottom_max);
  if((last_page_needed + 1) > get_page_count())
  {
    set_page_count(last_page_needed + 1);
  }

  //Now move everything further, completely on to the next page
  //and then do that again for the next page until all the pages are done:
  if(needs_moving_top && (y_needs_moving_top > y_start))
  {
    std::cout << "extra move: y_needs_moving_top=" << y_needs_moving_top << std::endl;
    const double extra_offset = 
      PrintLayoutUtils::get_offset_to_move_fully_to_next_page(
        page_Setup, property_units(),
        y_needs_moving_top, needs_moving_top_height);
    if(extra_offset)
    {
      needs_moving_top = move_items_down(y_needs_moving_top, extra_offset);
    }
    else
      needs_moving_top.reset();
  }

  return needs_moving_top;
}

double Canvas_PrintLayout::get_page_height() const
{
  const Glib::RefPtr<const Gtk::PageSetup> page_setup = get_page_setup(); 
  return PrintLayoutUtils::get_page_height(page_setup, property_units());
}

double Canvas_PrintLayout::get_page_height(double& margin_top, double& margin_bottom) const
{
  const Glib::RefPtr<const Gtk::PageSetup> page_setup = get_page_setup();
  return PrintLayoutUtils::get_page_height(page_setup, property_units(), margin_top, margin_bottom);
}

} //namespace Glom
