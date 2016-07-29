/* Glom
 *
 * Copyright (C) 2015 Openismus GmbH
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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#include <glom/libglom/init.h>
#include <glom/libglom/data_structure/layout/layoutitem_field.h>
#include <iostream>
#include <cstdlib>

static
bool test_compare_empty_instances()
{
  Glom::LayoutItem_Field layout_item1;
  Glom::LayoutItem_Field layout_item2;
  if(!layout_item1.is_same_field(layout_item2))
  {
    std::cerr << G_STRFUNC << ": Glom::LayoutItem_Field::is_same_field() failed with empty instances.\n";
    return false;
  }

  return true;
}

static
bool test_compare_same_named_instances()
{
  Glom::LayoutItem_Field layout_item1;
  Glom::LayoutItem_Field layout_item2;
  layout_item1.set_name("one");
  layout_item2.set_name("two");
  if(layout_item1.is_same_field(layout_item2))
  {
    std::cerr << G_STRFUNC << ": Glom::LayoutItem_Field::is_same_field() failed with named field instances.\n";
    return false;
  }

  return true;
}

static
bool test_compare_same_named_instances_unrelated_differences()
{
  Glom::LayoutItem_Field layout_item1;
  Glom::LayoutItem_Field layout_item2;
  layout_item1.set_name("one");
  layout_item2.set_name("one");
  layout_item2.set_hidden(); //is_same_field() should ignore this.
  if(!layout_item1.is_same_field(layout_item2))
  {
    std::cerr << G_STRFUNC << ": Glom::LayoutItem_Field::is_same_field() failed with named field instances with unrelated differences.\n";
    return false;
  }

  return true;
}


static
bool test_compare_same_named_instances_with_relationship()
{
  Glom::LayoutItem_Field layout_item1;
  Glom::LayoutItem_Field layout_item2;
  layout_item1.set_name("one");
  layout_item2.set_name("one");

  auto relationship1 = std::make_shared<Glom::Relationship>();
  relationship1->set_name("relationship1");
  layout_item1.set_relationship(relationship1);

  auto relationship2 = std::make_shared<Glom::Relationship>();
  relationship2->set_name("relationship2");
  layout_item2.set_relationship(relationship2);

  if(layout_item1.is_same_field(layout_item2))
  {
    std::cerr << G_STRFUNC << ": Glom::LayoutItem_Field::is_same_field() failed with field instances with different relationships.\n";
    return false;
  }

  layout_item2.set_relationship(relationship1);
  if(!layout_item1.is_same_field(layout_item2))
  {
    std::cerr << G_STRFUNC << ": Glom::LayoutItem_Field::is_same_field() failed with field instances with same relationships.\n";
    return false;
  }

  return true;
}

static
bool test_compare_same_named_instances_with_related_relationship()
{
  Glom::LayoutItem_Field layout_item1;
  Glom::LayoutItem_Field layout_item2;
  layout_item1.set_name("one");
  layout_item2.set_name("one");

  auto relationship1 = std::make_shared<Glom::Relationship>();
  relationship1->set_name("relationship1");
  layout_item1.set_relationship(relationship1);
  layout_item2.set_relationship(relationship1);

  auto relationship_related1 = std::make_shared<Glom::Relationship>();
  relationship_related1->set_name("relationship_related1");
  layout_item1.set_related_relationship(relationship_related1);

  if(layout_item1.is_same_field(layout_item2))
  {
    std::cerr << G_STRFUNC << ": Glom::LayoutItem_Field::is_same_field() failed with field instances with different (one unset) related relationships.\n";
    return false;
  }

  auto relationship_related2 = std::make_shared<Glom::Relationship>();
  relationship_related2->set_name("relationship_related2");
  layout_item2.set_related_relationship(relationship_related2);

  if(layout_item1.is_same_field(layout_item2))
  {
    std::cerr << G_STRFUNC << ": Glom::LayoutItem_Field::is_same_field() failed with field instances with different related relationships.\n";
    return false;
  }

  layout_item2.set_related_relationship(relationship_related1);
  if(!layout_item1.is_same_field(layout_item2))
  {
    std::cerr << G_STRFUNC << ": Glom::LayoutItem_Field::is_same_field() failed with field instances with same related relationships.\n";
    return false;
  }

  return true;
}

int main()
{
  Glom::libglom_init();

  if(!test_compare_empty_instances())
    return EXIT_FAILURE;

  if(!test_compare_same_named_instances())
    return EXIT_FAILURE;

  if(!test_compare_same_named_instances_unrelated_differences())
    return EXIT_FAILURE;

  if(!test_compare_same_named_instances_with_relationship())
    return EXIT_FAILURE;

  if(!test_compare_same_named_instances_with_related_relationship())
    return EXIT_FAILURE;

  Glom::libglom_deinit();

  return EXIT_SUCCESS;
}
