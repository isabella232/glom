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
//#include <libgnome/gnome-i18n.h>
#include <bakery/App/App_Gtk.h> //For util_bold_message().
#include <glibmm/i18n.h>
#include <sstream> //For stringstream
#include <algorithm>

namespace Glom {
    static gboolean on_button_press ( GooCanvasItemView *view,
                                      GooCanvasItemView *target,
                                      GdkEventButton *event,
                                      gpointer data ) {
        return static_cast<Dialog_RelationshipsOverview*>(data)->on_button_press_canvas(view,target,event);
    }
    
    static gboolean on_motion_notify (GooCanvasItemView *view,
                                      GooCanvasItemView *target,
                                      GdkEventMotion *event,
                                      gpointer data) {
        return static_cast<Dialog_RelationshipsOverview*>(data)->on_motion_canvas ( view, target, event );
    }
        
    static gboolean on_button_release (GooCanvasItemView *view,
                                       GooCanvasItemView *target,
                                       GdkEventButton *event,
                                       gpointer data) {
        return static_cast<Dialog_RelationshipsOverview*>(data)->on_button_release_canvas (view, target, event);
    }
    
    static void on_item_view_created_c ( GooCanvasView *view, GooCanvasItemView *item_view,
                                         GooCanvasItem *item, gpointer data ) {
        static_cast<Dialog_RelationshipsOverview*>(data)->on_item_view_created ( view, item_view, item );
    }
    
    gboolean Dialog_RelationshipsOverview::on_button_release_canvas (GooCanvasItemView *view,
                                                                     GooCanvasItemView *target,
                                                                     GdkEventButton *event ) {
        GooCanvasView *canvas_view = goo_canvas_item_view_get_canvas_view (view);
        goo_canvas_view_pointer_ungrab (canvas_view, view, event->time);
        this->m_dragging = FALSE;
        
        return TRUE;
    }
    
    gboolean Dialog_RelationshipsOverview::on_button_press_canvas (GooCanvasItemView *view,
                                                                   GooCanvasItemView *target,
                                                                   GdkEventButton *event ) {
        GooCanvasView *canvas_view;
        GdkCursor *fleur;
        GooCanvasItem *item;
        
        switch (event->button) {
        case 1:
            item = goo_canvas_item_view_get_item (view);
            while ( item != NULL && !goo_canvas_item_is_container(item) )
                item = goo_canvas_item_get_parent ( item );
            goo_canvas_item_raise ( item, NULL );
            
            this->m_drag_x = event->x;
            this->m_drag_y = event->y;
            
            fleur = gdk_cursor_new (GDK_FLEUR);
            canvas_view = goo_canvas_item_view_get_canvas_view (view);
            goo_canvas_view_pointer_grab (canvas_view, view,
                                          static_cast<GdkEventMask>(GDK_POINTER_MOTION_MASK | GDK_BUTTON_RELEASE_MASK),
                                          fleur,
                                          event->time );
            gdk_cursor_unref (fleur);
            this->m_dragging = TRUE;
            break;
            
        default:
            break;
        }
        
        return TRUE;
    }

    gboolean Dialog_RelationshipsOverview::on_motion_canvas (GooCanvasItemView *view,
                                                             GooCanvasItemView *target,
                                                             GdkEventMotion *event ) {
        if ( view != NULL ) {
            GooCanvasItem *item = goo_canvas_item_view_get_item (view);
            while ( item != NULL && !goo_canvas_item_is_container(item) )
                item = goo_canvas_item_get_parent ( item );
            
            if ( item != NULL && this->m_dragging && (event->state & GDK_BUTTON1_MASK)) {
                double new_x = event->x;
                double new_y = event->y;

                TableView * tv = m_tables[item];
                goo_canvas_item_translate (item, new_x - this->m_drag_x, new_y - this->m_drag_y);
                tv->x1 += new_x - this->m_drag_x;
                tv->y1 += new_y - this->m_drag_y;
                tv->x2 += new_x - this->m_drag_x;
                tv->y2 += new_y - this->m_drag_y;
                
                m_document->set_table_overview_position ( tv->tableName, tv->x1, tv->y1 );
                m_modified = true;
                updateRelationships ( tv );
                for ( std::vector<TableView*>::iterator it = tv->updateOnMove.begin();
                      it != tv->updateOnMove.end(); ++it )
                    updateRelationships ( *it );
            }
        }
        return TRUE;
    }

    void Dialog_RelationshipsOverview::on_item_view_created ( GooCanvasView *view,
                                                              GooCanvasItemView *view_item,
                                                              GooCanvasItem *item ) {
        if ( goo_canvas_item_is_container(item) ) {
            g_signal_connect ( view_item, "motion_notify_event",
                               G_CALLBACK(on_motion_notify), static_cast<gpointer>(this) );
            g_signal_connect ( view_item, "button_press_event",
                               (GtkSignalFunc) on_button_press, this);
            g_signal_connect ( view_item, "button_release_event",
                               (GtkSignalFunc) on_button_release, this);
        }
    }
    
    void Dialog_RelationshipsOverview::on_response ( int id ) {
        if ( m_modified && m_document )
            m_document->set_modified ();
        hide ();
    }

    Dialog_RelationshipsOverview::Dialog_RelationshipsOverview(BaseObjectType* cobject,
                                                               const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
        : Gtk::Dialog(cobject),
          m_dragging(false),
          m_scrolledwindow_canvas(0),
          m_modified(false),
          m_document(0) {
        
        refGlade->get_widget("scrolledwindow_canvas", m_scrolledwindow_canvas);
        //this->signal_cancelled.connect(sigc::mem_fun(*this, &Dialog_RelationshipsOverview::on_cancel));
        m_model = goo_canvas_model_simple_new ();
        GtkWidget *canvas_temp = goo_canvas_view_new ();
        goo_canvas_view_set_model ( GOO_CANVAS_VIEW (canvas_temp), GOO_CANVAS_MODEL(m_model) );
        g_signal_connect ( canvas_temp, "item-view-created",
                           G_CALLBACK(on_item_view_created_c), static_cast<gpointer>(this) );
        
        m_canvas = Glib::wrap ( canvas_temp );
        m_scrolledwindow_canvas->add(*m_canvas);
        m_canvas->show ();
    }

    void Dialog_RelationshipsOverview::updateModel () {
        TableView *tv, *tv_to;
        GooCanvasItem *root, *table_group, *table_rect, *table_text; //, *item;
        root = goo_canvas_model_get_root_item ( GOO_CANVAS_MODEL(m_model) );
        while ( goo_canvas_item_get_n_children(root) > 0 )
            goo_canvas_item_remove_child ( root, 0 );
        
        for ( TableIterator it = m_tables.begin(); it != m_tables.end(); it++ )
            delete it->second;
        m_tables.clear ();
        m_tableNames.clear ();
        
        m_document = dynamic_cast<Document_Glom*>(get_document());
        if ( m_document ) {
            Document_Glom::type_listTableInfo tables = m_document->get_tables();
            int sizex, sizey, table_height, max_table_height = 0;
            std::string title;
            sizex = sizey = 10;
            const int table_width = 200;
            const int field_height = 20;
            Document_Glom::type_vecFields::size_type i, j;
            for ( Document_Glom::type_listTableInfo::iterator it = tables.begin(); it != tables.end(); ++it ) {
                Document_Glom::type_vecFields fields = m_document->get_table_fields ( (*it)->get_name() );
                table_group = goo_canvas_group_new ( root );
                tv = new TableView ();
                tv->tableName = (*it)->get_name();
                tv->group = table_group;
                m_tables[table_group] = tv;
                m_tableNames[(*it)->get_name()] = tv;
                
                table_height = field_height * (fields.size() + 1);
                title = "<b>";
                title += (*it)->get_title_or_name().c_str();
                title += "</b>";
                if ( !m_document->get_table_overview_position ( tv->tableName, tv->x1, tv->y1 ) )
                {
                    tv->x1 = sizex;
                    tv->y1 = sizey;
                    m_document->set_table_overview_position ( tv->tableName, tv->x1, tv->y1 );
                    m_modified = true;
                }
                tv->x2 = tv->x1 + table_width;
                tv->y2 = tv->y1 + table_height;
                
                table_rect = goo_canvas_rect_new ( table_group, tv->x1, tv->y1, table_width, table_height,
                                                   "line-width", 2.0, "radius-x", 4.0,
                                                   "radius-y", 4.0, "stroke-color", "black",
                                                   "fill-color", "white", NULL );
                
                table_text = goo_canvas_text_new ( table_group, title.c_str(),
                                                   tv->x1 + 5, tv->y1 + 5, table_width - 10,
                                                   GTK_ANCHOR_NORTH_WEST, "font", "sans 10",
                                                   "use-markup", true, NULL );
                goo_canvas_polyline_new_line ( table_group, tv->x1, tv->y1 + field_height, tv->x1 + table_width,
                                               tv->y1 + field_height, "stroke-color", "black",
                                               "line-width", 1.0, NULL );
                int y = field_height;
                
                for ( Document_Glom::type_vecFields::iterator it = fields.begin(); it != fields.end(); ++it ) {
                    if ( (*it)->get_primary_key() ) {
                        title = "<u>";
                        title += (*it)->get_title_or_name().c_str();
                        title += "</u>";
                        goo_canvas_text_new ( table_group, title.c_str(),
                                              tv->x1 + 5, tv->y1 + 5 + y, table_width - 10,
                                              GTK_ANCHOR_NORTH_WEST, "font", "sans 10",
                                                           "use-markup", true, NULL );
                    } else {
                        goo_canvas_text_new ( table_group, (*it)->get_title_or_name().c_str(),
                                                           tv->x1 + 5, tv->y1 + 5 + y, table_width - 10,
                                                           GTK_ANCHOR_NORTH_WEST, "font", "sans 10", NULL );
                    }
                    y += field_height;
                }
                sizex += table_width + 10;
                max_table_height = table_height > max_table_height ? table_height : max_table_height;
            }
            
            for ( Document_Glom::type_listTableInfo::iterator it = tables.begin(); it != tables.end(); ++it ) {
                Document_Glom::type_vecRelationships relationships = m_document->get_relationships ( (*it)->get_name() );
                Document_Glom::type_vecFields fields = m_document->get_table_fields ( (*it)->get_name() );
                Document_Glom::type_vecFields to_fields;
                
                tv = m_tableNames[(*it)->get_name()];
                for ( Document_Glom::type_vecRelationships::iterator rit = relationships.begin();
                      rit != relationships.end(); rit++ ) {
                    if ( m_document->get_field((*rit)->get_to_table(), (*rit)->get_to_field())->get_primary_key() ) {
                        for ( i = 0; i < fields.size() && fields[i]->get_name() != (*rit)->get_from_field();
                              ++i ) {}
                        
                        tv_to = m_tableNames[(*rit)->get_to_table()];
                        to_fields = m_document->get_table_fields((*rit)->get_to_table());
                        
                        for ( j = 0; j < to_fields.size() && to_fields[j]->get_name() != (*rit)->get_to_field();
                              ++j ) {}
                        
                        /*
                        std::cout << (*rit)->get_from_table() << "::" << (*rit)->get_from_field() << "[" << i
                                  << "] -> " << (*rit)->get_to_table() << "::" << (*rit)->get_to_field()
                                  << "[" << j << "]" << std::endl;
                        */
                        
                        tv->relationships[std::make_pair(tv_to, j)] = i;
                        if ( std::find(tv_to->updateOnMove.begin(), tv_to->updateOnMove.end(), tv) == tv_to->updateOnMove.end() )
                            tv_to->updateOnMove.push_back ( tv );
                    }
                }
                updateRelationships ( tv );
            }
            
            goo_canvas_view_set_bounds ( GOO_CANVAS_VIEW(m_canvas->gobj()), 0, 0, sizex, max_table_height );
        }
    }

    void Dialog_RelationshipsOverview::updateRelationships ( TableView *table_from ) {
        //std::cout << "updateRelationships: " << item << std::endl;
        
        TableView *table_to;
        double x_from, y_from, x_to, y_to;
        const int field_height = 20;
        
        GooCanvasItem *root = goo_canvas_model_get_root_item(GOO_CANVAS_MODEL(m_model));
        for ( std::vector<GooCanvasItem*>::iterator it = table_from->lines.begin();
              it != table_from->lines.end(); ++it ) {
            goo_canvas_item_remove_child ( root, goo_canvas_item_find_child(root, *it) );
        }
        table_from->lines.clear();
        
        for ( RelationshipIterator it = table_from->relationships.begin();
              it != table_from->relationships.end(); ++it ) {
            
            table_to = it->first.first;
            if ( table_to->x1 - table_from->x1 > 0 ) {
                if ( table_to->x1 - table_from->x2 < 0 ) {
                    x_from = table_from->x2;
                    x_to = table_to->x2;
                } else {
                    x_from = table_from->x2;
                    x_to = table_to->x1;
                }
            } else {
                if ( table_from->x1 - table_to->x2 < 0 ) {
                    x_from = table_from->x1;
                    x_to = table_to->x1;
                } else {
                    x_from = table_from->x1;
                    x_to = table_to->x2;
                }
            }
            
            y_from = table_from->y1 + (1.5 + it->second) * field_height;
            y_to = table_to->y1 + (1.5 + it->first.second) * field_height;
            
            table_from->lines.push_back ( goo_canvas_polyline_new_line ( root,
                                                                         x_from, y_from, x_to, y_to,
                                                                         "line-width", 1.0,
                                                                         "stroke-color", "black",
                                                                         "start-arrow", FALSE,
                                                                         "end-arrow", TRUE,
                                                                         "arrow-width", 10.0,
                                                                         "arrow-length", 10.0, NULL ) );
        }
    }
    
    Dialog_RelationshipsOverview::~Dialog_RelationshipsOverview() {
        //remove_view(&canvas);
        //g_object_unref ( canvas );
        delete m_canvas;
        g_object_unref ( m_model );
    }
    
} //namespace Glom
