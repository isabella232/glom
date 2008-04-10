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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef GLOM_SPAWN_WITH_FEEDBACK_H
#define GLOM_SPAWN_WITH_FEEDBACK_H

#include <gtkmm/window.h>

namespace Glom
{

namespace Spawn
{


/** Execute a command-line command, and wait for it to return.
 * @param command The command-line command.
 * @param message A human-readable message to be shown, for instance in a dialog, while waiting.
 * @parent_window Make the dialog transient to this window.
 */
bool execute_command_line_and_wait(const std::string& command, const Glib::ustring& message, Gtk::Window* parent_window);

/** Execute a command-line command, and repeatedly call a second command that tests whether the first command has finished.
 * @param command The command-line command.
 * @param message A human-readable message to be shown, for instance in a dialog, while waiting. 
 * @parent_window Make the dialog transient to this window.
 * @success_text If this is not empty, then the second command will only be considered to have succeeded when this text is found in its stdout output.
 */
bool execute_command_line_and_wait_until_second_command_returns_success(const std::string& command, const std::string& second_command, const Glib::ustring& message, Gtk::Window* parent_window, const std::string& success_text = std::string());

//bool execute_command_line_and_wait_fixed_seconds(const std::string& command, unsigned int seconds, const Glib::ustring& message, Gtk::Window* parent_window);


} //Spawn

} //Glom

#endif //GLOM_SPAWN_WITH_FEEDBACK_H
