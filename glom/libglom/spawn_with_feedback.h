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

#ifndef GLOM_SPAWN_WITH_FEEDBACK_H
#define GLOM_SPAWN_WITH_FEEDBACK_H

#include <glibmm/ustring.h>
#include <functional>

namespace Glom
{

namespace Spawn
{

/** This callback should show UI to indicate that work is still happening.
 * For instance, a pulsing ProgressBar.
 */
typedef std::function<void()> SlotProgress;

/** Execute a command-line command, and wait for it to return.
 * @param command The command-line command.
 * @param message A human-readable message to be shown, for instance in a dialog, while waiting.
 * @slot_progress A callback to call while the work is still happening.
 */
bool execute_command_line_and_wait(const std::string& command, const SlotProgress& slot_progress);

/** Execute a command-line command, and wait for it to return.
 * @param command The command-line command.
 * @param message A human-readable message to be shown, for instance in a dialog, while waiting.
 * @slot_progress A callback to call while the work is still happening.
 * @output The stdout output of the command.
 */
bool execute_command_line_and_wait(const std::string& command, const SlotProgress& slot_progress, std::string& output);

/** Execute a command-line command, and repeatedly call a second command that tests whether the first command has finished.
 * @param command The command-line command.
 * @param message A human-readable message to be shown, for instance in a dialog, while waiting. 
 * @slot_progress A callback to call while the work is still happening.
 * @success_text If this is not empty, then the second command will only be considered to have succeeded when this text is found in its stdout output.
 */
bool execute_command_line_and_wait_until_second_command_returns_success(const std::string& command, const std::string& second_command, const SlotProgress& slot_progress, const std::string& success_text = std::string());


} //Spawn

} //Glom

#endif //GLOM_SPAWN_WITH_FEEDBACK_H
