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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#include <libglom/data_structure/layout/layoutitem_portal.h>
#include <libglom/document/document.h> //For the utility functions.
#include <iostream>
#include <glibmm/i18n.h>

namespace Glom
{

LayoutItem_Portal::LayoutItem_Portal()
: m_print_layout_row_height(20), //arbitrary default.
  m_print_layout_row_line_width(1), //Sensible default.
  m_print_layout_column_line_width(1), //Sensible default.
  m_navigation_type(LayoutItem_Portal::NAVIGATION_AUTOMATIC),
  m_rows_count_min(6), //Sensible default.
  m_rows_count_max(6) //Sensible default.
{
}

LayoutItem_Portal::LayoutItem_Portal(const LayoutItem_Portal& src)
: LayoutGroup(src),
  UsesRelationship(src),
  //HasTitleSingular(src),
  m_navigation_relationship_specific(src.m_navigation_relationship_specific),
  m_print_layout_row_height(src.m_print_layout_row_height),
  m_print_layout_row_line_width(src.m_print_layout_row_line_width),
  m_print_layout_column_line_width(src.m_print_layout_column_line_width),
  m_print_layout_line_color(src.m_print_layout_line_color),
  m_navigation_type(src.m_navigation_type),
  m_rows_count_min(src.m_rows_count_min),
  m_rows_count_max(src.m_rows_count_max)
{
}

LayoutItem_Portal::~LayoutItem_Portal()
{
}

LayoutItem* LayoutItem_Portal::clone() const
{
  return new LayoutItem_Portal(*this);
}


LayoutItem_Portal& LayoutItem_Portal::operator=(const LayoutItem_Portal& src)
{
  LayoutGroup::operator=(src);
  UsesRelationship::operator=(src);
  //HasTitleSingular::operator=(src);

  m_navigation_relationship_specific = src.m_navigation_relationship_specific;
  m_print_layout_row_height = src.m_print_layout_row_height;
  m_print_layout_row_line_width = src.m_print_layout_row_line_width;
  m_print_layout_column_line_width = src.m_print_layout_column_line_width;
  m_print_layout_line_color = src.m_print_layout_line_color;
  m_navigation_type = src.m_navigation_type;
  m_rows_count_min = src.m_rows_count_min;
  m_rows_count_max = src.m_rows_count_max;

  return *this;
}

void LayoutItem_Portal::change_related_field_item_name(const Glib::ustring& table_name, const Glib::ustring& field_name, const Glib::ustring& field_name_new)
{
  LayoutGroup::change_related_field_item_name(table_name, field_name, field_name_new);
}

void LayoutItem_Portal::change_field_item_name(const Glib::ustring& table_name, const Glib::ustring& field_name, const Glib::ustring& field_name_new)
{
  //Look at each item:
  for(const auto& item : m_list_items)
  {
    auto field_item = std::dynamic_pointer_cast<LayoutItem_Field>(item);
    if(field_item)
    {
      if(field_item->get_table_used(Glib::ustring()) == table_name) //If it's a related table (this would be a self-relationship)
      {
        if(field_item->get_name() == field_name)
          field_item->set_name(field_name_new); //Change it.
      }
      else
      {
        std::shared_ptr<const Relationship> relationship = get_relationship();
        if(relationship && (relationship->get_to_table() == table_name) && (field_item->get_name() == field_name))
          field_item->set_name(field_name_new); //Change it.
      }
    }
    else
    {
      auto sub_group = std::dynamic_pointer_cast<LayoutGroup>(item);
      if(sub_group)
        sub_group->change_field_item_name(table_name, field_name, field_name_new);
    }
  }
}

std::shared_ptr<UsesRelationship> LayoutItem_Portal::get_navigation_relationship_specific()
{
  if(get_navigation_type() == LayoutItem_Portal::NAVIGATION_SPECIFIC)
    return m_navigation_relationship_specific;
  else
    return std::shared_ptr<UsesRelationship>();
}

std::shared_ptr<const UsesRelationship> LayoutItem_Portal::get_navigation_relationship_specific() const
{
  if(get_navigation_type() == LayoutItem_Portal::NAVIGATION_SPECIFIC)
    return m_navigation_relationship_specific;
  else
    return std::shared_ptr<UsesRelationship>();
}

void LayoutItem_Portal::set_navigation_relationship_specific(const std::shared_ptr<UsesRelationship>& relationship)
{
  m_navigation_relationship_specific = relationship;
  m_navigation_type = LayoutItem_Portal::NAVIGATION_SPECIFIC;
}

void LayoutItem_Portal::reset_navigation_relationship()
{
    m_navigation_relationship_specific = std::shared_ptr<UsesRelationship>();
    m_navigation_type = LayoutItem_Portal::NAVIGATION_AUTOMATIC;
}

Glib::ustring LayoutItem_Portal::get_from_table() const
{
  Glib::ustring from_table;

  std::shared_ptr<const Relationship> relationship = get_relationship();
  if(relationship)
    from_table = relationship->get_from_table();

  return from_table;
}

double LayoutItem_Portal::get_print_layout_row_height() const
{
  return m_print_layout_row_height;
}

void LayoutItem_Portal::set_print_layout_row_height(double row_height)
{
  m_print_layout_row_height = row_height;
}

LayoutItem_Portal::navigation_type LayoutItem_Portal::get_navigation_type() const
{
  return m_navigation_type;
}

void LayoutItem_Portal::set_navigation_type(LayoutItem_Portal::navigation_type type)
{
  m_navigation_type = type;
}

void LayoutItem_Portal::get_rows_count(gulong& rows_count_min, gulong& rows_count_max) const
{
  rows_count_min = m_rows_count_min;
  rows_count_max = m_rows_count_max;
}
  
void LayoutItem_Portal::set_rows_count(gulong rows_count_min, gulong rows_count_max)
{
  m_rows_count_min = rows_count_min;
  m_rows_count_max = rows_count_max;

  //Keep the values sane:
  if(m_rows_count_max < m_rows_count_min)
    m_rows_count_max = m_rows_count_min;
}

double LayoutItem_Portal::get_print_layout_row_line_width() const
{
  return m_print_layout_row_line_width;
}

void LayoutItem_Portal::set_print_layout_row_line_width(double width)
{
  m_print_layout_row_line_width = width;
}

double LayoutItem_Portal::get_print_layout_column_line_width() const
{
  return m_print_layout_column_line_width;
}
  
void LayoutItem_Portal::set_print_layout_column_line_width(double width)
{
  m_print_layout_column_line_width = width;
}

Glib::ustring LayoutItem_Portal::get_print_layout_line_color() const
{
  return m_print_layout_line_color;
}

void LayoutItem_Portal::set_print_layout_line_color(const Glib::ustring& color)
{
  m_print_layout_line_color = color;
}

void LayoutItem_Portal::get_suitable_table_to_view_details(Glib::ustring& table_name, std::shared_ptr<const UsesRelationship>& relationship, const Document* document) const
{
  //Initialize output parameters:
  table_name = Glib::ustring();

  std::shared_ptr<const UsesRelationship> navigation_relationship;

  //Check whether a relationship was specified:
  if(get_navigation_type() == LayoutItem_Portal::NAVIGATION_AUTOMATIC)
  {
    //std::cout << "debug: decide automatically." << std::endl;
    //Decide automatically:
    navigation_relationship = get_portal_navigation_relationship_automatic(document);
    //if(navigation_relationship && navigation_relationship->get_relationship())
    //  std::cout << "  navigation_relationship->get_relationship()=" <<  navigation_relationship->get_relationship()->get_name() << std::endl;
    //if(navigation_relationship && navigation_relationship->get_related_relationship())
    //  std::cout << "  navigation_relationship->get_related_relationship()=" <<  navigation_relationship->get_related_relationship()->get_name() << std::endl;
  }
  else
  {
    navigation_relationship = get_navigation_relationship_specific();
    //std::cout << "debug: " << G_STRFUNC << ": Using specific nav." << std::endl;
  }


  //Get the navigation table name from the chosen relationship:
  const auto directly_related_table_name = get_table_used(Glib::ustring() /* not relevant */);

  // The navigation_table_name (and therefore, the table_name output parameter,
  // as well) stays empty if the navrel type was set to none.
  Glib::ustring navigation_table_name;
  if(navigation_relationship)
  {
    navigation_table_name = navigation_relationship->get_table_used(directly_related_table_name);
  }
  else if(get_navigation_type() != LayoutItem_Portal::NAVIGATION_NONE)
  {
    //An empty result from get_portal_navigation_relationship_automatic() or 
    //get_navigation_relationship_specific() means we should use the directly related table:
    navigation_table_name = directly_related_table_name;
  }

  if(navigation_table_name.empty())
  {
    //std::cerr << G_STRFUNC << ": navigation_table_name is empty." << std::endl;
    return;
  }
  
  if(!document)
  {
    std::cerr << G_STRFUNC << ": document is null" << std::endl;
    return;
  }
  
  if(document->get_table_is_hidden(navigation_table_name))
  {
    std::cerr << G_STRFUNC << ": navigation_table_name indicates a hidden table: " << navigation_table_name << std::endl;
    return;
  }

  table_name = navigation_table_name;
  relationship = navigation_relationship;
}

std::shared_ptr<const UsesRelationship> LayoutItem_Portal::get_portal_navigation_relationship_automatic(const Document* document) const
{
  if(!document)
  {
    std::cerr << G_STRFUNC << ": document was null" << std::endl;
    return std::shared_ptr<const UsesRelationship>();
  }

  //If the related table is not hidden then we can just navigate to that:
  const auto direct_related_table_name = get_table_used(Glib::ustring() /* parent table - not relevant */);
  if(!(document->get_table_is_hidden(direct_related_table_name)))
  {
    //Non-hidden tables can just be shown directly. Navigate to it:
    return std::shared_ptr<const UsesRelationship>();
  }
  else
  {
    //If the related table is hidden,
    //then find a suitable related non-hidden table by finding the first layout field that mentions one:
    std::shared_ptr<const LayoutItem_Field> field = get_field_is_from_non_hidden_related_record(document);
    if(field)
    {
      return field; //Returns the UsesRelationship base part. (A relationship belonging to the portal's related table.)
      //std::shared_ptr<UsesRelationship> result = std::make_shared<UsesRelationship>();
      //result->set_relationship( get_relationship() );
      //result->set_related_relationship( field->get_relationship() );

      //return result;
    }
    else
    {
      //Instead, find a key field that's used in a relationship,
      //and pretend that we are showing the to field as a related field:
      std::shared_ptr<const Relationship> used_in_relationship;
      std::shared_ptr<const LayoutItem_Field> field_identifies = get_field_identifies_non_hidden_related_record(used_in_relationship, document);
      if(field_identifies)
      {
        auto result = std::make_shared<UsesRelationship>();

        auto rel_nonconst = std::const_pointer_cast<Relationship>(used_in_relationship);
        result->set_relationship(rel_nonconst);

        return result;
      }
    }
  }

  //There was no suitable related table to show:
  return std::shared_ptr<const UsesRelationship>();
}

std::shared_ptr<const LayoutItem_Field> LayoutItem_Portal::get_field_is_from_non_hidden_related_record(const Document* document) const
{
  //Find the first field that is from a non-hidden related table.
  std::shared_ptr<LayoutItem_Field> result;

  if(!document)
  {
    std::cerr << G_STRFUNC << ": document is null" << std::endl;
    return result;
  }
  
  const auto parent_table_name = get_table_used(Glib::ustring() /* parent table - not relevant */);

  auto items = get_items();
  for(const auto& item : items)
  {
    auto field = std::dynamic_pointer_cast<const LayoutItem_Field>(item);
    if(field)
    {
      if(field->get_has_relationship_name())
      {
        const auto table_name = field->get_table_used(parent_table_name);
        if(!(document->get_table_is_hidden(table_name)))
          return field;
      }

    }
  }

  return result;
}

std::shared_ptr<const LayoutItem_Field> LayoutItem_Portal::get_field_identifies_non_hidden_related_record(std::shared_ptr<const Relationship>& used_in_relationship, const Document* document) const
{
  //Find the first field that is from a non-hidden related table.
  std::shared_ptr<LayoutItem_Field> result;

  if(!document)
  {
    std::cerr << G_STRFUNC << ": document is null" << std::endl;
    return result;
  }

  const auto parent_table_name = get_table_used(Glib::ustring() /* parent table - not relevant */);

  auto items = get_items();
  for(const auto& item : items)
  {
    auto field = std::dynamic_pointer_cast<const LayoutItem_Field>(item);
    if(field && !(field->get_has_relationship_name()))
    {
      std::shared_ptr<const Relationship> relationship = document->get_field_used_in_relationship_to_one(parent_table_name, field);
      if(relationship)
      {
        const auto table_name = relationship->get_to_table();
        if(!(table_name.empty()))
        {
          if(!(document->get_table_is_hidden(table_name)))
          {
            used_in_relationship = relationship;
            return field;
          }
        }
      }
    }
  }

  return result;
}

Glib::ustring LayoutItem_Portal::get_title_or_name(const Glib::ustring& locale) const
{
  auto title = get_title_used(Glib::ustring() /* parent table - not relevant */, locale);
  if(title.empty())
    title = get_relationship_name_used();
  
  if(title.empty()) //TODO: This prevents "" as a real title.
   title = _("Undefined Table");

  return title;
}

Glib::ustring LayoutItem_Portal::get_title(const Glib::ustring& locale) const
{
  auto title = get_title_used(Glib::ustring() /* parent table - not relevant */, locale);
  if(title.empty()) //TODO: This prevents "" as a real title.
   title = _("Undefined Table");

  return title;
}

} //namespace Glom
