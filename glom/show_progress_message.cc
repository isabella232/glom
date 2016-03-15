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

#include "config.h" // For GLOM_ENABLE_CLIENT_ONLY

#include <glom/show_progress_message.h>
#include <glom/appwindow.h>


namespace Glom
{

ShowProgressMessage::ShowProgressMessage(const Glib::ustring& message)
: m_app(AppWindow::get_appwindow()),
  m_message(message)
{
  g_return_if_fail(m_app);
  m_app->set_progress_message(message);
}

ShowProgressMessage::~ShowProgressMessage()
{
  g_return_if_fail(m_app);
  m_app->clear_progress_message();
}

void ShowProgressMessage::pulse()
{
  g_return_if_fail(m_app);
  m_app->set_progress_message(m_message);
}

} //namespace Glom

