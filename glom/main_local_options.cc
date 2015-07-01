/* Glom
 *
 * Copyright (C) 2012 Murray Cumming
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

#include "config.h"

#include <iostream>

#include <glom/main_local_options.h>

// For sanity checks:
#include <libglom/data_structure/glomconversions.h>
#include <glom/python_embed/glom_python.h>

#include <glibmm/optionentry.h>

#include <glibmm/i18n.h>

namespace Glom
{

LocalOptionGroup::LocalOptionGroup()
: Glib::OptionGroup("glom-extra", _("Extra Glom options"), _("Extra command-line options for glom")),
  m_arg_version(false),
  m_arg_debug_date_check(false),
  m_debug_date_check_result(false)
{
  Glib::OptionEntry entry;
  entry.set_long_name("version");
  entry.set_short_name('V');
  entry.set_description(_("The version of this application."));
  add_entry(entry, m_arg_version);

  entry.set_long_name("debug-date-check");
  entry.set_short_name(0);
  entry.set_description(_("Show how Glom outputs a date in this locale, then stop."));
  add_entry(entry, m_arg_debug_date_check);
}


bool LocalOptionGroup::handle_options()
{
  if(m_arg_version)
  {
    std::cout << PACKAGE_STRING << std::endl;
    return false; //Then stop.
  }
  
  // Some more sanity checking:
  // These print errors to the stdout if they fail.
  // In future we might refuse to start if they fail.
  m_debug_date_check_result = true;
  const bool test1 =
    Glom::Conversions::sanity_check_date_text_representation_uses_4_digit_years(m_arg_debug_date_check /* show debug output */);
  if(!test1)
  {
    std::cerr << G_STRFUNC << ": Glom: ERROR: Date presentation sanity checks failed. Glom will not display dates correctly. This needs attention from a translator. Please file a bug. See http://www.glom.org." << std::endl;
    m_debug_date_check_result = false;
  }

  const auto test2 = Glom::Conversions::sanity_check_date_parsing();
  if(!test2)
  {
    std::cerr << G_STRFUNC << ": Glom: ERROR: Date parsing sanity checks failed. Glom will not interpret dates correctly. This needs attention from a translator. Please file a bug. See http://www.glom.org." << std::endl;
    m_debug_date_check_result = false;
  }

  return true;
}

bool LocalOptionGroup::get_debug_date_check_result(bool& stop) const
{
  //This command-line option is documented as stopping executing after checking,
  //so that the execution result can be checked: 
  stop = m_arg_debug_date_check;
  
  return m_debug_date_check_result;
}


} //namespace Glom

