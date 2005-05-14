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
 
#ifndef GLOM_DATASTRUCTURE_LAYOUTITEM_FIELD_H
#define GLOM_DATASTRUCTURE_LAYOUTITEM_FIELD_H

#include "layoutitem.h"
#include "../field.h"
#include "../numeric_format.h"
#include "../relationship.h"

class LayoutItem_Field : public LayoutItem
{
public:

  LayoutItem_Field();
  LayoutItem_Field(const LayoutItem_Field& src);
  LayoutItem_Field& operator=(const LayoutItem_Field& src);
  virtual ~LayoutItem_Field();

  virtual LayoutItem* clone() const;

  bool operator==(const LayoutItem_Field& src) const;

  virtual void set_name(const Glib::ustring& name);
  virtual Glib::ustring get_name() const; //For use with our std::find_if() predicate.

  virtual Glib::ustring get_title_or_name() const;

  //virtual Glib::ustring get_table_name() const;
  //virtual void set_table_name(const Glib::ustring& table_name);

  /** Get a text representation for the field, such as Relationship::FieldName.
   */
  virtual Glib::ustring get_layout_display_name() const;

  virtual bool get_has_relationship_name() const;
  virtual Glib::ustring get_relationship_name() const;
  //virtual void set_relationship_name(const Glib::ustring& relationship_name);

  bool get_has_choices() const;

  bool get_has_related_choices() const;
  void set_has_related_choices(bool val = true);

  bool get_has_custom_choices() const;
  void set_has_custom_choices(bool val = true);

  typedef std::list<Gnome::Gda::Value> type_list_values;
  virtual type_list_values get_choices_custom() const;
  virtual void set_choices_custom(const type_list_values& choices);

  bool get_choices_restricted() const;
  void set_choices_restricted(bool val = true);

  void get_choices(Glib::ustring& relationship_name, Glib::ustring& field, Glib::ustring& field_second) const;
  void set_choices(const Glib::ustring& relationship_name, const Glib::ustring& field, const Glib::ustring& field_second);

  virtual Glib::ustring get_part_type_name() const;

  //This is filled in by looking at the database structure:
  Field m_field;
  //TODO: This might occasionally be different on different layouts: Glib::ustring m_title;

  NumericFormat m_numeric_format; //Only used for numeric fields.

  bool get_text_format_multiline() const;
  void set_text_format_multiline(bool value = true);

  bool get_editable_and_allowed() const;

  /// For extra fields, needed for SQL queries. The user should never be able to made an item hidden - he can just remove it.
  bool get_hidden() const;
  void set_hidden(bool val = true);

  //Not saved to the document:
  bool m_priv_view;
  bool m_priv_edit;

  //TODO_Performance: This is just cached data, so we don't need to always lookup the relationship details from the document, from the name.
  //Mayeb use a smartpointer?
  Relationship m_relationship;
  Relationship m_choices_related_relationship;

protected:
  //Glib::ustring m_relationship_name; //bool m_related;
  bool m_hidden;

  type_list_values m_choices_custom_list; //A drop-down list of possible values for the field.
  bool m_choices_restricted;
  bool m_choices_custom, m_choices_related;

  bool m_text_format_multiline;

  Glib::ustring m_choices_related_field, m_choices_related_field_second;
};

#endif //GLOM_DATASTRUCTURE_LAYOUTITEM_FIELD_H



