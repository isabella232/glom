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
#include <glom/libglom/data_structure/field.h>
#include <algorithm> //find_if used in various places.

namespace Glom
{

class Box_DB_Table : public Box_DB
{
public: 
  Box_DB_Table();
  Box_DB_Table(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Box_DB_Table();

  virtual bool init_db_details(const Glib::ustring& table_name);
  virtual bool refresh_data_from_database();

  virtual Glib::ustring get_table_name();

  //TODO: Put this somewhere more sensible:
  typedef std::map<Gnome::Gda::ValueType, Glib::ustring> type_map_valuetypes;

protected:

  //virtual Glib::RefPtr<Gnome::Gda::DataModel> record_new(Gnome::Gda::Value primary_key_value);

  Gnome::Gda::Value get_entered_field_data_field_only(const sharedptr<const Field>& field) const;
  virtual Gnome::Gda::Value get_entered_field_data(const sharedptr<const LayoutItem_Field>& field) const;

  //static sharedptr<Field> get_field_primary_key(const type_vecFields& fields);

  unsigned long get_last_auto_increment_value(const Glib::RefPtr<Gnome::Gda::DataModel>& data_model, const Glib::ustring& field_name);


  //static type_vecFields get_fields_for_datamodel(const Glib::RefPtr<Gnome::Gda::DataModel>& data_model); 
  static Glib::ustring postgres_get_field_definition_for_sql(const Gnome::Gda::FieldAttributes& field_info);

  Glib::ustring m_table_name;
};

} //namespace Glom

#endif //BOX_DB_TABLE_H
