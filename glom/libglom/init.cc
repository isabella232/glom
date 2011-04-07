/* Glom
 *
 * Copyright (C) 2001-2009 Murray Cumming
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

#include <Python.h> //Include it before anything else to avoid "_POSIX_C_SOURCE redefined".
#if PY_VERSION_HEX >= 0x02040000
# include <datetime.h> /* From Python */
#endif

#include <glom/libglom/connectionpool.h>
#include <giomm.h>
#include <libgdamm.h>
#include <iostream>

#include <pygobject.h>

//TODO: Remove this redefine when Python fixes the compiler error in their macro:
// http://bugs.python.org/issue7463
// Note that this sets a local copy of PyDateTimeAPI (in Python's datetime.h
// header) so it _must_ be repeated and called before any code that use the
// Python PyDate* macros (!) such as PyDateTime_Check
#undef PyDateTime_IMPORT
#define PyDateTime_IMPORT \
        PyDateTimeAPI = (PyDateTime_CAPI*) PyCObject_Import((char*)"datetime", \
                                                            (char*)"datetime_CAPI")

namespace Glom
{

void libglom_init()
{
  if (!Glib::thread_supported())
    Glib::thread_init(0); //So we can use GMutex.

  Gnome::Gda::init();
  Gio::init();

  Py_Initialize();
  PyDateTime_IMPORT; //A macro, needed to use PyDate_Check(), PyDateTime_Check(), etc.
  if(!PyDateTimeAPI)
  {
    //Give people a clue on stdout:
    std::cerr << G_STRFUNC << ": PyDateTime_IMPORT (a python module import) failed." << std::endl;

    //This gives more information. When this happens it is generally a linker
    //failure while importing a python module:
    //See https://bugzilla.gnome.org/show_bug.cgi?id=644702
    PyErr_Print();
  }

  //Initialize PyGObject, so that functions such as pygobject_new() work
  //instead of crashing.
  pygobject_init(2, 28, 0);
}

void libglom_deinit()
{
  //We use python for calculated-fields:
  Py_Finalize();

  //Clean up singletons:
  Glom::ConnectionPool::delete_instance();
}

} //namespace Glom
