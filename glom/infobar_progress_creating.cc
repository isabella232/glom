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

#include <gtkmm/dialog.h>
#include <glom/application.h>
#include <glom/infobar_progress_creating.h>
#include <gtkmm/main.h>
#include <glibmm/i18n.h>

namespace Glom
{

const char* Infobar_ProgressCreating::glade_id("infobar_progress");
const bool Infobar_ProgressCreating::glade_developer(false);

Infobar_ProgressCreating::Infobar_ProgressCreating(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::InfoBar(cobject),
  m_progress(0),
  m_label_message(0)
{
  builder->get_widget("progressbar", m_progress);
  builder->get_widget("label_message", m_label_message);
}

Infobar_ProgressCreating::~Infobar_ProgressCreating()
{
}

void Infobar_ProgressCreating::pulse()
{
  m_progress->pulse();
}

void Infobar_ProgressCreating::set_message(const Glib::ustring& title, const Glib::ustring& secondary_text)
{
  m_label_message->set_text("<b>" + title + "</b>\n\n" + secondary_text);
}

void Infobar_ProgressCreating::set_message(const Glib::ustring& secondary_text)
{
  set_message(_("Processing"), secondary_text);
}

} //namespace Glom
