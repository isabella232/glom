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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef GLOM_UTILITY_WIDGETS_FILECHOOSERDIALOG_GLOM_H
#define GLOM_UTILITY_WIDGETS_FILECHOOSERDIALOG_GLOM_H

#include <gtkmm/filechooserdialog.h>
#include <gtkmm/label.h>
#include <gtkmm/box.h>
#include <gtkmm/frame.h>
#include <gtkmm/entry.h>
#include <gtkmm/radiobutton.h>

namespace Glom
{

class FileChooserDialog : public Gtk::FileChooserDialog
{
public:
  FileChooserDialog(const Glib::ustring& title, Gtk::FileChooserAction action, const Glib::ustring& backend);
  FileChooserDialog(Gtk::Window& parent, const Glib::ustring& title, Gtk::FileChooserAction action, const Glib::ustring& backend);
  FileChooserDialog (const Glib::ustring& title, Gtk::FileChooserAction action = Gtk::FILE_CHOOSER_ACTION_OPEN);
  FileChooserDialog (Gtk::Window& parent, const Glib::ustring& title, Gtk::FileChooserAction action = Gtk::FILE_CHOOSER_ACTION_OPEN);
  virtual ~FileChooserDialog();

  void set_extra_message(const Glib::ustring& message);
  void set_extra_newdb_details(const Glib::ustring& title, bool self_hosted = true);

  Glib::ustring get_extra_newdb_details(bool& self_hosted);

protected:
  void create_child_widgets();

  Gtk::VBox m_extra_widget;
  Gtk::Label m_label_extra_message;

  /* New database details: */
  Gtk::Entry m_entry_title;
  Gtk::RadioButton m_radiobutton_server_central;
  Gtk::RadioButton m_radiobutton_server_selfhosted;
  Gtk::Button m_button_ok;
};

} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_FILECHOOSERDIALOG_GLOM_H

