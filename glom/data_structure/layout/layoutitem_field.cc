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
 
#include "layoutitem_field.h"

LayoutItem_Field::LayoutItem_Field()
: m_priv_view(false),
  m_priv_edit(false),
  m_hidden(false),
  m_choices_restricted(false),
  m_choices_custom(false),
  m_choices_related(false)
{
}

LayoutItem_Field::LayoutItem_Field(const LayoutItem_Field& src)
: LayoutItem(src),
  m_field(src.m_field),
  m_numeric_format(src.m_numeric_format),
  m_priv_view(src.m_priv_view),
  m_priv_edit(src.m_priv_edit),
  //m_table_name(src.m_table_name),
  m_relationship(src.m_relationship),
  m_choices_related_relationship(src.m_choices_related_relationship),
  //m_relationship_name(src.m_relationship_name),
  m_hidden(src.m_hidden),
  m_choices_custom_list(src.m_choices_custom_list),
  m_choices_restricted(src.m_choices_restricted),
  m_choices_custom(src.m_choices_custom),
  m_choices_related(src.m_choices_related),
  m_choices_related_field(src.m_choices_related_field),
  m_choices_related_field_second(src.m_choices_related_field_second)
{
//g_warning("LayoutItem_Field::LayoutItem_Field: m_choices_related_relationship=%s, src.m_choices_related_relationship=%s", m_choices_related_relationship.c_str(), src.m_choices_related_relationship.c_str());

}

LayoutItem_Field::~LayoutItem_Field()
{
}

LayoutItem* LayoutItem_Field::clone() const
{
  return new LayoutItem_Field(*this);
}

bool LayoutItem_Field::operator==(const LayoutItem_Field& src) const
{
  return LayoutItem::operator==(src) &&
    (m_field == src.m_field) &&
    (m_numeric_format == src.m_numeric_format) &&
    (m_priv_view == src.m_priv_view) &&
    (m_priv_edit == src.m_priv_edit) &&
    (m_relationship == src.m_relationship) &&
    (m_hidden == src.m_hidden) &&
    (m_choices_custom_list == src.m_choices_custom_list) &&
    (m_choices_restricted == src.m_choices_restricted) &&
    (m_choices_custom == src.m_choices_custom) &&
    (m_choices_related == src.m_choices_related) &&
    (m_choices_related_relationship == src.m_choices_related_relationship) &&
    (m_choices_related_field == src.m_choices_related_field) &&
    (m_choices_related_field_second == src.m_choices_related_field_second);
}


LayoutItem_Field& LayoutItem_Field::operator=(const LayoutItem_Field& src)
{
  LayoutItem::operator=(src);

  m_field = src.m_field;
  m_numeric_format = src.m_numeric_format;
  m_priv_view = src.m_priv_view;
  m_priv_edit = src.m_priv_edit;

  //m_table_name = src.m_table_name;
  m_relationship = src.m_relationship;
 // m_relationship_name = src.m_relationship_name;

  m_hidden = src.m_hidden;

  m_choices_custom_list = src.m_choices_custom_list;
  m_choices_restricted = src.m_choices_restricted;
  m_choices_custom = src.m_choices_custom;
  m_choices_related = src.m_choices_related;
  m_choices_related_relationship = src.m_choices_related_relationship;
  m_choices_related_field = src.m_choices_related_field;
  m_choices_related_field_second = src.m_choices_related_field_second;

//g_warning("LayoutItem_Field::operator=: m_choices_related_relationship=%s, src.m_choices_related_relationship=%s", m_choices_related_relationship.c_str(), src.m_choices_related_relationship.c_str());
  return *this;
}

void LayoutItem_Field::set_name(const Glib::ustring& name)
{
  m_field.set_name(name);
}

Glib::ustring LayoutItem_Field::get_name() const
{
  return m_field.get_name();
}

/*
Glib::ustring LayoutItem_Field::get_table_name() const
{
  return m_table_name;
}

void LayoutItem_Field::set_table_name(const Glib::ustring& table_name)
{
  m_table_name = table_name;
}
*/

bool LayoutItem_Field::get_has_relationship_name() const
{
  return m_relationship.get_name_not_empty();
}

Glib::ustring LayoutItem_Field::get_relationship_name() const
{
  return m_relationship.get_name();
}

/*
void LayoutItem_Field::set_relationship_name(const Glib::ustring& relationship_name)
{
  m_relationship_name = relationship_name;
}
*/

bool LayoutItem_Field::get_editable_and_allowed() const
{
  return m_editable && m_priv_edit;
}

Glib::ustring LayoutItem_Field::get_layout_display_name() const
{
  Glib::ustring result = m_field.get_name();
  if(get_has_relationship_name())
    result == get_relationship_name() + "::" + result;

  return result;
}

bool LayoutItem_Field::get_hidden() const
{
  return m_hidden;
}

void LayoutItem_Field::set_hidden(bool val)
{
  m_hidden = val;
}

bool LayoutItem_Field::get_has_choices() const
{
  return m_choices_related || m_choices_custom;
}

LayoutItem_Field::type_list_values LayoutItem_Field::get_choices_custom() const
{
  return m_choices_custom_list;
}

void LayoutItem_Field::set_choices_custom(const type_list_values& choices)
{
  m_choices_custom_list = choices;
}

bool LayoutItem_Field::get_choices_restricted() const
{
  return m_choices_restricted;
}

void LayoutItem_Field::set_choices_restricted(bool val)
{
  m_choices_restricted = val;
}

bool LayoutItem_Field::get_has_custom_choices() const
{
  return m_choices_custom;
}

void LayoutItem_Field::set_has_custom_choices(bool val)
{
  m_choices_custom = val;
}

bool LayoutItem_Field::get_has_related_choices() const
{
  return m_choices_related;
}

void LayoutItem_Field::set_has_related_choices(bool val)
{
  m_choices_related = val;
}

void LayoutItem_Field::get_choices(Glib::ustring& relationship_name, Glib::ustring& field, Glib::ustring& field_second) const
{
  relationship_name = m_choices_related_relationship.get_name();
  field = m_choices_related_field;
  field_second = m_choices_related_field_second;

  //g_warning("LayoutItem_Field::get_choices, %s, %s, %s", m_choices_related_relationship.c_str(), m_choices_related_field.c_str(), m_choices_related_field_second.c_str());
}

void LayoutItem_Field::set_choices(const Glib::ustring& relationship_name, const Glib::ustring& field, const Glib::ustring& field_second)
{
  m_choices_related_relationship.set_name(relationship_name);
  m_choices_related_field = field;
  m_choices_related_field_second = field_second;
}

