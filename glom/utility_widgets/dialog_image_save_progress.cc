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

#include "dialog_image_save_progress.h"

#include <gtkmm/messagedialog.h>
#include <iostream>
#include <glibmm/i18n.h>

namespace Glom
{

const char* DialogImageSaveProgress::glade_id("dialog_image_save_progress");
const bool DialogImageSaveProgress::glade_developer(false);

DialogImageSaveProgress::DialogImageSaveProgress(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Dialog(cobject)
{
  builder->get_widget("progress_bar", m_progress_bar);

  if(!m_progress_bar)
    throw std::runtime_error("Missing widgets from glade file for DialogImageSaveProgress");
}

DialogImageSaveProgress::~DialogImageSaveProgress()
{
}

void DialogImageSaveProgress::save(const Glib::ustring& uri)
{
  //TODO: Support non-local URIs when we do this properly, using Gio::File.
  const std::string filepath = Glib::filename_from_uri(uri);
  m_progress_bar->set_text(Glib::ustring::compose("Saving %1...", 
    Glib::filename_display_basename(filepath)));

  try
  {
    // Open the file for reading:
    m_pixbuf->save(filepath, GLOM_IMAGE_FORMAT);
  }
  catch(const Glib::Error& ex)
  {
    error(ex.what());
  }
  
  //response(Gtk::RESPONSE_ACCEPT);
}

void DialogImageSaveProgress::error(const Glib::ustring& error_message)
{
  Gtk::MessageDialog dialog(*this, _("Error Saving"), Gtk::MESSAGE_ERROR);
  dialog.set_title(_("Error saving image"));
  dialog.set_secondary_text(error_message);

  dialog.run();
  response(Gtk::RESPONSE_REJECT);
}


void DialogImageSaveProgress::set_pixbuf(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf)
{
  m_pixbuf = pixbuf;
}

} // namespace Glom
