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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#include "config.h"
//We need to include this before anything else, to avoid redefinitions:
#include <libglom/python_embed/py_glom_record.h>
#include <libglom/python_embed/py_glom_ui.h>
#include <libglom/python_embed/pygdavalue_conversions.h>

#include <boost/python.hpp>

//#define NO_IMPORT_PYGOBJECT //To avoid a multiple definition in pygtk.
//#include <pygobject.h> //For the PyGObject and PyGBoxed struct definitions.

#include "glom_python.h"
#include <libglom/data_structure/glomconversions.h>
#include <list>
#include <glib.h> //For g_warning().
#include <glibmm/i18n.h>

#include <gtkmm/messagedialog.h>

#include <iostream>

namespace Glom
{

static std::list<Glib::ustring> ustring_tokenize(const Glib::ustring& msg, const Glib::ustring& separators, int maxParts)
{
  std::list<Glib::ustring> result;
  Glib::ustring str = msg;
  bool nocount = false;
  if(maxParts == -1)
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

// Use this for errors not (directly) caused by the user
static void HandlePythonError()
{
  if(PyErr_Occurred())
    PyErr_Print();
}

// Show python coding errors of the user
static Glib::ustring get_traceback()
{
  // Python equivalent:
  // import traceback, sys
  // return "".join(traceback.format_exception(sys.exc_type,
  //    sys.exc_value, sys.exc_traceback))
  //
  // See https://wiki.python.org/moin/boost.python/EmbeddingPython
  //   for the Boost::Python code that this is based on:


  PyObject* type = 0;
  PyObject* value = 0;
  PyObject* traceback = 0;
  PyErr_Fetch(&type, &value, &traceback);

  if(!traceback)
  {
    std::cerr << G_STRFUNC << ": traceback = 0" << std::endl;
  }

  PyErr_NormalizeException(&type, &value, &traceback);


  boost::python::handle<> hexc(type);
  boost::python::handle<> hval(boost::python::allow_null(value));
  boost::python::handle<> htb(boost::python::allow_null(traceback));
  if(!hval)
  {
    const std::string result = boost::python::extract<std::string>(boost::python::str(hexc));
    return result;
  }
  else
  {
    boost::python::object traceback(boost::python::import("traceback"));
    boost::python::object format_exception(traceback.attr("format_exception"));
    boost::python::object formatted_list(format_exception(hexc,hval,htb));
    boost::python::object formatted(boost::python::str("").join(formatted_list));
    const std::string result = boost::python::extract<std::string>(formatted);
    return result;
  }
}

static void ShowTrace()
{
    std::cerr << G_STRFUNC << ": Glom: Python Error:" << std::endl << get_traceback() << std::endl;

}

/** Import a python module, warning about exceptions.
 * Compare with boost::python::object() to detect failure.
 */
static boost::python::object import_module(const char* name)
{
  boost::python::object module_glom; //Defaults to PyNone.
  try
  {
    module_glom = boost::python::import(name);
  }
  catch(const boost::python::error_already_set& ex)
  {
    std::cerr << G_STRFUNC << ": boost::python::import() failed while importing module: "<< name << std::endl;
    ShowTrace();
  }

  if(module_glom == boost::python::object())
  {
    std::cerr << G_STRFUNC << ": Glom: A python import of a module failed: " << name << std::endl;
  }

  return module_glom;
}

bool glom_python_module_is_available()
{
  const char* name = "glom_" GLOM_ABI_VERSION_UNDERLINED;
  const boost::python::object module_glom = import_module(name);
  return module_glom != boost::python::object();
}

bool gir_python_module_is_available()
{
  const char* name = "gi.repository";
  const boost::python::object module_glom = import_module(name);
  return module_glom != boost::python::object();
}

bool gda_python_module_is_available()
{
  const char* name = "gi.repository.Gda";
  const boost::python::object module_glom = import_module(name);
  return module_glom != boost::python::object();
}

static boost::python::object glom_python_call(Field::glom_field_type result_type,
  const Document* pDocument,
  const Glib::ustring& func_impl,
  Glib::ustring& error_message,
  const boost::python::object& param1,
  const boost::python::object& param2 = boost::python::object())
{
  //std::cout << "glom_evaluate_python_function_implementation()" << std::endl;
  //for(type_map_fields::const_iterator iter = field_values.begin(); iter != field_values.end(); ++iter)
  //{
  //  std::cout << "  field_value: name=" << iter->first << ", value=" << iter->second.to_string() << std::endl;
  //}

  g_assert(result_type != Field::TYPE_INVALID);

  //g_warning("glom_evaluate_python_function_implementation: func=%s", func_impl.c_str());

  Gnome::Gda::Value valueResult;

  Glib::ustring func_def;

  //Indent the function implementation (required by python syntax):
  typedef std::list<Glib::ustring> type_listStrings;
  type_listStrings listStrings = ustring_tokenize(func_impl, "\n", -1);
  for(type_listStrings::const_iterator iter = listStrings.begin(); iter != listStrings.end(); ++iter)
  {
    func_def += "  " + *iter + '\n';
  }


  //prefix the def line:
  const Glib::ustring func_name = "glom_calc_field_value";
  Glib::ustring func_signature;
  if(!param2.ptr())
    func_signature = func_name + "(record)";
  else
    func_signature = func_name + "(record, ui)";

  func_def = "def " + func_signature + ":\n  import glom_" GLOM_ABI_VERSION_UNDERLINED "\n  from gi.repository import Gda\n" + func_def;

  //We did this in main(): Py_Initialize();

  boost::python::object pMain = import_module("__main__");
  boost::python::object pDict(pMain.attr("__dict__")); //TODO: Does boost::python have an equivalent for PyModule_GetDict()?
  //TODO: Complain that this doesn't work:
  //boost::python::dict pDict = pMain.attr("__dict__"); //TODO: Does boost::python have an equivalent for PyModule_GetDict()?
  if(!pDict)
  {
     std::cerr << G_STRFUNC << ": pDict is null" << std::endl;
     error_message = get_traceback();
     return boost::python::object();
  }

  //Allow the function to import from our script library:
  if(pDocument)
  {
    const std::vector<Glib::ustring> module_names = pDocument->get_library_module_names();
    for(std::vector<Glib::ustring>::const_iterator iter = module_names.begin(); iter != module_names.end(); ++iter)
    {
      const Glib::ustring name = *iter;
      const Glib::ustring script = pDocument->get_library_module(name);
      if(!name.empty() && !script.empty())
      {
        //TODO: Is there a boost::python equivalent for Py_CompileString()?
        PyObject* cObject = Py_CompileString(script.c_str(), name.c_str() /* "filename", for debugging info */,  Py_file_input /* "start token" for multiple lines of code. */); //Returns a reference.
        boost::python::object objectCompiled( (boost::python::handle<>(cObject)) ); //Takes the reference.

        if(!objectCompiled.ptr()) //TODO: We'd probably have an exception instead if we don't use boost's allow_null.
          HandlePythonError();

        PyObject* cObjectExeced = PyImport_ExecCodeModule(const_cast<char*>(name.c_str()), cObject); //Returns a reference. //This should make it importable.
        boost::python::object objectExeced( (boost::python::handle<>(cObjectExeced)) ); //Takes the reference.

        if(!objectExeced.ptr())
          HandlePythonError();

        //TODO: When do these stop being importable? Should we unload them somehow later?
      }
    }
  }

  //TODO: Is this necessary?
  boost::python::object module_glom = import_module("glom_" GLOM_ABI_VERSION_UNDERLINED);
  if(module_glom == boost::python::object())
  {
    g_warning("Could not import python glom module.");
    return boost::python::object(); // don't crash
  }

  //TODO: Is this necessary?
  boost::python::object module_gda = import_module("gi.repository.Gda");
  if(module_gda == boost::python::object())
  {
    g_warning("Could not import python gi.repository.Gda module.");
    return boost::python::object();
  }

  //Create the function definition:
  /*
  boost::python::object pyValue;
  try
  {
    pyValue = boost::python::eval(func_def.c_str(), pDict, pDict); //TODO: This throws.
  }
  catch(const boost::python::error_already_set& ex)
  {
    std::cerr << G_STRFUNC << ": Glom: boost::python::eval() threw boost::python_error_already_set." << std::endl;
    HandlePythonError();
  }
  */

  //TODO: Complain that exec(std::string(something), pMain) doesn't work.
  boost::python::object pyValue;
  try
  {
    //TODO: The second dict is optional, and the documentation suggests using pMain as the first argument, but you'll get a
    //"TypeError: 'module' object does not support item assignment" error if you omit it.
    //TODO: Make sure that's documented.
    pyValue = boost::python::exec(func_def.c_str(), pDict, pDict);
  }
  catch(const boost::python::error_already_set& ex)
  {
    std::cerr << G_STRFUNC << ":  boost::python::exec() threw error_already_set when using text= " << std::endl << func_def << std::endl;
    error_message = get_traceback();
    return boost::python::object();
  }

  if(!pyValue.ptr())
  {
    std::cerr << G_STRFUNC << ": boost::python::exec failed." << std::endl;
    error_message = get_traceback();
    return boost::python::object();
  }

  //Call the function:
  boost::python::object pFunc;
  try
  {
    pFunc = pDict[func_name.c_str()];
  }
  catch(const boost::python::error_already_set& ex)
  {
    std::cerr << G_STRFUNC << ":  pDict[func_name] threw error_already_set when func_name= " << std::endl << func_name << std::endl;
    error_message = get_traceback();
    return boost::python::object();
  }

  if(!pFunc.ptr())
  {
    std::cerr << G_STRFUNC << ": pDict[func_name] failed." << std::endl;
    HandlePythonError();
    return boost::python::object();
  }

  if(!PyCallable_Check(pFunc.ptr()))
  {
    HandlePythonError();
    g_warning("pFunc is not callable.");
    return boost::python::object();
  }

  //Call the function with the parameters:
  boost::python::object pyResultCpp;

  try
  {
    if(!param2.ptr())
    {
      pyResultCpp = pFunc(param1);
    }
    else
    {
        pyResultCpp = pFunc(param1, param2);
    }
  }
  catch(const boost::python::error_already_set& ex)
  {
    std::cerr << G_STRFUNC << ": Glom: Exception caught from pFunc(objRecord). func_name=" << std::endl << func_name << std::endl;
    error_message = get_traceback();
    //std::cerr << G_STRFUNC << ":   traceback=" << error_message << std::endl;
  }

  if(!(pyResultCpp.ptr()))
  {
    g_warning("pyResult.ptr() was null");
    HandlePythonError();
  }

  //TODO: Why do we do this?
#if PY_MAJOR_VERSION < 3
  //There is no Py_FlushLine in Python 3
  //Py_FlushLine();
#endif
  PyErr_Clear();

  //We did this in main(): Py_Finalize();

  return pyResultCpp;
}

void glom_execute_python_function_implementation(const Glib::ustring& func_impl,
  const type_map_fields& field_values,
  const Document* pDocument,
  const Glib::ustring& table_name,
  const sharedptr<const Field>& key_field,
  const Gnome::Gda::Value& key_field_value,
  const Glib::RefPtr<Gnome::Gda::Connection>& opened_connection,
  const PythonUICallbacks& callbacks,
  Glib::ustring& error_message)
{
  //Import the glom module so that boost::python::object(new PyGlomRecord) can work.
  boost::python::object module_glom = import_module("glom_" GLOM_ABI_VERSION_UNDERLINED);
  if(module_glom == boost::python::object())
  {
    g_warning("Could not import python glom module.");
    return; // don't crash
  }

  boost::python::object objRecord(new PyGlomRecord);
  boost::python::extract<PyGlomRecord*> extractor(objRecord);
  if(!extractor.check())
  {
    std::cerr << ("extract<PyGlomRecord*> failed.") << std::endl;
  }

  PyGlomRecord* pParam = extractor;
  if(pParam)
  {
    //Fill the record's details:
    pParam->set_fields(field_values, pDocument, table_name, key_field, key_field_value, opened_connection);
    pParam->set_read_only();
  }

  //Pass an additional ui parameter for use by scripts:
  boost::python::object objUI(new PyGlomUI(callbacks));

  glom_python_call(Field::TYPE_TEXT, pDocument, func_impl, error_message, objRecord, objUI);
}

Gnome::Gda::Value glom_evaluate_python_function_implementation(Field::glom_field_type result_type,
  const Glib::ustring& func_impl,
  const type_map_fields& field_values,
  const Document* pDocument,
  const Glib::ustring& table_name,
  const sharedptr<const Field>& key_field,
  const Gnome::Gda::Value& key_field_value,
  const Glib::RefPtr<Gnome::Gda::Connection>& opened_connection,
  Glib::ustring& error_message)
{
  //Import the glom module so that boost::python::object(new PyGlomRecord) can work.
  boost::python::object module_glom = import_module("glom_" GLOM_ABI_VERSION_UNDERLINED);
  if(module_glom == boost::python::object())
  {
    g_warning("Could not import python glom module.");
    return Gnome::Gda::Value(); // don't crash
  }

  boost::python::object objRecord(new PyGlomRecord);

  boost::python::extract<PyGlomRecord*> extractor(objRecord);
  if(!extractor.check())
  {
    std::cerr << ("extract<PyGlomRecord*> failed.") << std::endl;
    return Gnome::Gda::Value();
  }

  PyGlomRecord* pParam = extractor;
  if(pParam)
  {
    //Fill the record's details:
    pParam->set_fields(field_values, pDocument, table_name, key_field, key_field_value, opened_connection);
  }

  const boost::python::object pyResultCpp = glom_python_call(result_type, pDocument, func_impl, error_message, objRecord);

  //Deal with the various possible return types:
  Gnome::Gda::Value valueResult;
  bool object_is_gda_value = false;

  GValue value = {0, {{0}}};
  const bool test = glom_pygda_value_from_pyobject(&value, pyResultCpp);

  if(test)
    object_is_gda_value = true;

  if(object_is_gda_value && G_IS_VALUE(&value))
  {
    valueResult = Gnome::Gda::Value(&value);
    if(valueResult.get_value_type() == 0)
    {
      std::cerr << G_STRFUNC << ": valueResult (before convert_value()) has a GType of 0 before convert_value()." << std::endl;
    }

    //Make sure that the value is of the expected Gda type:
    //TODO_Performance:
    valueResult = Glom::Conversions::convert_value(valueResult, result_type);
    if(valueResult.get_value_type() == 0)
    {
      std::cerr << G_STRFUNC << ": valueResult has a GType of 0 after convert_value()." << std::endl;
    }

    //std::cout << "debug: " << G_STRFUNC << ": valueResult Gda type=" << g_type_name(valueResult.get_value_type()) << std::endl;
    g_value_unset(&value);
  }
  else
  {
    //g_warning("debug: pyResult is not a gda.value");

    //For instance, if one of the fields was empty, then the calculation might want to return an empty value,
    //instead of returning 0.
    if(pyResultCpp == boost::python::object()) //Check if it is PyNone
    {
      //The result should be an appropriate empty value for this field type:
      valueResult = Conversions::get_empty_value(result_type);
      //std::cout << "debug: " << G_STRFUNC << ": empty value Gda type=" << g_type_name(valueResult.get_value_type()) << std::endl;
    }
    else
    {
      //TODO: Handle numeric/date/time types:
      //(though I don't think this code is ever reached. murrayc)

      //Treat this as a string or something that can be converted to a string:
      const char* pchResult = 0;
      try
      {
        pchResult = boost::python::extract<const char*>(pyResultCpp);
      }
      catch(const boost::python::error_already_set& ex)
      {
        std::cerr << G_STRFUNC << ": Glom: Exception caught from boost::python::extract() while converting result to a const char*." << std::endl;
        ShowTrace();
        return valueResult;
      }

      if(pchResult)
      {
        bool success = false;
        valueResult = Conversions::parse_value(result_type, pchResult, success, true /* iso_format */);
        std::cout << "debug: " << G_STRFUNC << ": parsed value Gda type=" << g_type_name(valueResult.get_value_type()) << std::endl;
      }
      else
        HandlePythonError();
    }
  }

  return valueResult;
}


} //namespace Glom
