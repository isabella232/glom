#include <Python.h>
#include <glib.h> //For g_warning().
#include <glibmm/ustring.h>
#include <iostream>
#include <list>

std::list<Glib::ustring> ustring_tokenize(const Glib::ustring& msg, const Glib::ustring& separators, int maxParts)
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
    Glib::ustring tmp = str.substr(0,pos);
    str=str.erase(0, pos+separators.size());
    result.push_back(tmp);
    count++;
  }
  result.push_back(str);

  return result;
}

void evaluate_function_implementation(const Glib::ustring& func_impl)
{
  Glib::ustring func_def;

  typedef std::list<Glib::ustring> type_listStrings;
  type_listStrings listStrings = ustring_tokenize(func_impl, "\n", -1);
  for(const auto& item : listStrings)
  {
    func_def += "  " + item + '\n';
  }

  //Indent the function implementation (required by python syntax):

  //prefix the def line:
  func_def = "def glom_calc_field_value():\n" + func_def;

  std::cout << func_def << std::endl;
  
  Py_Initialize();
  //PyDateTime_IMPORT; //A macro, needed to use PyDate_Check(), PyDateTime_Check(), etc.

  PyObject* pMain = PyImport_AddModule((gchar*)"__main__");
  PyObject* pDict = PyModule_GetDict(pMain);

  //Create the function definition:
  PyObject* pyValue = PyRun_String(func_def.c_str(), Py_file_input, pDict, pDict);
  if(pyValue)
  {
    Py_DECREF(pyValue);
    pyValue = 0;
  }

  //Call the function:
  {
    Glib::ustring call_func = "glom_calc_field_value()";
    PyObject* pyValue = PyRun_String(call_func.c_str(), Py_eval_input, pDict, pDict);
    if(!pyValue)
    {
      g_warning("pyValue was null");
      PyErr_Print();
    }
    else
    {
      PyObject* pyStringObject = PyObject_Str(pyValue);
      if(pyStringObject)
      {
        if(PyUnicode_Check(pyStringObject))
        {
          PyObject* pyStr = PyUnicode_AsEncodedString(pyStringObject, "utf-8", "Error ~");
          const auto pchResult = PyBytes_AS_STRING(pyStr);
          if(pchResult)
          {
            g_warning("result is %s", pchResult);
            Py_DECREF(pyStr);
          }
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

#if PY_MAJOR_VERSION < 3
  //There is no Py_FlushLine in Python 3
  //Py_FlushLine();
#endif
  PyErr_Clear();


  Py_Finalize();

}

int main ()
{
  //Glib::ustring func_impl = "import time\nreturn time.clock()";
  Glib::ustring func_impl = "count = 0\nfor i in range(0, 100): count += i\nreturn count";
  evaluate_function_implementation(func_impl);


  return EXIT_SUCCESS;
}
