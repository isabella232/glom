/* Glom
 *
 * Copyright (C) 2001-2005 Murray Cumming
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

#ifndef GLOM_PYTHON_GLOM_RECORD_H
#define GLOM_PYTHON_GLOM_RECORD_H

#include <boost/python.hpp>

#include <libglom/document/document.h>
#include <libglom/data_structure/field.h>
#include <glibmm/ustring.h>

namespace Glom
{

class PyGlomRelated;

class PyGlomRecord
{
public:
  PyGlomRecord();
  ~PyGlomRecord();

  /* Prevent python code from changing data in the database via this object.
   * For instance, this should be used in a field calculation,
   * though changing data would be OK from a script.
   * This is not exposed via Python.
   */
  void set_read_only();

  std::string get_table_name() const;

  //TODO: Use a more specific type somehow?
  boost::python::object get_connection();

  boost::python::object get_related();

  //Available, for instance, in python via record["name_first"]
  typedef std::map<Glib::ustring, Gnome::Gda::Value> type_map_field_values;

  //[] notation:
  type_map_field_values::size_type len() const;
  boost::python::object getitem(const boost::python::object& item);
  void setitem(const boost::python::object& /* key */, const boost::python::object& /* value */);

  void set_fields(const PyGlomRecord::type_map_field_values& field_values,
    const std::shared_ptr<const Document>& document,
    const Glib::ustring& table_name,
    const std::shared_ptr<const Field>& key_field,
    const Gnome::Gda::Value& key_field_value,
    const Glib::RefPtr<Gnome::Gda::Connection>& opened_connection);

public:
  std::shared_ptr<const Document> m_document;
  Glib::ustring m_table_name;
  type_map_field_values m_map_field_values;
private:
  std::shared_ptr<const Field> m_key_field;
  Gnome::Gda::Value m_key_field_value;

  boost::python::object m_related; //Actually a PyGlomRelated

  Glib::RefPtr<Gnome::Gda::Connection> m_connection;

private:
  bool m_read_only;
};



} //namespace Glom

#endif //GLOM_PYTHON_GLOM_RECORD_H
