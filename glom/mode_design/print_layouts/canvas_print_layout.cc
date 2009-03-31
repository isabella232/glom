
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

#include "canvas_print_layout.h"
#include <glom/utils_ui.h> //For bold_message()).
#include <gtkmm/stock.h>
#include <glom/mode_design/print_layouts/dialog_text_formatting.h>
#include <glom/mode_data/dialog_layout_list_related.h>

//TODO: Remove these when we can just use a CanvasLayoutItem in a GooCanvasTable:
#include <glom/utility_widgets/canvas/canvas_table_movable.h>
#include <glom/utility_widgets/canvas/canvas_image_movable.h>
#include <glom/utility_widgets/canvas/canvas_text_movable.h>
#include <libglom/data_structure/glomconversions.h>

#include <glom/glade_utils.h>
#include <glibmm/i18n.h>

namespace Glom
{

/*
static bool on_group_button_press_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventButton* event)
{
  return true; // Let the child items get the event.
}
*/

Canvas_PrintLayout::Canvas_PrintLayout()
: m_modified(false),
  m_dialog_format(0)
{
  setup_context_menu();

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


void Canvas_PrintLayout::set_print_layout(const Glib::ustring& table_name, const sharedptr<const PrintLayout>& print_layout)
{
  m_table_name = table_name;
  m_modified = false;

  remove_all_items(m_items_group);
  add_layout_group(print_layout->m_layout_group, true /* is top-level */);

  Glib::RefPtr<Gtk::PageSetup> page_setup;
  const Glib::ustring key_file_text = print_layout->get_page_setup();
  if(!key_file_text.empty())
  {
    Glib::KeyFile key_file;
    key_file.load_from_data(key_file_text);
    //TODO: Use this when gtkmm and GTK+ have been fixed: page_setup = Gtk::PageSetup::create(key_file);
    page_setup = Glib::wrap(gtk_page_setup_new_from_key_file(key_file.gobj(), NULL, NULL));
  }

  set_page_setup(page_setup);

  m_modified = false;
}

sharedptr<PrintLayout> Canvas_PrintLayout::get_print_layout()
{
  sharedptr<PrintLayout> result = sharedptr<PrintLayout>::create();
  fill_layout_group(result->m_layout_group);

  //Page Setup:
  Glib::KeyFile key_file;
  m_page_setup->save_to_key_file(key_file);
  result->set_page_setup(key_file.to_data());

  return result;
}

/*
Glib::RefPtr<CanvasLayoutItem> Canvas_PrintLayout::create_canvas_item(const sharedptr<LayoutItem>& item)
{
  Glib::RefPtr<CanvasLayoutItem> result = CanvasLayoutItem::create(item);

  return result;
}
*/


void Canvas_PrintLayout::add_layout_group_children(const sharedptr<LayoutGroup>& group)
{
  //TODO: Add them inside the group item (when we actually use this code):
  for(LayoutGroup::type_list_items::const_iterator iter = group->m_list_items.begin(); iter != group->m_list_items.end(); ++iter)
  {
    sharedptr<LayoutItem> item = *iter;

    sharedptr<LayoutItem_Portal> portal = sharedptr<LayoutItem_Portal>::cast_dynamic(item);
    sharedptr<LayoutGroup> group = sharedptr<LayoutGroup>::cast_dynamic(item);
    if(group && !portal)
    {
      add_layout_group(group);
      continue;
    }
    else
    {
      Glib::RefPtr<CanvasLayoutItem> canvas_item = CanvasLayoutItem::create(item);
      if(canvas_item)
        add_canvas_layout_item(canvas_item);
    }
  }

  m_modified = true;
}

void Canvas_PrintLayout::add_canvas_layout_item(const Glib::RefPtr<CanvasLayoutItem> item)
{
  if(!item)
    return;

  CanvasEditable::add_item(item, m_items_group);
  item->raise();

  //Connect signals handlers:
  //TODO: Avoid the bind of a RefPtr. It has been known to cause memory/ref-counting problems: 
  item->signal_show_context().connect(
    sigc::bind(
      sigc::mem_fun(*this, &Canvas_PrintLayout::on_item_show_context_menu), 
      item) );

}

void Canvas_PrintLayout::add_layout_group(const sharedptr<LayoutGroup>& group, bool is_top_level)
{
  //row[model_parts->m_columns.m_col_item] = sharedptr<LayoutItem>(static_cast<LayoutItem*>(group->clone()));

  //Add the group item:
  if(!is_top_level)
  {
    Glib::RefPtr<CanvasLayoutItem> canvas_item = CanvasLayoutItem::create(group);
    if(canvas_item)
      add_canvas_layout_item(canvas_item);
  }

  //Add the group's children.
  add_layout_group_children(group);

  m_modified = true;
}


void Canvas_PrintLayout::fill_layout_group(const sharedptr<LayoutGroup>& group)
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
        sharedptr<LayoutItem> layout_item = canvas_item->get_layout_item();
        //std::cout << "DEBUG: saving layout_item type=" << layout_item->get_part_type_name() << std::endl;
        update_layout_position_from_canvas(layout_item, canvas_item);

        group->add_item(layout_item);
      //}
    }

    //TODO: Recurse.
  }
}


void Canvas_PrintLayout::setup_context_menu()
{
  m_context_menu_action_group = Gtk::ActionGroup::create();

  m_context_menu_action_group->add(Gtk::Action::create("ContextMenu", "Context Menu") );

/*
  Glib::RefPtr<Gtk::Action> action =  Gtk::Action::create("ContextInsertField", _("Field"));
  m_context_menu_action_group->add(action,
    sigc::mem_fun(*this, &Canvas_PrintLayout::on_context_menu_insert_field) );

  action =  Gtk::Action::create("ContextInsertText", _("Text"));
  m_context_menu_action_group->add(action,
    sigc::mem_fun(*this, &Canvas_PrintLayout::on_context_menu_insert_text) );
*/

  m_action_edit =  Gtk::Action::create("ContextMenuEdit", Gtk::Stock::EDIT);
  m_context_menu_action_group->add(m_action_edit);
  m_action_formatting =  Gtk::Action::create("ContextMenuFormatting", _("_Formatting"));
  m_context_menu_action_group->add(m_action_formatting);
  m_action_delete =  Gtk::Action::create("ContextMenuDelete", Gtk::Stock::DELETE);
  m_context_menu_action_group->add(m_action_delete);

  m_action_edit->signal_activate().connect( sigc::mem_fun(*this, &Canvas_PrintLayout::on_context_menu_edit) );
  m_action_formatting->signal_activate().connect( sigc::mem_fun(*this, &Canvas_PrintLayout::on_context_menu_formatting) );
  m_action_delete->signal_activate().connect( sigc::mem_fun(*this, &Canvas_PrintLayout::on_context_menu_delete) );


  m_context_menu_uimanager = Gtk::UIManager::create();
  m_context_menu_uimanager->insert_action_group(m_context_menu_action_group);

  #ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
  #endif
    Glib::ustring ui_info = 
      "<ui>"
      "  <popup name='ContextMenu'>"
      "    <menuitem action='ContextMenuEdit' />"
      "    <menuitem action='ContextMenuFormatting' />"
      "    <menuitem action='ContextMenuDelete' />"
      "  </popup>"
      "</ui>";

  #ifdef GLIBMM_EXCEPTIONS_ENABLED
    m_context_menu_uimanager->add_ui_from_string(ui_info);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << "building menus failed: " <<  ex.what();
  }
  #else
  std::auto_ptr<Glib::Error> error;
  m_context_menu_uimanager->add_ui_from_string(ui_info, error);
  if(error.get() != NULL)
  {
    std::cerr << "building menus failed: " << error->what();
  }
  #endif

  //Get the menu:
  m_context_menu = dynamic_cast<Gtk::Menu*>( m_context_menu_uimanager->get_widget("/ContextMenu") ); 
}


void Canvas_PrintLayout::on_item_show_context_menu(guint button, guint32 activate_time, Glib::RefPtr<CanvasLayoutItem> item)
{
  if(!m_context_menu)
    return;

  m_context_item = item;

  m_context_menu->popup(button, activate_time);
}

void Canvas_PrintLayout::update_layout_position_from_canvas(const sharedptr<LayoutItem> layout_item, const Glib::RefPtr<const CanvasLayoutItem>& canvas_item)
{
  if(!layout_item || !canvas_item)
    return;

  //Get the actual position:
  double x = 0;
  double y = 0;
  canvas_item->get_xy(x, y);
  //std::cout << "Canvas_PrintLayout::update_layout_position_from_canvas(): x=" << x << std::endl;

  double width = 0;
  double height = 0;
  canvas_item->get_width_height(width, height);
  layout_item->set_print_layout_position(x, y, width, height); 
}

sharedptr<LayoutItem_Portal> Canvas_PrintLayout::offer_related_records(const sharedptr<LayoutItem_Portal>& portal, Gtk::Window* parent)
{
  sharedptr<LayoutItem_Portal> result = portal;

  Dialog_Layout_List_Related* dialog = 0;

  Glib::RefPtr<Gtk::Builder> refXml = Gtk::Builder::create_from_file(Utils::get_glade_file_path("glom_developer.glade"), "window_data_layout");
  if(refXml)
    refXml->get_widget_derived("window_data_layout", dialog);
  
  if(!dialog)
  {
    std::cerr << "Canvas_PrintLayout::offer_related_records(): dialog was NULL." << std::endl;
    return result;
  }

  add_view(dialog); //Give it access to the document.

  dialog->set_document("layout_name_unused_for_portals", "", /* layout_platform */ get_document(), portal, m_table_name);

  if(parent)
    dialog->set_transient_for(*parent);

  Utils::show_window_until_hide(dialog);

  result = dialog->get_portal_layout();

  delete dialog;
  dialog = 0;

  return result;
}

void Canvas_PrintLayout::on_context_menu_edit()
{
  Gtk::Window* parent = dynamic_cast<Gtk::Window*>(get_toplevel());

  sharedptr<LayoutItem> layout_item = m_context_item->get_layout_item();
  update_layout_position_from_canvas(layout_item, m_context_item);

  sharedptr<LayoutItem_Field> field = sharedptr<LayoutItem_Field>::cast_dynamic(layout_item);
  if(field)
  {
    sharedptr<LayoutItem_Field> field_chosen = offer_field_list(field, m_table_name, parent);
    if(field_chosen)
      m_context_item->set_layout_item(field_chosen);
  }
  else
  {
    sharedptr<LayoutItem_Text> text = sharedptr<LayoutItem_Text>::cast_dynamic(layout_item);
    if(text)
    {
      text = Base_DB::offer_textobject(text, parent, false /* don't show title */);
      m_context_item->set_layout_item(text);
    }
    else
    {
      sharedptr<LayoutItem_Image> image = sharedptr<LayoutItem_Image>::cast_dynamic(layout_item);
      if(image)
      {
        image = Base_DB::offer_imageobject(image, parent, false /* don't show title */);
        m_context_item->set_layout_item(image);
      }
      else
      {
        sharedptr<LayoutItem_Portal> portal = sharedptr<LayoutItem_Portal>::cast_dynamic(layout_item);
        if(portal)
        {
          portal = offer_related_records(portal, parent);
          m_context_item->set_layout_item(portal);
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
  
  sharedptr<LayoutItem> layout_item = m_context_item->get_layout_item();
  update_layout_position_from_canvas(layout_item, m_context_item);

  sharedptr<LayoutItem_Field> layout_item_field = sharedptr<LayoutItem_Field>::cast_dynamic(layout_item);
  sharedptr<LayoutItem_Text> layout_item_text = sharedptr<LayoutItem_Text>::cast_dynamic(layout_item);
  if(!layout_item_field && !layout_item_text)
    return;

  if(m_dialog_format)
  {
     remove_view(m_dialog_format);
     delete m_dialog_format;
     m_dialog_format = 0;
  }

  try
  {
    Glib::RefPtr<Gtk::Builder> refXmlFormatting = Gtk::Builder::create_from_file(Utils::get_glade_file_path("glom_developer.glade"), "window_text_format");
    refXmlFormatting->get_widget_derived("window_text_format", m_dialog_format);
    add_view(m_dialog_format);

    m_dialog_format->signal_hide().connect( sigc::mem_fun(*this, &Canvas_PrintLayout::on_dialog_format_hide) );
  }
  catch(const Gtk::BuilderError& ex)
  {
    std::cerr << ex.what() << std::endl;
  }


  if(!m_dialog_format)
    return;

  //We need an if here, because they have no common base class.
  //TODO: Maybe they should.
  FieldFormatting formatting;
  if(layout_item_field)
    formatting = layout_item_field->m_formatting;
  else
    formatting = layout_item_text->m_formatting;

  m_dialog_format->m_box_formatting->set_formatting(formatting);

  m_dialog_format->show();
}

void Canvas_PrintLayout::on_context_menu_delete()
{
  m_context_item->remove();
  m_context_item.reset();
}

void Canvas_PrintLayout::on_dialog_format_hide()
{
  if(!m_dialog_format || !m_context_item)
    return;

  sharedptr<LayoutItem> layout_item = m_context_item->get_layout_item();
  sharedptr<LayoutItem_Field> layout_item_field = sharedptr<LayoutItem_Field>::cast_dynamic(layout_item);
  sharedptr<LayoutItem_Text> layout_item_text = sharedptr<LayoutItem_Text>::cast_dynamic(layout_item);
  if(!layout_item_field && !layout_item_text)
    return;

  if(layout_item_field)
    m_dialog_format->m_box_formatting->get_formatting(layout_item_field->m_formatting);
  else if(layout_item_text)
    m_dialog_format->m_box_formatting->get_formatting(layout_item_text->m_formatting);

  m_context_item->set_layout_item(layout_item); //Redraw the child item with the new formatting.

  delete m_dialog_format;
  m_dialog_format = 0;
}

Glib::RefPtr<Goocanvas::Polyline> Canvas_PrintLayout::create_margin_line(double x1, double y1, double x2, double y2)
{
  Glib::RefPtr<Goocanvas::Polyline> line = 
    Goocanvas::Polyline::create(x1, y1, x2, y2);
  line->property_line_width() = 0.5;
  line->property_stroke_color() = "light gray";
  m_bounds_group->add_child(line);
  return line;
}

void Canvas_PrintLayout::set_page_setup(const Glib::RefPtr<Gtk::PageSetup>& page_setup)
{
  m_page_setup = page_setup;
  if(!m_page_setup)
    return;

  //Change the scroll extents to match the page size:
  Gtk::PaperSize paper_size = m_page_setup->get_paper_size();
  Goocanvas::Bounds bounds;
  bounds.set_x1(0);
  bounds.set_y1(0);

  const Gtk::Unit units = property_units();
  //std::cout << "Canvas_PrintLayout::set_page_setup(): width=" << paper_size.get_width(units) << ", height=" paper_size.get_height(units) << std::endl;

  if(m_page_setup->get_orientation() == Gtk::PAGE_ORIENTATION_PORTRAIT) //TODO: Handle the reverse orientations too?
  {
    bounds.set_x2( paper_size.get_width(units) );
    bounds.set_y2( paper_size.get_height(units) );
  }
  else
  {
    bounds.set_y2( paper_size.get_width(units) );
    bounds.set_x2( paper_size.get_height(units) );
  }

  //std::cout << "Canvas_PrintLayout::set_page_setup(): portrait page width=" << paper_size.get_width(units) << std::endl;

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
  m_bounds_group->add_child(m_bounds_rect);

  //Make sure that the bounds rect is at the bottom, 
  //and that the grid and margins are just above it:
  if(m_grid)
    m_grid->lower();
  
  m_margin_left = create_margin_line(page_setup->get_left_margin(units), bounds.get_y1(), page_setup->get_left_margin(units), bounds.get_y2());
  m_margin_right = create_margin_line(bounds.get_x2() - page_setup->get_right_margin(units), bounds.get_y1(), bounds.get_x2() - page_setup->get_right_margin(units), bounds.get_y2());
  m_margin_top = create_margin_line(bounds.get_x1(), page_setup->get_top_margin(units), bounds.get_x2(), page_setup->get_top_margin(units));
  m_margin_bottom = create_margin_line(bounds.get_x1(), bounds.get_y2() - page_setup->get_bottom_margin(units), bounds.get_x2(), bounds.get_y2() - page_setup->get_bottom_margin(units));

  m_bounds_group->lower();
}

Glib::RefPtr<Gtk::PageSetup> Canvas_PrintLayout::get_page_setup()
{
  return m_page_setup;
}

void Canvas_PrintLayout::fill_with_data(const FoundSet& found_set)
{
  fill_with_data(m_items_group, found_set);
}

void Canvas_PrintLayout::fill_with_data(const Glib::RefPtr<Goocanvas::Group>& canvas_group, const FoundSet& found_set)
{
  //A map of the text representation (e.g. field_name or relationship::field_name) to the index:
  typedef std::map<Glib::ustring, guint> type_map_layout_fields_index;
  type_map_layout_fields_index map_fields_index;

  typedef std::list< sharedptr<LayoutItem_Portal> > type_list_portals;
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
      
    sharedptr<LayoutItem> layout_item = canvas_item->get_layout_item();
    if(!layout_item)
      continue;
      
    sharedptr<LayoutItem_Field> layoutitem_field = sharedptr<LayoutItem_Field>::cast_dynamic(layout_item);
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
      sharedptr<LayoutItem_Portal> layoutitem_portal = sharedptr<LayoutItem_Portal>::cast_dynamic(layout_item);
      if(layoutitem_portal)
      {
        //Fill the related records table:
        sharedptr<Relationship> relationship = layoutitem_portal->get_relationship();
        if(relationship)
        {
          sharedptr<Field> from_field = get_fields_for_table_one_field(relationship->get_from_table(), relationship->get_from_field());
          const Gnome::Gda::Value from_key_value = get_field_value_in_database(from_field, found_set, 0 /* TODO: window */);
          fill_with_data_portal(canvas_item, from_key_value);
        }
      }
    }
  }

  if(fieldsToGet.empty())
    return;

  Glib::ustring sql_query = Utils::build_sql_select_with_where_clause(found_set.m_table_name,
    fieldsToGet,
    found_set.m_where_clause, Glib::ustring() /* extra_join */, found_set.m_sort_clause);

  if(!sql_query.empty())
    sql_query += " LIMIT 1";
  else
    return;
  
  bool records_found = false;
  Glib::RefPtr<Gnome::Gda::DataModel> datamodel;
  try
  {
    datamodel = query_execute_select(sql_query);
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
      
    sharedptr<LayoutItem> layout_item = canvas_item->get_layout_item();
    if(!layout_item)
      continue;
      
    sharedptr<LayoutItem_Field> layoutitem_field = sharedptr<LayoutItem_Field>::cast_dynamic(layout_item);
    if(layoutitem_field)
    { 
      type_map_layout_fields_index::const_iterator iterFind = map_fields_index.find( layoutitem_field->get_layout_display_name() );
      if(iterFind != map_fields_index.end())
      {
        //Set the data from the database:
        const guint col_index = iterFind->second;
        const Gnome::Gda::Value value = datamodel->get_value_at(col_index, 0);
        canvas_item->set_db_data(value);
      }
    }
    else
    {
      //Clear the no-image pixbuf from images:
      canvas_item->remove_empty_indicators();
    }
  }
}


void Canvas_PrintLayout::fill_with_data_portal(const Glib::RefPtr<CanvasLayoutItem>& canvas_item, const Gnome::Gda::Value& foreign_key_value)
{
  if(!canvas_item)
    return;

  sharedptr<LayoutItem> layout_item = canvas_item->get_layout_item();
  sharedptr<LayoutItem_Portal> portal = sharedptr<LayoutItem_Portal>::cast_dynamic(layout_item);
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

  LayoutGroup::type_list_items child_layout_items = portal->get_items();
  
  //Build and run the SQL query for this portal:
  type_vecLayoutFields fields_shown = get_portal_fields_to_show(portal);

  FoundSet found_set;
  found_set.m_table_name = portal->get_table_used(Glib::ustring() /* parent table_name, not used. */);
  set_found_set_where_clause_for_portal(found_set, portal, foreign_key_value);

  const Glib::ustring sql_query = Utils::build_sql_select_with_where_clause(found_set.m_table_name, fields_shown, found_set.m_where_clause, found_set.m_extra_join, found_set.m_sort_clause, found_set.m_extra_group_by);
  //std::cout << "DEBUG: sql_query=" << sql_query << std::endl;
  Glib::RefPtr<Gnome::Gda::DataModel> datamodel = query_execute_select(sql_query);
  if(!(datamodel))
    return;
    
  const int db_rows_count = datamodel->get_n_rows();
  if(!(db_rows_count > 0))
    return;
    
  const int db_columns_count = datamodel->get_n_columns();
  if(!db_columns_count)
    return;

  double row_height_ignored = 0;
  const int rows_count = CanvasLayoutItem::get_rows_count_for_portal(portal, row_height_ignored);
  const int cols_count = child_layout_items.size();

  //Set the DB value for each cell:
  for(int row = 0; row < rows_count; ++row)
  {
    int db_col = 0;
    LayoutGroup::type_list_items::iterator iter_child_layout_items = child_layout_items.begin();
    for(int col = 0; col < cols_count; ++col)
    {
      //Glib::RefPtr<Goocanvas::Item> canvas_child = base_item->get_cell_child(row, col); //TODO: Add this to GooCanvas::Table.
      Glib::RefPtr<Goocanvas::Item> canvas_child = get_canvas_table_cell_child(canvas_table, row, col); //TODO: Add this to GooCanvas::Table.
      if(!canvas_child)
        std::cerr << "Canvas_PrintLayout::fill_with_data_portal(): canvas_child is NULL." << std::endl;

      if(iter_child_layout_items == child_layout_items.end())
        continue;
      
      sharedptr<LayoutItem> child_layout_item = *iter_child_layout_items;
      sharedptr<LayoutItem_Field> field = sharedptr<LayoutItem_Field>::cast_dynamic(child_layout_item);
      if(field)
      {
        Gnome::Gda::Value db_value;
        if( row < datamodel->get_n_rows() )
          db_value = datamodel->get_value_at(db_col, row);
          
        set_canvas_item_field_value(canvas_child, field, db_value);
        ++db_col;
      }

      ++iter_child_layout_items;
    }
  }
}

void Canvas_PrintLayout::set_canvas_item_field_value(const Glib::RefPtr<Goocanvas::Item> canvas_item, const sharedptr<LayoutItem_Field> field, const Gnome::Gda::Value& value)
{
  if(!field)
    return;

  //Expect the appropriate canvas item, depending on the field type:
  if(field->get_glom_type() == Field::TYPE_IMAGE)
  {
    Glib::RefPtr<CanvasImageMovable> canvas_image = Glib::RefPtr<CanvasImageMovable>::cast_dynamic(canvas_item);
    if(!canvas_image)
      return;

    Glib::RefPtr<Gdk::Pixbuf> pixbuf = Utils::get_pixbuf_for_gda_value(value);
    canvas_image->property_pixbuf() = pixbuf;
  }
  else //text, numbers, date, time, boolean:
  {
    Glib::RefPtr<CanvasTextMovable> canvas_text = Glib::RefPtr<CanvasTextMovable>::cast_dynamic(canvas_item);
    if(!canvas_text)
    {
      std::cerr << "Canvas_PrintLayout::set_canvas_item_field_value(): The canvas item is not of the expected type. Instead it is of type." << std::endl;
      return;
    }

    //FieldFormatting& formatting = field->m_formatting;
    //check_and_apply_formatting(canvas_item, formatting);
    const Glib::ustring text = 
      Conversions::get_text_for_gda_value(field->get_glom_type(), value, field->m_formatting.m_numeric_format);
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

guint Canvas_PrintLayout::get_zoom_percent() const
{
  return (guint)(get_scale() * (double)100);
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

Glib::RefPtr<Goocanvas::Item> Canvas_PrintLayout::get_canvas_table_cell_child(const Glib::RefPtr<Goocanvas::Table>& table, int row, int col)
{
  Glib::RefPtr<Goocanvas::Item> result;

  if(!table)
    return result;

  const int count = table->get_n_children();
  for(int i = 0; i < count; ++i)
  {
    Glib::RefPtr<Goocanvas::Item> child = table->get_child(i);
    if(!child)
      continue;

    int column_value = 0;
    table->get_child_property(child, "column", column_value);
    int row_value = 0;
    table->get_child_property(child, "row", row_value);
       
    //This assumes that all items occupy only one cell:
    if( (column_value == col) &&
        (row_value == row) )
    {
      return child;
    }
  }

  return result;
}



Base_DB::type_vecLayoutFields Canvas_PrintLayout::get_portal_fields_to_show(const sharedptr<LayoutItem_Portal>& portal)
{
  const Document* document = get_document();
  if(!document)
    std::cerr << "Canvas_PrintLayout::get_portal_fields_to_show(): document is NULL." << std::endl;

  if(document && portal)
  {
    Document::type_list_layout_groups mapGroups;
    mapGroups.push_back(portal);

    sharedptr<const Relationship> relationship = portal->get_relationship();
    if(relationship)
    {
      type_vecLayoutFields result = get_table_fields_to_show_for_sequence(portal->get_table_used(Glib::ustring() /* not relevant */), mapGroups);

      //If the relationship does not allow editing, then mark all these fields as non-editable:
      if(!(portal->get_relationship_used_allows_edit()))
      {
        for(type_vecLayoutFields::iterator iter = result.begin(); iter != result.end(); ++iter)
        {
          sharedptr<LayoutItem_Field> item = *iter;
          if(item)
            item->set_editable(false);
        }
      }

      return result;
    }
  }

  return type_vecLayoutFields();
}


} //namespace Glom


