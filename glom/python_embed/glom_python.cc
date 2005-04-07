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
#include "py_glom_record.h"
#include <Python.h>
#include "compile.h" /* for the PyCodeObject */
#include "eval.h" /* for PyEval_EvalCode */

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

Gnome::Gda::Value glom_evaluate_python_function_implementation(Field::glom_field_type result_type, const Glib::ustring& func_impl)
{
  Glib::ustring result;

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
  func_def = "def " + func_name + "(record):\n" + func_def;

  //We did this in main(): Py_Initialize();

  PyObject* pMain = PyImport_AddModule("__main__");
  PyObject* pDict = PyModule_GetDict(pMain);

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
    }

    //The function's parameter:
    PyObject* pArgs = PyTuple_New(1);
    //PyObject* pParam = PyString_FromString("test value"); //This test did not need the extra ref.
    PyGlomRecord* pParam = PyObject_New(PyGlomRecord, PyGlomRecord_GetPyType());
    Py_INCREF(pParam); //TODO: As I understand it, PyObject_New() should return a new reference, so this should not be necessary.

    PyTuple_SetItem(pArgs, 0, (PyObject*)pParam); //The pParam reference is taken by PyTuple_SetItem().

    //Call the function with this parameter:
    PyObject* pyValue = PyObject_CallObject(pFunc, pArgs);
    Py_DECREF(pArgs);

    if(!pyValue)
    {
      g_warning("pyValue was null");
      HandlePythonError();
    }
    else
    {
      PyObject* pyStringObject = PyObject_Str(pyValue);
      if(pyStringObject)
      {
        if(PyString_Check(pyStringObject))
        {
          const char* pchResult = PyString_AsString(pyStringObject);
          if(pchResult)
            result = pchResult;
          else
            g_warning("pchResult is null");
        }
        else
          g_warning("PyString_Check returned false");
      }
      else
        g_warning("pyStringObject is null");

      Py_DECREF(pyValue);
    }
  }

  Py_FlushLine();
  PyErr_Clear();


  //We did this in main(): Py_Finalize();

  //TODO: Get the Value as a PyValue and convert directly to a Gnome::Gda::Value without parsing text:
  bool success = false;
  Gnome::Gda::Value valueResult = GlomConversions::parse_value(result_type, result, success, true /* iso_format */);

  return valueResult;
}

/*

examples:

  return record.fields["name_first"] + " " + record.fields["name_last"]

  return record.related_records["contacts"][0].fields["name_first"]

  return record.related_records["invoice lines"].sum("cost") )
*/
