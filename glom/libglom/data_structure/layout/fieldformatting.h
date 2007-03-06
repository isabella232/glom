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

#ifndef GLOM_DATASTRUCTURE_FIELDFORMATTING_H
#define GLOM_DATASTRUCTURE_FIELDFORMATTING_H

#include "usesrelationship.h"
#include "../numeric_format.h"
#include "../relationship.h"
#include <glom/libglom/sharedptr.h>
#include <libgdamm.h>

namespace Glom
{

class FieldFormatting : public UsesRelationship //The UsesRelationship base has the relationship for the choices.
{
public:

  FieldFormatting();
  FieldFormatting(const FieldFormatting& src);
  FieldFormatting& operator=(const FieldFormatting& src);
  virtual ~FieldFormatting();

  bool operator==(const FieldFormatting& src) const;

  bool get_has_choices() const;

  bool get_has_related_choices() const;
  void set_has_related_choices(bool val = true);

  bool get_has_custom_choices() const;
  void set_has_custom_choices(bool val = true);

  typedef std::list<Glib::ValueBase> type_list_values;
  virtual type_list_values get_choices_custom() const;
  virtual void set_choices_custom(const type_list_values& choices);

  bool get_choices_restricted() const;
  void set_choices_restricted(bool val = true);

  void get_choices(sharedptr<Relationship>& relationship_name, Glib::ustring& field, Glib::ustring& field_second) const;
  void set_choices(const sharedptr<Relationship>& relationship_name, const Glib::ustring& field, const Glib::ustring& field_second);

  bool get_text_format_multiline() const;
  void set_text_format_multiline(bool value = true);

  guint get_text_format_multiline_height_lines() const;
  void set_text_format_multiline_height_lines(guint value);

  void change_field_name(const Glib::ustring& table_name, const Glib::ustring& field_name, const Glib::ustring& field_name_new);

  NumericFormat m_numeric_format; //Only used for numeric fields.

protected:

  type_list_values m_choices_custom_list; //A drop-down list of possible values for the field.
  bool m_choices_restricted;
  bool m_choices_custom, m_choices_related;

  bool m_text_format_multiline;
  guint m_text_multiline_height_lines; //The height in number of lines of text.
  //Glib::ustring m_text_multiline_width_example; //An example string from which to calculate the width.

  Glib::ustring m_choices_related_field, m_choices_related_field_second;
};

} //namespace Glom

#endif //GLOM_DATASTRUCTURE_FIELDFORMATTING_H



