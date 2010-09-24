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

#ifndef GLOM_UTILITY_WIDGETS_DIALOG_IMAGE_PROGRESS_H
#define GLOM_UTILITY_WIDGETS_DIALOG_IMAGE_PROGRESS_H

#include <gtkmm/dialog.h>
#include <gtkmm/builder.h>
#include <gtkmm/progressbar.h>
#include <gdkmm/pixbufloader.h>
#include <giomm/file.h>
#include <giomm/fileinputstream.h>
#include <libgda/libgda.h>
#include <memory>

namespace Glom
{

class Dialog_Image_Progress : public Gtk::Dialog
{
public:
  static const char* glade_id;
  static const bool glade_developer;

  Dialog_Image_Progress(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~Dialog_Image_Progress();

  void load(const Glib::ustring& uri);

  std::auto_ptr<GdaBinary> get_image_data() { return m_data; }
  Glib::RefPtr<Gdk::Pixbuf> get_pixbuf() { return m_loader->get_pixbuf(); }

private:
  void error(const Glib::ustring& error_message);

  void on_file_read(const Glib::RefPtr<Gio::AsyncResult>& result);
  void on_query_info(const Glib::RefPtr<Gio::AsyncResult>& result);
  void on_stream_read(const Glib::RefPtr<Gio::AsyncResult>& result, unsigned int offset);
  void on_read_next(unsigned int at);

  Glib::RefPtr<Gdk::PixbufLoader> m_loader;
  std::auto_ptr<GdaBinary> m_data;
  Gtk::ProgressBar* m_progress_bar;

  Glib::RefPtr<Gio::File> m_file;
  Glib::RefPtr<Gio::FileInputStream> m_stream;
  unsigned int m_stream_size;
};

} //namespace Glom

#endif // GLOM_UTILITY_WIDGETS_DIALOG_IMAGE_PROGRESS_H
