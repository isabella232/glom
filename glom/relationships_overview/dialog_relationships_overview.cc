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

Dialog_RelationshipsOverview::Dialog_RelationshipsOverview(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::Dialog(cobject),
  m_scrolledwindow_canvas(0)
{
  refGlade->get_widget("scrolledwindow_canvas", m_scrolledwindow_canvas);
  m_scrolledwindow_canvas->add(m_canvas);
  
  show_all_children();
}

Dialog_RelationshipsOverview::~Dialog_RelationshipsOverview()
{
  //Delete all canvas items:
  for(type_map_items::iterator iter = m_map_items.begin(); iter != m_map_items.end(); ++iter)
  {
    TableCanvasItem* pItem = iter->second;
    delete pItem;
  }
  
  m_map_items.clear();
}

void Dialog_RelationshipsOverview::load_from_document()
{
  Document_Glom* document = dynamic_cast<Document_Glom*>(get_document());
  if(document)
  {
    Document_Glom::type_listTableInfo list_tables = document->get_tables();
    for(Document_Glom::type_listTableInfo::const_iterator iter = list_tables.begin(); iter != list_tables.end(); ++iter)
    {
      //Add a canvas item for each table:
      const TableInfo& table_info = *iter;
      const Glib::ustring table_name = table_info.get_name();
      if(!table_name.empty())
      {
        Gnome::Canvas::Group* parent_group = m_canvas.root();
        if(parent_group)
        {
          TableCanvasItem* pItem = new TableCanvasItem(*parent_group);
          m_map_items[table_name] = pItem;
        }
      }
    }
  }
}










