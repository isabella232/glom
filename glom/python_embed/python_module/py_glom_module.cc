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
#include "py_glom_relatedrecord.h"

static PyMethodDef pyglom_methods[] = {
    {NULL, 0, 0, 0}  /* Sentinel */
};

PyMODINIT_FUNC
initglom(void) 
{
  PyObject* m;

  //pyglom_RecordType.tp_new = PyType_GenericNew;

  if(PyType_Ready(PyGlomRecord_GetPyType()) < 0)
    return;

  if(PyType_Ready(PyGlomRelated_GetPyType()) < 0)
    return;

  if(PyType_Ready(PyGlomRelatedRecord_GetPyType()) < 0)
    return;


  m = Py_InitModule3("glom", pyglom_methods,
                      "Python module for Glom caluclated fields.");


  Py_INCREF(PyGlomRecord_GetPyType());
  PyModule_AddObject(m, "Record", (PyObject *)PyGlomRecord_GetPyType());

  Py_INCREF(PyGlomRelated_GetPyType());
  PyModule_AddObject(m, "Related", (PyObject *)PyGlomRelated_GetPyType());

  Py_INCREF(PyGlomRelated_GetPyType());
  PyModule_AddObject(m, "RelatedRecord", (PyObject *)PyGlomRelated_GetPyType());


  if(PyErr_Occurred())
    Py_FatalError ("Can't initialise glom module");
}





