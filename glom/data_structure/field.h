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

#ifndef GLOM_DATASTRUCTURE_FIELD_H
#define GLOM_DATASTRUCTURE_FIELD_H

#include <libgdamm.h>
#include "layout/fieldformatting.h"
#include "../sharedptr.h"

//Predicate, for use with std::find_if():

template<class T_Element>
class predicate_FieldHasName
{
public:
  predicate_FieldHasName(const Glib::ustring& strName)
  {
    m_strName = strName;
  }

  virtual ~predicate_FieldHasName()
  {
  }

  bool operator() (const T_Element& element)
  {
    return (element.get_name() == m_strName);
  }

  bool operator() (const sharedptr<T_Element>& element)
  {
    return (element->get_name() == m_strName);
  }

protected:
  Glib::ustring m_strName;
};


//Field info, such as Name, Title, definitions, and, sometimes, contents.
class Field
{
public:
  enum glom_field_type
  {
    TYPE_INVALID,
    TYPE_NUMERIC,
    TYPE_TEXT,
    TYPE_DATE,
    TYPE_TIME,
    TYPE_BOOLEAN,
    TYPE_IMAGE //Always stored as a standard format.
  };

  Field();
  Field(const Field& src);
  ~Field();

  Field& operator=(const Field& src);

  bool operator==(const Field& src) const;
  bool operator!=(const Field& src) const;

  glom_field_type get_glom_type() const;
  void set_glom_type(glom_field_type fieldtype);


  /// This forwards to the Gnome::Gda::FieldAttributes::get_name, so that we can use it in the same predicate template.
  Glib::ustring get_name() const;

  /// This forwards to the Gnome::Gda::FieldAttributes::set_name, for convenience
  void set_name(const Glib::ustring& value);

  /// This forwards to the Gnome::Gda::FieldAttributes::get_auto_increment.
  bool get_auto_increment() const;

  /// This forwards to the Gnome::Gda::FieldAttributes::set_auto_increment.
  void set_auto_increment(bool val = true);

  /// This forwards to the Gnome::Gda::FieldAttributes::get_primary_key.
  bool get_primary_key() const;

  /// This forwards to the Gnome::Gda::FieldAttributes::set_primary_key.
  void set_primary_key(bool val = true);

  /// This forwards to the Gnome::Gda::FieldAttributes::get_unique_key.
  bool get_unique_key() const;

  /// This forwards to the Gnome::Gda::FieldAttributes::set_unique_key.
  void set_unique_key(bool val = true);

  /// This forwards to the Gnome::Gda::FieldAttributes::get_default_value.
  Gnome::Gda::Value get_default_value() const;

  /// This forwards to the Gnome::Gda::FieldAttributes::set_default_value.
  void set_default_value(const Gnome::Gda::Value& val);


  //TODO_Performance: Lots of code calls this just to call one of its methods:
  Gnome::Gda::FieldAttributes get_field_info() const;
  void set_field_info(const Gnome::Gda::FieldAttributes& fieldInfo);

  /// Ignores any part of FieldAttributes that libgda does not properly fill.
  bool field_info_from_database_is_equal(const Gnome::Gda::FieldAttributes& field);

  //These are not used much:
  Gnome::Gda::Value get_data() const;
  void set_data(const Gnome::Gda::Value& value);

  Glib::ustring get_title() const;
  void set_title(const Glib::ustring& strTitle);

  Glib::ustring get_title_or_name() const; //Title, or Name if there is no title.

  //Lookup stuff:
  bool get_is_lookup() const;

  Glib::ustring get_lookup_relationship() const;
  void set_lookup_relationship(const Glib::ustring& strRelationship);

  Glib::ustring get_lookup_field() const;
  void set_lookup_field(const Glib::ustring& strField);

  Glib::ustring get_sql_type() const;

  /** Escape the string so that it can be used in a SQL command.
   */
  //Glib::ustring sql(const Glib::ustring& str) const;

  /** Escape the value so that it can be used in a SQL command.
   */
  Glib::ustring sql(const Gnome::Gda::Value& value) const;
  
  /** Escape the value so that it can be used in a SQL command for a find.
   */
  Glib::ustring sql_find(const Gnome::Gda::Value& value) const;

  /** Get a suitable operator to use when finding records.
   * For instance, == for numbers, or LIKE for text.
   */
  Glib::ustring sql_find_operator() const;

  Glib::ustring get_calculation() const;
  void set_calculation(const Glib::ustring& calculation);
  bool get_has_calculation() const;

  /* Discover what fields are used in the calculation,
   * so the value can be recalculated when their values change.
   */
  typedef std::list<Glib::ustring> type_list_strings;
  type_list_strings get_calculation_fields() const;

  type_list_strings get_calculation_relationships() const;

  void set_visible(bool val = true);
  bool get_visible() const;

  typedef std::map<glom_field_type, Glib::ustring> type_map_type_names;

  /// Get canonical type names for internal use, such as in the XML of the document.
  static type_map_type_names get_type_names();

  /// Get translated type names.
  static type_map_type_names get_type_names_ui();

  /// Get translated type names of types that should be offered to the user.
  static type_map_type_names get_usable_type_names();

  /// Get the translated name for a glom type.
  static Glib::ustring get_type_name_ui(glom_field_type glom_type);

  /// Get the type from a translated name.
  static glom_field_type get_type_for_ui_name(const Glib::ustring& glom_type);

  static glom_field_type get_glom_type_for_gda_type(Gnome::Gda::ValueType gda_type);
  static Gnome::Gda::ValueType get_gda_type_for_glom_type(Field::glom_field_type glom_type);
  
  static bool get_conversion_possible(glom_field_type field_type_src, glom_field_type field_type_dest);

  FieldFormatting m_default_formatting;

protected:

  static void init_map();

  //The glom type to be used for the gda type:
  typedef std::map<Gnome::Gda::ValueType, glom_field_type> type_map_gda_type_to_glom_type;
  static type_map_gda_type_to_glom_type m_map_gda_type_to_glom_type;

  //The gda type to be used for the glom type:
  typedef std::map<glom_field_type, Gnome::Gda::ValueType> type_map_glom_type_to_gda_type;
  static type_map_glom_type_to_gda_type m_map_glom_type_to_gda_type;
  
  typedef std::list<glom_field_type> type_list_conversion_targets;
  typedef std::map<glom_field_type, type_list_conversion_targets> type_map_conversions;
 
  static type_map_type_names m_map_type_names; //These are canonical, for internal use.
  static type_map_type_names m_map_type_names_ui; //These are translated.
  static type_map_conversions m_map_conversions; //Map of types to list of possibnle conversion targets types.
  static bool m_maps_inited;

  glom_field_type m_glom_type;
  Gnome::Gda::FieldAttributes m_field_info;

  Gnome::Gda::Value m_data; //Not used much.
  Glib::ustring m_strTitle;
  Glib::ustring m_strLookupRelationship, m_strLookupField;
  Glib::ustring m_calculation;
  bool m_visible; //Whether it will be shown to the user.
};

#endif //GLOM_DATASTRUCTURE_FIELD_H

