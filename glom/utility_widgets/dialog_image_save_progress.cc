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

#include "dialog_image_save_progress.h"

#include <gtkmm/messagedialog.h>
#include <glibmm/main.h>
#include <iostream>
#include <glibmm/i18n.h>

namespace
{

// Write the file in chunks of this size:
const unsigned int CHUNK_SIZE = 2048;

} // anonymous namespace

namespace Glom
{

const char* DialogImageSaveProgress::glade_id("dialog_image_save_progress");
const bool DialogImageSaveProgress::glade_developer(false);

DialogImageSaveProgress::DialogImageSaveProgress(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Dialog(cobject),
  m_data(nullptr)
{
  builder->get_widget("progress_bar", m_progress_bar);

  if(!m_progress_bar)
  {
    std::cerr << G_STRFUNC << ": Missing widgets from glade file for DialogImageSaveProgress\n";
  }
}

void DialogImageSaveProgress::save(const Glib::ustring& uri)
{
  g_assert(m_data);

  const auto data = gda_binary_get_data(const_cast<GdaBinary*>(m_data));
  if(!data)
    return;

  const auto data_len = gda_binary_get_size(const_cast<GdaBinary*>(m_data));
  if(data_len == 0)
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
  try
  {
    m_stream->write_async(data,
      std::min<gsize>(CHUNK_SIZE, data_len),
      sigc::bind(sigc::mem_fun(*this, &DialogImageSaveProgress::on_stream_write), 0));
  }
  catch(const Gio::Error& ex)
  {
    std::cerr << G_STRFUNC << ": exception: " << ex.what() << std::endl;
    response(Gtk::RESPONSE_REJECT);
    return;
  }
}

void DialogImageSaveProgress::on_stream_write(const Glib::RefPtr<Gio::AsyncResult>& result, unsigned int offset)
{
  try
  {
    const auto size = m_stream->write_finish(result);
    g_assert(size >= 0); // Would have thrown an exception otherwise

    // Set progress
    const auto data_len = gda_binary_get_size(const_cast<GdaBinary*>(m_data));
    m_progress_bar->set_fraction(static_cast<double>(offset + size) / data_len);

    // Write next chunk, if any
    if(  static_cast<gssize>(offset + size) < static_cast<gssize>(data_len))
      // Even if choose a priority lower than GDK_PRIORITY_REDRAW + 10 for the
      // write_async we don't see the progressbar progressing while the image
      // is loading. Therefore we put an idle inbetween.
      Glib::signal_idle().connect(sigc::bind_return(sigc::bind(sigc::mem_fun(*this, &DialogImageSaveProgress::on_write_next), offset + size), false));
    else
      // We are done saving the image, close the progress dialog
      response(Gtk::RESPONSE_ACCEPT);
  }
  catch(const Glib::Error& ex)
  {
    error(ex.what());
    response(Gtk::RESPONSE_REJECT);
  }
}

void DialogImageSaveProgress::error(const Glib::ustring& error_message)
{
  Gtk::MessageDialog dialog(*this, _("Error Saving"), Gtk::MESSAGE_ERROR);
  dialog.set_title(_("Error saving image"));
  dialog.set_secondary_text(error_message);

  dialog.run();
  response(Gtk::RESPONSE_REJECT);
}

void DialogImageSaveProgress::on_write_next(unsigned int at)
{
  const auto data_len = gda_binary_get_size(const_cast<GdaBinary*>(m_data));
  g_assert(at < static_cast<gsize>(data_len));

  const auto data = gda_binary_get_data(const_cast<GdaBinary*>(m_data));
  m_stream->write_async(static_cast<const guchar*>(data) + at, std::min<gsize>(CHUNK_SIZE, data_len - at),
    sigc::bind(sigc::mem_fun(*this, &DialogImageSaveProgress::on_stream_write), at));
}

void DialogImageSaveProgress::set_image_data(const GdaBinary* data)
{
  m_data = data;
}

} // namespace Glom
