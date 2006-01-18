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

#include "dialog_progress_creating.h"
#include "gtkmm/main.h"
#include <glibmm/i18n.h>

Dialog_ProgressCreating::Dialog_ProgressCreating(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::Window(cobject),
  m_progress(0)
{
  //set_modal();
  refGlade->get_widget("progressbar", m_progress);
  //m_progress->show();
}

Dialog_ProgressCreating::~Dialog_ProgressCreating()
{
}

void Dialog_ProgressCreating::pulse()
{
  m_progress->pulse();
  raise();

  //Allow GTK+ to perform all updates for us
  //Without this, the progress bar will appear to do nothing.
  while(Gtk::Main::instance()->events_pending())
    Gtk::Main::instance()->iteration();
}
