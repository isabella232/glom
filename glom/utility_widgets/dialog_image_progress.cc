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

#include "dialog_image_progress.h"

#include <gtkmm/messagedialog.h>
#include <glibmm/i18n.h>

namespace
{

// Read the file in chunks of this size:
const unsigned int CHUNK_SIZE = 2048;

} // anonymous namespace

namespace Glom
{

Dialog_Image_Progress::Dialog_Image_Progress(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Dialog(cobject)
{
  builder->get_widget("image_loading_progress_bar", m_progress_bar);

  if(!m_progress_bar)
    throw std::runtime_error("Missing widgets from glade file for Dialog_Image_Progress");
}

Dialog_Image_Progress::~Dialog_Image_Progress()
{
  if(m_data.get())
    g_free(m_data->data);
  if(m_loader)
  {
    try
    {
      m_loader->close();
    }
    catch(const Glib::Error& ex)
    {
      // Ignore error, it's normal for close() to through when the image has
      // not yet been loaded completely, for example when cancelling the
      // dialog.
    }
  }

  // TODO: Cancel outstanding async operations in destructor?
}

void Dialog_Image_Progress::load(const Glib::ustring& uri)
{
  // Can only load one file with data 
  g_assert(!m_data.get());

  m_loader = Gdk::PixbufLoader::create();
  m_data.reset(new GdaBinary);
  m_data->data = NULL;
  m_data->binary_length = 0;

  m_file = Gio::File::create_for_uri(uri);
  m_progress_bar->set_text(Glib::ustring::compose("Loading %1...", m_file->get_parse_name()));

  try
  {
    // Open the file for reading:
    m_file->read_async(sigc::mem_fun(*this, &Dialog_Image_Progress::on_file_read));
  }
  catch(const Glib::Error& ex)
  {
    error(ex.what());
  }
}

void Dialog_Image_Progress::on_file_read(const Glib::RefPtr<Gio::AsyncResult>& result)
{
  try
  {
    m_stream = m_file->read_finish(result);
    // Query size of the file, so that we can show progress:
    m_stream->query_info_async(sigc::mem_fun(*this, &Dialog_Image_Progress::on_query_info), G_FILE_ATTRIBUTE_STANDARD_SIZE);
  }
  catch(const Glib::Error& ex)
  {
    error(ex.what());
  }
}

void Dialog_Image_Progress::on_query_info(const Glib::RefPtr<Gio::AsyncResult>& result)
{
  try
  {
    Glib::RefPtr<Gio::FileInfo> info = m_stream->query_info_finish(result);
    m_data->binary_length = info->get_size();
    // We need to use the glib allocater here:
    m_data->data = static_cast<guchar*>(g_try_malloc(m_data->binary_length));
    if(!m_data->data)
      error(_("Not enough memory available to load the image"));

    // Read the first chunk from the file
    m_stream->read_async(m_data->data, std::min<gsize>(CHUNK_SIZE, m_data->binary_length), sigc::bind(sigc::mem_fun(*this, &Dialog_Image_Progress::on_stream_read), 0));
  }
  catch(const Glib::Error& ex)
  {
    error(ex.what());
  }
}

void Dialog_Image_Progress::on_stream_read(const Glib::RefPtr<Gio::AsyncResult>& result, unsigned int offset)
{
  try
  {
    gssize size = m_stream->read_finish(result);
    g_assert(size >= 0); // Would have thrown an exception otherwise
    // Cannot read more data than there is available in the file:
    g_assert(offset + size <= static_cast<gsize>(m_data->binary_length));
    // Load image
    m_loader->write(m_data->data + offset, size);
    // Set progress
    m_progress_bar->set_fraction(static_cast<double>(offset + size) / m_data->binary_length);
    // Read next chunk, if any
    if(offset + size < static_cast<gsize>(m_data->binary_length))
      // Even if choose a priority lower than GDK_PRIORITY_REDRAW + 10 for the
      // read_async we don't see the progressbar progressing while the image
      // is loading. Therefore we put an idle inbetween.
      Glib::signal_idle().connect(sigc::bind_return(sigc::bind(sigc::mem_fun(*this, &Dialog_Image_Progress::on_read_next), offset + size), false));
    else
      // We are done loading the image, close the progress dialog
      response(Gtk::RESPONSE_ACCEPT);
  }
  catch(const Glib::Error& ex)
  {
    error(ex.what());
  }
}

void Dialog_Image_Progress::on_read_next(unsigned int at)
{
  g_assert(at < static_cast<gsize>(m_data->binary_length));

  m_stream->read_async(m_data->data + at, std::min<gsize>(CHUNK_SIZE, m_data->binary_length - at), sigc::bind(sigc::mem_fun(*this, &Dialog_Image_Progress::on_stream_read), at));
}

void Dialog_Image_Progress::error(const Glib::ustring& error_message)
{
  Gtk::MessageDialog dialog(*this, Glib::ustring::compose(_("Error loading %1"), m_file->get_parse_name()), Gtk::MESSAGE_ERROR);
  dialog.set_title(_("Error loading image"));
  dialog.set_secondary_text(error_message);

  dialog.run();
  response(Gtk::RESPONSE_REJECT);
}

} // namespace Glom
