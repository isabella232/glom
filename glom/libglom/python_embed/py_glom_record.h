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

#ifndef GLOM_PYTHON_GLOM_RECORD_H
#define GLOM_PYTHON_GLOM_RECORD_H

#define NO_IMPORT_PYGOBJECT //To avoid a multiple definition in pygtk.
#include <pygobject.h> //For the PyGObject and PyGBoxed struct definitions.

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

  std::string get_table_name() const;

  //TODO: Use a more specific type somehow?
  boost::python::object get_connection();

  boost::python::object get_related();

  //[] notation:
  long len() const;
  boost::python::object getitem(boost::python::object item);

public:
  //PyObject* m_fields_dict; //Dictionary (map) of field names (string) to field values (Gnome::Gda::Value).
  //PyGObject* m_py_gda_connection; //"derived" from PyObject.
  Document* m_document;
  Glib::ustring m_table_name;

  boost::python::object m_related; //Actually a PyGlomRelated

  //Available, for instance, in python via record["name_first"]
  typedef std::map<Glib::ustring, Gnome::Gda::Value> type_map_field_values;
  type_map_field_values m_map_field_values;

  Glib::RefPtr<Gnome::Gda::Connection> m_connection;
};

void PyGlomRecord_SetFields(PyGlomRecord* self, const PyGlomRecord::type_map_field_values& field_values, Document* document, const Glib::ustring& table_name, const Glib::RefPtr<Gnome::Gda::Connection>& opened_connection);

} //namespace Glom

#endif //GLOM_PYTHON_GLOM_RECORD_H
