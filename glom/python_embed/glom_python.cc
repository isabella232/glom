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

#include <config.h>
//We need to include this before anything else, to avoid redefinitions:
#include <libglom/python_embed/py_glom_record.h>
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
void HandlePythonError()
{
  if(PyErr_Occurred())
    PyErr_Print();
}

// Show python coding errors of the user
void ShowTrace()
{
  // Python equivilant:
  // import traceback, sys
  // return "".join(traceback.format_exception(sys.exc_type,
  //    sys.exc_value, sys.exc_traceback))

  PyObject *type, *value, *traceback;

  PyErr_Fetch(&type, &value, &traceback);
  
  if(!traceback)
  {
    std::cerr << "traceback = 0" << std::endl;
  }

  PyObject *tracebackModule = PyImport_ImportModule((char*)"traceback");
  gchar* chrRetval = 0;
  if(tracebackModule)
  {
      PyObject* tbList = PyObject_CallMethod(
          tracebackModule,
          (char*)"format_exception",
          (char*)"OOO",
          type,
          value == 0 ? Py_None : value,
          traceback == 0 ? Py_None : traceback);
      
      if(!tbList)
      {
        std::cerr << "Glom: format_exception failed while trying to show Python TraceBack." << std::endl;
        return;
      }

      boost::python::str emptyString("");
      PyObject* strRetval = PyObject_CallMethod(emptyString.ptr(), (char*)"join",
            (char*)"O", tbList);
      if(strRetval)
        chrRetval = g_strdup(PyString_AsString(strRetval));
    
      Py_DECREF(tbList);
      Py_DECREF(strRetval);
      Py_DECREF(tracebackModule);
    }
    else
    {
        std::cerr << "Unable to import traceback module." << std::endl;
        
    }

    Py_DECREF(type);
    Py_XDECREF(value);
    Py_XDECREF(traceback);
  
  if(chrRetval)
  {
    std::cerr << "Glom: Python Error:" << std::endl << chrRetval << std::endl;
    
    //TODO: Move this to the caller.
    //Glib::ustring message = _("Python Error: \n");
    //message += chrRetval;
    //Gtk::MessageDialog dialog(message, false, Gtk::MESSAGE_ERROR);
    //dialog.run();
  }
  g_free(chrRetval);
}

bool glom_python_module_is_available()
{
  const gchar* name = "glom_" GLOM_ABI_VERSION_UNDERLINED;
  boost::python::object module_glom = boost::python::import(name);

  if(!module_glom)
  {
    g_warning("Glom: A python import of %s failed.\n", name);
  }

  return module_glom != 0;
}

bool gda_python_module_is_available()
{
  const gchar* name = "gda";
  boost::python::object module_glom = boost::python::import(name); //TODO: unref this?

  if(!module_glom)
  {
    g_warning("Glom: A python import of %s failed.\n", name);
  }

  return module_glom != 0;
}



void glom_execute_python_function_implementation(const Glib::ustring& func_impl, const type_map_fields& field_values, Document* pDocument, const Glib::ustring& table_name, const Glib::RefPtr<Gnome::Gda::Connection>& opened_connection)
{
  glom_evaluate_python_function_implementation(Field::TYPE_TEXT, func_impl, field_values, pDocument, table_name, opened_connection);
}

Gnome::Gda::Value glom_evaluate_python_function_implementation(Field::glom_field_type result_type, const Glib::ustring& func_impl,
    const type_map_fields& field_values, Document* pDocument, const Glib::ustring& table_name, const Glib::RefPtr<Gnome::Gda::Connection>& opened_connection)
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
  func_def = "def " + func_name + "(record):\n  import glom_" GLOM_ABI_VERSION_UNDERLINED "\n  import gda\n" + func_def;
  //We did this in main(): Py_Initialize();

  boost::python::object pMain = boost::python::import("__main__");
  boost::python::object pDict(pMain.attr("__dict__")); //TODO: Does boost::python have an equivalent for PyModule_GetDict()?
  //TODO: Complain that this doesn't work:
  //boost::python::dict pDict = pMain.attr("__dict__"); //TODO: Does boost::python have an equivalent for PyModule_GetDict()?
  if(!pDict)
  {
     std::cerr << "glom_evaluate_python_function_implementation(): pDict is null" << std::endl;
     ShowTrace();
     return valueResult;
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
        boost::python::handle<> objectCompiled(cObject); //Takes the reference.

        if(!objectCompiled)
          HandlePythonError();
          
      
        cObject = PyImport_ExecCodeModule(const_cast<char*>(name.c_str()), cObject); //Returns a reference. //This should make it importable.
        boost::python::handle<> object(cObject); //Takes the reference.

        if(!object)
          HandlePythonError();

        //TODO: When do these stop being importable? Should we unload them somehow later?
      }
    }
  }

  //TODO: Is this necessary?
  boost::python::object module_glom = boost::python::import("glom_" GLOM_ABI_VERSION_UNDERLINED);
  if(!module_glom)
  {
    g_warning("Could not import python glom module.");
    return valueResult; // don't crash
  }
  
  //TODO: Is this necessary?
  boost::python::object module_gda = boost::python::import("gda");
  if(!module_gda)
  {
    g_warning("Could not import python gda module.");
    return valueResult;
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
    std::cerr << "Glom: boost::python::eval() threw boost::python_error_already_set." << std::endl;
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
    std::cerr << "glom_evaluate_python_function_implementation():  boost::python::exec() threw error_already_set when using text= " << std::endl << func_def << std::endl;
    ShowTrace();
    return valueResult;
  }
  
  if(!pyValue.ptr())
  {
    std::cerr << "glom_evaluate_python_function_implementation(): boost::python::exec failed." << std::endl;
    ShowTrace();
    return valueResult;
  }

  //Call the function:
  {
    boost::python::object pFunc;
    try
    {
      pFunc = pDict[func_name.c_str()];
    }
    catch(const boost::python::error_already_set& ex)
    {
      std::cerr << "glom_evaluate_python_function_implementation():  pDict[func_name] threw error_already_set when func_name= " << std::endl << func_name << std::endl;
      ShowTrace();
      return valueResult;
    }
  
    if(!pFunc.ptr())
    {
      std::cerr << "glom_evaluate_python_function_implementation(): pDict[func_name] failed." << std::endl;
      HandlePythonError();
      return valueResult;
    }

    if(!PyCallable_Check(pFunc.ptr()))
    {
      HandlePythonError();
      g_warning("pFunc is not callable.");
      return valueResult;
    }

    //The function's parameter:
  
    //PyObject* pParam = PyString_FromString("test value"); //This test did not need the extra ref.

    boost::python::object objRecord(new PyGlomRecord);
    boost::python::extract<PyGlomRecord*> extractor(objRecord);
    if(!extractor.check())
    {
      std::cerr << ("extract<PyGlomRecord*> failed.") << std::endl;
      return valueResult;
    }
    
    PyGlomRecord* pParam = extractor;
    if(pParam)
    {
      //Fill the record's details:
      PyGlomRecord_SetFields(pParam, field_values, pDocument, table_name, opened_connection);

      //Call the function with this parameter:
      boost::python::object pyResultCpp = pFunc(objRecord);

      if(!(pyResultCpp.ptr()))
      {
        g_warning("pyResult.ptr() was null");
        HandlePythonError();
      }
      else
      {
        //Deal with the various possible return types:
        bool object_is_gda_value = false;

        GValue value = {0, {{0}}};
        const bool test = glom_pygda_value_from_pyobject(&value, pyResultCpp);

        if(test)
          object_is_gda_value = true;

        if(object_is_gda_value && G_IS_VALUE(&value))
        {
          valueResult = Gnome::Gda::Value(&value);
          //Make sure that the value is of the expected Gda type:
          //TODO_Performance:
          valueResult = Glom::Conversions::convert_value(valueResult, result_type);
          //std::cout << "DEBUG: glom_evaluate_python_function_implementation(): valueResult Gda type=" << g_type_name(valueResult.get_value_type()) << std::endl;
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
            //std::cout << "DEBUG: glom_evaluate_python_function_implementation(): empty value Gda type=" << g_type_name(valueResult.get_value_type()) << std::endl;
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
              std::cerr << "Glom: Exception caught from boost::python::extract() while converting result to a const char*." << std::endl << func_name << std::endl;
              ShowTrace();
              return valueResult;
            }

            if(pchResult)
            {
              bool success = false;
              valueResult = Conversions::parse_value(result_type, pchResult, success, true /* iso_format */);
              std::cout << "DEBUG: glom_evaluate_python_function_implementation(): parsed value Gda type=" << g_type_name(valueResult.get_value_type()) << std::endl;
            }
            else
              HandlePythonError();
          }
        }
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

} //namespace Glom
