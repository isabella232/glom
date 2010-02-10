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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
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

  boost::python::object sum(const std::string& field_name) const;
  boost::python::object count(const std::string& field_name) const;
  boost::python::object min(const std::string& field_name) const;
  boost::python::object max(const std::string& field_name) const;

  //[] notation:
  long len() const;
  boost::python::object getitem(boost::python::object item);

//TODO: protected:

  boost::python::object generic_aggregate(const std::string& field_name, const std::string& aggregate) const;

  //PyObject* m_fields_dict; //Dictionary (map) of field names (string) to field values (Gnome::Gda::Value).
  //PyGlomRecord* m_record_parent;
  Document* m_document;

  sharedptr<const Relationship> m_relationship;
  Glib::ustring m_from_key_value_sqlized;

  //Available, for instance, in python via record["name_first"]
  typedef std::map<Glib::ustring, Gnome::Gda::Value> type_map_field_values;
  mutable type_map_field_values m_map_field_values; //A cache.
};

void PyGlomRelatedRecord_SetRelationship(PyGlomRelatedRecord* self, const sharedptr<const Relationship>& relationship, const Glib::ustring& from_key_value_sqlized, Document* document);

/*
void PyGlomRelatedRecord_SetConnection(PyGlomRelatedRecord* self, const Glib::RefPtr<Gnome::Gda::Connection>& connection);
*/

} //namespace Glom


#endif //GLOM_PYTHON_GLOM_RELATEDRECORD_H
