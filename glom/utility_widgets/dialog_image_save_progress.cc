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
: Gtk::Dialog(cobject),
  m_data(0)
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
  g_assert(m_data);

  if(m_data->data == 0)
    return;
    
  if(m_data->binary_length == 0)
    return;

  m_file = Gio::File::create_for_uri(uri);
  m_progress_bar->set_text(Glib::ustring::compose("Saving %1...", m_file->get_parse_name()));

  m_stream.reset();
   
  try
  {
    if(m_file->query_exists())
    {
      m_stream = m_file->replace(); //Instead of append_to().
    }
    else
    {
      //By default files created are generally readable by everyone, but if we pass FILE_CREATE_PRIVATE in flags the file will be made readable only to the current user, to the level that is supported on the target filesystem.
      //TODO: Do we want to specify 0660 exactly? (means "this user and his group can read and write this non-executable file".)
      m_stream = m_file->create_file();
    }
  }
  catch(const Gio::Error& ex)
  {
    std::cerr << G_STRFUNC << ": exception: " << ex.what() << std::endl;
    response(Gtk::RESPONSE_REJECT);
    return;
  }
  
  //Write the data to the output uri
  gssize bytes_written = 0;
  try
  {
    bytes_written = m_stream->write(m_data->data, m_data->binary_length);
  }
  catch(const Gio::Error& ex)
  {
    std::cerr << G_STRFUNC << ": exception: " << ex.what() << std::endl;
    response(Gtk::RESPONSE_REJECT);
    return;
  }

  if(bytes_written != m_data->binary_length)
  {
    std::cerr << G_STRFUNC << ": unexpected number of bytes written: bytes_written=" << bytes_written <<
       ", binary_length=" << m_data->binary_length << std::endl;
  }
  
  response(Gtk::RESPONSE_ACCEPT);
}

void DialogImageSaveProgress::error(const Glib::ustring& error_message)
{
  Gtk::MessageDialog dialog(*this, _("Error Saving"), Gtk::MESSAGE_ERROR);
  dialog.set_title(_("Error saving image"));
  dialog.set_secondary_text(error_message);

  dialog.run();
  response(Gtk::RESPONSE_REJECT);
}


void DialogImageSaveProgress::set_image_data(const GdaBinary& data)
{
  m_data = &data;
}

} // namespace Glom
