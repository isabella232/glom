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

#ifndef GLOM_DIALOG_RELATIONSHIPS_OVERVIEW
#define GLOM_DIALOG_RELATIONSHIPS_OVERVIEW

#include <glom/libglom/document/document_glom.h>
#include <gtkmm/dialog.h>
#include <gtkmm/widget.h>
#include <libglademm.h>
#include <goocanvas.h>
#include <map>
#include <vector>

#include "relationships_canvas.h"

namespace Glom
{

class Dialog_RelationshipsOverview
 : public Gtk::Dialog,
   public View_Composite_Glom
{
public:
  Dialog_RelationshipsOverview(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);

  virtual ~Dialog_RelationshipsOverview();
  
  bool on_button_press_canvas(GooCanvasItem *view, GooCanvasItem *target, GdkEventButton *event);
  bool on_button_release_canvas(GooCanvasItem *view, GooCanvasItem *target, GdkEventButton *event);
  bool on_motion_canvas ( GooCanvasItem *view, GooCanvasItem *target, GdkEventMotion *event );
  
  virtual void load_from_document(); //overridden.

protected:
  class TableView;
  
  void update_model();
  void update_relationships(TableView* table_from);
  void on_response(int id);
  
  bool m_modified;
  bool m_dragging;
  gdouble m_drag_x, m_drag_y;
  Gtk::Widget *m_canvas;

  typedef std::map<GooCanvasItem*, TableView*> type_map_item_tables;
  type_map_item_tables m_tables;

  typedef std::map<Glib::ustring, TableView*> type_map_table_names;
  type_map_table_names m_table_names;
  static int m_last_size_x, m_last_size_y;
 
  typedef std::map<GooCanvasItem*, TableView*>::iterator TableIterator;
  typedef std::map< std::pair<TableView*, int>, int >::iterator RelationshipIterator;

  class TableView
  {
  public:
    TableView();

    Glib::ustring m_table_name;
    
    GooCanvasItem* m_group;
    
    typedef std::vector<GooCanvasItem*> type_vec_canvasitems;
    type_vec_canvasitems m_lines;
    
    typedef std::map< std::pair<TableView*, int>, int> type_map_relationships;
    type_map_relationships m_relationships;

    typedef std::vector<TableView*> type_vec_tableviews;
    type_vec_tableviews m_update_on_move;
    
    float x1, y1, x2, y2;
  };
};

}; //namespace Glom

#endif //GLOM_DIALOG_RELATIONSHIPS_OVERVIEW
