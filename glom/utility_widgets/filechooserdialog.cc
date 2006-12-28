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

#include <glom/utility_widgets/filechooserdialog.h>

namespace Glom
{

FileChooserDialog::FileChooserDialog(const Glib::ustring& title, Gtk::FileChooserAction action, const Glib::ustring& backend)
: Gtk::FileChooserDialog(title, action, backend)
{
  create_child_widgets();
}

FileChooserDialog::FileChooserDialog(Gtk::Window& parent, const Glib::ustring& title, Gtk::FileChooserAction action, const Glib::ustring& backend)
: Gtk::FileChooserDialog(parent, title, action, backend)
{
  create_child_widgets();
}

FileChooserDialog::FileChooserDialog (const Glib::ustring& title, Gtk::FileChooserAction action)
: Gtk::FileChooserDialog(title, action)
{
  create_child_widgets();
}

FileChooserDialog::FileChooserDialog (Gtk::Window& parent, const Glib::ustring& title, Gtk::FileChooserAction action)
: Gtk::FileChooserDialog(parent, title, action)
{
  create_child_widgets();
}

FileChooserDialog::~FileChooserDialog()
{
}


void FileChooserDialog::set_extra_message(const Glib::ustring& message)
{
  m_label_extra_message.set_text(message);
}

void FileChooserDialog::create_child_widgets()
{
  m_extra_widget.pack_start(m_label_extra_message);
  m_label_extra_message.show();
  set_extra_widget(m_extra_widget);
  m_extra_widget.show();
}

} //namespace Glom


