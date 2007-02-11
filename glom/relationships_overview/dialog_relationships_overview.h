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

namespace Glom {

class Dialog_RelationshipsOverview  : public Gtk::Dialog,
                                      public View_Composite_Glom {
public:
    Dialog_RelationshipsOverview(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
    void updateModel ();
    virtual ~Dialog_RelationshipsOverview();
    gboolean on_button_press_canvas ( GooCanvasItemView *view, GooCanvasItemView *target,
                                      GdkEventButton *event );
    gboolean on_button_release_canvas ( GooCanvasItemView *view, GooCanvasItemView *target,
                                        GdkEventButton *event );
    gboolean on_motion_canvas ( GooCanvasItemView *view, GooCanvasItemView *target, GdkEventMotion *event );
    void on_item_view_created ( GooCanvasView *view, GooCanvasItemView *item, GooCanvasItem *item );
    //Glib::SignalProxy1< void,const Glib::RefPtr<Dialog_RelationshipsOverview>& > signal_canvas_motion ();
    //Glib::SignalProxy3 < gboolean, GooCanvasItemView *,
    //GooCanvasItemView *, GdkEventMotion * > signal_canvas_motion ();
    
protected:
    void on_response ( int id );
    
    gboolean m_modified;
    gboolean m_dragging;
    gdouble m_drag_x, m_drag_y;
    Gtk::ScrolledWindow *m_scrolledwindow_canvas;
    Gtk::Widget *m_canvas;
    GooCanvasModelSimple *m_model;
    Document_Glom *m_document;
    
    class TableView {
    public:
        Glib::ustring tableName;
        
        GooCanvasItem *group;
        GooCanvasBounds bounds;
        
        std::vector<GooCanvasItem*> lines;
        
        // pair<Foreign table, foreign field number> -> local field number
        std::map<std::pair<TableView*, int>, int> relationships;
        std::vector<TableView*> updateOnMove;
        
        float x1, y1, x2, y2;
    };
    std::map<GooCanvasItem*, TableView*> m_tables;
    std::map<Glib::ustring, TableView*> m_tableNames;
    typedef std::map<GooCanvasItem*, TableView*>::iterator TableIterator;
    typedef std::map< std::pair<TableView*, int>, int >::iterator RelationshipIterator;
    
    void updateRelationships ( TableView * );
    

  //Gnome::Canvas::Group m_canvas_group;
};

}; //namespace Glom

#endif //GLOM_DIALOG_RELATIONSHIPS_OVERVIEW
