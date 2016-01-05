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

#ifndef GLOM_APPSTATE_H
#define GLOM_APPSTATE_H

#include <sigc++/sigc++.h>

namespace Glom
{

/** There is one instance per document.
 * This is for storing volatile application state.
 * It is not for configuration that should be the same after the application is closed and restarted - use gconf for that.
 */
class AppState
{
public:
  AppState();

  enum class userlevels
  {
    OPERATOR,
    DEVELOPER
  };

  /** Returns whether we are in developer mode.
   * Some functionality will be deactivated when not in developer mode.
   */
  userlevels get_userlevel() const;

  /** This will cause the userlevel_changed signal to be emitted.
  */
  void set_userlevel(userlevels value);

  /// Use this to set the initial UI state:
  void emit_userlevel_changed();

  typedef sigc::signal<void, userlevels> type_signal_userlevel_changed;

  /// The user interface should handle this signal and alter itself accordingly.
  type_signal_userlevel_changed signal_userlevel_changed();

private:

  userlevels m_userlevel;
  type_signal_userlevel_changed m_signal_userlevel_changed;
};

} //namespace Glom

#endif //GLOM_APPSTATE_H
