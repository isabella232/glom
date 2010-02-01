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

//We need to include this before anything else, to avoid redefinitions:
#include <Python.h>
#include <compile.h> /* for the PyCodeObject */
#include <eval.h> /* for PyEval_EvalCode */
#include <objimpl.h> /* for PyObject_New() */

#include <libglom/python_embed/py_glom_record.h>
#include <libglom/python_embed/py_glom_related.h>
#include <libglom/python_embed/pygdavalue_conversions.h> //For pygda_value_as_pyobject().

#include <libglom/data_structure/field.h>
#include <glibmm/ustring.h>

namespace Glom
{

//Set the object's member data, from the parameters supplied when creating the object:
PyGlomRecord::PyGlomRecord()
{
  PyGlomRecord *self_record = this;

  //static char *kwlist[] = {"test", 0};

  //if(!PyArg_ParseTupleAndKeywords(args, kwds, "|i", kwlist,
   //                                 &self->m_test))
   // return -1;

  if(self_record)
  {
    self_record->m_related = 0;
    self_record->m_pMap_field_values = new PyGlomRecord::type_map_field_values();
  }
}

PyGlomRecord::~PyGlomRecord()
{
  PyGlomRecord *self_record = this;

  if(self_record->m_pMap_field_values)
  {
    delete self_record->m_pMap_field_values;
    self_record->m_pMap_field_values = 0;
  }

  if(self_record->m_table_name)
  {
    delete self_record->m_table_name;
    self_record->m_table_name = 0;
  }

  if(self_record->m_connection)
  {
    delete self_record->m_connection;
    self_record->m_connection = 0;
  }
}

boost::python::object PyGlomRecord::get_connection()
{
  PyGlomRecord *self_record = this;

  if( !self_record->m_connection || !(*(self_record->m_connection)) )
  {
    PyObject* cobject = Py_None;

    //TODO: Is there some way to take the extra reference with boost::python, withour using borrowed()?
    Py_INCREF(cobject);
    return boost::python::object( boost::python::borrowed(cobject) );
  }
  else
  {
    PyObject* cobject = pygobject_new( G_OBJECT( (*(self_record->m_connection))->gobj()) ); //Creates a pygda Connection object.
    return boost::python::object( boost::python::borrowed(cobject) );
  }
}

/*
boost::python::object PyGlomRecord::get_related()
{
  PyGlomRecord *self_record = this;

  //We initialize it here, so that this work never happens if it's not needed:
  if(!(self_record->m_related))
  {
    //Return a new RelatedRecord:
    self_record->m_related = new PyGlomRelated();

    //Fill it:
    Document::type_vec_relationships vecRelationships = self_record->m_document->get_relationships(*(self_record->m_table_name));
    PyGlomRelated::type_map_relationships map_relationships;
    for(Document::type_vec_relationships::const_iterator iter = vecRelationships.begin(); iter != vecRelationships.end(); ++iter)
    {
      if(*iter)
        map_relationships[(*iter)->get_name()] = *iter;
    }

    PyGlomRelated_SetRelationships(self_record->m_related, map_relationships);

    self_record->m_related->m_record = self_record;
    Py_XINCREF(self_record); //unreffed in the self->m_related's _dealloc. //TODO: Is this a circular reference?
  }

  Py_INCREF(self_record->m_related); //Should we do this?
  return self_record->m_related;
}
*/

long PyGlomRecord::len() const
{
  if(!m_pMap_field_values)
     return 0;
     
  return m_pMap_field_values->size();
}

boost::python::object PyGlomRecord::getitem(boost::python::object cppitem)
{
  const std::string key = boost::python::extract<std::string>(cppitem);
  if(!m_pMap_field_values)
    return boost::python::object();
    
  PyGlomRecord::type_map_field_values::const_iterator iterFind = m_pMap_field_values->find(key);
  if(iterFind != m_pMap_field_values->end())
  {
    return glom_pygda_value_as_boost_pyobject(iterFind->second);
  }

  return boost::python::object();   
}

void PyGlomRecord_SetFields(PyGlomRecord* self, const PyGlomRecord::type_map_field_values& field_values, Document* document, const Glib::ustring& table_name, const Glib::RefPtr<Gnome::Gda::Connection>& opened_connection)
{
  *(self->m_pMap_field_values) = field_values; //This was allocated in Record_new().

  if(self->m_table_name == 0)
    self->m_table_name = new Glib::ustring(table_name); //Deleted in Record_dealloc().

  if(self->m_document == 0)
    self->m_document = document;

  if(self->m_connection == 0)
    self->m_connection = new Glib::RefPtr<Gnome::Gda::Connection>(opened_connection);  //Deleted in Record_dealloc().

  /*
  if(self->m_fields_dict == 0)
    self->m_fields_dict = PyDict_New();

  PyDict_Clear( self->m_fields_dict );

  //TODO: Cache this in one place:
  PyObject* module_gda = PyImport_ImportModule("gda");
  if(!module_gda)
    g_warning("Could not import python gda module.");

  PyObject* module_gda_dict = PyModule_GetDict(module_gda);
  PyObject* pyTypeGdaValue = PyDict_GetItemString(module_gda_dict, "Value"); //TODO: Unref this?
  if(!pyTypeGdaValue || !PyType_Check(pyTypeGdaValue))
    g_warning("Could not get gda.Value from gda_module.");

  //Add the new pairs:
  for(type_map_fields::const_iterator iter = fields.begin(); iter != fields.end(); ++iter)
  {
    //Add each name/value pair:

    //PyObject* pyValue = _PyObject_New((PyTypeObject*)pyTypeGdaValue);
    //if(!pyValue)
    //  g_warning("_PyObject_New() failed.");

    //PyObject_New() does not call the derived constructor. Stupid PyObject_New().
    //PyObject* new_args = PyTuple_New(0);
    //pyValue->ob_type->tp_init(pyValue, new_args, 0);
    //Py_DECREF(new_args);

    //PyObject_Call() instantiates a type when passed that type as the object to call. Strange Python.
    PyObject* new_args = PyTuple_New(0);
    PyObject* pyValue = PyObject_Call(pyTypeGdaValue, new_args, 0);
    Py_DECREF(new_args);
    if(!pyValue)
    {
      g_warning("PyObject_Call() failed to instantiate the type.");
      Record_HandlePythonError();
    }

    //g_warning("pyValue->op_type->tp_name=%s", pyValue->ob_type->tp_name);

    PyGBoxed* pygBoxed = (PyGBoxed*)(pyValue);
    GdaValue* pGdaValue = (GdaValue*)pygBoxed->boxed;

    if(!pGdaValue)
      g_warning("pygBoxed->boxed is NULL");

    gda_value_set_from_value(pGdaValue, (iter->second).gobj());

    PyDict_SetItemString(self->m_fields_dict, iter->first.c_str(), pyValue);
  }
  */
}

} //namespace Glom


