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

#include <glom/libglom/connectionpool.h>
#include <giomm.h>
#include <libgdamm.h>

namespace Glom
{

void libglom_init()
{
  g_thread_init(NULL); //So we can use GMutex.
  Gnome::Gda::init();
  Gio::init();

  Py_Initialize();
}

void libglom_deinit()
{
  //We use python for calculated-fields:
  Py_Finalize();

  //Clean up singletons:
  Glom::ConnectionPool::delete_instance();
}

} //namespace Glom

