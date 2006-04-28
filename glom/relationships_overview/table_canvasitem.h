/* Glom
 *
 * Copyright (C) 2001-2005 Murray Cumming
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


#ifndef GLOM_RELATIONSHIPS_OVERVIEW_TABLE_CANVASITEM_H
#define GLOM_RELATIONSHIPS_OVERVIEW_TABLE_CANVASITEM_H

#include <libgnomecanvasmm.h>
#include "relationshipscanvas_tablewidget.h"
#include "../base_db.h"
#include <glom/libglom/data_structure/tableinfo.h>


class TableCanvasItem
  : public Gnome::Canvas::Group,
    public Base_DB
{
public: 
  TableCanvasItem(Gnome::Canvas::Group& parent_group, const sharedptr<const TableInfo>& table_info);
  virtual ~TableCanvasItem();

  virtual void load_from_document(); //View override

protected: 
  void on_cell_data_name(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter);

  //Tree model columns:
  class ModelColumns_Fields : public Gtk::TreeModel::ColumnRecord
  {
  public:

    ModelColumns_Fields()
    { add(m_col_field); }

    Gtk::TreeModelColumn< sharedptr<Field> > m_col_field;
  };

  ModelColumns_Fields m_ColumnsFields;
  Glib::RefPtr<Gtk::ListStore> m_model_fields;

  Gnome::Canvas::Widget* m_canvas_widget;
  RelationshipsCanvasTableWidget* m_widget;
  Gtk::TreeView* m_treeview_fields;
  Gnome::Canvas::Text* m_canvas_text_title;
  Gnome::Canvas::Rect* m_canvas_rect;

  sharedptr<const TableInfo> m_table_info;
};

#endif //GLOM_RELATIONSHIPS_OVERVIEW_TABLE_CANVASITEM_H
