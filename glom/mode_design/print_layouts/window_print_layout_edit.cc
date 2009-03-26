
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

#include "window_print_layout_edit.h"
#include <glom/box_db_table.h>
#include "canvas_layout_item.h"
#include <libglom/data_structure/layout/layoutitem_line.h>
#include <libglom/data_structure/layout/layoutitem_portal.h>
//#include <libgnome/gnome-i18n.h>
#include <libglom/utils.h> //For bold_message()).
#include <gtkmm/scrolledwindow.h>
#include <glibmm/i18n.h>

namespace Glom
{

Window_PrintLayout_Edit::Window_PrintLayout_Edit(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Window(cobject),
  m_entry_name(0),
  m_entry_title(0),
  m_label_table_name(0),
  m_label_table(0),
  m_button_close(0),
  m_box(0),
  m_drag_preview_requested(false),
  m_vruler(0),
  m_hruler(0),
  m_context_menu(0)
{
  set_default_size(640, 480);

  add_view(&m_canvas);

  builder->get_widget("vbox_menu", m_box_menu);
  builder->get_widget("vbox_canvas", m_box_canvas);
  builder->get_widget("vbox_inner", m_box);

  //builder->get_widget("label_name", m_label_name);
  builder->get_widget("label_table_name", m_label_table_name);
  builder->get_widget("entry_name", m_entry_name);
  builder->get_widget("entry_title", m_entry_title);

  builder->get_widget("vruler", m_vruler);
  builder->get_widget("hruler", m_hruler);

  //I'm not sure what set_metric() does, but using Gtk::CENTIMETERS leads to our max being ignored/used-weirdly. murrayc.
  m_hruler->set_metric(Gtk::PIXELS);
  m_vruler->set_metric(Gtk::PIXELS);

  builder->get_widget("handle_box", m_palette_handle_box);


  builder->get_widget("button_close", m_button_close);
  m_button_close->signal_clicked().connect( sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_button_close) );

  m_scrolled_window.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  m_scrolled_window.add(m_canvas);
  m_scrolled_window.show_all();
  m_box_canvas->pack_start(m_scrolled_window);
  m_canvas.show();

  //Make the canvas a drag-and-drop destination:
  const GtkTargetEntry* target_entry = egg_tool_palette_get_drag_target_item();
  Gtk::TargetEntry toolbar_target(*target_entry);
  m_drag_targets.push_back(toolbar_target);

  //Note that we don't use Gtk::DEST_DEFAULT_DEFAULTS because that would prevent our signal handlers from being used:
  m_canvas.drag_dest_set(m_drag_targets, Gtk::DEST_DEFAULT_HIGHLIGHT, Gdk::ACTION_COPY);
  m_canvas.signal_drag_drop().connect(
      sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_canvas_drag_drop) );
  m_canvas.signal_drag_motion().connect(
      sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_canvas_drag_motion) );
  m_canvas.signal_drag_data_received().connect(
      sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_canvas_drag_data_received) );
  m_canvas.signal_drag_leave().connect(
      sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_canvas_drag_leave) );

  init_menu();

  m_palette_handle_box->add(m_toolbar);
  m_toolbar.show();

  m_scrolled_window.get_hadjustment()->signal_changed().connect(
    sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_scroll_value_changed) );
  m_scrolled_window.get_hadjustment()->signal_value_changed().connect(
    sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_scroll_value_changed) );
  m_scrolled_window.get_vadjustment()->signal_changed().connect(
    sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_scroll_value_changed) );
  m_scrolled_window.get_vadjustment()->signal_value_changed().connect(
    sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_scroll_value_changed) );

  //Fill composite view:
  //add_view(m_box);

  setup_context_menu();
  m_canvas.signal_show_context().connect(sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_canvas_show_context_menu));

  show_all_children();
}

void Window_PrintLayout_Edit::init_menu()
{
  m_action_group = Gtk::ActionGroup::create();

  m_action_group->add(Gtk::Action::create("Menu_File", _("_File")));
  m_action_group->add(Gtk::Action::create("Action_Menu_File_PageSetup", _("Page _Setup")),
    sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_file_page_setup));

  m_action_group->add(Gtk::Action::create("Menu_Edit", Gtk::Stock::EDIT));
  m_action_group->add(Gtk::Action::create("Action_Menu_Edit_Cut", Gtk::Stock::CUT));
  m_action_group->add(Gtk::Action::create("Action_Menu_Edit_Copy", Gtk::Stock::COPY));
  m_action_group->add(Gtk::Action::create("Action_Menu_Edit_Paste", Gtk::Stock::PASTE));
  m_action_group->add(Gtk::Action::create("Action_Menu_Edit_Delete", Gtk::Stock::DELETE));

  m_action_group->add(Gtk::Action::create("Menu_Insert", _("_Insert")));
  m_action_group->add(Gtk::Action::create("Action_Menu_Insert_Field", _("Insert _Field")),
                        sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_insert_field) );
  m_action_group->add(Gtk::Action::create("Action_Menu_Insert_Text", _("Insert _Text")),
                        sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_insert_text) );
  m_action_group->add(Gtk::Action::create("Action_Menu_Insert_Image", _("Insert _Image")),
                        sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_insert_image) );
  m_action_group->add(Gtk::Action::create("Action_Menu_Insert_RelatedRecords", _("Insert _Related Records")),
                        sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_insert_relatedrecords) );
  m_action_group->add(Gtk::Action::create("Action_Menu_Insert_LineHorizontal", _("Insert _Horizontal Line")),
                        sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_insert_line_horizontal) );
  m_action_group->add(Gtk::Action::create("Action_Menu_Insert_LineVertical", _("Insert _Vertical Line")),
                        sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_insert_line_vertical) );

  m_action_group->add(Gtk::Action::create("Menu_View", _("_View")));
  m_action_showgrid = Gtk::ToggleAction::create("Action_Menu_View_ShowGrid", _("Show Grid"));
  m_action_group->add(m_action_showgrid, sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_view_show_grid));
  m_action_showrules = Gtk::ToggleAction::create("Action_Menu_View_ShowRules", _("Show Rules"));
  m_action_group->add(m_action_showrules, sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_view_show_rules));

  Gtk::RadioAction::Group group_zoom;
  m_action_zoom_fit_page_width = Gtk::RadioAction::create(group_zoom, "Action_Menu_View_ZoomFitPageWidth", _("Fit Page _Width"));
  m_action_group->add(m_action_zoom_fit_page_width,
    sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_view_fitpagewidth));
  m_action_group->add(Gtk::RadioAction::create(group_zoom, "Action_Menu_View_Zoom200", _("Zoom 200%")),
    sigc::bind( sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_view_zoom), 200));
  m_action_group->add(Gtk::RadioAction::create(group_zoom, "Action_Menu_View_Zoom100", Gtk::Stock::ZOOM_100),
    sigc::bind( sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_view_zoom), 100));

  Glib::RefPtr<Gtk::Action> action_50 = Gtk::RadioAction::create(group_zoom, "Action_Menu_View_Zoom50", _("Zoom 50%"));
  m_action_group->add(action_50,
    sigc::bind( sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_view_zoom), 50));
  m_action_zoom_fit_page_width->activate();

  m_action_group->add(Gtk::RadioAction::create(group_zoom, "Action_Menu_View_Zoom25", _("Zoom 25%")),
    sigc::bind( sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_view_zoom), 25));

  //Build part of the menu structure, to be merged in by using the "PH" placeholders:
  static const Glib::ustring ui_description =
    "<ui>"
    "  <menubar name='Menubar'>"
    "      <menu action='Menu_File'>"
    "        <menuitem action='Action_Menu_File_PageSetup' />"
    "      </menu>"
    "      <menu action='Menu_Edit'>"
    "        <menuitem action='Action_Menu_Edit_Cut' />"
    "        <menuitem action='Action_Menu_Edit_Copy' />"
    "        <menuitem action='Action_Menu_Edit_Paste' />"
    "        <menuitem action='Action_Menu_Edit_Delete' />"
    "      </menu>"
    "      <menu action='Menu_Insert'>"
    "        <menuitem action='Action_Menu_Insert_Field' />"
    "        <menuitem action='Action_Menu_Insert_Text' />"
    "        <menuitem action='Action_Menu_Insert_Image' />"
    "        <menuitem action='Action_Menu_Insert_RelatedRecords' />"
    "        <menuitem action='Action_Menu_Insert_LineHorizontal' />"
    "        <menuitem action='Action_Menu_Insert_LineVertical' />"
    "      </menu>"
    "      <menu action='Menu_View'>"
    "        <menuitem action='Action_Menu_View_ShowGrid' />"
    "        <menuitem action='Action_Menu_View_ShowRules' />"
    "        <separator />"
    "        <menuitem action='Action_Menu_View_ZoomFitPageWidth' />"
    "        <menuitem action='Action_Menu_View_Zoom200' />"
    "        <menuitem action='Action_Menu_View_Zoom100' />"
    "        <menuitem action='Action_Menu_View_Zoom50' />"
    "        <menuitem action='Action_Menu_View_Zoom25' />"
    "      </menu>"
    "  </menubar>"
    "</ui>";

  //Add menu:
  m_uimanager = Gtk::UIManager::create();
  m_uimanager->insert_action_group(m_action_group);
  m_uimanager->add_ui_from_string(ui_description);

  //Menubar:
  Gtk::MenuBar* pMenuBar = static_cast<Gtk::MenuBar*>(m_uimanager->get_widget("/Menubar"));
  m_box_menu->pack_start(*pMenuBar, Gtk::PACK_SHRINK);
  pMenuBar->show();

  //TODO: Add a toolbar if it would be useful:
  //Gtk::Toolbar* pToolBar = static_cast<Gtk::Toolbar*>(m_uimanager->get_widget("/Bakery_ToolBar"));
  //m_HandleBox_Toolbar.add(*pToolBar);
  //m_HandleBox_Toolbar.show();

  add_accel_group(m_uimanager->get_accel_group());
}

Glib::RefPtr<Gdk::Pixbuf> Window_PrintLayout_Edit::get_icon_for_toolbar_item(Gtk::ToolItem& item)
{
  Glib::RefPtr<Gdk::Pixbuf> result;

  //Set the icon to show when dragging:
  Glib::RefPtr<Gtk::Action> action = item.get_action();
  if(!action)
    return result;

  const Gtk::StockID stock_id = action->property_stock_id();
  if(!(stock_id.get_string().empty())) //The operator bool() is only in later versions of gtkmm 2.*.x
  {
    result = item.render_icon(stock_id, Gtk::ICON_SIZE_LARGE_TOOLBAR);
  }
  else
  {
    const Glib::ustring icon_name = action->property_icon_name();

    Glib::RefPtr<Gdk::Screen> screen = item.get_screen();
    if(!screen)
      return result;

    int width = 0;
    int height = 0;
    if(!Gtk::IconSize::lookup(Gtk::ICON_SIZE_LARGE_TOOLBAR, width, height))
    {
      //An arbitrary default:
      width = height = 24;
    }

    Glib::RefPtr<Gtk::IconTheme> icon_theme = Gtk::IconTheme::get_for_screen(screen);
    if(!icon_theme)
      return result;

    result = icon_theme->load_icon(icon_name, MIN(width, height), (Gtk::IconLookupFlags)0);
  }

  return result;
}

/*
void Window_PrintLayout_Edit::make_toolbar_items_draggable()
{
  const int count = m_toolbar->get_n_items();
  for(int i = 0; i < count; ++i)
  {
    Gtk::ToolItem* item = m_toolbar->get_nth_item(i);
    if(!item)
      continue;

    //Allow this widget to be dragged:
    item->set_use_drag_window();
    item->drag_source_set(m_drag_targets, Gdk::BUTTON1_MASK, Gdk::ACTION_COPY);

    //Set the icon to be shown when dragging:
    Glib::RefPtr<Gdk::Pixbuf> pixbuf = get_icon_for_toolbar_item(*item);
    if(pixbuf)
      item->drag_source_set_icon(pixbuf);

    //item->signal_drag_begin().connect(
    //  sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_toolbar_item_drag_begin) );

    //item->signal_drag_end().connect(
    //  sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_toolbar_item_drag_end) );

    //Let the item supply some data when the destination asks for it:
    Glib::RefPtr<Gtk::Action> action = item->get_action();
    item->signal_drag_data_get().connect(
      sigc::bind( sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_toolbar_item_drag_data_get), action) );
  }
}
*/

/*
void Window_PrintLayout_Edit::on_toolbar_item_drag_begin(const Glib::RefPtr<Gdk::DragContext>& drag_context)
{
  std::cout << "Window_PrintLayout_Edit::on_toolbar_item_drag_begin" << std::endl;
}

void Window_PrintLayout_Edit::on_toolbar_item_drag_end(const Glib::RefPtr<Gdk::DragContext>& drag_context)
{
  std::cout << "Window_PrintLayout_Edit::on_toolbar_item_drag_end" << std::endl;
}
*/


//TODO: I don't know what this really means. murrayc.
const int DRAG_DATA_FORMAT = 8; // 8 bits format

void Window_PrintLayout_Edit::on_toolbar_item_drag_data_get(const Glib::RefPtr<Gdk::DragContext>& drag_context, Gtk::SelectionData& selection_data, guint /* info */, guint /* time */)
{
  PrintLayoutToolbarButton::enumItems type = PrintLayoutToolbarButton::get_item_type_from_selection_data(drag_context, selection_data);

  selection_data.set(selection_data.get_target(), DRAG_DATA_FORMAT,
          (const guchar*)&type,
          1 /* 1 byte */);
}


bool Window_PrintLayout_Edit::on_canvas_drag_drop(const Glib::RefPtr<Gdk::DragContext>& drag_context, int /* x */, int /* y */, guint timestamp)
{
  const Glib::ustring target = m_canvas.drag_dest_find_target(drag_context);
  if(target.empty())
    return false;

  //Cause our drag_data_received callback to be called:
  //Note that this isn't necessary when using DEST_DEFAULT_DEFAULTS (or DEST_DEFAULT_DROP), 
  //because that would allow us to just return true to make this happen automatically.
  m_canvas.drag_get_data(drag_context, target, timestamp);

  return true; //Allow the drop.
}

bool Window_PrintLayout_Edit::on_canvas_drag_motion(const Glib::RefPtr<Gdk::DragContext>& drag_context, int x, int y, guint timestamp)
{
  const Glib::ustring target = m_canvas.drag_dest_find_target(drag_context);
  if(target.empty())
    return false;

  m_canvas.drag_highlight();

  //Create the temporary canvas item if necesary:
  if(!m_layout_item_dropping)
  {
    //We need to examine the SelectionData:
    //This will cause our drag_data_received callback to be called, with that information.
    //Note: This does not work (and grabs the cursor) if we call dest_set() with the flags for default actions, such as DEST_DEFAULT_DEFAULTS.
    m_drag_preview_requested = true;
    m_canvas.drag_get_data(drag_context, target, timestamp);
    return true;
  }

  drag_context->drag_status(Gdk::ACTION_COPY, timestamp);

  //Move the temporary canvas item to the new position:
  double item_x = x;
  double item_y = y;
  m_canvas.convert_from_pixels(item_x, item_y);

  m_layout_item_dropping->set_xy(item_x, item_y);

  return true; //Allow the drop.
}

sharedptr<LayoutItem> Window_PrintLayout_Edit::create_empty_item(PrintLayoutToolbarButton::enumItems item_type)
{
  sharedptr<LayoutItem> layout_item;

  if(item_type == PrintLayoutToolbarButton::ITEM_FIELD)
  {
    layout_item = sharedptr<LayoutItem_Field>::create();
    layout_item->set_print_layout_position(0, 0, 50, 10);
  }
  else if(item_type == PrintLayoutToolbarButton::ITEM_TEXT)
  {
    sharedptr<LayoutItem_Text> layout_item_derived = sharedptr<LayoutItem_Text>::create();

    // Note to translators: This is the default contents of a text item on a print layout: 
    layout_item_derived->set_text(_("text"));
    layout_item = layout_item_derived;
    layout_item->set_print_layout_position(0, 0, 50, 10);
  }
  else if(item_type == PrintLayoutToolbarButton::ITEM_IMAGE)
  {
    layout_item = sharedptr<LayoutItem_Image>::create();
    layout_item->set_print_layout_position(0, 0, 50, 50);
  }
  else if(item_type == PrintLayoutToolbarButton::ITEM_LINE_HORIZONTAL)
  {
    sharedptr<LayoutItem_Line> layout_item_derived = sharedptr<LayoutItem_Line>::create();
    layout_item_derived->set_coordinates(0, 0, 100, 0);
    layout_item = layout_item_derived;
  }
  else if(item_type == PrintLayoutToolbarButton::ITEM_LINE_VERTICAL)
  {
    sharedptr<LayoutItem_Line> layout_item_derived = sharedptr<LayoutItem_Line>::create();
    layout_item_derived->set_coordinates(0, 0, 0, 100);
    layout_item = layout_item_derived;
  }
  else if(item_type == PrintLayoutToolbarButton::ITEM_PORTAL)
  {
    sharedptr<LayoutItem_Portal> portal = sharedptr<LayoutItem_Portal>::create();
    portal->set_print_layout_row_height(10); //Otherwise it will be 0, which is useless.
    layout_item = portal;
    layout_item->set_print_layout_position(0, 0, 100, 50);
  }
  else
  {
    std::cerr << "Window_PrintLayout_Edit::create_empty_item(): Unhandled item type: " << item_type << std::endl;
  }

  return layout_item;
}

void Window_PrintLayout_Edit::on_canvas_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& drag_context, int x, int y, const Gtk::SelectionData& selection_data, guint /* info */, guint timestamp)
{
  //This is called when an item is dropped on the canvas,
  //or after our drag_motion handler has called drag_get_data()): 
  
  //Discover what toolbar item was dropped:
  const PrintLayoutToolbarButton::enumItems item_type = PrintLayoutToolbarButton::get_item_type_from_selection_data(drag_context, selection_data);
  
  if(m_drag_preview_requested)
  {
    //Create the temporary drag item if necessary:
    if(!m_layout_item_dropping)
    {
      sharedptr<LayoutItem> layout_item = create_empty_item(item_type);

      //Show it on the canvas, at the position:
      if(layout_item)
      {
        m_layout_item_dropping = CanvasLayoutItem::create(layout_item);
        m_canvas.add_canvas_layout_item(m_layout_item_dropping);

        double item_x = x;
        double item_y = y;
        m_canvas.convert_from_pixels(item_x, item_y);
        m_layout_item_dropping->set_xy(item_x, item_y);
      }
    }

    drag_context->drag_status(Gdk::ACTION_COPY, timestamp);
    m_drag_preview_requested = false;
  }
  else
  {
    //Drop an item:
    drag_context->drag_finish(false, false, timestamp);
    m_canvas.drag_unhighlight();

    //Add the item to the canvas:
    sharedptr<LayoutItem> layout_item = create_empty_item(item_type);
    Glib::RefPtr<CanvasLayoutItem> item = CanvasLayoutItem::create(layout_item);
    m_canvas.add_canvas_layout_item(item);
    double item_x = x;
    double item_y = y;
    m_canvas.convert_from_pixels(item_x, item_y);
    item->set_xy(item_x, item_y);
   
    if(m_layout_item_dropping)
    {
      m_layout_item_dropping->remove();
      m_layout_item_dropping.reset();
    }
  }
}


void Window_PrintLayout_Edit::on_canvas_drag_leave(const Glib::RefPtr<Gdk::DragContext>& /* drag_context */, guint /* timestamp */)
{
  //Remove the temporary drag item if the cursor was dragged out of the drop widget:
  if(m_layout_item_dropping)
  {
    m_layout_item_dropping->remove();
    m_layout_item_dropping.reset();
  }
}


Window_PrintLayout_Edit::~Window_PrintLayout_Edit()
{
  remove_view(&m_canvas);
}

bool Window_PrintLayout_Edit::init_db_details(const Glib::ustring& table_name)
{
  Document_Glom* document = dynamic_cast<Document_Glom*>(get_document());
  if(!document)
    return false;

  Glib::ustring table_label = _("None selected");

  //Show the table title (if any) and name:
  Glib::ustring table_title = document->get_table_title(table_name);
  if(table_title.empty())
    table_label = table_name;
  else
    table_label = table_title + " (" + table_name + ")";

  if(m_label_table)
    m_label_table->set_text(table_label);

  return true;

/*
  if(m_box)
  {
    m_box->load_from_document();

    Dialog_Design::init_db_details(table_name);

    m_box->init_db_details(table_name);
  }
*/
  return true;
}


Glib::ustring Window_PrintLayout_Edit::get_original_name() const
{
  return m_name_original;
}

void Window_PrintLayout_Edit::set_print_layout(const Glib::ustring& table_name, const sharedptr<const PrintLayout>& print_layout)
{
  m_modified = false;

  m_name_original = print_layout->get_name();
  m_print_layout = sharedptr<PrintLayout>(new PrintLayout(*print_layout)); //Copy it, so we only use the changes when we want to.
  m_canvas.set_print_layout(table_name, m_print_layout);
  m_table_name = table_name;

  //Dialog_Layout::set_document(layout, document, table_name, table_fields);

  //Set the table name and title:
  m_label_table_name->set_text(table_name);

  m_entry_name->set_text(print_layout->get_name()); 
  m_entry_title->set_text(print_layout->get_title());

  set_ruler_sizes();

  m_modified = false;
}



void Window_PrintLayout_Edit::enable_buttons()
{

}

sharedptr<PrintLayout> Window_PrintLayout_Edit::get_print_layout()
{
  m_print_layout = m_canvas.get_print_layout();
  m_print_layout->set_name( m_entry_name->get_text() );
  m_print_layout->set_title( m_entry_title->get_text() );

/*
  m_print_layout->m_layout_group->remove_all_items();

  m_print_layout->m_layout_group->remove_all_items();

  //The Header and Footer parts are implicit (they are the whole header or footer treeview)
  sharedptr<LayoutItem_Header> header = sharedptr<LayoutItem_Header>::create();
  sharedptr<LayoutGroup> group_temp = header;
  fill_print_layout_parts(group_temp, m_model_parts_header);
  if(header->get_items_count())
    m_print_layout->m_layout_group->add_item(header);

  fill_print_layout_parts(m_print_layout->m_layout_group, m_model_parts_main);

  sharedptr<LayoutItem_Footer> footer = sharedptr<LayoutItem_Footer>::create();
  group_temp = footer;
  fill_print_layout_parts(group_temp, m_model_parts_footer);
  if(footer->get_items_count())
    m_print_layout->m_layout_group->add_item(footer);

*/
  return m_print_layout;
}

void Window_PrintLayout_Edit::on_context_menu_insert_field()
{
  on_menu_insert_field();
}

void Window_PrintLayout_Edit::on_context_menu_insert_text()
{
  on_menu_insert_text();
}

void Window_PrintLayout_Edit::setup_context_menu()
{
  m_context_menu_action_group = Gtk::ActionGroup::create();

  m_context_menu_action_group->add(Gtk::Action::create("ContextMenu", "Context Menu") );
  m_context_menu_action_group->add(Gtk::Action::create("ContextMenuInsert", _("Insert")) );

  Glib::RefPtr<Gtk::Action> action =  Gtk::Action::create("ContextInsertField", _("Field"));
  m_context_menu_action_group->add(action,
    sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_context_menu_insert_field) );

  action =  Gtk::Action::create("ContextInsertText", _("Text"));
  m_context_menu_action_group->add(action,
    sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_context_menu_insert_text) );

  /*
  action =  Gtk::Action::create("ContextDelete", Gtk::Stock::DELETE);
  m_context_menu_action_group->add(action,
    sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_context_menu_delete) );
  */

  m_context_menu_uimanager = Gtk::UIManager::create();
  m_context_menu_uimanager->insert_action_group(m_context_menu_action_group);

  #ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
  #endif
    Glib::ustring ui_info = 
      "<ui>"
      "  <popup name='ContextMenu'>"
      "    <menu action='ContextMenuInsert'>"
      "      <menuitem action='ContextInsertField'/>"
      "      <menuitem action='ContextInsertText'/>"
      "    </menu>"
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


void Window_PrintLayout_Edit::on_canvas_show_context_menu(guint button, guint32 activate_time)
{
  if(m_context_menu)
    m_context_menu->popup(button, activate_time);
}

bool Window_PrintLayout_Edit::get_is_item_at(double x, double y)
{
  Glib::RefPtr<Goocanvas::Item> item_hit = m_canvas.get_item_at(x, y, false);
  if(!item_hit)
   return false;

  Glib::RefPtr<CanvasLayoutItem> layout_item = Glib::RefPtr<CanvasLayoutItem>::cast_dynamic(item_hit);
  return layout_item;
}

void Window_PrintLayout_Edit::set_default_position(const sharedptr<LayoutItem>& item)
{
  if(!item)
    return;

  double item_x = 10;
  double item_y = 10;
  m_canvas.convert_from_pixels(item_x, item_y);
  
  //TODO: This doesn't seem to actually work:
  while(get_is_item_at(item_x, item_y))
  {
    item_x += 10;
    item_y += 10;
  }

  //Get the old position so we can preserve the width and height:
  double old_x = 0;
  double old_y = 0;
  double old_width = 0;
  double old_height = 0;
  item->get_print_layout_position(old_x, old_y, old_width, old_height);

  item->set_print_layout_position(item_x, item_y, old_width, old_height);
}

void Window_PrintLayout_Edit::on_menu_insert_field()
{
  sharedptr<LayoutItem> layout_item = create_empty_item(PrintLayoutToolbarButton::ITEM_FIELD);

  // Note to translators: This is the default contents of a text item on a print layout: 
  set_default_position(layout_item);

  Glib::RefPtr<CanvasLayoutItem> item = CanvasLayoutItem::create(layout_item);
  m_canvas.add_canvas_layout_item(item);
}

void Window_PrintLayout_Edit::on_menu_insert_text()
{
  sharedptr<LayoutItem> layout_item = create_empty_item(PrintLayoutToolbarButton::ITEM_TEXT);
  set_default_position(layout_item);

  Glib::RefPtr<CanvasLayoutItem> item = CanvasLayoutItem::create(layout_item);
  m_canvas.add_canvas_layout_item(item);
}

void Window_PrintLayout_Edit::on_menu_insert_image()
{
  sharedptr<LayoutItem> layout_item = create_empty_item(PrintLayoutToolbarButton::ITEM_IMAGE);
  // Note to translators: This is the default contents of a text item on a print layout: 
  //layout_item->set_text(_("text"));
  set_default_position(layout_item);

  Glib::RefPtr<CanvasLayoutItem> item = CanvasLayoutItem::create(layout_item);
  m_canvas.add_canvas_layout_item(item);
}

void Window_PrintLayout_Edit::on_menu_insert_relatedrecords()
{
  sharedptr<LayoutItem> layout_item = create_empty_item(PrintLayoutToolbarButton::ITEM_PORTAL);
  set_default_position(layout_item);

  Glib::RefPtr<CanvasLayoutItem> item = CanvasLayoutItem::create(layout_item);
  m_canvas.add_canvas_layout_item(item);
}

void Window_PrintLayout_Edit::on_menu_insert_line_horizontal()
{
  sharedptr<LayoutItem> layout_item = create_empty_item(PrintLayoutToolbarButton::ITEM_LINE_HORIZONTAL);

  /*
  double item_x = m_drop_x;
  double item_y = m_drop_y;
  m_canvas.convert_from_pixels(item_x, item_y);
  */

  // Note to translators: This is the default contents of a text item on a print layout: 
  //layout_item->set_text(_("text"));
  //layout_item->set_coordinates(item_x, item_y, item_x + 100, item_y);

  Glib::RefPtr<CanvasLayoutItem> item = CanvasLayoutItem::create(layout_item);
  m_canvas.add_canvas_layout_item(item);
}

void Window_PrintLayout_Edit::on_menu_insert_line_vertical()
{
  sharedptr<LayoutItem> layout_item = create_empty_item(PrintLayoutToolbarButton::ITEM_LINE_VERTICAL);

  /*
  double item_x = m_drop_x;
  double item_y = m_drop_y;
  m_canvas.convert_from_pixels(item_x, item_y);
  */

  // Note to translators: This is the default contents of a text item on a print layout: 
  //layout_item->set_text(_("text"));
  //layout_item->set_coordinates(item_x, item_y, item_x, item_y + 100);

  Glib::RefPtr<CanvasLayoutItem> item = CanvasLayoutItem::create(layout_item);
  m_canvas.add_canvas_layout_item(item);
}

void Window_PrintLayout_Edit::on_button_close()
{
  hide();
}

void Window_PrintLayout_Edit::on_menu_view_show_grid()
{
  if(m_action_showgrid->get_active())
  {
    m_canvas.set_grid_gap(20);
  }
  else
  {
    m_canvas.remove_grid();
  }
}

void Window_PrintLayout_Edit::on_menu_view_show_rules()
{
  //TODO:
}

void Window_PrintLayout_Edit::on_menu_view_zoom(guint percent)
{
  m_canvas.set_zoom_percent(percent);
}

void Window_PrintLayout_Edit::on_menu_view_fitpagewidth()
{
  //Get the canvas size:
  Goocanvas::Bounds bounds;
  m_canvas.get_bounds(bounds);
  
  double canvas_width_pixels = bounds.get_x2() - bounds.get_x1();
  double canvas_height_pixels = bounds.get_y2() - bounds.get_y1();
  m_canvas.convert_to_pixels(canvas_width_pixels, canvas_height_pixels);
  canvas_width_pixels = canvas_width_pixels / m_canvas.property_scale();

  //Get the viewport size:
  Gtk::Widget* child = m_scrolled_window.get_child();
  if(child)
  {
    Gtk::Allocation widget_allocation = child->get_allocation();
    const double viewport_width = widget_allocation.get_width();
    if(canvas_width_pixels)
    {
      //scale the canvas so it fits perfectly in the viewport:
      const double scale = viewport_width / canvas_width_pixels;
      m_canvas.set_zoom_percent((guint)(scale * 100));
    }
  }
}

void Window_PrintLayout_Edit::on_menu_file_page_setup()
{
  Glib::RefPtr<Gtk::PageSetup> page_setup = m_canvas.get_page_setup();

  //Show the page setup dialog, asking it to start with the existing settings:
  Glib::RefPtr<Gtk::PrintSettings> print_settings = Gtk::PrintSettings::create(); //TODO: Do we really need to get this from the user and store it?
  page_setup = Gtk::run_page_setup_dialog(*this, page_setup, print_settings);

  //Save the chosen page setup dialog for use when printing, previewing, or
  //showing the page setup dialog again:
  m_canvas.set_page_setup(page_setup);

  set_ruler_sizes();
}

void Window_PrintLayout_Edit::set_ruler_sizes()
{
  //Note: We should use the page size if we decide not to make the canvas bounds == page size.
  on_scroll_value_changed();

  /*
  double x1 = 0;
  double y1 = 0;
  double x2 = 0;
  double y2 = 0;
  m_canvas.get_bounds(x1, y1, x2, y2);

  m_hruler->set_range(x1, x2, 0, x2);
  m_vruler->set_range(y1, y2, 0, y2);
  */
}

void Window_PrintLayout_Edit::on_scroll_value_changed()
{
  double width = m_scrolled_window.get_hadjustment()->get_page_size();
  double height = m_scrolled_window.get_vadjustment()->get_page_size();
  double x = m_scrolled_window.get_hadjustment()->get_value();
  double y = m_scrolled_window.get_vadjustment()->get_value();
  
  //This definitely seems to give the correct mm values.
  //(It understands the canvas units and scale):
  m_canvas.convert_from_pixels(width, height); 
  m_canvas.convert_from_pixels(x, y);

  //std::cout << "DEBUG: Calling m_hruler->set_range(" << x << ", " << x + width << ", 0, " <<  x + width << std::endl;

  m_hruler->set_range(x, x + width, 0, x + width);
  m_vruler->set_range(y, y + height, 0, y + height);
}

bool Window_PrintLayout_Edit::on_configure_event(GdkEventConfigure* event)
{
  const bool result = Gtk::Window::on_configure_event(event);

  //If we are in fit-page-width then rescale the canvas:
  if(m_action_zoom_fit_page_width->get_active())
    on_menu_view_fitpagewidth();

  return result;
}


} //namespace Glom


