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

#include "py_glom_record.h"
#include "py_glom_related.h"
#include "pygdavalue_conversions.h" //For pygda_value_as_pyobject().

#include "../../data_structure/field.h"
#include <glibmm/ustring.h>


//Allocate a new object:
//TODO: Why not parse the args here as well as in Record_init()?
static PyObject *
Record_new(PyTypeObject *type, PyObject * /* args */, PyObject * /* kwds */)
{
  PyGlomRecord *self  = (PyGlomRecord*)type->tp_alloc(type, 0);
  if(self)
  {
    self->m_related = 0;

    self->m_pMap_field_values = new PyGlomRecord::type_map_field_values();
  }

  return (PyObject*)self;
}

//Set the object's member data, from the parameters supplied when creating the object:
static int
Record_init(PyGlomRecord *self, PyObject * /* args */, PyObject * /* kwds */)
{
  //static char *kwlist[] = {"test", NULL};

  //if(!PyArg_ParseTupleAndKeywords(args, kwds, "|i", kwlist,
   //                                 &self->m_test))
   // return -1;

  if(self)
  {
    self->m_related = 0;

    if(self->m_pMap_field_values == 0)
      self->m_pMap_field_values = new PyGlomRecord::type_map_field_values();
  }

  return 0;
}

static void
Record_dealloc(PyGlomRecord* self)
{
  if(self->m_pMap_field_values)
  {
    delete self->m_pMap_field_values;
    self->m_pMap_field_values = 0;
  }

  self->ob_type->tp_free((PyObject*)self);
}


static PyObject *
Record__get_related(PyGlomRecord* self, void* /* closure */)
{
  //We initialize it here, so that this work never happens if it's not needed:
  if(!(self->m_related))
  {
    //Return a new RelatedRecord:
    PyObject* new_args = PyTuple_New(0);
    self->m_related = (PyGlomRelated*)PyObject_Call((PyObject*)PyGlomRelated_GetPyType(), new_args, 0);
    Py_DECREF(new_args);

    //Fill it:
    Document_Glom::type_vecRelationships vecRelationships = self->m_document->get_relationships(*(self->m_table_name));
    PyGlomRelated::type_map_relationships map_relationships;
    for(Document_Glom::type_vecRelationships::const_iterator iter = vecRelationships.begin(); iter != vecRelationships.end(); ++iter)
    {
      if(*iter)
        map_relationships[(*iter)->get_name()] = *iter;
    }

    PyGlomRelated_SetRelationships(self->m_related, map_relationships);

    self->m_related->m_record = self;
    Py_XINCREF(self); //unreffed in the self->m_related's _dealloc. //TODO: Is this a circular reference?
  }

  Py_INCREF(self->m_related); //Should we do this?
  return (PyObject*)self->m_related;
}


static PyGetSetDef Record_getseters[] = {
    {"related",
     (getter)Record__get_related, (setter)0, 0, 0
    },
    {NULL, 0, 0, 0, 0, }  // Sentinel
};



static int
Record_tp_as_mapping_length(PyGlomRecord *self)
{
  return self->m_pMap_field_values->size();
}

static PyObject *
Record_tp_as_mapping_getitem(PyGlomRecord *self, PyObject *item)
{
  if(PyString_Check(item))
  {
    const char* pchKey = PyString_AsString(item);
    if(pchKey)
    {
      const Glib::ustring key(pchKey);
      if(self && self->m_pMap_field_values)
      {
        PyGlomRecord::type_map_field_values::const_iterator iterFind = self->m_pMap_field_values->find(key);
        if(iterFind != self->m_pMap_field_values->end())
        {
          return pygda_value_as_pyobject(iterFind->second.gobj(), true /* copy */);
        }
        else
        {
          g_warning("Record_tp_as_mapping_getitem(): item not found in m_pMap_field_values. size=%d, item=%s", self->m_pMap_field_values->size(), pchKey);
        }
      }
      else
      {
        g_warning("Record_tp_as_mapping_getitem(): self or self->m_pMap_field_values is NULL.");
      }
    }
    else
    {
       g_warning("Record_tp_as_mapping_getitem(): PyString_AsString(item) returned NULL.");
    }
  }
  else
  {
    g_warning("Record_tp_as_mapping_getitem(): PyString_Check(item) failed.");
  }


  g_warning("Record_tp_as_mapping_getitem(): return null.");
  PyErr_SetString(PyExc_IndexError, "field not found");
  return NULL;
}

/*
static int
Record_tp_as_mapping_setitem(PyGObject *self, PyObject *item, PyObject *value)
{
  Py_INCREF(Py_None);
  return Py_None;
}
*/

static PyMappingMethods Record_tp_as_mapping = {
    (inquiry)Record_tp_as_mapping_length,
    (binaryfunc)Record_tp_as_mapping_getitem,
    (objobjargproc)0 /* Record_tp_as_mapping_setitem */
};


static PyTypeObject pyglom_RecordType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "glom.Record",             /*tp_name*/
    sizeof(PyGlomRecord), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)Record_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    &Record_tp_as_mapping,      /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    "Glom objects",           /* tp_doc */
    0,                  /* tp_traverse */
    0,                   /* tp_clear */
    0,                   /* tp_richcompare */
    0,                   /* tp_weaklistoffset */
    0,                   /* tp_iter */
    0,                   /* tp_iternext */
    0 /* Record_methods */,             /* tp_methods */
    0 /* Record_members */,             /* tp_members */
    Record_getseters,                   /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)Record_init,      /* tp_init */
    0,                         /* tp_alloc */
    Record_new,                 /* tp_new */
    0, 0, 0, 0, 0, 0, 0, 0,
};

PyTypeObject* PyGlomRecord_GetPyType()
{
  return &pyglom_RecordType;
}



/*
static void Record_HandlePythonError()
{
  if(PyErr_Occurred())
    PyErr_Print();
}
*/


void PyGlomRecord_SetFields(PyGlomRecord* self, const PyGlomRecord::type_map_field_values& field_values, Document_Glom* document, const Glib::ustring& table_name)
{
  *(self->m_pMap_field_values) = field_values;
  self->m_table_name = new Glib::ustring(table_name);
  self->m_document = document;

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



