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

#include "dialog_relationships_overview.h"
#include "../mode_data/dialog_choose_relationship.h"
#include "printoperation_relationshipsoverview.h"
#include <goocanvas.h>
#include <glibmm/i18n.h>
#include <algorithm>

namespace Glom
{

//static:
int Dialog_RelationshipsOverview::m_last_size_x = 0;
int Dialog_RelationshipsOverview::m_last_size_y = 0;

Dialog_RelationshipsOverview::TableView::TableView()
: x1(0),
  y1(0),
  x2(0),
  y2(0)
{
}

Dialog_RelationshipsOverview::Dialog_RelationshipsOverview(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
  : Gtk::Dialog(cobject),
    m_menu(0),
    m_modified(false),
    m_dragging(false),
    m_drag_x(0),
    m_drag_y(0)
{
  //Add a menu:
  Gtk::VBox* vbox = 0;
  refGlade->get_widget("vbox_placeholder_menubar", vbox);

  m_refActionGroup = Gtk::ActionGroup::create();

  m_refActionGroup->add(Gtk::Action::create("Overview_MainMenu_File", _("_File")) );
  m_refActionGroup->add(Gtk::Action::create("Overview_MainMenu_File_Print", Gtk::Stock::PRINT),
    sigc::mem_fun(*this, &Dialog_RelationshipsOverview::on_menu_file_print) );

  Glib::RefPtr<Gtk::UIManager> m_refUIManager = Gtk::UIManager::create();

  m_refUIManager->insert_action_group(m_refActionGroup);

  #ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
  #endif
    static const Glib::ustring ui_description =
    "<ui>"
#ifdef GLOM_ENABLE_MAEMO
    "  <popup name='Overview_MainMenu'>"
#else
    "  <menubar name='Overview_MainMenu'>"
#endif
    "    <menu action='Overview_MainMenu_File'>"
    "      <menuitem action='Overview_MainMenu_File_Print' />"
    "    </menu>"
#ifdef GLOM_ENABLE_MAEMO
    "  </popup>"
#else
    "  </menubar>"
#endif
    "</ui>";

  #ifdef GLIBMM_EXCEPTIONS_ENABLED
    m_refUIManager->add_ui_from_string(ui_description);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << "building menus failed: " <<  ex.what();
  }
  #else
  std::auto_ptr<Glib::Error> error;
  m_refUIManager->add_ui_from_string(ui_info, error);
  if(error.get() != NULL)
  {
    std::cerr << "building menus failed: " << error->what();
  }
  #endif

  //Get the menu:
  m_menu = dynamic_cast<Gtk::MenuBar*>( m_refUIManager->get_widget("/Overview_MainMenu") ); 
  if(!m_menu)
    g_warning("menu not found");

  vbox->pack_start(*m_menu, Gtk::PACK_SHRINK);
  m_menu->show();


  //Get the scolled window and add the canvas to it:
  Gtk::ScrolledWindow* scrolledwindow_canvas = 0;
  refGlade->get_widget("scrolledwindow_canvas", scrolledwindow_canvas);
  
  scrolledwindow_canvas->add(m_canvas);
  m_canvas.show();
  
  if(m_last_size_x != 0 && m_last_size_y != 0 )
  {
    set_size_request(m_last_size_x, m_last_size_y);
  }
}

Dialog_RelationshipsOverview::~Dialog_RelationshipsOverview()
{
  get_size(m_last_size_x, m_last_size_y);
}


void Dialog_RelationshipsOverview::update_model()
{
  Glib::RefPtr<Goocanvas::Item> root = m_canvas.get_root_item();
  while(root->get_n_children() > 0)
    root->remove_child(0);
  
  for(type_map_item_tables::iterator iter = m_tables.begin(); iter != m_tables.end(); iter++)
    delete iter->second;
  
  m_tables.clear();
  m_table_names.clear();
  
  Document_Glom* document = dynamic_cast<Document_Glom*>(get_document());
  if(document)
  {
    int table_height, max_table_height = 0;
    int sizex = 10;
    int sizey = 10;
    const int table_width = 200;
    const int field_height = 20;
    
    Document_Glom::type_vecFields::size_type i, j;

    Document_Glom::type_listTableInfo tables = document->get_tables();
    for(Document_Glom::type_listTableInfo::iterator iter = tables.begin(); iter != tables.end(); ++iter)
    {
      sharedptr<TableInfo> info = *iter;

      Document_Glom::type_vecFields fields = document->get_table_fields(info->get_name());

      Glib::RefPtr<Goocanvas::Item> table_group = Glib::wrap(goo_canvas_group_new(root->gobj(), 0));
      table_group->signal_motion_notify_event().connect( sigc::mem_fun(*this, &Dialog_RelationshipsOverview::on_table_group_motion_notify_event));
      table_group->signal_button_press_event().connect(
        sigc::bind(sigc::mem_fun(*this, &Dialog_RelationshipsOverview::on_table_group_button_press_event), table_group));
      table_group->signal_button_release_event().connect(sigc::mem_fun(*this, &Dialog_RelationshipsOverview::on_table_group_button_release_event));
      
      TableView* tv = new TableView();
      tv->m_table_name = info->get_name();

      //TODO: This is a workaround, needed to fix a refcount problem. It's not used otherwise.
      //This shouldn't be necessary, if root keeps a reference to it too.
      tv->m_group = table_group; 

      m_tables[table_group] = tv;
      m_table_names[info->get_name()] = tv;
      
      table_height = field_height * (fields.size() + 1);
      Glib::ustring title = "<b>";
      title += info->get_title_or_name().c_str();
      title += "</b>";
      
      if(!document->get_table_overview_position(tv->m_table_name, tv->x1, tv->y1))
      {
        tv->x1 = sizex;
        tv->y1 = sizey;
        document->set_table_overview_position(tv->m_table_name, tv->x1, tv->y1);
        m_modified = true;
      }

      tv->x2 = tv->x1 + table_width;
      tv->y2 = tv->y1 + table_height;
      
      goo_canvas_rect_new(table_group->gobj(), tv->x1, tv->y1, table_width, table_height,
                          "line-width", 2.0, "radius-x", 4.0,
                          "radius-y", 4.0, "stroke-color", "black",
                          "fill-color", "white", 0);
      
      goo_canvas_text_new(table_group->gobj(), title.c_str(),
                          tv->x1 + 5, tv->y1 + 5, table_width - 10,
                          GTK_ANCHOR_NORTH_WEST, "font", "sans 10",
                          "use-markup", true, 0);
      goo_canvas_polyline_new_line(table_group->gobj(), tv->x1, tv->y1 + field_height, tv->x1 + table_width,
                        tv->y1 + field_height, "stroke-color", "black",
                        "line-width", 1.0, 0);

      int y = field_height;
      for(Document_Glom::type_vecFields::iterator iter = fields.begin(); iter != fields.end(); ++iter)
      {
        sharedptr<Field> field = *iter;

        if(field->get_primary_key())
        {
          title = "<u>";
          title += field->get_title_or_name().c_str();
          title += "</u>";
          goo_canvas_text_new(table_group->gobj(), title.c_str(),
                      tv->x1 + 5, tv->y1 + 5 + y, table_width - 10,
                      GTK_ANCHOR_NORTH_WEST, "font", "sans 10",
                              "use-markup", true, 0);
        }
        else
        {
          goo_canvas_text_new(table_group->gobj(), field->get_title_or_name().c_str(),
                              tv->x1 + 5, tv->y1 + 5 + y, table_width - 10,
                              GTK_ANCHOR_NORTH_WEST, "font", "sans 10", 0);
        }
        
        y += field_height;
      }
      
      sizex += table_width + 10;
      max_table_height = table_height > max_table_height ? table_height : max_table_height;
    }
    
    for(Document_Glom::type_listTableInfo::iterator iter = tables.begin(); iter != tables.end(); ++iter)
    {
      sharedptr<TableInfo> info = *iter;

      Document_Glom::type_vecRelationships m_relationships = document->get_relationships(info->get_name());
      Document_Glom::type_vecFields fields = document->get_table_fields(info->get_name());
      Document_Glom::type_vecFields to_fields;
      
      TableView* tv = m_table_names[info->get_name()];
      for(Document_Glom::type_vecRelationships::iterator rit = m_relationships.begin(); rit != m_relationships.end(); rit++)
      {
        if(document->get_field((*rit)->get_to_table(), (*rit)->get_to_field())->get_primary_key())
        {
          for(i = 0; i < fields.size() && fields[i]->get_name() != (*rit)->get_from_field();
              ++i) {}
          
          TableView* tv_to = m_table_names[(*rit)->get_to_table()];
          to_fields = document->get_table_fields((*rit)->get_to_table());
          
          for(j = 0; j < to_fields.size() && to_fields[j]->get_name() != (*rit)->get_to_field();
              ++j) {}

          tv->m_relationships[std::make_pair(tv_to, j)] = i;
          if(std::find(tv_to->m_update_on_move.begin(), tv_to->m_update_on_move.end(), tv) == tv_to->m_update_on_move.end())
            tv_to->m_update_on_move.push_back(tv);
        }
      }
      update_relationships(tv);
    }
    
    goo_canvas_set_bounds(m_canvas.gobj(), 0, 0, sizex, max_table_height);
    
  }
  else
  {
    std::cout << "ERROR: Could not retrieve the Glom document." << std::endl;
  }
}

void Dialog_RelationshipsOverview::update_relationships(TableView* table_from)
{
  Glib::RefPtr<Goocanvas::Item> root = m_canvas.get_root_item();
  if(!root)
    return;

  for(TableView::type_vec_canvasitems::iterator iter = table_from->m_lines.begin();  iter != table_from->m_lines.end(); ++iter)
  {
    Glib::RefPtr<Goocanvas::Item> item = *iter;
    if(item)
      root->remove_child(root->find_child(item));
  }

  table_from->m_lines.clear();
  
  for(TableView::type_map_relationships::iterator iter = table_from->m_relationships.begin();
      iter != table_from->m_relationships.end(); ++iter)
  {
    double x_from, y_from, x_to, y_to = 0;

    TableView* table_to = iter->first.first;
    if(table_to->x1 - table_from->x1 > 0)
    {
      if(table_to->x1 - table_from->x2 < 0)
      {
        x_from = table_from->x2;
        x_to = table_to->x2;
      }
      else
      {
        x_from = table_from->x2;
        x_to = table_to->x1;
      }
    }
    else
    {
      if(table_from->x1 - table_to->x2 < 0)
      {
        x_from = table_from->x1;
        x_to = table_to->x1;
      }
      else
      {
        x_from = table_from->x1;
        x_to = table_to->x2;
      }
    }
    
    const int field_height = 20;
 
    y_from = table_from->y1 + (1.5 + iter->second) * field_height;
    y_to = table_to->y1 + (1.5 + iter->first.second) * field_height;
    
    table_from->m_lines.push_back( Glib::wrap( goo_canvas_polyline_new_line (root->gobj(),
                                                                  x_from, y_from, x_to, y_to,
                                                                  "line-width", 1.0,
                                                                  "stroke-color", "black",
                                                                  "start-arrow", false,
                                                                  "end-arrow", true,
                                                                  "arrow-width", 10.0,
                                                                  "arrow-length", 10.0, 0)));
  }
}

void Dialog_RelationshipsOverview::load_from_document()
{
  update_model();
}


bool Dialog_RelationshipsOverview::on_table_group_button_release_event(const Glib::RefPtr<Goocanvas::Item>& target,
                                                              GdkEventButton* event)
{
  m_canvas.pointer_ungrab(target, event->time);
  m_dragging = false;
  return true;
}

bool Dialog_RelationshipsOverview::on_table_group_button_press_event(const Glib::RefPtr<Goocanvas::Item>& target,
                                                            GdkEventButton* event, const Glib::RefPtr<Goocanvas::Item>& view)
{
  
  
  switch(event->button)
  {
    case 1:
    {
      Glib::RefPtr<Goocanvas::Item> item = target;
      while(item && !item->is_container())
        item = item->get_parent();
      
      item->raise();
    
      m_drag_x = event->x;
      m_drag_y = event->y;
    
      Gdk::Cursor fleur(Gdk::FLEUR);
      m_canvas.pointer_grab(view, Gdk::POINTER_MOTION_MASK | Gdk::BUTTON_RELEASE_MASK,
                  fleur,
                  event->time);
      m_dragging = true;
      break;
    }
    default:
      break;
  }
  
  return true;
}

bool Dialog_RelationshipsOverview::on_table_group_motion_notify_event(const Glib::RefPtr<Goocanvas::Item>& target, GdkEventMotion* event)
{
  Glib::RefPtr<Goocanvas::Item> item = target;
    while(item && !item->is_container())
      item = item->get_parent();
    
  if(item && m_dragging && (event->state & Gdk::BUTTON1_MASK))
  {
    double new_x = event->x;
    double new_y = event->y;

    TableView* tv = m_tables[item];
    item->translate(new_x - m_drag_x, new_y - m_drag_y);
    tv->x1 += new_x - m_drag_x;
    tv->y1 += new_y - m_drag_y;
    tv->x2 += new_x - m_drag_x;
    tv->y2 += new_y - m_drag_y;
      
    Document_Glom* document = get_document();

    //TODO: Delay this until when we close the window (probably do it in a save_to_document() override),
    //to prevent us from writing to disk every time something is moved.
    document->set_table_overview_position(tv->m_table_name, tv->x1, tv->y1);
    m_modified = true;
    update_relationships(tv);

    for(TableView::type_vec_tableviews::iterator iter = tv->m_update_on_move.begin(); iter != tv->m_update_on_move.end(); ++iter)
      update_relationships(*iter);
  }

  return true;
}

void Dialog_RelationshipsOverview::on_response(int id)
{
  if(m_modified && get_document())
    get_document()->set_modified();
    
  hide();
}

void Dialog_RelationshipsOverview::on_menu_file_print()
{
  print_or_preview(Gtk::PRINT_OPERATION_ACTION_PRINT_DIALOG);
}

void Dialog_RelationshipsOverview::on_menu_file_save()
{
}

void Dialog_RelationshipsOverview::print_or_preview(Gtk::PrintOperationAction print_action)
{
  //Create a new PrintOperation with our PageSetup and PrintSettings:
  //(We use our derived PrintOperation class)
  Glib::RefPtr<PrintOperationRelationshipsOverview> print = PrintOperationRelationshipsOverview::create();
  print->set_canvas(&m_canvas);

  print->set_track_print_status();
  //print->set_default_page_setup(m_refPageSetup);
  //print->set_print_settings(m_refSettings);

  //print->signal_done().connect(sigc::bind(sigc::mem_fun(*this,
  //                &ExampleWindow::on_printoperation_done), print));

  try
  {
    print->run(print_action /* print or preview */, *this);
  }
  catch (const Gtk::PrintError& ex)
  {
    //See documentation for exact Gtk::PrintError error codes.
    std::cerr << "An error occurred while trying to run a print operation:"
        << ex.what() << std::endl;
  }
}

} //namespace Glom
