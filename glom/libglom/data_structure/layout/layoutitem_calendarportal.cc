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

#include <libglom/data_structure/layout/layoutitem_calendarportal.h>
#include <glibmm/i18n.h>

namespace Glom
{

LayoutItem_CalendarPortal::LayoutItem_CalendarPortal()
{
}

LayoutItem_CalendarPortal::LayoutItem_CalendarPortal(const LayoutItem_CalendarPortal& src)
: LayoutItem_Portal(src),
  m_date_field(src.m_date_field)
{
}

LayoutItem_CalendarPortal::~LayoutItem_CalendarPortal()
{
}

LayoutItem* LayoutItem_CalendarPortal::clone() const
{
  return new LayoutItem_CalendarPortal(*this);
}


LayoutItem_CalendarPortal& LayoutItem_CalendarPortal::operator=(const LayoutItem_CalendarPortal& src)
{
  LayoutItem_Portal::operator=(src);
  m_date_field = src.m_date_field;

  return *this;
}

Glib::ustring LayoutItem_CalendarPortal::get_part_type_name() const
{
  //Translators: This is the name of a UI element (a layout part name).
  return _("Calendar Portal");
}


void LayoutItem_CalendarPortal::change_related_field_item_name(const Glib::ustring& table_name, const Glib::ustring& field_name, const Glib::ustring& field_name_new)
{
  LayoutItem_Portal::change_related_field_item_name(table_name, field_name, field_name_new);
}

void LayoutItem_CalendarPortal::change_field_item_name(const Glib::ustring& table_name, const Glib::ustring& field_name, const Glib::ustring& field_name_new)
{
  LayoutItem_Portal::change_field_item_name(table_name, field_name, field_name_new);
  
  std::shared_ptr<const Relationship> relationship = get_relationship();
        
  if(relationship && (relationship->get_to_table() == table_name) && (m_date_field->get_name() == field_name))
      m_date_field->set_name(field_name_new); //Change it.
}

std::shared_ptr<Field> LayoutItem_CalendarPortal::get_date_field()
{
  return m_date_field;
}

std::shared_ptr<const Field> LayoutItem_CalendarPortal::get_date_field() const
{
  return m_date_field;
}

void LayoutItem_CalendarPortal::set_date_field(const std::shared_ptr<Field>& field)
{
  m_date_field = field;
}

} //namespace Glom
