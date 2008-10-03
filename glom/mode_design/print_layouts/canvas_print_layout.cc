
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
#include <bakery/App/App_Gtk.h> //For util_bold_message().
#include <gtkmm/stock.h>
#include <glom/mode_design/print_layouts/dialog_text_formatting.h>
#include <glom/mode_data/dialog_layout_list_related.h>
#include <glom/libglom/glade_utils.h>
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
  //TODO: How does this have any effect?: m_items_group->property_pointer_events() = Goocanvas::CANVAS_EVENTS_NONE;
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

  //TODO: Use PageSetup::copy() when we can use gtkmm 2.14:
  Glib::RefPtr<Gtk::PageSetup> page_setup;
  if(print_layout->get_page_setup())
    page_setup = Glib::wrap( gtk_page_setup_copy(const_cast<GtkPageSetup*>(print_layout->get_page_setup()->gobj())) );

  set_page_setup(page_setup);

  m_modified = false;
}

sharedptr<PrintLayout> Canvas_PrintLayout::get_print_layout()
{
  sharedptr<PrintLayout> result = sharedptr<PrintLayout>::create();
  fill_layout_group(result->m_layout_group);
  result->set_page_setup(m_page_setup);
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
        std::cout << "DEBUG: saving layout_item type=" << layout_item->get_part_type_name() << std::endl;
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

  Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(Utils::get_glade_file_path("glom_developer.glade"), "window_data_layout");
  if(refXml)
    refXml->get_widget_derived("window_data_layout", dialog);
  
  if(!dialog)
  {
    std::cerr << "Canvas_PrintLayout::offer_related_records(): dialog was NULL." << std::endl;
    return result;
  }

  add_view(dialog); //Give it access to the document.

  dialog->set_document("TODO_layout_name", get_document(), portal, m_table_name);

  if(parent)
    dialog->set_transient_for(*parent);

  Utils::show_window_until_hide(dialog);

  if(portal)
  {
    double x = 0;
    double y = 0;
    double width =0;
    double height = 0;
    portal->get_print_layout_position(x, y, width, height);
    std::cout << "DEBUG before: x=" << x << ", y=" << y << ", width=" << width << ", height=" << height << std::endl;
  }

  result = dialog->get_portal_layout();
  if(result)
  {
    double x = 0;
    double y = 0;
    double width =0;
    double height = 0;
    result->get_print_layout_position(x, y, width, height);
    std::cout << "DEBUG after: x=" << x << ", y=" << y << ", width=" << width << ", height=" << height << std::endl;
  }

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

  m_context_item.clear();
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
    Glib::RefPtr<Gnome::Glade::Xml> refXmlFormatting = Gnome::Glade::Xml::create(Utils::get_glade_file_path("glom_developer.glade"), "window_text_format");
    refXmlFormatting->get_widget_derived("window_text_format", m_dialog_format);
    add_view(m_dialog_format);

    m_dialog_format->signal_hide().connect( sigc::mem_fun(*this, &Canvas_PrintLayout::on_dialog_format_hide) );
  }
  catch(const Gnome::Glade::XmlError& ex)
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
  m_context_item.clear();
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
    m_bounds_group.clear();
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

void Canvas_PrintLayout::fill_with_data(const Glib::RefPtr<Goocanvas::Group>& layout_group, const FoundSet& found_set)
{
  //A map of the text representation (e.g. field_name or relationship::field_name) to the index:
  typedef std::map<Glib::ustring, guint> type_map_layout_fields_index;
  type_map_layout_fields_index map_fields_index;
  
  //Get list of fields to get from the database.
  Utils::type_vecLayoutFields fieldsToGet;
  const int count = layout_group->get_n_children();
  guint field_i = 0;
  for(int i = 0; i < count; ++i)
  {
    Glib::RefPtr<Goocanvas::Item> base_canvas_item = layout_group->get_child(i);
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
        fill_with_data(canvas_item, found_set);
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
    datamodel = query_execute(sql_query);
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
    Glib::RefPtr<Goocanvas::Item> base_canvas_item = layout_group->get_child(i);
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


void Canvas_PrintLayout::fill_with_data_portal(const sharedptr<CanvasLayoutItem>& layout_portal, const FoundSet& found_set)
{
  //TODO.
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
  m_bounds_group->property_visibility() = Goocanvas::CANVAS_ITEM_HIDDEN;
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

} //namespace Glom


