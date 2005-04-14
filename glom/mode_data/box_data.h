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

#ifndef BOX_DATA_H
#define BOX_DATA_H

#include "../box_db_table.h"
#include "dialog_layout.h"

class Box_Data : public Box_DB_Table
{
public: 
  Box_Data();
  virtual ~Box_Data();

  virtual void init_db_details(const Glib::ustring& strTableName, const Glib::ustring& strWhereClause = Glib::ustring());
  virtual void refresh_db_details(const Glib::ustring& strWhereClause = Glib::ustring());

  virtual void print_layout(); //A test, for now.

  typedef std::vector<LayoutItem_Field> type_vecLayoutFields;

  virtual Glib::ustring get_WhereClause() const;

  virtual void set_unstored_data(bool bVal);
  virtual bool get_unstored_data() const;

  virtual bool confirm_discard_unstored_data() const;

  virtual void show_layout_dialog();

  //Signals:

  //signal_find: Used by _Find sub-classes.
  //Should be a MI class, derived by those sub-classes. TODO.
  //where_clause.
  sigc::signal<void, Glib::ustring> signal_find;

  //g++ 3.4 needs this to be public when used from Box_Data_Details. I'm not sure why. murrayc.
  virtual void on_dialog_layout_hide();

protected:
  virtual void fill_from_database(); //override.

  virtual type_vecLayoutFields get_fields_to_show() const;
  virtual Glib::ustring build_sql_select(const Glib::ustring& table_name, const type_vecLayoutFields& fieldsToGet, const Field& primary_key_field, const Gnome::Gda::Value& primary_key_value);
  //virtual Glib::ustring build_sql_select_with_where_clause(const Glib::ustring& table_name, const type_vecLayoutFields& fieldsToGet, const Glib::ustring& where_clause);
  virtual bool get_related_record_exists(const Relationship& relationship, const Field& key_field, const Gnome::Gda::Value& key_value);
  virtual bool add_related_record_for_field(const LayoutItem_Field& layout_item_parent, const Relationship& relationship, const Field& primary_key_field, const Gnome::Gda::Value& primary_key_value_provided);

  type_vecLayoutFields get_table_fields_to_show(const Glib::ustring& table_name) const;
  type_vecLayoutFields get_table_fields_to_show(const Glib::ustring& table_name, const Document_Glom::type_mapLayoutGroupSequence& mapGroupSequence) const;
  void get_table_fields_to_show_add_group(const Glib::ustring& table_name, const Privileges& table_privs, const type_vecFields& all_db_fields, const LayoutGroup& group, Box_Data::type_vecLayoutFields& vecFields) const;

  /** Get the layout groups, with the Field information filled in.
   */
  Document_Glom::type_mapLayoutGroupSequence get_data_layout_groups(const Glib::ustring& layout);
  void fill_layout_group_field_info(LayoutGroup& group, const Privileges& table_privs);

  typedef std::pair<LayoutItem_Field, Relationship> type_pairFieldTrigger;
  typedef std::list<type_pairFieldTrigger> type_list_lookups;

  /** Get the fields whose values should be looked up when @a field_name changes, with
   * the relationship used to lookup the value.
   */
  type_list_lookups get_lookup_fields(const Glib::ustring& field_name) const;

  /** Get the fields whose values should be recalculated when @a field_name changes.
   */
  type_vecLayoutFields get_calculated_fields(const Glib::ustring& field_name) const;

  /** Get the fields that are in related tables, via a relationship using @a field_name changes.
  */
  type_vecLayoutFields get_related_fields(const Glib::ustring& field_name) const;

  /** Get the value of the @a source_field from the @a relationship, using the @a key_value.
   */
  Gnome::Gda::Value get_lookup_value(const Relationship& relationship, const Field& source_field, const Gnome::Gda::Value & key_value);

  virtual void do_calculations(const LayoutItem_Field& field_changed, const Field& primary_key, const Gnome::Gda::Value& primary_key_value);
  void set_field_value_in_database(const LayoutItem_Field& field_layout, const Gnome::Gda::Value& field_value, const Field& primary_key, const Gnome::Gda::Value& primary_key_value);

  virtual bool record_delete(const Gnome::Gda::Value& primary_key_value);
  virtual Glib::RefPtr<Gnome::Gda::DataModel> record_new(bool use_entered_data = true, const Gnome::Gda::Value& primary_key_value = Gnome::Gda::Value()); //New record with all entered field values.
  Gnome::Gda::Value generate_next_auto_increment(const Glib::ustring& table_name, const Glib::ustring field_name);

  virtual bool get_field_primary_key(Field& field) const = 0;
  virtual Gnome::Gda::Value get_primary_key_value_selected() = 0;
  virtual bool get_field(const Glib::ustring& name, Field& field) const;

  ///Get the table name. It's either the current table or the relationship's to_table:
  Glib::ustring get_layout_item_table_name(const LayoutItem_Field& layout_item, const Glib::ustring table_name);

  //Signal handlers:
  virtual void on_Button_Find(); //only used by _Find sub-classes. Should be MI.

  static bool get_field_primary_key_index(const type_vecFields& fields, guint& field_column);
  static bool get_field_primary_key_index(const type_vecLayoutFields& fields, guint& field_column);

  typedef std::map<Glib::ustring, Gnome::Gda::Value> type_map_fields;
  //TODO: Performance: This is massively inefficient:
  type_map_fields get_record_field_values(const Gnome::Gda::Value& primary_key_value);


  static Glib::ustring xslt_process(const xmlpp::Document& xml_document, const std::string& filepath_xslt);

  Gtk::Button m_Button_Find; //only used by _Find sub-classes. Should be MI.
  Gtk::Label m_Label_FindStatus;

  bool m_bUnstoredData;

  Dialog_Layout* m_pDialogLayout;
  Glib::ustring m_layout_name;

  Glib::ustring m_strWhereClause;

  type_vecLayoutFields m_Fields;
};

#endif
