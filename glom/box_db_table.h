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


#ifndef BOX_DB_TABLE_H
#define BOX_DB_TABLE_H

#include "box_db.h"
#include "data_structure/field.h"
#include <algorithm> //find_if used in various places.


class Box_DB_Table : public Box_DB
{
public: 
  Box_DB_Table();
  Box_DB_Table(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Box_DB_Table();
  
  virtual void initialize(const Glib::ustring& strDatabaseName, const Glib::ustring& strTableName, const Glib::ustring& strWhereClause = "");
  
  virtual Glib::ustring get_TableName();

  //TODO: Put this somewhere more sensible:
  typedef std::map<Gnome::Gda::ValueType, Glib::ustring> type_map_valuetypes;
  static type_map_valuetypes get_field_type_names();

  static type_vecFields get_fields_for_table_from_database(const Glib::ustring& table_name);

  /** Gets the fields from the database, and adds any missing information by looking in the document.
  */
  type_vecFields get_fields_for_table(const Glib::ustring& table_name) const;

  Field get_fields_for_table_one_field(const Glib::ustring& table_name, const Glib::ustring& field_name) const;      

protected:
  virtual void fill_fields();

  virtual Glib::ustring get_PrimaryKey_Name();

  virtual bool record_delete(const Glib::ustring& strPrimaryKeyValue);
  virtual Glib::RefPtr<Gnome::Gda::DataModel> record_new(Glib::ustring strPrimaryKeyValue = "");

  virtual guint get_Entered_Field_count() const;
  virtual Field get_Entered_Field(guint index) const;


  
  bool get_field(const Glib::ustring& name, Field& field) const;
  bool get_field_index(const Glib::ustring& name, guint& field_index) const; //It's almost certainly a bad idea to use this.
  bool get_field_primary_key(guint& field_column) const;
  static bool get_field_primary_key(const type_vecFields& fields, guint& field_column);
  unsigned long get_last_auto_increment_value(const Glib::RefPtr<Gnome::Gda::DataModel>& data_model, const Glib::ustring field_name);



  static type_vecFields get_fields_for_datamodel(const Glib::RefPtr<Gnome::Gda::DataModel>& data_model);
  
  static Glib::ustring postgres_get_field_definition_for_sql(const Gnome::Gda::FieldAttributes& field_info);

  Glib::ustring m_strTableName;
  Glib::ustring m_strWhereClause;

  type_vecFields m_Fields;



};

#endif //BOX_DB_TABLE_H
