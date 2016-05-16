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

#ifndef GLOM_MAIN_LOCAL_OPTIONS_H
#define GLOM_MAIN_LOCAL_OPTIONS_H

#include <glibmm/optiongroup.h>

namespace Glom
{


//These options can be run by the local (short-lived) instance:
//However, real separation (or even real remote handling) of OptionGroups is
//not possible yet:
//https://bugzilla.gnome.org/show_bug.cgi?id=634990#c6
//This only works at all because we use Gio::APPLICATION_NON_UNIQUE .
class LocalOptionGroup : public Glib::OptionGroup
{
public:
  LocalOptionGroup();

  /**
   * @result If this is false then the GApplication, or main() should return EXIT_FAILURE.
   */
  bool handle_options();

  bool get_debug_date_check_result(bool& stop) const;

private:

  //These int instances should live as long as the OptionGroup to which they are added,
  //and as long as the OptionContext to which those OptionGroups are added.
  bool m_arg_version;
  bool m_arg_debug_date_check;

  bool m_debug_date_check_result;
};




} //namespace Glom

#endif //GLOM_MAIN_LOCAL_OPTIONS
