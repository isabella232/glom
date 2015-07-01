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

#ifndef GLOM_DATASTRUCTURE_FIELD_H
#define GLOM_DATASTRUCTURE_FIELD_H

#include <libgdamm/column.h>
#include <libgdamm/holder.h>
#include <libgdamm/connection.h>
#include <libglom/data_structure/translatable_item.h>
#include <libglom/data_structure/layout/formatting.h>
#include <libglom/sharedptr.h>

//Predicate, for use with std::find_if():

namespace Glom
{

/** A predicate for use with std::find_if() to find a Field or LayoutItem which refers 
 * to the same field, looking at just the name.
 */
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

  bool operator() (const std::shared_ptr<T_Element>& element)
  {
    return (element->get_name() == m_strName);
  }

  bool operator() (const std::shared_ptr<const T_Element>& element)
  {
    return (element->get_name() == m_strName);
  }

private:
  Glib::ustring m_strName;
};


//Field info, such as Name, Title, definitions, and, sometimes, contents.
class Field : public TranslatableItem
{
public:
  /* Possible formats when converting from/to SQL representation.
   * TODO: Maybe we should move the code that does the conversion between gda
   * type and SQL into the connectionpool backends. */
  enum sql_format
  {
    SQL_FORMAT_POSTGRES,
    SQL_FORMAT_SQLITE
  };

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
  Field(Field&& src) = delete;
  ~Field();

  Field& operator=(const Field& src);
  Field& operator=(Field&& src) = delete;

  bool operator==(const Field& src) const;
  bool operator!=(const Field& src) const;

  Field* clone() const;

  glom_field_type get_glom_type() const;
  void set_glom_type(glom_field_type fieldtype);


  /// This forwards to the Glib::RefPtr<Gnome::Gda::Column>::get_name, so that we can use it in the same predicate template.
  virtual Glib::ustring get_name() const;

  /// This forwards to the Glib::RefPtr<Gnome::Gda::Column>::set_name, for convenience
  virtual void set_name(const Glib::ustring& value);

  /// This forwards to the Glib::RefPtr<Gnome::Gda::Column>::get_auto_increment.
  bool get_auto_increment() const;

  /// This forwards to the Glib::RefPtr<Gnome::Gda::Column>::set_auto_increment.
  void set_auto_increment(bool val = true);

  /// This forwards to the Glib::RefPtr<Gnome::Gda::Column>::get_primary_key.
  bool get_primary_key() const;

  /// This forwards to the Glib::RefPtr<Gnome::Gda::Column>::set_primary_key.
  void set_primary_key(bool val = true);

  /// This forwards to the Glib::RefPtr<Gnome::Gda::Column>::get_unique_key.
  bool get_unique_key() const;

  /// This forwards to the Glib::RefPtr<Gnome::Gda::Column>::set_unique_key.
  void set_unique_key(bool val = true);

  /// This forwards to the Glib::RefPtr<Gnome::Gda::Column>::get_default_value.
  Gnome::Gda::Value get_default_value() const;

  /// This forwards to the Glib::RefPtr<Gnome::Gda::Column>::set_default_value.
  void set_default_value(const Gnome::Gda::Value& value);


  //TODO_Performance: Lots of code calls this just to call one of its methods:
  Glib::RefPtr<Gnome::Gda::Column> get_field_info();
  Glib::RefPtr<const Gnome::Gda::Column> get_field_info() const;
  void set_field_info(const Glib::RefPtr<Gnome::Gda::Column>& fieldInfo);

  /// Ignores any part of FieldAttributes that libgda does not properly fill.
  bool field_info_from_database_is_equal(const Glib::RefPtr<const Gnome::Gda::Column>& field);

  //Lookup stuff:
  bool get_is_lookup() const;

  std::shared_ptr<Relationship> get_lookup_relationship() const;
  void set_lookup_relationship(const std::shared_ptr<Relationship>& strRelationship);

  Glib::ustring get_lookup_field() const;
  void set_lookup_field(const Glib::ustring& strField);

  Glib::ustring get_sql_type() const;
  Glib::ustring get_gda_type_name() const;

  /** Escape and quote the value so that it can be used in a SQL command.
   */
  Glib::ustring sql(const Gnome::Gda::Value& value, const Glib::RefPtr<Gnome::Gda::Connection>& connection) const;

  /** Get the canonical format for a file, for instance for 
   * a default value or for example data.
   * This does not add quotes for text fields so the caller may need to do that.
   * Note that this does not do any extra escaping such as an XML file might need.
   * However, this will escape data as per the CSV RFC.
   */
  Glib::ustring to_file_format(const Gnome::Gda::Value& value) const;

  /** See to_file_format(const Gnome::Gda::Value& value).
   */
  static Glib::ustring to_file_format(const Gnome::Gda::Value& value, glom_field_type glom_type);

  /** Parse the value from the canonical file format. See to_file_format()
   * This does note remove quotes from text values so the caller may need to do that.
   */
  Gnome::Gda::Value from_file_format(const Glib::ustring& str, bool& success) const;

  static Gnome::Gda::Value from_file_format(const Glib::ustring& str, glom_field_type glom_type, bool& success, bool old_image_format = false);

  /** Escape the value so that it can be used in a SQL command for a find.
   */
  Glib::ustring sql_find(const Gnome::Gda::Value& value, const Glib::RefPtr<Gnome::Gda::Connection>& connection) const;

  /** Get a suitable operator to use when finding records.
   * For instance, == for numbers, or LIKE for text.
   */
  Gnome::Gda::SqlOperatorType sql_find_operator() const;

  Glib::ustring get_calculation() const;
  void set_calculation(const Glib::ustring& calculation);
  bool get_has_calculation() const;

  /* Discover what fields are used in the calculation,
   * so the value can be recalculated when their values change.
   */
  typedef std::vector<Glib::ustring> type_list_strings;
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

  static glom_field_type get_glom_type_for_gda_type(GType gda_type);
  static GType get_gda_type_for_glom_type(Field::glom_field_type glom_type);

  static bool get_conversion_possible(glom_field_type field_type_src, glom_field_type field_type_dest);

  Formatting m_default_formatting;

private:

  static void init_map();

  GType get_gda_data_type_with_fallback(const Gnome::Gda::Value& value);

  //The glom type to be used for the gda type:
  typedef std::map<GType, glom_field_type> type_map_gda_type_to_glom_type;
  static type_map_gda_type_to_glom_type m_map_gda_type_to_glom_type;

  //The gda type to be used for the glom type:
  typedef std::map<glom_field_type, GType> type_map_glom_type_to_gda_type;
  static type_map_glom_type_to_gda_type m_map_glom_type_to_gda_type;

  typedef std::vector<glom_field_type> type_list_conversion_targets;
  typedef std::map<glom_field_type, type_list_conversion_targets> type_map_conversions;

  static type_map_type_names m_map_type_names; //These are canonical, for internal use.
  static type_map_type_names m_map_type_names_ui; //These are translated.
  static type_map_conversions m_map_conversions; //Map of types to list of possibnle conversion targets types.
  static bool m_maps_inited;

  glom_field_type m_glom_type;
  Glib::RefPtr<Gnome::Gda::Column> m_field_info;

  std::shared_ptr<Relationship> m_lookup_relationship;
  Glib::ustring m_strLookupField;
  Glib::ustring m_calculation;
  bool m_visible; //Whether it will be shown to the user.

  //Things that libgda cannot easily tell us:
  bool m_primary_key;
  bool m_unique_key;
};

} //namespace Glom

#endif //GLOM_DATASTRUCTURE_FIELD_H

