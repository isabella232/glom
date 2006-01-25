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

#include "table_canvasitem.h"

TableCanvasItem::TableCanvasItem(Gnome::Canvas::Group& parent_group, const sharedptr<const TableInfo>& table_info)
: Gnome::Canvas::Group(parent_group),
  m_canvas_widget(0),
  m_widget(0),
  m_treeview_fields(0),
  m_canvas_text_title(0),
  m_canvas_rect(0),
  m_table_info(table_info)
{
  m_canvas_rect = new Gnome::Canvas::Rect(*this);
  m_canvas_rect->property_outline_color() = "black";

  m_canvas_text_title = new Gnome::Canvas::Text(*this);
  m_canvas_text_title->property_text() = table_info->get_title_or_name();

  try
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "alignment_placeholder_canvasitem");

    refXml->get_widget_derived("alignment_placeholder_canvasitem", m_widget);

    refXml->get_widget("treeview_fields", m_treeview_fields);

    m_model_fields = Gtk::ListStore::create(m_ColumnsFields);
    m_treeview_fields->set_model(m_model_fields);
    m_treeview_fields->set_size_request(100,100);

    Gtk::CellRendererText* cell_name = new Gtk::CellRendererText();
    Gtk::TreeView::Column* view_column = new Gtk::TreeViewColumn("name");
    view_column->pack_start(*cell_name);
    m_treeview_fields->append_column(*view_column);
    view_column->set_cell_data_func(*cell_name, sigc::mem_fun(*this, &TableCanvasItem::on_cell_data_name));


  }
  catch(const Gnome::Glade::XmlError& ex)
  {
    std::cerr << ex.what() << std::endl;
  }

  m_widget->show();

  m_canvas_widget = new Gnome::Canvas::Widget(*this);
  m_canvas_widget->property_widget() = m_widget;
  m_canvas_widget->property_y() = 50; //TODO: m_canvas_text_title->property_height();

  m_canvas_rect->property_y2() = 200;
  m_canvas_rect->property_x2() = 100;
}

void TableCanvasItem::load_from_document()
{ 
  Document_Glom* document = get_document();
  if(document)
  {
    m_model_fields->clear();

    const Document_Glom::type_vecFields fields = document->get_table_fields(m_table_info->get_name());
    for(Document_Glom::type_vecFields::const_iterator iter = fields.begin(); iter != fields.end(); ++iter)
    {
      Gtk::TreeModel::iterator iterTree = m_model_fields->append();
      Gtk::TreeModel::Row row = *iterTree;
      row[m_ColumnsFields.m_col_field] = *iter;
    }
  }
}

TableCanvasItem::~TableCanvasItem()
{
  delete m_canvas_text_title;
  delete m_canvas_rect;
  delete m_canvas_widget;
  delete m_widget;
}


void TableCanvasItem::on_cell_data_name(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter)
{
  //Set the view's cell properties depending on the model's data:
  Gtk::CellRendererText* renderer_text = dynamic_cast<Gtk::CellRendererText*>(renderer);
  if(renderer_text)
  {
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;

      sharedptr<Field> field = row[m_ColumnsFields.m_col_field];
      renderer_text->property_text() = field->get_title_or_name();
    }
  }
}
