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

#ifndef GLOM_UTILITY_WIDGETS_DIALOG_IMAGE_SAVE_PROGRESS_H
#define GLOM_UTILITY_WIDGETS_DIALOG_IMAGE_SAVE_PROGRESS_H

#include <gtkmm/dialog.h>
#include <gtkmm/builder.h>
#include <gtkmm/progressbar.h>
#include <giomm/file.h>
#include <giomm/fileoutputstream.h>
#include <libgda/libgda.h>

namespace Glom
{

class DialogImageSaveProgress : public Gtk::Dialog
{
public:
  static const char* glade_id;
  static const bool glade_developer;

  DialogImageSaveProgress(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

  void save(const Glib::ustring& uri);

  void set_image_data(const GdaBinary& data);

private:
  void on_stream_write(const Glib::RefPtr<Gio::AsyncResult>& result, unsigned int offset);
  void error(const Glib::ustring& error_message);
  void on_write_next(unsigned int at);

  Gtk::ProgressBar* m_progress_bar;
  const GdaBinary* m_data;

  Glib::RefPtr<Gio::File> m_file;
  Glib::RefPtr<Gio::FileOutputStream> m_stream;
};

} //namespace Glom

#endif // GLOM_UTILITY_WIDGETS_DIALOG_IMAGE_SAVE_PROGRESS_H
