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
    TYPE_TIME
  };
  
  Field();
  Field(const Field& src);
  virtual ~Field();

  Field& operator=(const Field& src);

  bool operator==(const Field& src) const;
  bool operator!=(const Field& src) const;

  /// This forwards to the Gnome::Gda::FieldAttributes::get_name, so that we can use it in the same predicate template.
  virtual Glib::ustring get_name() const;

   /// This forwards to the Gnome::Gda::FieldAttributes::set_name, for convenience
  virtual void set_name(const Glib::ustring& value);

  virtual glom_field_type get_glom_type() const;
  virtual void set_glom_type(glom_field_type fieldtype);

  virtual Gnome::Gda::FieldAttributes get_field_info() const;
  virtual void set_field_info(const Gnome::Gda::FieldAttributes& fieldInfo);

  /// Ignores any part of FieldAttributes that libgda does not properly fill.
  virtual bool field_info_from_database_is_equal(const Gnome::Gda::FieldAttributes& field);

  virtual Glib::ustring get_data() const;
  virtual void set_data(const Glib::ustring& strData);

  virtual Glib::ustring get_title() const;
  virtual void set_title(const Glib::ustring& strTitle);

  virtual Glib::ustring get_title_or_name() const; //Title, or Name if there is no title.

  //Lookup stuff:
  virtual bool get_is_lookup() const;

  virtual Glib::ustring get_lookup_relationship() const;
  virtual void set_lookup_relationship(const Glib::ustring& strRelationship);

  virtual Glib::ustring get_lookup_field() const;
  virtual void set_lookup_field(const Glib::ustring& strField);

  Glib::ustring get_sql_type() const;

  Glib::ustring get_default_value_as_string() const;

  /** Escape the string so that it can be used in a SQL command.
   */
 Glib::ustring sql(const Glib::ustring& str) const;

  /** Escape the value so that it can be used in a SQL command.
   */
  static Glib::ustring sql(const Gnome::Gda::Value& value);

  /** Get text to show to the user.
   */
  Glib::ustring value_to_string(const Gnome::Gda::Value& value) const;


  typedef std::map<glom_field_type, Glib::ustring> type_map_type_names;
  static type_map_type_names get_type_names();
  static type_map_type_names get_usable_type_names();

  static Glib::ustring get_type_name(glom_field_type glom_type);
  static glom_field_type get_type_for_name(const Glib::ustring& glom_type);

  static glom_field_type get_glom_type_for_gda_type(Gnome::Gda::ValueType gda_type);
  static Gnome::Gda::ValueType get_gda_type_for_glom_type(Field::glom_field_type glom_type);
      
protected:

  static void init_map();

  //The glom type to be used for the gda type:
  typedef std::map<Gnome::Gda::ValueType, glom_field_type> type_map_gda_type_to_glom_type;
  static type_map_gda_type_to_glom_type m_map_gda_type_to_glom_type;

  //The gda type to be used for the glom type:
  typedef std::map<glom_field_type, Gnome::Gda::ValueType> type_map_glom_type_to_gda_type;
  static type_map_glom_type_to_gda_type m_map_glom_type_to_gda_type;

  static type_map_type_names m_map_type_names;
  static bool m_maps_inited;
  
  glom_field_type m_glom_type;
  Gnome::Gda::FieldAttributes m_field_info;

  Glib::ustring m_strData;
  Glib::ustring m_strTitle;
  Glib::ustring m_strLookupRelationship, m_strLookupField;
};

#endif //GLOM_DATASTRUCTURE_FIELD_H

