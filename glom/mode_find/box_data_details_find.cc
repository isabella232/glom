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
#include <glibmm/i18n.h>

Box_Data_Details_Find::Box_Data_Details_Find()
: Box_Data_Details(false)
{
   //m_strHint = _("Enter the search criteria and click [Find]\n Glom will then change to Data mode to display the results.");

  //Instead of nav buttons:
  m_HBox.pack_end(m_Button_Find, Gtk::PACK_SHRINK);

  m_Button_Find.property_can_default() = true; //TODO: Make this a real method in gtkmm?

  show_all_children();
}

Box_Data_Details_Find::~Box_Data_Details_Find()
{
}

bool Box_Data_Details_Find::init_db_details(const Glib::ustring& table_name)
{
  return Box_Data_Details::init_db_details(table_name, Gnome::Gda::Value());
}

bool Box_Data_Details_Find::fill_from_database()
{
  Bakery::BusyCursor(*get_app_window());

  bool result = Box_DB_Table::fill_from_database();

  m_FieldsShown = get_fields_to_show();

  create_layout(); //TODO: Only do this when the layout has changed.

  return result;
}

void Box_Data_Details_Find::fill_related()
{
  //Clear existing pages:
  //m_Notebook_Related.pages().clear();

  //Get relationships from the document:
  Document_Glom::type_vecRelationships vecRelationships = get_document()->get_relationships(m_table_name);

  //Add the relationships:
  for(Document_Glom::type_vecRelationships::iterator iter = vecRelationships.begin(); iter != vecRelationships.end(); iter++)
  {
     /*
     const Relationship& relationship = *iter;


     Box_Data_List_Related* pBox = Gtk::manage(new Box_Data_List_Related());
     std::cout <<  "Box_Data_Details::fill_related() 2:" << relationship->get_name() << std::endl;
     m_Notebook_Related.pages().push_back( Gtk::Notebook_Helpers::TabElem(*pBox, relationship->get_name()) );
     std::cout <<  "Box_Data_Details::fill_related() 2.5:" << std::endl;

     guint rowKey = m_FieldsShown.get_index(relationship->get_from_field());
     Glib::ustring strKeyValue = m_AddDel.get_value(rowKey);
     strKeyValue = m_FieldsShown[rowKey].sql(strKeyValue); //Quote/Escape it if necessary.

     std::cout <<  "Box_Data_Details::fill_related() 3:" << std::endl;
     pBox->init_db_details(get_database_name(), relationship->get_to_table(), relationship->get_to_field(), strKeyValue);
     pBox->show_all();
     */
  }
}


void Box_Data_Details_Find::on_flowtable_field_edited(const sharedptr<const LayoutItem_Field>& /* id */, const Gnome::Gda::Value& /* value */)
{
  //Don't do anything.
  //This just blocks the method in the base class.
}

Gtk::Widget* Box_Data_Details_Find::get_default_button() //override
{
  return &m_Button_Find;
}
 
