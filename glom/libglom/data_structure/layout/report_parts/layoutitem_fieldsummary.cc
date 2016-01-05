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
 
#include <libglom/data_structure/layout/report_parts/layoutitem_fieldsummary.h>
#include <glibmm/i18n.h>

namespace Glom
{

LayoutItem_FieldSummary::LayoutItem_FieldSummary()
: m_summary_type(summaryType::INVALID)
{
}

LayoutItem_FieldSummary::LayoutItem_FieldSummary(const LayoutItem_FieldSummary& src)
: LayoutItem_Field(src),
  m_summary_type(src.m_summary_type)
{
}

LayoutItem* LayoutItem_FieldSummary::clone() const
{
  return new LayoutItem_FieldSummary(*this);
}

bool LayoutItem_FieldSummary::operator==(const LayoutItem_FieldSummary& src) const
{
  return LayoutItem_Field::operator==(src) &&
    (m_summary_type == src.m_summary_type);
}

LayoutItem_FieldSummary& LayoutItem_FieldSummary::operator=(const LayoutItem_FieldSummary& src)
{
  LayoutItem_Field::operator=(src);

  m_summary_type = src.m_summary_type;

  return *this;
}

Glib::ustring LayoutItem_FieldSummary::get_title_or_name(const Glib::ustring& locale) const noexcept
{
  const auto field_title = get_full_field_details()->get_title_or_name(locale);

  return get_summary_type_name(m_summary_type) + ": " + field_title; //TODO: Allow a more human-readable title for summary headings.
}

Glib::ustring LayoutItem_FieldSummary::get_title(const Glib::ustring& locale) const noexcept
{
  const auto field_title = get_full_field_details()->get_title(locale);

  return get_summary_type_name(m_summary_type) + ": " + field_title; //TODO: Allow a more human-readable title for summary headings.
}

Glib::ustring LayoutItem_FieldSummary::get_part_type_name() const
{
  //Translators: This is the name of a UI element (a layout part name).
  return _("Field Summary");
}

Glib::ustring LayoutItem_FieldSummary::get_report_part_id() const
{
  return "field";
}

LayoutItem_FieldSummary::summaryType LayoutItem_FieldSummary::get_summary_type() const
{
  return m_summary_type;
}

void LayoutItem_FieldSummary::set_summary_type(summaryType summary_type)
{
  m_summary_type = summary_type;
}

Glib::ustring LayoutItem_FieldSummary::get_summary_type_sql() const
{
  if(m_summary_type == summaryType::INVALID)
    return "INVALID";
  else if(m_summary_type == summaryType::SUM)
    return "SUM";
  else if(m_summary_type == summaryType::AVERAGE)
    return "AVG";
  else if(m_summary_type == summaryType::COUNT)
    return "COUNT";
  else
    return "INVALID";
}

void LayoutItem_FieldSummary::set_summary_type_from_sql(const Glib::ustring& summary_type)
{
  if(summary_type == "SUM")
    m_summary_type = summaryType::SUM;
  else if(summary_type == "AVG")
    m_summary_type = summaryType::AVERAGE;
  else if(summary_type == "COUNT")
    m_summary_type = summaryType::COUNT;
  else
    m_summary_type = summaryType::INVALID;
}

void LayoutItem_FieldSummary::set_field(const std::shared_ptr<LayoutItem_Field>& field)
{
  if(field)
    LayoutItem_Field::operator=(*field);
}

Glib::ustring LayoutItem_FieldSummary::get_layout_display_name() const
{
  auto result = get_layout_display_name_field();

  if(m_summary_type == summaryType::INVALID)
    result = _("No summary chosen");
  else
    result = get_summary_type_name(m_summary_type) + '(' + result + ')';

  return result;
}

Glib::ustring LayoutItem_FieldSummary::get_layout_display_name_field() const
{
  //Get the name for just the field part.
  return LayoutItem_Field::get_layout_display_name();
}

//static:
Glib::ustring LayoutItem_FieldSummary::get_summary_type_name(summaryType summary_type)
{
  if(summary_type == summaryType::INVALID)
    return _("Invalid");
  else if(summary_type == summaryType::SUM)
    return _("Sum");
  else if(summary_type == summaryType::AVERAGE)
    return _("Average");
  else if(summary_type == summaryType::COUNT)
    return _("Count");
  else
    return _("Invalid");
}

} //namespace Glom
