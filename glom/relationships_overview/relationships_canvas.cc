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

#include "relationships_canvas.h"

namespace Glom
{

RelationshipsCanvas::RelationshipsCanvas()
{
}

RelationshipsCanvas::~RelationshipsCanvas()
{
  remove_all();
}

void RelationshipsCanvas::remove_all()
{
  //Delete all canvas items:
  for(type_map_items::iterator iter = m_map_items.begin(); iter != m_map_items.end(); ++iter)
  {
    TableCanvasItem* pItem = iter->second;
    remove_view(pItem);
    delete pItem;
  }

  m_map_items.clear();
 }

void RelationshipsCanvas::load_from_document()
{  
  remove_all();

  Document_Glom* document = dynamic_cast<Document_Glom*>(get_document());
  if(document)
  {
    double x = 0;
    double y = 0;

    Document_Glom::type_listTableInfo list_tables = document->get_tables();
    for(Document_Glom::type_listTableInfo::const_iterator iter = list_tables.begin(); iter != list_tables.end(); ++iter)
    {
      //Add a canvas item for each table:
      const sharedptr<const TableInfo> table_info = *iter;
      const Glib::ustring table_name = table_info->get_name();
      if(!table_name.empty())
      {
        Gnome::Canvas::Group* parent_group = root();
        if(parent_group)
        {
          TableCanvasItem* pItem = new TableCanvasItem(*parent_group, table_info);
          add_view(pItem); //Give it access to the document.

          pItem->show();
          pItem->load_from_document();

          m_map_items[table_name] = pItem;

          pItem->move(x, y);
          x += 20;
          y += 20;

          pItem->signal_event().connect( sigc::bind(sigc::mem_fun(*this, &RelationshipsCanvas::on_item_event), pItem) );
        }
      }
    }
  }
}

bool RelationshipsCanvas::on_item_event(GdkEvent* event, Gnome::Canvas::Item* item)
{
  static double x, y;
  double item_x, item_y;
  static bool dragging;

  item_x = event->button.x;
  item_y = event->button.y;

  item->get_parent_group()->w2i(item_x, item_y);

  switch(event->type)
  {
    case GDK_BUTTON_PRESS:
    {
      switch(event->button.button) {
      case 1:
      {
        x = item_x;
        y = item_y;
  
        item->grab(GDK_POINTER_MOTION_MASK | GDK_BUTTON_RELEASE_MASK,
                    Gdk::Cursor(Gdk::FLEUR),
                    event->button.time);
        dragging = true;
        break;
      }
  
      default:
        break;
      }
  
      break;
    }
  
    case GDK_MOTION_NOTIFY:
    {
      if(dragging && (event->motion.state & GDK_BUTTON1_MASK))
      {
        double new_x = item_x;
        double new_y = item_y;
  
        item->move(new_x - x, new_y - y);
        x = new_x;
        y = new_y;
      }
      break;
    }
  
    case GDK_BUTTON_RELEASE:
    {
      item->ungrab(event->button.time);
      dragging = false;
      break;
    }
  
    default:
      break;
  }

  return false;
}

} //namespace Glom
