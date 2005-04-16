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

#define NO_IMPORT_PYGTK //To avoid a multiple definition in pygtk.
#include <pygtk/pygtk.h> //For the PyGObject and PyGBoxed struct definitions.

#include <Python.h>
#include "../../document/document_glom.h"
#include "../../data_structure/field.h"
#include <glibmm/ustring.h>

class PyGlomRelated;

struct PyGlomRecord
{
public:
  PyObject_HEAD

  //PyObject* m_fields_dict; //Dictionary (map) of field names (string) to field values (Gnome::Gda::Value).
  //PyGObject* m_py_gda_connection; //"derived" from PyObject.
  Document_Glom* m_document;
  Glib::ustring* m_table_name;

  PyGlomRelated* m_related;

  //Available, for instance, in python via record["name_first"]
  typedef std::map<Glib::ustring, Gnome::Gda::Value> type_map_field_values;
  //We use a pointer because python will not run the class/struct's default constructor.
  type_map_field_values* m_pMap_field_values;
};

PyTypeObject* PyGlomRecord_GetPyType();

void PyGlomRecord_SetFields(PyGlomRecord* self, const PyGlomRecord::type_map_field_values& field_values, Document_Glom* document, const Glib::ustring& table_name);


#endif //GLOM_PYTHON_GLOM_RECORD_H
