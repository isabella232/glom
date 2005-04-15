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

#include "py_glom_related.h"

#include "../../data_structure/field.h"
#include <glibmm/ustring.h>


//Allocate a new object:
//TODO: Why not parse the args here as well as in Related_init()?
static PyObject *
Related_new(PyTypeObject *type, PyObject * /* args */, PyObject * /* kwds */)
{
  PyGlomRelated *self  = (PyGlomRelated*)type->tp_alloc(type, 0);
  if(self)
  {
    self->m_record = 0;
  }

  return (PyObject*)self;
}

//Set the object's member data, from the parameters supplied when creating the object:
static int
Related_init(PyGlomRelated *self, PyObject* /* args */, PyObject* /* kwds */)
{
  if(self)
  {
    self->m_record = 0;
  }

  return 0;
}

static void
Related_dealloc(PyGlomRelated* self)
{
  self->ob_type->tp_free((PyObject*)self);
}

static PyTypeObject pyglom_RelatedType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "glom.Related",             /*tp_name*/
    sizeof(PyGlomRelated), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)Related_dealloc, /*tp_dealloc*/
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
    "Glom objects",           /* tp_doc */
    0,                  /* tp_traverse */
    0,                   /* tp_clear */
    0,                   /* tp_richcompare */
    0,                   /* tp_weaklistoffset */
    0,                   /* tp_iter */
    0,                   /* tp_iternext */
    0 /* Related_methods */,             /* tp_methods */
    0 /* Related_members */,             /* tp_members */
    0,                   /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)Related_init,      /* tp_init */
    0,                         /* tp_alloc */
    Related_new,                 /* tp_new */
    0, 0, 0, 0, 0, 0, 0, 0,
};

PyTypeObject* PyGlomRelated_GetPyType()
{
  return &pyglom_RelatedType;
}

/*
static void Related_HandlePythonError()
{
  if(PyErr_Occurred())
    PyErr_Print();
}
*/



