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

#include "fieldformatting.h"
#include <libglom/data_structure/layout/fieldformatting.h>
#include <libglom/data_structure/glomconversions.h>
#include <libglom/document/document.h>
#include <glibmm/i18n.h>

const guint MULTILINE_TEXT_DEFAULT_HEIGHT_LINES = 6;
//const char* MULTILINE_TEXT_DEFAULT_WIDTH_EXAMPLE = "abcdefghijklmnopqrstuvwxyz"
namespace Glom
{

FieldFormatting::FieldFormatting()
: m_choices_restricted(false),
  m_choices_restricted_as_radio_buttons(false),
  m_choices_custom(false),
  m_choices_related(false),
  m_text_format_multiline(false),
  m_text_multiline_height_lines(MULTILINE_TEXT_DEFAULT_HEIGHT_LINES),
  m_horizontal_alignment(HORIZONTAL_ALIGNMENT_AUTO),
  m_choices_related_show_all(true) //Because this a the simpler, more often useful, default.
{
}

FieldFormatting::FieldFormatting(const FieldFormatting& src)
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
  m_choices_related_field_second(src.m_choices_related_field_second),
  m_choices_related_show_all(src.m_choices_related_show_all)
{
}

FieldFormatting::~FieldFormatting()
{
}

bool FieldFormatting::operator==(const FieldFormatting& src) const
{
  return UsesRelationship::operator==(src) &&
    (m_numeric_format == src.m_numeric_format) &&
    (m_choices_custom_list == src.m_choices_custom_list) &&
    (m_choices_restricted == src.m_choices_restricted) &&
    (m_choices_restricted_as_radio_buttons == src.m_choices_restricted_as_radio_buttons) &&
    (m_choices_custom == src.m_choices_custom) &&
    (m_choices_related == src.m_choices_related) &&
    (m_choices_related_field == src.m_choices_related_field) &&
    (m_choices_related_field_second == src.m_choices_related_field_second) &&
    (m_text_format_multiline == src.m_text_format_multiline) &&
    (m_text_multiline_height_lines == src.m_text_multiline_height_lines) &&
    (m_text_font == src.m_text_font) &&
    (m_text_color_foreground == src.m_text_color_foreground) &&
    (m_text_color_background == src.m_text_color_background) &&
    (m_horizontal_alignment == src.m_horizontal_alignment) &&
    (m_choices_related_show_all == src.m_choices_related_show_all);
}


FieldFormatting& FieldFormatting::operator=(const FieldFormatting& src)
{
  UsesRelationship::operator=(src);

  m_numeric_format = src.m_numeric_format;

  m_choices_custom_list = src.m_choices_custom_list;
  m_choices_restricted = src.m_choices_restricted;
  m_choices_restricted_as_radio_buttons = src.m_choices_restricted_as_radio_buttons;
  m_choices_custom = src.m_choices_custom;
  m_choices_related = src.m_choices_related;
  m_choices_related_field = src.m_choices_related_field;
  m_choices_related_field_second = src.m_choices_related_field_second;
  m_choices_related_show_all = src.m_choices_related_show_all;

  m_text_format_multiline = src.m_text_format_multiline;
  m_text_multiline_height_lines = src.m_text_multiline_height_lines;
  m_text_font = src.m_text_font;
  m_text_color_foreground = src.m_text_color_foreground;
  m_text_color_background = src.m_text_color_background;
  m_horizontal_alignment = src.m_horizontal_alignment;

//g_warning("FieldFormatting::operator=: m_choices_related_relationship=%s, src.m_choices_related_relationship=%s", m_choices_related_relationship->c_str(), src.m_choices_related_relationship->c_str());
  return *this;
}

bool FieldFormatting::get_text_format_multiline() const
{
  return m_text_format_multiline;
}

void FieldFormatting::set_text_format_multiline(bool value)
{
  m_text_format_multiline = value;
}

guint FieldFormatting::get_text_format_multiline_height_lines() const
{
  return m_text_multiline_height_lines;
}

void FieldFormatting::set_text_format_multiline_height_lines(guint value)
{
  //Do not allow inappropriate values. TODO: Respond when they are entered, not later here.
  if(value < 2)
    value = MULTILINE_TEXT_DEFAULT_HEIGHT_LINES;

  m_text_multiline_height_lines = value;
}

void FieldFormatting::set_text_format_font(const Glib::ustring& font_desc)
{
  m_text_font = font_desc;
}

Glib::ustring FieldFormatting::get_text_format_font() const
{
  return m_text_font;
}

void FieldFormatting::set_text_format_color_foreground(const Glib::ustring& color)
{
  m_text_color_foreground = color;
}

Glib::ustring FieldFormatting::get_text_format_color_foreground() const
{
  return m_text_color_foreground;
}

Glib::ustring FieldFormatting::get_text_format_color_foreground_to_use(const Gnome::Gda::Value& value) const
{
  if(m_numeric_format.m_alt_foreground_color_for_negatives)
  {
    //TODO: Use some other color if the alternative color is too similar to the foreground color.
    if(Conversions::get_double_for_gda_value_numeric(value) < 0)
      return NumericFormat::get_alternative_color_for_negatives();
  }

  return m_text_color_foreground;
}

void FieldFormatting::set_text_format_color_background(const Glib::ustring& color)
{
  m_text_color_background = color;
}

Glib::ustring FieldFormatting::get_text_format_color_background() const
{
  return m_text_color_background;
}

void FieldFormatting::set_horizontal_alignment(HorizontalAlignment alignment)
{
  m_horizontal_alignment = alignment;
}

FieldFormatting::HorizontalAlignment FieldFormatting::get_horizontal_alignment() const
{
  return m_horizontal_alignment;
}

bool FieldFormatting::get_has_choices() const
{
  return ( m_choices_related && get_has_relationship_name() && !m_choices_related_field.empty() ) ||
         ( m_choices_custom && !m_choices_custom_list.empty() );
}

FieldFormatting::type_list_values FieldFormatting::get_choices_custom() const
{
  return m_choices_custom_list;
}

void FieldFormatting::set_choices_custom(const type_list_values& choices)
{
  m_choices_custom_list = choices;
}

bool FieldFormatting::get_choices_restricted(bool& as_radio_buttons) const
{
  as_radio_buttons = m_choices_restricted_as_radio_buttons;
  return m_choices_restricted;
}

void FieldFormatting::set_choices_restricted(bool val, bool as_radio_buttons)
{
  m_choices_restricted = val;
  m_choices_restricted_as_radio_buttons = as_radio_buttons;
}

bool FieldFormatting::get_has_custom_choices() const
{
  return m_choices_custom;
}

void FieldFormatting::set_has_custom_choices(bool val)
{
  m_choices_custom = val;
}

bool FieldFormatting::get_has_related_choices() const
{
  return m_choices_related;
}

void FieldFormatting::set_has_related_choices(bool val)
{
  m_choices_related = val;
}

void FieldFormatting::get_choices_related(sharedptr<const Relationship>& relationship, Glib::ustring& field, Glib::ustring& field_second, bool& show_all) const
{
  relationship = get_relationship();

  field = m_choices_related_field;
  field_second = m_choices_related_field_second;
  show_all = m_choices_related_show_all;

  //g_warning("FieldFormatting::get_choices, %s, %s, %s", m_choices_related_relationship->c_str(), m_choices_related_field.c_str(), m_choices_related_field_second.c_str());
}

void FieldFormatting::set_choices_related(const sharedptr<const Relationship>& relationship, const Glib::ustring& field, const Glib::ustring& field_second, bool show_all)
{
  set_relationship(relationship);

  m_choices_related_field = field;
  m_choices_related_field_second = field_second;
  m_choices_related_show_all = show_all;
}

void FieldFormatting::get_choices_related(const Document* document, sharedptr<const Relationship>& relationship, sharedptr<const LayoutItem_Field>& field, sharedptr<const LayoutItem_Field>& field_second, bool& show_all) const
{
  //Initialize output parameters:
  field.clear();
  field_second.clear();

  Glib::ustring choice_field, choice_second;
  get_choices_related(relationship, choice_field, choice_second, show_all);

  if(!relationship)
    return;

  if(choice_field.empty())
    return;

  const Glib::ustring to_table = relationship->get_to_table();

  sharedptr<LayoutItem_Field> temp = sharedptr<LayoutItem_Field>::create();
  sharedptr<const Field> field_details = document->get_field(to_table, choice_field);
  temp->set_full_field_details(field_details);
  field = temp;

  if(!choice_second.empty())
  {
    sharedptr<LayoutItem_Field> temp = sharedptr<LayoutItem_Field>::create();
    sharedptr<const Field> field_details = document->get_field(to_table, choice_second);
    temp->set_full_field_details(field_details);
    field_second = temp;
  }
}

void FieldFormatting::change_field_name(const Glib::ustring& table_name, const Glib::ustring& field_name, const Glib::ustring& field_name_new)
{
  //Update choices:
  if(get_has_relationship_name() && get_table_used(Glib::ustring()) == table_name)
  {
    if(m_choices_related_field == field_name)
       m_choices_related_field = field_name_new;

    if(m_choices_related_field_second == field_name)
       m_choices_related_field_second = field_name_new;
  }
}

} //namespace Glom
