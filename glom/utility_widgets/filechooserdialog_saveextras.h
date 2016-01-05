/* Glom
 *
 * Copyright (C) 2006 Murray Cumming
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

#ifndef GLOM_UTILITY_WIDGETS_FILECHOOSERDIALOG_SAVEEXTRAS_H
#define GLOM_UTILITY_WIDGETS_FILECHOOSERDIALOG_SAVEEXTRAS_H

#include "config.h" //For GLOM_ENABLE_SQLITE
#include "config.h" // For GLOM_ENABLE_CLIENT_ONLY,

#include <libglom/document/document.h>

#include <gtkmm/box.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/label.h>
#include <gtkmm/frame.h>
#include <gtkmm/entry.h>
#include <gtkmm/radiobutton.h>

namespace Glom
{

class FileChooserDialog_SaveExtras : public Gtk::FileChooserDialog
{
public:
  FileChooserDialog_SaveExtras(const Glib::ustring& title, Gtk::FileChooserAction action, const Glib::ustring& backend);
  FileChooserDialog_SaveExtras(Gtk::Window& parent, const Glib::ustring& title, Gtk::FileChooserAction action, const Glib::ustring& backend);
  explicit FileChooserDialog_SaveExtras(const Glib::ustring& title, Gtk::FileChooserAction action = Gtk::FILE_CHOOSER_ACTION_OPEN);
  FileChooserDialog_SaveExtras(Gtk::Window& parent, const Glib::ustring& title, Gtk::FileChooserAction action = Gtk::FILE_CHOOSER_ACTION_OPEN);

  void set_extra_message(const Glib::ustring& message);
  void set_extra_newdb_title(const Glib::ustring& title);
#ifndef GLOM_ENABLE_CLIENT_ONLY
  void set_extra_newdb_hosting_mode(Document::HostingMode mode);
  //void set_extra_newdb_self_hosted(bool self_hosted = true);
#endif // !GLOM_ENABLE_CLIENT_ONLY

  Glib::ustring get_extra_newdb_title() const;

#ifndef GLOM_ENABLE_CLIENT_ONLY
  Document::HostingMode get_extra_newdb_hosting_mode() const;
#endif // !GLOM_ENABLE_CLIENT_ONLY

private:
  void create_child_widgets();

  Gtk::Box m_extra_widget;
  Gtk::Label m_label_extra_message;

  /* New database details: */
  Gtk::Entry m_entry_title;
#ifndef GLOM_ENABLE_CLIENT_ONLY

#ifdef GLOM_ENABLE_POSTGRESQL
  Gtk::RadioButton m_radiobutton_server_postgres_central;
  Gtk::RadioButton m_radiobutton_server_postgres_selfhosted;
#endif // GLOM_ENABLE_POSTGRESQL

#ifdef GLOM_ENABLE_SQLITE
  Gtk::RadioButton m_radiobutton_server_sqlite;
#endif // GLOM_ENABLE_SQLITE

#ifdef GLOM_ENABLE_MYSQL
  Gtk::RadioButton m_radiobutton_server_mysql_central;
  Gtk::RadioButton m_radiobutton_server_mysql_selfhosted;
#endif // GLOM_ENABLE_MYSQL

#endif // !GLOM_ENABLE_CLIENT_ONLY
};

} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_FILECHOOSERDIALOG_GLOM_H

