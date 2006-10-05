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
 
#include "glom/libglom/data_structure/layout/layoutitem_field.h"

class FieldCreator
{
public:
  Glom::sharedptr<Glom::LayoutItem_Field> get_field()
  {
    m_field = Glom::sharedptr<Glom::LayoutItem_Field>::create();
    m_field->set_name("testname");
    return m_field;
  }
protected:
  Glom::sharedptr<Glom::LayoutItem_Field> m_field;
};

int
main(int argc, char* argv[])
{
  Glom::sharedptr<Glom::LayoutItem> layoutitem;
  Glom::sharedptr<Glom::LayoutItem_Field> layoutitem_field;
  {
     FieldCreator creator;
     layoutitem = creator.get_field();
  }

  layoutitem_field =  Glom::sharedptr<Glom::LayoutItem_Field>::cast_dynamic(layoutitem);
  std::cout << "name=" << layoutitem_field->get_name() << std::endl;

  return 0;
}





