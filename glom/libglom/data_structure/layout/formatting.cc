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

#include <libglom/data_structure/layout/formatting.h>
#include <libglom/data_structure/glomconversions.h>
#include <libglom/document/document.h>
#include <glibmm/i18n.h>

const guint MULTILINE_TEXT_DEFAULT_HEIGHT_LINES = 6;
//const char* MULTILINE_TEXT_DEFAULT_WIDTH_EXAMPLE = "abcdefghijklmnopqrstuvwxyz"
namespace Glom
{

Formatting::Formatting()
: m_choices_restricted(false),
  m_choices_restricted_as_radio_buttons(false),
  m_choices_custom(false),
  m_choices_related(false),
  m_text_format_multiline(false),
  m_text_multiline_height_lines(MULTILINE_TEXT_DEFAULT_HEIGHT_LINES),
  m_horizontal_alignment(HorizontalAlignment::AUTO),
  m_choices_related_show_all(true) //Because this a the simpler, more often useful, default.
{
}

Formatting::Formatting(const Formatting& src)
: UsesRelationship(src),
  m_numeric_format(src.m_numeric_format),
  m_choices_custom_list(src.m_choices_custom_list),
  m_choices_restricted(src.m_choices_restricted),
  m_choices_restricted_as_radio_buttons(src.m_choices_restricted_as_radio_buttons),
  m_choices_custom(src.m_choices_custom),
  m_choices_related(src.m_choices_related),
  m_text_format_multiline(src.m_text_format_multiline),
  m_text_multiline_height_lines(src.m_text_multiline_height_lines),
  m_text_font(src.m_text_font),
  m_text_color_foreground(src.m_text_color_foreground),
  m_text_color_background(src.m_text_color_background),
  m_horizontal_alignment(src.m_horizontal_alignment),
  m_choices_related_field(src.m_choices_related_field),
  m_choices_extra_layout_group(src.m_choices_extra_layout_group),
  m_choices_related_sort_fields(src.m_choices_related_sort_fields),
  m_choices_related_show_all(src.m_choices_related_show_all)
{
}

bool Formatting::operator==(const Formatting& src) const
{
  return UsesRelationship::operator==(src) &&
    (m_numeric_format == src.m_numeric_format) &&
    (m_choices_custom_list == src.m_choices_custom_list) &&
    (m_choices_restricted == src.m_choices_restricted) &&
    (m_choices_restricted_as_radio_buttons == src.m_choices_restricted_as_radio_buttons) &&
    (m_choices_custom == src.m_choices_custom) &&
    (m_choices_related == src.m_choices_related) &&
    (m_choices_related_field == src.m_choices_related_field) &&
    (m_choices_extra_layout_group == src.m_choices_extra_layout_group) &&
    (m_choices_related_sort_fields == src.m_choices_related_sort_fields) &&
    (m_text_format_multiline == src.m_text_format_multiline) &&
    (m_text_multiline_height_lines == src.m_text_multiline_height_lines) &&
    (m_text_font == src.m_text_font) &&
    (m_text_color_foreground == src.m_text_color_foreground) &&
    (m_text_color_background == src.m_text_color_background) &&
    (m_horizontal_alignment == src.m_horizontal_alignment) &&
    (m_choices_related_show_all == src.m_choices_related_show_all);
}


Formatting& Formatting::operator=(const Formatting& src)
{
  UsesRelationship::operator=(src);

  m_numeric_format = src.m_numeric_format;

  m_choices_custom_list = src.m_choices_custom_list;
  m_choices_restricted = src.m_choices_restricted;
  m_choices_restricted_as_radio_buttons = src.m_choices_restricted_as_radio_buttons;
  m_choices_custom = src.m_choices_custom;
  m_choices_related = src.m_choices_related;
  m_choices_related_field = src.m_choices_related_field;
  m_choices_extra_layout_group = src.m_choices_extra_layout_group;
  m_choices_related_sort_fields = src.m_choices_related_sort_fields;
  m_choices_related_show_all = src.m_choices_related_show_all;

  m_text_format_multiline = src.m_text_format_multiline;
  m_text_multiline_height_lines = src.m_text_multiline_height_lines;
  m_text_font = src.m_text_font;
  m_text_color_foreground = src.m_text_color_foreground;
  m_text_color_background = src.m_text_color_background;
  m_horizontal_alignment = src.m_horizontal_alignment;

  //std::cerr << G_STRFUNC << ": m_choices_related_relationship=" << m_choices_related_relationship << ", src.m_choices_related_relationship=" << src.m_choices_related_relationship << std::endl;
  return *this;
}

bool Formatting::get_text_format_multiline() const
{
  return m_text_format_multiline;
}

void Formatting::set_text_format_multiline(bool value)
{
  m_text_format_multiline = value;
}

guint Formatting::get_text_format_multiline_height_lines() const
{
  return m_text_multiline_height_lines;
}

void Formatting::set_text_format_multiline_height_lines(guint value)
{
  //Do not allow inappropriate values. TODO: Respond when they are entered, not later here.
  if(value < 2)
    value = MULTILINE_TEXT_DEFAULT_HEIGHT_LINES;

  m_text_multiline_height_lines = value;
}

void Formatting::set_text_format_font(const Glib::ustring& font_desc)
{
  m_text_font = font_desc;
}

Glib::ustring Formatting::get_text_format_font() const
{
  return m_text_font;
}

void Formatting::set_text_format_color_foreground(const Glib::ustring& color)
{
  m_text_color_foreground = color;
}

Glib::ustring Formatting::get_text_format_color_foreground() const
{
  return m_text_color_foreground;
}

Glib::ustring Formatting::get_text_format_color_foreground_to_use(const Gnome::Gda::Value& value) const
{
  if(m_numeric_format.m_alt_foreground_color_for_negatives)
  {
    //TODO: Use some other color if the alternative color is too similar to the foreground color.
    if(Conversions::get_double_for_gda_value_numeric(value) < 0)
      return NumericFormat::get_alternative_color_for_negatives();
  }

  return m_text_color_foreground;
}

void Formatting::set_text_format_color_background(const Glib::ustring& color)
{
  m_text_color_background = color;
}

Glib::ustring Formatting::get_text_format_color_background() const
{
  return m_text_color_background;
}

void Formatting::set_horizontal_alignment(HorizontalAlignment alignment)
{
  m_horizontal_alignment = alignment;
}

Formatting::HorizontalAlignment Formatting::get_horizontal_alignment() const
{
  return m_horizontal_alignment;
}

bool Formatting::get_has_choices() const
{
  return ( m_choices_related && get_has_relationship_name() && m_choices_related_field ) ||
         ( m_choices_custom && !m_choices_custom_list.empty() );
}

Formatting::type_list_values Formatting::get_choices_custom() const
{
  return m_choices_custom_list;
}

void Formatting::set_choices_custom(const type_list_values& choices)
{
  m_choices_custom_list = choices;
}

Glib::ustring Formatting::get_custom_choice_original_for_translated_text(const Glib::ustring& text, const Glib::ustring& locale) const
{
  for(const auto& value : m_choices_custom_list)
  {
    if(!value)
      continue;

    if(value->get_title(locale) == text)
      return value->get_title_original();
  }

  return Glib::ustring();
}

Glib::ustring Formatting::get_custom_choice_translated(const Glib::ustring& original_text, const Glib::ustring& locale) const
{
  for(const auto& value : m_choices_custom_list)
  {
    if(!value)
      continue;

    if(value->get_title_original() == original_text)
      return value->get_title(locale);
  }

  return Glib::ustring();
}


bool Formatting::get_choices_restricted(bool& as_radio_buttons) const
{
  as_radio_buttons = m_choices_restricted_as_radio_buttons;
  return m_choices_restricted;
}

void Formatting::set_choices_restricted(bool val, bool as_radio_buttons)
{
  m_choices_restricted = val;
  m_choices_restricted_as_radio_buttons = as_radio_buttons;
}

bool Formatting::get_has_custom_choices() const
{
  return m_choices_custom;
}

void Formatting::set_has_custom_choices(bool val)
{
  m_choices_custom = val;
}

bool Formatting::get_has_related_choices() const
{
  return m_choices_related;
}

bool Formatting::get_has_related_choices(bool& show_all, bool& with_second) const
{
  show_all = m_choices_related_show_all;
  with_second = (bool)m_choices_extra_layout_group;
  return m_choices_related;
}

void Formatting::set_has_related_choices(bool val)
{
  m_choices_related = val;
}

void Formatting::set_choices_related(const std::shared_ptr<const Relationship>& relationship, const std::shared_ptr<LayoutItem_Field>& field, const std::shared_ptr<LayoutGroup>& extra_layout, const type_list_sort_fields& sort_fields, bool show_all)
{
  set_relationship(relationship);

  m_choices_related_field = field;
  m_choices_extra_layout_group = extra_layout;
  m_choices_related_sort_fields = sort_fields;
  m_choices_related_show_all = show_all;
}

void Formatting::get_choices_related(std::shared_ptr<const Relationship>& relationship, std::shared_ptr<const LayoutItem_Field>& field, std::shared_ptr<const LayoutGroup>& extra_layout, type_list_sort_fields& sort_fields, bool& show_all) const
{
  relationship = get_relationship();

  field = m_choices_related_field;
  extra_layout = m_choices_extra_layout_group;
  sort_fields = m_choices_related_sort_fields;
  show_all = m_choices_related_show_all;
}


void Formatting::get_choices_related(std::shared_ptr<const Relationship>& relationship, std::shared_ptr<LayoutItem_Field>& field, std::shared_ptr<LayoutGroup>& extra_layout, type_list_sort_fields& sort_fields, bool& show_all)
{
  relationship = get_relationship();

  field = m_choices_related_field;
  extra_layout = m_choices_extra_layout_group;
  sort_fields = m_choices_related_sort_fields;
  show_all = m_choices_related_show_all;
}

std::shared_ptr<const Relationship> Formatting::get_choices_related_relationship(bool& show_all) const
{
  show_all = m_choices_related_show_all;
  return get_relationship();
}

bool Formatting::change_field_item_name(const Glib::ustring& table_name, const Glib::ustring& field_name_old, const Glib::ustring& field_name_new)
{
  if(!m_choices_related_field)
    return false; //Nothing changed.

  auto relationship = get_relationship();
  
  const Glib::ustring field_table = 
    m_choices_related_field->get_table_used( relationship->get_to_table() );

  if((field_table == table_name) &&
     (m_choices_related_field->get_name() == field_name_old))
  {
    //Change it:
    m_choices_related_field->set_name(field_name_new);
    return true; //something changed.
  }
  
  if(m_choices_extra_layout_group)
  {
    m_choices_extra_layout_group->change_field_item_name(table_name, 
      field_name_old, field_name_new);
  }

  return false; //Nothing changed.
}

} //namespace Glom
