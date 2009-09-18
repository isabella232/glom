#include <Python.h>
#include <cstdio>
#include <libglom/init.h>

#include "config.h"
#include "glom/python_embed/glom_python.h"

#ifndef G_OS_WIN32
extern "C" void __libc_freeres(void);
#endif

namespace Glom
{
bool glom_python_module_is_available()
{
  const gchar* name = "glom_" GLOM_ABI_VERSION_UNDERLINED;
  PyObject* module_glom = PyImport_ImportModule((char*)name); //TODO: unref this?

  if(!module_glom)
  {
    g_warning("Glom: A python import of %s failed.\n", name);
  }

  return module_glom != 0;
}

bool gda_python_module_is_available()
{
  const gchar* name = "gda";
  PyObject* module_glom = PyImport_ImportModule((char*)name); //TODO: unref this?

  if(!module_glom)
  {
    g_warning("Glom: A python import of %s failed.\n", name);
  }

  return module_glom != 0;
}

}

int main ()
{
#ifndef G_OS_WIN32
  atexit(__libc_freeres);
#endif
  Glom::libglom_init();  // Calls PyInitialize()
  
  if (!Glom::gda_python_module_is_available())
    return EXIT_FAILURE;
  if (!Glom::glom_python_module_is_available())
    return EXIT_FAILURE;
  
  Glom::libglom_deinit(); // Calls Py_Finalize();
  
  return EXIT_SUCCESS;
}
