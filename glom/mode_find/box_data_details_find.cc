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

#include "box_data_details_find.h"

Box_Data_Details_Find::Box_Data_Details_Find()
: Box_Data_Details(false)
{
   m_strHint = gettext("Enter the search criteria and click [Find]\n Glom will then change to Data mode to display the results.");

  //Instead of nav buttons:
  m_HBox.pack_end(m_Button_Find, Gtk::PACK_EXPAND_WIDGET);

  show_all();
}

Box_Data_Details_Find::~Box_Data_Details_Find()
{
}



void Box_Data_Details_Find::fill_from_database()
{
/* TOOD_port
  Box_DB_Table::fill_from_database();

  for(guint i = 0; i < m_Fields.size(); i ++)
  {
    //Field value:
    guint uiRow = i;
    if(uiRow >= m_AddDel.get_count())
      uiRow = m_AddDel.add_item();

    //Field name:
    const Glib::ustring& strFieldName = m_Fields[i].get_name();
    m_AddDel.set_item_title(uiRow, strFieldName);
  }

  */
  fill_end();
}

void Box_Data_Details_Find::fill_related()
{
  //Clear existing pages:
  m_Notebook_Related.pages().clear();

  //Get relationships from the document:
  Document_Glom::type_vecRelationships vecRelationships = m_pDocument->get_relationships(m_strTableName);

  //Add the relationships:
  for(Document_Glom::type_vecRelationships::iterator iter = vecRelationships.begin(); iter != vecRelationships.end(); iter++)
  {
     /*
     const Relationship& relationship = *iter;


     Box_Data_List_Related* pBox = Gtk::manage(new Box_Data_List_Related());
     std::cout <<  "Box_Data_Details::fill_related() 2:" << relationship.get_name() << std::endl;
     m_Notebook_Related.pages().push_back( Gtk::Notebook_Helpers::TabElem(*pBox, relationship.get_name()) );
     std::cout <<  "Box_Data_Details::fill_related() 2.5:" << std::endl;

     guint rowKey = m_Fields.get_index(relationship.get_from_field());
     Glib::ustring strKeyValue = m_AddDel.get_value(rowKey);
     strKeyValue = m_Fields[rowKey].sql(strKeyValue); //Quote/Escape it if necessary.

     std::cout <<  "Box_Data_Details::fill_related() 3:" << std::endl;
     pBox->initialize(get_databaseName(), relationship.get_to_table(), relationship.get_to_field(), strKeyValue);
     pBox->show_all();
     */
  }
}


void Box_Data_Details_Find::on_AddDel_user_changed(guint row, guint col)
{
  //Don't do anything.
  //This just blocks the method in the base class.
}
