/* Glom
 *
 * Copyright (C) 2011 Openismus GmbH
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

#ifndef GLOM_SHOW_PROGRESS_MESSAGE_H
#define GLOM_SHOW_PROGRESS_MESSAGE_H

#include "config.h" // For GLOM_ENABLE_CLIENT_ONLY
#include <glibmm/ustring.h>

namespace Glom
{

class AppWindow;

/** Use this class to ensure that the progress message is cleared upon exiting a
 * method with multiple return points.
 */
class ShowProgressMessage
{
public:
  explicit ShowProgressMessage(const Glib::ustring &message);
  ~ShowProgressMessage();

  void pulse();

private:
  AppWindow* const m_app;
  Glib::ustring m_message;
};

} //namespace Glom

#endif // GLOM_SHOW_PROGRESS_MESSAGE_H
