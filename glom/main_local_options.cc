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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "config.h"

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
  m_arg_debug_date_check(false)
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


bool main_handle_local_options(const LocalOptionGroup& group)
{
  if(group.m_arg_version)
  {
    std::cout << PACKAGE_STRING << std::endl;
    return EXIT_SUCCESS;
  }
  
  // Some more sanity checking:
  // These print errors to the stdout if they fail.
  // In future we might refuse to start if they fail.
  bool date_check_ok = true;
  const bool test1 =
    Glom::Conversions::sanity_check_date_text_representation_uses_4_digit_years(group.m_arg_debug_date_check /* show debug output */);
  if(!test1)
  {
    std::cerr << "Glom: ERROR: Date presentation sanity checks failed. Glom will not display dates correctly. This needs attention from a translator. Please file a bug. See http://www.glom.org." << std::endl;
    date_check_ok = false;
  }

  const bool test2 = Glom::Conversions::sanity_check_date_parsing();
  if(!test2)
  {
    std::cerr << "Glom: ERROR: Date parsing sanity checks failed. Glom will not interpret dates correctly. This needs attention from a translator. Please file a bug. See http://www.glom.org." << std::endl;
    date_check_ok = false;
  }

  if(group.m_arg_debug_date_check)
  {
    return date_check_ok; //This command-line option is documented as stopping afterwards.
  }
  
  return true;
}


} //namespace Glom

