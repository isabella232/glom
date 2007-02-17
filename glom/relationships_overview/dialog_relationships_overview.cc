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
#include <glibmm/i18n.h>
#include <algorithm>

namespace Glom
{

Dialog_RelationshipsOverview::TableView::TableView()
: m_group(0),
  x1(0),
  y1(0),
  x2(0),
  y2(0)
{
}



static bool on_button_press(GooCanvasItem* view,
                              GooCanvasItem* target,
                              GdkEventButton* event,
                              gpointer data)
{
  return static_cast<Dialog_RelationshipsOverview*>(data)->on_button_press_canvas(view, target, event);
}
  
static bool on_motion_notify(GooCanvasItem* view,
                              GooCanvasItem* target,
                              GdkEventMotion* event,
                              gpointer data)
{
  return static_cast<Dialog_RelationshipsOverview*>(data)->on_motion_canvas(view, target, event);
}
  
static bool on_button_release(GooCanvasItem* view,
                                GooCanvasItem* target,
                                GdkEventButton* event,
                                gpointer data)
{
  return static_cast<Dialog_RelationshipsOverview*>(data)->on_button_release_canvas(view, target, event);
}
  

bool Dialog_RelationshipsOverview::on_button_release_canvas(GooCanvasItem* view,
                                                              GooCanvasItem* target,
                                                              GdkEventButton* event)
{
  goo_canvas_pointer_ungrab(GOO_CANVAS(m_canvas->gobj()), target, event->time);
  m_dragging = false;
  return true;
}

bool Dialog_RelationshipsOverview::on_button_press_canvas(GooCanvasItem* view,
                                                            GooCanvasItem* target,
                                                            GdkEventButton* event)
{
  
  
  switch(event->button)
  {
    case 1:
    {
      GooCanvasItem* item = target;
      while(item && !goo_canvas_item_is_container(item))
        item = goo_canvas_item_get_parent(item);
      
      goo_canvas_item_raise(item, 0);
    
      m_drag_x = event->x;
      m_drag_y = event->y;
    
      GdkCursor* fleur = gdk_cursor_new(GDK_FLEUR);
      goo_canvas_pointer_grab(GOO_CANVAS(m_canvas->gobj()), view,
                  static_cast<GdkEventMask>(GDK_POINTER_MOTION_MASK | GDK_BUTTON_RELEASE_MASK),
                  fleur,
                  event->time);
      gdk_cursor_unref(fleur);
      m_dragging = true;
      break;
    }
    default:
      break;
  }
  
  return true;
}

bool Dialog_RelationshipsOverview::on_motion_canvas(GooCanvasItem* view,
                                                      GooCanvasItem* target,
                                                      GdkEventMotion* event)
{
  if(view)
  {
    GooCanvasItem* item = target;
    while(item && !goo_canvas_item_is_container(item))
      item = goo_canvas_item_get_parent(item);
    
    if(item && m_dragging && (event->state & GDK_BUTTON1_MASK))
    {
      double new_x = event->x;
      double new_y = event->y;

      TableView* tv = m_tables[item];
      goo_canvas_item_translate(item, new_x - m_drag_x, new_y - m_drag_y);
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

      for(TableView::type_vec_tableviews::iterator it = tv->m_update_on_move.begin(); it != tv->m_update_on_move.end(); ++it)
        update_relationships(*it);
    }
  }

  return true;
}

void Dialog_RelationshipsOverview::on_response(int id)
{
  if(m_modified && get_document())
    get_document()->set_modified();
    
  hide();
}

int Dialog_RelationshipsOverview::m_last_size_x = 0;
int Dialog_RelationshipsOverview::m_last_size_y = 0;

Dialog_RelationshipsOverview::Dialog_RelationshipsOverview(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
  : Gtk::Dialog(cobject),
    m_modified(false),
    m_dragging(false),
    m_drag_x(0),
    m_drag_y(0)
{
  Gtk::ScrolledWindow* scrolledwindow_canvas = 0;
  refGlade->get_widget("scrolledwindow_canvas", scrolledwindow_canvas);
  
  m_canvas = Glib::wrap(goo_canvas_new());
  scrolledwindow_canvas->add(*m_canvas);
  m_canvas->show();
  
  if(m_last_size_x != 0 && m_last_size_y != 0 )
  {
    set_size_request(m_last_size_x, m_last_size_y);
  }
}

void Dialog_RelationshipsOverview::update_model()
{
  TableView* tv,* tv_to;
  GooCanvasItem* root,* table_group,* table_rect,* table_text; //,* item;
  root = goo_canvas_get_root_item(GOO_CANVAS(m_canvas->gobj()));
  while(goo_canvas_item_get_n_children(root) > 0)
    goo_canvas_item_remove_child(root, 0);
  
  for(TableIterator it = m_tables.begin(); it != m_tables.end(); it++)
    delete it->second;
  
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
    for(Document_Glom::type_listTableInfo::iterator it = tables.begin(); it != tables.end(); ++it)
    {
      Document_Glom::type_vecFields fields = document->get_table_fields((*it)->get_name());

      table_group = goo_canvas_group_new(root, 0);
      g_signal_connect(table_group, "motion_notify_event",
                  G_CALLBACK(on_motion_notify), static_cast<gpointer>(this));
      g_signal_connect(table_group, "button_press_event",
                  (GtkSignalFunc) on_button_press, this);
      g_signal_connect(table_group, "button_release_event",
                  (GtkSignalFunc) on_button_release, this);
      
      tv = new TableView();
      tv->m_table_name = (*it)->get_name();
      tv->m_group = table_group;

      m_tables[table_group] = tv;
      m_table_names[(*it)->get_name()] = tv;
      
      table_height = field_height * (fields.size() + 1);
      Glib::ustring title = "<b>";
      title += (*it)->get_title_or_name().c_str();
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
      
      table_rect = goo_canvas_rect_new(table_group, tv->x1, tv->y1, table_width, table_height,
                          "line-width", 2.0, "radius-x", 4.0,
                          "radius-y", 4.0, "stroke-color", "black",
                          "fill-color", "white", 0);
      
      table_text = goo_canvas_text_new(table_group, title.c_str(),
                          tv->x1 + 5, tv->y1 + 5, table_width - 10,
                          GTK_ANCHOR_NORTH_WEST, "font", "sans 10",
                          "use-markup", true, 0);
      goo_canvas_polyline_new_line(table_group, tv->x1, tv->y1 + field_height, tv->x1 + table_width,
                        tv->y1 + field_height, "stroke-color", "black",
                        "line-width", 1.0, 0);
      int y = field_height;
      
      for(Document_Glom::type_vecFields::iterator it = fields.begin(); it != fields.end(); ++it)
      {
        if((*it)->get_primary_key())
        {
          title = "<u>";
          title += (*it)->get_title_or_name().c_str();
          title += "</u>";
          goo_canvas_text_new(table_group, title.c_str(),
                      tv->x1 + 5, tv->y1 + 5 + y, table_width - 10,
                      GTK_ANCHOR_NORTH_WEST, "font", "sans 10",
                              "use-markup", true, 0);
        }
        else
        {
          goo_canvas_text_new(table_group, (*it)->get_title_or_name().c_str(),
                              tv->x1 + 5, tv->y1 + 5 + y, table_width - 10,
                              GTK_ANCHOR_NORTH_WEST, "font", "sans 10", 0);
        }
        
        y += field_height;
      }
      
      sizex += table_width + 10;
      max_table_height = table_height > max_table_height ? table_height : max_table_height;
    }
    
    for(Document_Glom::type_listTableInfo::iterator it = tables.begin(); it != tables.end(); ++it)
    {
      Document_Glom::type_vecRelationships m_relationships = document->get_relationships((*it)->get_name());
      Document_Glom::type_vecFields fields = document->get_table_fields((*it)->get_name());
      Document_Glom::type_vecFields to_fields;
      
      tv = m_table_names[(*it)->get_name()];
      for(Document_Glom::type_vecRelationships::iterator rit = m_relationships.begin();
          rit != m_relationships.end(); rit++)
      {
        if(document->get_field((*rit)->get_to_table(), (*rit)->get_to_field())->get_primary_key())
        {
          for(i = 0; i < fields.size() && fields[i]->get_name() != (*rit)->get_from_field();
              ++i) {}
          
          tv_to = m_table_names[(*rit)->get_to_table()];
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
    
    goo_canvas_set_bounds(GOO_CANVAS(m_canvas->gobj()), 0, 0, sizex, max_table_height);
    
  }
  else
  {
    std::cout << "ERROR: Could not retrieve the Glom document." << std::endl;
  }
}

void Dialog_RelationshipsOverview::update_relationships(TableView* table_from)
{
  GooCanvasItem* root = goo_canvas_get_root_item(GOO_CANVAS(m_canvas->gobj()));

  for(TableView::type_vec_canvasitems::iterator it = table_from->m_lines.begin();  it != table_from->m_lines.end(); ++it)
  {
    goo_canvas_item_remove_child(root, goo_canvas_item_find_child(root, *it));
  }

  table_from->m_lines.clear();
  
  for(RelationshipIterator it = table_from->m_relationships.begin();
      it != table_from->m_relationships.end(); ++it)
  {
    double x_from, y_from, x_to, y_to = 0;

    TableView* table_to = it->first.first;
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
 
    y_from = table_from->y1 + (1.5 + it->second) * field_height;
    y_to = table_to->y1 + (1.5 + it->first.second) * field_height;
    
    table_from->m_lines.push_back( goo_canvas_polyline_new_line (root,
                                                                  x_from, y_from, x_to, y_to,
                                                                  "line-width", 1.0,
                                                                  "stroke-color", "black",
                                                                  "start-arrow", false,
                                                                  "end-arrow", true,
                                                                  "arrow-width", 10.0,
                                                                  "arrow-length", 10.0, 0));
  }
}

Dialog_RelationshipsOverview::~Dialog_RelationshipsOverview()
{
  get_size(m_last_size_x, m_last_size_y);
  delete m_canvas;
}

void Dialog_RelationshipsOverview::load_from_document()
{
  update_model();
}

} //namespace Glom
