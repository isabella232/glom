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
#include "compile.h" /* for the PyCodeObject */
#include "eval.h" /* for PyEval_EvalCode */

#include "py_glom_record.h"

#include "../data_structure/field.h"
#include <glibmm/ustring.h>


//Allocate a new object:
//TODO: Why not parse the args here as well as in Record_init()?
static PyObject *
Record_new(PyTypeObject *type, PyObject * /* args */, PyObject * /* kwds */)
{
  PyGlomRecord *self  = (PyGlomRecord*)type->tp_alloc(type, 0);
  if(self)
  {
    self->m_test = 0;
    self->m_fields_dict = PyDict_New();
  }

  return (PyObject*)self;
}

//Set the object's member data, from the parameters supplied when creating the object:
static int
Record_init(PyGlomRecord *self, PyObject *args, PyObject *kwds)
{
  static char *kwlist[] = {"test", NULL};

  if(!PyArg_ParseTupleAndKeywords(args, kwds, "|i", kwlist,
                                    &self->m_test))
    return -1;

  return 0;
}

static void
Record_dealloc(PyGlomRecord* self)
{
  Py_XDECREF(self->m_fields_dict);
  self->m_fields_dict = 0;

  self->ob_type->tp_free((PyObject*)self);
}

static PyObject *
Record_get_fields(PyGlomRecord* self, void* /* closure */)
{
    Py_INCREF(self->m_fields_dict); //Provide a reference, for the caller to unreference().
    return self->m_fields_dict;
}

static PyGetSetDef Record_getseters[] = {
    {"fields",
     (getter)Record_get_fields, (setter)0, 0, 0
    }, 
    {NULL, 0, 0, 0, 0, }  /* Sentinel */
};

static PyTypeObject pyglom_RecordType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pyglom.Record",             /*tp_name*/
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
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    "PyGlom objects",           /* tp_doc */
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

static PyMethodDef pyglomrecord_methods[] = {
    {NULL, 0, 0, 0}  /* Sentinel */
};

PyMODINIT_FUNC
initpyglomrecord(void) 
{
    PyObject* m;

    pyglom_RecordType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&pyglom_RecordType) < 0)
        return;

    m = Py_InitModule3("pyglomrecord", pyglomrecord_methods,
                       "Example module that creates an extension type.");

    Py_INCREF(&pyglom_RecordType);
    PyModule_AddObject(m, "PyGlom", (PyObject *)&pyglom_RecordType);
}


//typedef std::map<Glib::ustring, Gnome::Gda::Value> type_map_fields;
void PyGlomRecord_SetFields(PyGlomRecord* /* self */, const type_map_fields& fields)
{
  for(type_map_fields::const_iterator iter = fields.begin(); iter != fields.end(); ++iter)
  {
    //TODO: self->m_fields_dict iter->first //name, iter->second //value.
  }
}



