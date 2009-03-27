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

#include <glom/dialog_progress_creating.h>
#include <gtkmm/main.h>
#include <gtkmm/dialog.h>
#include <glibmm/i18n.h>

namespace Glom
{

Dialog_ProgressCreating::Dialog_ProgressCreating(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Window(cobject),
  m_progress(0),
  m_label_message(0),
  m_response_id(Gtk::RESPONSE_OK),
  m_running(false)
{
  //set_modal();
  builder->get_widget("progressbar", m_progress);
  builder->get_widget("label_message", m_label_message);
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

  // People are supposed to either use run(), or run their own mainloop.
  // This method is a rather bad hack. I changed spawn.cc to use this
  // properly, and it does not seem to be used elsewhere. armin.

  //while(Gtk::Main::instance()->events_pending())
  //  Gtk::Main::instance()->iteration();
}

void Dialog_ProgressCreating::set_message(const Glib::ustring& title, const Glib::ustring& secondary_text)
{
  set_title(title);
  m_label_message->set_text(secondary_text);
}

void Dialog_ProgressCreating::response(int response_id)
{
  if(!m_running)
   return;

  m_response_id = response_id;
  Gtk::Main::quit();
  m_running = false;
}

int Dialog_ProgressCreating::run()
{
  // Cannot nest
  if(m_running)
    return Gtk::RESPONSE_CANCEL;

  show();
  m_running = true;
  Gtk::Main::run();
  return m_response_id;
}

bool Dialog_ProgressCreating::on_delete_event(GdkEventAny* /* event */)
{
  if(m_running)
    response(Gtk::RESPONSE_DELETE_EVENT);

  return true;
}

} //namespace Glom
