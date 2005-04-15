/* Glom
 *
 * Copyright (C) 2001-2004 Murray Cumming
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
#include "python_module/py_glom_record.h"

#define NO_IMPORT_PYGTK //To avoid a multiple definition in pygtk.
#include <pygtk/pygtk.h> //For the PyGObject and PyGBoxed struct definitions.

#include <Python.h>
#include <compile.h> /* for the PyCodeObject */
#include <eval.h> /* for PyEval_EvalCode */

#include "glom_python.h"
#include "../data_structure/glomconversions.h"
#include <list>
#include <glib.h> //For g_warning().

std::list<Glib::ustring> ustring_tokenize(const Glib::ustring& msg, const Glib::ustring& separators, int maxParts)
{
  std::list<Glib::ustring> result;
  Glib::ustring str = msg;
  bool nocount = false;
  if (maxParts == -1)
    nocount = true;

  int count = 0;

  while(str.find(separators) != Glib::ustring::npos && (nocount? true : count!=maxParts))
  {
    unsigned int pos = str.find(separators);
    Glib::ustring tmp = str.substr(0, pos);
    str = str.erase(0, pos + separators.size());
    result.push_back(tmp);
    count++;
  }
  result.push_back(str);

  return result;
}

void HandlePythonError()
{
  if(PyErr_Occurred())
    PyErr_Print();
}

Gnome::Gda::Value glom_evaluate_python_function_implementation(Field::glom_field_type result_type, const Glib::ustring& func_impl,
    const type_map_fields& field_values)
{
  g_assert(result_type != Field::TYPE_INVALID);

  g_warning("glom_evaluate_python_function_implementation: func=%s", func_impl.c_str());

  Gnome::Gda::Value valueResult;

  Glib::ustring func_def;

  //Indent the function implementation (required by python syntax):
  typedef std::list<Glib::ustring> type_listStrings;
  type_listStrings listStrings = ustring_tokenize(func_impl, "\n", -1);
  for(type_listStrings::const_iterator iter = listStrings.begin(); iter != listStrings.end(); ++iter)
  {
    func_def += "  " + *iter + "\n";
  }


  //prefix the def line:
  const Glib::ustring func_name = "glom_calc_field_value";
  //TODO: When pygda packages are available: func_def = "def " + func_name + "(record):\n  import gda\n" + func_def;
  func_def = "def " + func_name + "(record):\n  import glom\n  import gda\n" + func_def;
  //We did this in main(): Py_Initialize();

  PyObject* pMain = PyImport_AddModule("__main__");
  PyObject* pDict = PyModule_GetDict(pMain);

  PyObject* module_glom = PyImport_ImportModule("glom");
  if(!module_glom)
    g_warning("Could not import python glom module.");

  PyObject* module_glom_dict = PyModule_GetDict(module_glom);
  //This seems to be different to PyGlomRecord_GetPyType() - we can PyObject_Call() this one to instantiate it.
  PyObject* pyTypeGlomRecord = PyDict_GetItemString(module_glom_dict, "Record"); //TODO: Unref this?
  if(!pyTypeGlomRecord || !PyType_Check(pyTypeGlomRecord))
    g_warning("Could not get glom.Record from glom_module.");


  PyObject* module_gda = PyImport_ImportModule("gda");
  if(!module_gda)
    g_warning("Could not import python gda module.");

  PyObject* module_gda_dict = PyModule_GetDict(module_gda);
  PyObject* pyTypeGdaValue = PyDict_GetItemString(module_gda_dict, "Value"); //TODO: Unref this?
  if(!pyTypeGdaValue || !PyType_Check(pyTypeGdaValue))
    g_warning("Could not get gda.Value from gda_module.");


  //Create the function definition:
  PyObject* pyValue = PyRun_String(func_def.c_str(), Py_file_input, pDict, pDict);
  if(pyValue)
  {
    Py_DECREF(pyValue);
    pyValue = 0;
  }
  else
    HandlePythonError();

  //Call the function:
  {
    PyObject* pFunc = PyDict_GetItemString(pDict, func_name.c_str()); //The result is borrowed, so should not be dereferenced.
    if(!pFunc)
      HandlePythonError();

    if(!PyCallable_Check(pFunc))
    {
      g_warning("pFunc is not callable.");
      return valueResult;
    }

    //The function's parameter:
  
    //PyObject* pParam = PyString_FromString("test value"); //This test did not need the extra ref.

    PyObject* new_args = PyTuple_New(0);
    PyGlomRecord* pParam = (PyGlomRecord*)PyObject_Call((PyObject*)pyTypeGlomRecord, new_args, 0);
    //PyGlomRecord* pParam = (PyGlomRecord*)PyObject_Call((PyObject*)PyGlomRecord_GetPyType(), new_args, 0);
    Py_DECREF(new_args);
    if(pParam)
    {
      Py_INCREF(pParam); //TODO: As I understand it, PyObject_New() should return a new reference, so this should not be necessary.

      //Fill the record's details:
      PyGlomRecord_SetFields(pParam, field_values);

      PyObject* pArgs = PyTuple_New(1);
      PyTuple_SetItem(pArgs, 0, (PyObject*)pParam); //The pParam reference is taken by PyTuple_SetItem().

      //Call the function with this parameter:
      PyObject* pyResult = PyObject_CallObject(pFunc, pArgs);
      Py_DECREF(pArgs);

      if(!pyResult)
      {
        g_warning("pyResult was null");
        HandlePythonError();
      }
      else
      {
        //Deal with the various possible return types:
        bool object_is_gda_value = false;

        //This is a hack - see below:
        //if( strcmp(pyResult->ob_type->tp_name, "gda.Value") == 0 )
        //  object_is_gda_value = true;

        //g_warning("debug: pyResult->ob_type->tp_name=%s", pyResult->ob_type->tp_name);
        //g_warning("debug: pyTypeGdaValue->tp_name=%s", ((PyTypeObject*)pyTypeGdaValue)->tp_name);

        int test = PyType_IsSubtype(pyResult->ob_type, (PyTypeObject*)pyTypeGdaValue);
        if(test == -1)
          HandlePythonError();

        if(test == 1) // 0 means false, -1 means error.
          object_is_gda_value = true;

        if(object_is_gda_value)
        {
          //Cast it to the "derived" struct type:
          //All boxed types are wrapped with PyGBoxed in pygtk, without their own special structs,
          //and GValue is a boxed-type.
          PyGBoxed* pygtkobject = (PyGBoxed*)pyResult;
          if(pygtkobject)
          {
            GdaValue* cboxed = (GdaValue*)pygtkobject->boxed;
            if(cboxed)
              valueResult = Glib::wrap(cboxed, true /* take_copy */);
            g_warning("PyGBoxed::boxed is null");
          }
          else
            g_warning("PyGBoxed is null");
        }
        else
        {
          //g_warning("debug: pyResult is not a gda.value");

          //TODO: Handle numeric/date/time types:

          //Treat this as a string or something that can be converted to a string:
          PyObject* pyObjectResult = PyObject_Str(pyResult);
          if(PyString_Check(pyObjectResult))
          {
            const char* pchResult = PyString_AsString(pyObjectResult);
            if(pchResult)
            {
              bool success = false;
              valueResult = GlomConversions::parse_value(result_type, pchResult, success, true /* iso_format */);
            }
            else
              g_warning("pchResult is null");
          }
          else
            g_warning("PyString_Check returned false");
        }

        Py_DECREF(pyResult);
      }
    }
  }

  Py_FlushLine();
  PyErr_Clear();


  //We did this in main(): Py_Finalize();

  return valueResult;
}

/*

examples:

  return record.fields["name_first"] + " " + record.fields["name_last"]

  return record.related_records["contacts"][0].fields["name_first"]

  return record.related_records["invoice lines"].sum("cost") )
*/
