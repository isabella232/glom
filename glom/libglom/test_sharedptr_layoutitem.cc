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
 
#include <libglom/data_structure/layout/layoutgroup.h>
#include <libglom/data_structure/layout/layoutitem_field.h>


int
main()
{
  Glom::sharedptr<Glom::LayoutItem_Field> field_copy;
  {
    Glom::sharedptr<Glom::LayoutGroup> group;
  
    {
       Glom::sharedptr<Glom::LayoutGroup> group_inscope = Glom::sharedptr<Glom::LayoutGroup>::create();
       std::cout << "group_inscope refcount = " << *(group_inscope._get_refcount()) << std::endl; //Should be 1.

       group_inscope->set_name("grouptestname");

       Glom::sharedptr<Glom::LayoutItem> itemgroup = group_inscope;
       std::cout << "itemgroup refcount = " << *(itemgroup._get_refcount()) << std::endl; //Should be 2.

       Glom::sharedptr<Glom::LayoutGroup> group_casted = Glom::sharedptr<Glom::LayoutGroup>::cast_dynamic(itemgroup); 

       std::cout << "itemgroup refcount = " << *(itemgroup._get_refcount()) << std::endl; //Should be 3.
       std::cout << "group_casted refcount = " << *(group_casted._get_refcount()) << std::endl; //Should be 3
       group = group_casted;
       std::cout << "group refcount = " << *(group._get_refcount()) << std::endl; //Should be 4.

       Glom::sharedptr<Glom::LayoutItem_Field> field_inscope = Glom::sharedptr<Glom::LayoutItem_Field>::create();
       std::cout << "field_inscope refcount = " << *(field_inscope._get_refcount()) << std::endl; //Should be 1.
       group->add_item(field_inscope);
       std::cout << "field_inscope refcount = " << *(field_inscope._get_refcount()) << std::endl; //Should be 2.

       field_inscope->set_name("fieldname");
    }

    std::cout << "group refcount = " << *(group._get_refcount()) << std::endl; //should be 1.

    std::cout << "groupname=" << group->get_name() << std::endl;

    group = Glom::glom_sharedptr_clone(group); //Test cloning.

    Glom::LayoutGroup::type_list_items items = group->get_items();
    for(Glom::LayoutGroup::type_list_items::iterator iter = items.begin(); iter != items.end(); ++iter)
    {
      Glom::sharedptr<Glom::LayoutItem> item = *iter;
      std::cout << "group item refcount = " << *(item._get_refcount()) << std::endl; //Should be 3 (1 in the group, 1 in the map from get_items(), 1 for item).

      Glom::sharedptr<Glom::LayoutItem_Field> item_casted = Glom::sharedptr<Glom::LayoutItem_Field>::cast_dynamic(item);
      std::cout << "group item_casted refcount = " << *(item_casted._get_refcount()) << std::endl; //Should be 4.
 
      std::cout << "group item_casted name = " << item_casted->get_name() << std::endl;

      field_copy = item_casted;
    }
  }

  std::cout << "field_copy refcount=" << *(field_copy._get_refcount()) << std::endl; //should be 1.
  std::cout << "field_copy name = " << field_copy->get_name() << std::endl;

  return 0;
}





