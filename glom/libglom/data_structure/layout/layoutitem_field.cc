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

#include <libglom/data_structure/layout/layoutitem_field.h>
#include <glibmm/i18n.h>

namespace Glom
{

LayoutItem_Field::LayoutItem_Field()
: m_priv_view(false),
  m_priv_edit(false),
  m_field_cache_valid(false),
  m_hidden(false),
  m_formatting_use_default(true)
{
}

LayoutItem_Field::LayoutItem_Field(const LayoutItem_Field& src)
: LayoutItem_WithFormatting(src),
  UsesRelationship(src),
  m_priv_view(src.m_priv_view),
  m_priv_edit(src.m_priv_edit),
  //m_table_name(src.m_table_name),
  m_field_cache_valid(src.m_field_cache_valid),
  m_hidden(src.m_hidden),
  m_formatting_use_default(src.m_formatting_use_default),
  m_title_custom(src.m_title_custom)
{
//std::cerr << G_STRFUNC << ": m_choices_related_relationship=" << m_choices_related_relationship << ", src.m_choices_related_relationship=" << src.m_choices_related_relationship << std::endl;

  m_field = src.m_field;
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
  auto result = LayoutItem_WithFormatting::operator==(src) &&
    UsesRelationship::operator==(src) &&
    (m_priv_view == src.m_priv_view) &&
    (m_priv_edit == src.m_priv_edit) &&
    (m_hidden == src.m_hidden) &&
    (m_formatting_use_default == src.m_formatting_use_default) &&
    (m_field_cache_valid == src.m_field_cache_valid);

  if(m_field && src.m_field)
    result = result && (*m_field == *(src.m_field));
  else
    result = result && (m_field == src.m_field);

  if(m_title_custom && src.m_title_custom)
    result = result && (*m_title_custom == *(src.m_title_custom));
  else
    result = result && (m_title_custom == src.m_title_custom);

  return result;
}

//Avoid using this, for performance:
LayoutItem_Field& LayoutItem_Field::operator=(const LayoutItem_Field& src)
{
  LayoutItem_WithFormatting::operator=(src);
  UsesRelationship::operator=(src);

  m_field = src.m_field;
  m_field_cache_valid = src.m_field_cache_valid;

  m_priv_view = src.m_priv_view;
  m_priv_edit = src.m_priv_edit;

  m_hidden = src.m_hidden;

  m_formatting_use_default = src.m_formatting_use_default;

  m_title_custom = src.m_title_custom;

  return *this;
}

void LayoutItem_Field::set_name(const Glib::ustring& name)
{
  if(get_name() != name)
    m_field_cache_valid = false;

  LayoutItem_WithFormatting::set_name(name);
}

Glib::ustring LayoutItem_Field::get_name() const
{
  return LayoutItem_WithFormatting::get_name();
}

Glib::ustring LayoutItem_Field::get_title_no_custom(const Glib::ustring& locale) const
{
  //Use the field's default title:
  if(m_field_cache_valid && m_field)
  {
    return m_field->get_title_or_name(locale);
  }
  else
    return get_name(); //We ignore TranslatableItem::get_title() for LayoutItem_Field.
}

Glib::ustring LayoutItem_Field::get_title_no_custom_translation(const Glib::ustring& locale, bool fallback) const
{
  //Use the field's default title:
  if(m_field_cache_valid && m_field)
  {
    return m_field->get_title_translation(locale, fallback);
  }
  else
    return Glib::ustring();
}

Glib::ustring LayoutItem_Field::get_title_or_name_no_custom(const Glib::ustring& locale) const
{
  //Use the field's default title:
  if(m_field_cache_valid && m_field)
  {
    return m_field->get_title(locale);
  }

  return Glib::ustring();
}

Glib::ustring LayoutItem_Field::get_title(const Glib::ustring& locale) const
{
  //Use the custom title (overriding the field's default title), if there is one:
  //This may even be empty if the developer specifies that.
  if(m_title_custom && m_title_custom->get_use_custom_title())
  {
    return m_title_custom->get_title(locale);
  }

  //Use the field's default title:
  return get_title_no_custom(locale);
}

Glib::ustring LayoutItem_Field::get_title_or_name(const Glib::ustring& locale) const
{
  //Use the custom title (overriding the field's default title), if there is one:
  //This may even be empty if the developer specifies that.
  if(m_title_custom && m_title_custom->get_use_custom_title())
  {
    return m_title_custom->get_title(locale);
  }

  //Use the field's default title:
  return get_title_no_custom(locale);
}

bool LayoutItem_Field::get_editable_and_allowed() const
{
  //The relationship might forbid editing of any fields through itself:
  if(get_has_relationship_name())
  {
    std::shared_ptr<const Relationship> rel = get_relationship();
    if(rel)
    {
      if(!(rel->get_allow_edit()))
        return false;
    }
  }
  else if(m_field && m_field->get_has_calculation())
  {
    return false; //Calculations can never be edited.
  }

  return get_editable() && m_priv_edit;
}

Glib::ustring LayoutItem_Field::get_layout_display_name() const
{
  Glib::ustring result;

  if(m_field_cache_valid && m_field)
    result = m_field->get_name();
  else
    result = get_name();

  //Indicate if it's a field in another table.
  if(get_has_related_relationship_name())
    result = get_related_relationship_name() + "::" + result;

  if(get_has_relationship_name())
    result = get_relationship_name() + "::" + result;

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

Glib::ustring LayoutItem_Field::get_part_type_name() const
{
  //Translators: This is the name of a UI element (a layout part name).
  return _("Field");
}

Glib::ustring LayoutItem_Field::get_report_part_id() const
{
  return "field";
}

bool LayoutItem_Field::get_formatting_use_default() const
{
  return m_formatting_use_default;
}

void LayoutItem_Field::set_formatting_use_default(bool use_default)
{
  m_formatting_use_default = use_default;
}

const Formatting& LayoutItem_Field::get_formatting_used() const
{
  if(m_formatting_use_default && m_field_cache_valid && m_field)
    return m_field->m_default_formatting;
  else
    return m_formatting;
}

Formatting::HorizontalAlignment LayoutItem_Field::get_formatting_used_horizontal_alignment(bool for_details_view) const
{
  const auto format = get_formatting_used();
  Formatting::HorizontalAlignment alignment = 
    format.get_horizontal_alignment();
  
  if(alignment == Formatting::HORIZONTAL_ALIGNMENT_AUTO)
  {
    //By default, right-align numbers on list views, unless they are ID fields.
    //And left-align them on details views, because that looks silly otherwise.
    if(!for_details_view && (m_field && !m_field->get_primary_key())) //TODO: Also prevent this when it is a foreign key.
    {
      //Align numbers to the right by default:
      alignment = (m_field->get_glom_type() == Field::TYPE_NUMERIC ? Formatting::HORIZONTAL_ALIGNMENT_RIGHT : Formatting::HORIZONTAL_ALIGNMENT_LEFT);
    }
    else
      alignment = Formatting::HORIZONTAL_ALIGNMENT_LEFT;
  }
  
  return alignment;
}

bool LayoutItem_Field::get_formatting_used_has_translatable_choices() const
{
  const auto formatting = get_formatting_used();
  if(!formatting.get_has_custom_choices())
    return false;

  bool as_radio_buttons = false; //Ignored.
  if(!formatting.get_choices_restricted(as_radio_buttons))
    return false;

  return true;
}


void LayoutItem_Field::set_full_field_details(const std::shared_ptr<const Field>& field)
{

  if(field)
  {
    //std::cout << "debug: " << G_STRFUNC << ": name=" << field->get_name() << std::endl;
    //std::cout << "debug: " << G_STRFUNC << ": field->get_title_or_name()=" << field->get_title_or_name() << std::endl;
    m_field = field;
    m_field_cache_valid = true;

    LayoutItem_WithFormatting::set_name(field->get_name()); //It seems to be OK to expect get_name() to work after setting _full_ details.
  }
  else
  {
    //std::cout << "LayoutItem_Field::set_full_field_details(null): previous name=" << m_name << std::endl;
    m_field = std::shared_ptr<const Field>();
    m_field_cache_valid = false;
  }
}

std::shared_ptr<const Field> LayoutItem_Field::get_full_field_details() const
{
  return m_field;
}

Field::glom_field_type LayoutItem_Field::get_glom_type() const
{
  if(m_field && m_field_cache_valid)
    return m_field->get_glom_type();
  else
    return Field::TYPE_INVALID;
}


std::shared_ptr<const CustomTitle> LayoutItem_Field::get_title_custom() const
{
  return m_title_custom;
}

std::shared_ptr<CustomTitle> LayoutItem_Field::get_title_custom()
{
  return m_title_custom;
}

void LayoutItem_Field::set_title_custom(const std::shared_ptr<CustomTitle>& title)
{
  m_title_custom = title;
}

bool LayoutItem_Field::is_same_field(const std::shared_ptr<const LayoutItem_Field>& field) const
{
  const auto uses_a = this;
  const auto uses_b = &(*field);
  if(!uses_a || !uses_b)
    return false; //Shouldn't happen.
    
  return (get_name() == field->get_name()) &&
         (*uses_a == *uses_b);
}

} //namespace Glom


