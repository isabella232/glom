#include <Python.h>
#include <cstdio>
#include <libglom/init.h>
#include <iostream>

#include "config.h"
#include "glom/python_embed/glom_python.h"

#ifdef __linux__
extern "C" void __libc_freeres(void);
#endif

namespace Glom
{
bool glom_python_module_is_available()
{
  const gchar* name = "glom_" GLOM_ABI_VERSION_UNDERLINED;
  PyObject* module_glom = PyImport_ImportModule((char*)name);

  if(!module_glom)
  {
    std::cerr << "Glom: A python import of " << name << " failed.\n";
  }

  Py_XDECREF(module_glom);

  return module_glom != nullptr;
}

bool gda_python_module_is_available()
{
  //TODO: How can we requests a specific version to avoid confusion
  //between the parallel-installed Gda-5.0 and Gda-6.0 APIs?

  //Python code usually uses "from gi.repository import Gda" so that
  //the code may use Gda. rather than gi.repository.Gda in the code.
  const gchar* name = "gi.repository.Gda";
  PyObject* module_glom = PyImport_ImportModule((char*)name);

  if(!module_glom)
  {
    std::cerr << "Glom: A python import of " << name << " failed.\n";
  }

  Py_XDECREF(module_glom);

  return module_glom != nullptr;
}

}

int main ()
{
#ifdef __linux__
  atexit(__libc_freeres);
#endif
  Glom::libglom_init();  // Calls PyInitialize()

  if(!Glom::gda_python_module_is_available())
    return EXIT_FAILURE;
  if(!Glom::glom_python_module_is_available())
    return EXIT_FAILURE;

  Glom::libglom_deinit(); // Calls Py_Finalize();

  return EXIT_SUCCESS;
}
