#include <Python.h>
#include <glib.h> //For g_warning().


void evaluate_function_implementation(const Glib::ustring& func_impl)
{
  Py_Initialize();
  //PyDateTime_IMPORT; //A macro, needed to use PyDate_Check(), PyDateTime_Check(), etc.

  PyObject* pMain = PyImport_AddModule("__main__");
  PyObject* pDict = PyModule_GetDict(pMain);

  PyObject* pyValue = PyRun_String("123", Py_eval_input /* evaluate a single statement, and return value */, pDict, pDict);

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
      if(PyString_Check(pyStringObject))
      {
        const auto pchResult = PyString_AsString(pyStringObject);
        if(pchResult)
          g_warning("result is %s", pchResult);
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

#if PY_MAJOR_VERSION < 3
  //There is no Py_FlushLine in Python 3
  //Py_FlushLine();
#endif
  PyErr_Clear();


  Py_Finalize();

}

int main ()
{
  Glib::ustring func_impl = "import time\nreturn time.clock()";
  evaluate_function_implementation(func_impl);


  return EXIT_SUCCESS;
}
