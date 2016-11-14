#include <cstdio>
#include <cstdlib>
#include <dlfcn.h>
#include "config.h"

int main()
{
  // Attempt to dynamically load the python module,
  // attempting to resolve all symbols immediately:
  const char* path = "glom/python_embed/python_module/.libs/glom_" GLOM_ABI_VERSION_UNDERLINED ".so";;
  void* lib = dlopen(path, RTLD_NOW);

  if(!lib)
  {
    const auto *error = dlerror();
    if(error)
    {
      fprintf (stderr, "%s\n", error);
    }

    return EXIT_FAILURE;
  }
  else
    dlclose(lib);

  return EXIT_SUCCESS;
}
