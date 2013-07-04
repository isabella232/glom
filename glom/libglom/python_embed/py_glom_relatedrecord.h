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

#ifndef GLOM_PYTHON_GLOM_RELATEDRECORD_H
#define GLOM_PYTHON_GLOM_RELATEDRECORD_H

#include <boost/python.hpp>

#include <libglom/document/document.h>
#include <libglom/data_structure/field.h>
#include <glibmm/ustring.h>

namespace Glom
{

class PyGlomRecord;

class PyGlomRelatedRecord
{
public:
  PyGlomRelatedRecord();
  ~PyGlomRelatedRecord();

  void set_relationship(const std::shared_ptr<const Relationship>& relationship, const Gnome::Gda::Value& from_key_value, const Document* document);

  boost::python::object sum(const std::string& field_name) const;
  boost::python::object count(const std::string& field_name) const;
  boost::python::object min(const std::string& field_name) const;
  boost::python::object max(const std::string& field_name) const;

  //Available, for instance, in python via record["name_first"]
  typedef std::map<Glib::ustring, Gnome::Gda::Value> type_map_field_values;

  //[] notation:
  type_map_field_values::size_type len() const;
  boost::python::object getitem(const boost::python::object& item);

private:

  boost::python::object generic_aggregate(const std::string& field_name, const std::string& aggregate) const;

  //PyObject* m_fields_dict; //Dictionary (map) of field names (string) to field values (Gnome::Gda::Value).
  //PyGlomRecord* m_record_parent;
  const Document* m_document;

  std::shared_ptr<const Relationship> m_relationship;
  Gnome::Gda::Value m_from_key_value;

  mutable type_map_field_values m_map_field_values; //A cache.
};

} //namespace Glom


#endif //GLOM_PYTHON_GLOM_RELATEDRECORD_H
