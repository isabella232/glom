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

#include <libglom/libglom_config.h>

#include <libglom/utils.h>
#include <libglom/privs.h>
#include <libglom/connectionpool.h>
#include <libglom/data_structure/glomconversions.h>
#include <libglom/string_utils.h>

#include <giomm/file.h>
#include <giomm/resource.h>
#include <glibmm/convert.h>
#include <glibmm/fileutils.h>
#include <glibmm/miscutils.h>

#include <iostream>
#include <fstream>

#include <locale>     // for locale, time_put
#include <iostream>   // for cout, endl
#include <iomanip>

#include <stack>

#include <fcntl.h> //For close(). This include is needed in mingw on MS Windows.

namespace Glom
{

/** Get just the first part of a locale, such as de_DE,
 * ignoring, for instance, .UTF-8 or \@euro at the end.
 */
Glib::ustring Utils::locale_simplify(const Glib::ustring& locale_id)
{
  Glib::ustring result = locale_id;

  //At least Ubuntu Natty provides a long string such as this: LC_CTYPE=en_US.UTF-8;LC_NUMERIC=en_US.UTF-8;LC_TIME=en_US.UTF-8;LC_COLLATE=en_US.UTF-8;LC_MONETARY=en_US.UTF-8;LC_MESSAGES=en_AG.utf8;LC_PAPER=en_US.UTF-8;LC_NAME=en_US.UTF-8;LC_ADDRESS=en_US.UTF-8;LC_TELEPHONE=en_US.UTF-8;LC_MEASUREMENT=en_US.UTF-8;LC_IDENTIFICATION=en_US.UTF-8
  //In Ubuntu Maverick, and earlier, it was apparently a simple string such as en_US.UTF-8.

  //Look for LC_ALL or LC_COLLATE
  //(We use the locale name only to identify translations
  //Otherwise just start with the whole string.
  Glib::ustring::size_type posCategory = result.find("LC_ALL=");
  if(posCategory != Glib::ustring::npos)
  {
    result = result.substr(posCategory);
  }
  else
  {
    posCategory = result.find("LC_COLLATE=");
    if(posCategory != Glib::ustring::npos)
    {
      result = result.substr(posCategory);
    }
  }

  //Get everything before the .:
  const Glib::ustring::size_type posDot = result.find('.');
  if(posDot != Glib::ustring::npos)
  {
    result = result.substr(0, posDot);
  }

  //Get everything before the @:
  const Glib::ustring::size_type posAt = result.find('@');
  if(posAt != Glib::ustring::npos)
  {
    result = result.substr(0, posAt);
  }

  //Get everything after the =, if any:
  const Glib::ustring::size_type posEquals = result.find('=');
  if(posEquals != Glib::ustring::npos)
  {
    result = result.substr(posEquals + 1);
  }

  return result;
}

Glib::ustring Utils::locale_language_id(const Glib::ustring& locale_id)
{
  const Glib::ustring::size_type posUnderscore = locale_id.find('_');
  if(posUnderscore != Glib::ustring::npos)
  {
    return locale_id.substr(0, posUnderscore);
  }
  else
  {
    //We assume that this locale ID specifies a language but no specific country.
    return locale_id;
  }
}

bool Utils::script_check_for_pygtk2(const Glib::ustring& script)
{
  //There is probably other code that this will not catch,
  //but this is better than nothing.
  //TODO: Instead override python's import mechanism somehow?
  if(script.find("import pygtk") != std::string::npos)
    return false;

  return true;
}

bool Utils::get_resource_exists(const std::string& resource_path)
{
  return Gio::Resource::get_file_exists_global_nothrow(resource_path);
}

} //namespace Glom
