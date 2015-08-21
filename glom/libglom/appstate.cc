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
#include <libglom/appstate.h>


namespace Glom
{

AppState::AppState()
: m_userlevel(userlevels::DEVELOPER)
{
}

AppState::~AppState()
{

}

AppState::userlevels AppState::get_userlevel() const
{
  return m_userlevel;
}

void AppState::set_userlevel(userlevels value)
{
  if(m_userlevel != value)
  {
    m_userlevel = value;

    //Tell the UI to respond accordingly:
    m_signal_userlevel_changed.emit(value);
  }
}

AppState::type_signal_userlevel_changed AppState::signal_userlevel_changed()
{
  return m_signal_userlevel_changed;
}

void AppState::emit_userlevel_changed()
{
   m_signal_userlevel_changed.emit(m_userlevel);
}

} //namespace Glom

