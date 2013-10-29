/* Glom
 *
 * Copyright (C) 2012 Openismus GmbH
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

#include <libglom/utils.h>
#include <libglom/init.h>
#include <iostream>

int main()
{
  Glom::libglom_init();

  Glib::ustring script;

  if(!Glom::Utils::script_check_for_pygtk2(script))
  {
    std::cerr << G_STRFUNC << ": script_check_for_pygtk2() failed unexpectedly." << std::endl;
    return EXIT_FAILURE;
  }

  script =
    "from gi.repository import Gtk";

  if(!Glom::Utils::script_check_for_pygtk2(script))
  {
    std::cerr << G_STRFUNC << ": script_check_for_pygtk2() failed unexpectedly." << std::endl;
    return EXIT_FAILURE;
  }

  script =
    "import pygtk\n" \
    "pygtk.require('2.0')\n" \
    "import gtk";

  if(Glom::Utils::script_check_for_pygtk2(script))
  {
    std::cerr << G_STRFUNC << ": script_check_for_pygtk2() succeeded unexpectedly." << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

