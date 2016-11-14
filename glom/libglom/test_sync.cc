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

#include <libglom/spawn_with_feedback.h>
#include <iostream>

int
main(int argc, char* argv[])
{
  Glib::init();


  const Glib::ustring command = "sh `LANG=C;LANGUAGE=C;echo $LANG;echo $LANGUAGE`";
  std::cout << "command=" << command << std::endl;
  try
  {
    std::string stdout_output;
    int return_status = 0;
    Glib::spawn_command_line_sync(command, &stdout_output, 0, &return_status);
    std::cout << " debug: output=" << stdout_output << std::endl;
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << ": Exception caught: " << ex.what() << std::endl;
  }

  return 0;
}





