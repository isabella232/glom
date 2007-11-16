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

#include <glom/libglom/utils.h>
#include <glom/utility_widgets/filechooserdialog.h>
#include <gtkmm/alignment.h>
#include <bakery/App/App_Gtk.h> //For util_bold_message().
#include <glibmm/i18n.h>

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
  //m_extra_widget.pack_start(m_label_extra_message);
  m_label_extra_message.set_alignment(0.0f, 0.5f);
  m_label_extra_message.show();

  Gtk::Frame* frame = Gtk::manage(new Gtk::Frame());
  Gtk::Label* frame_label = Gtk::manage(new Gtk::Label());
  frame_label->set_markup(Bakery::App_Gtk::util_bold_message(_("New Database")));
  frame_label->show();
  frame->set_label_widget(*frame_label);
  frame->set_shadow_type(Gtk::SHADOW_NONE);

  Gtk::Alignment* alignment = Gtk::manage(new Gtk::Alignment());
  alignment->set_padding(0, 0, 12, 0); //Add padding at the left.
  alignment->show();
  frame->add(*alignment);
  frame->show();

  Gtk::VBox* vbox = Gtk::manage(new Gtk::VBox(false, Utils::DEFAULT_SPACING_SMALL));
  alignment->add(*vbox);
  vbox->show();

  vbox->pack_start(m_label_extra_message); /* For instance, an extra hint when saving from an example, saying that a new file must be saved. */

  Gtk::Label* label_newdb = Gtk::manage(new Gtk::Label(_("Please choose a human-readable title for the new database. You can change this later in the database properties. It may contain any characters.")));
  vbox->pack_start(*label_newdb);
  label_newdb->show();

  Gtk::HBox* box_label = Gtk::manage(new Gtk::HBox(false, Utils::DEFAULT_SPACING_SMALL));
  Gtk::Label* label_title = Gtk::manage(new Gtk::Label(_("Title")));
  box_label->pack_start(*label_title, Gtk::PACK_SHRINK);
  label_title->show();
  box_label->pack_start(m_entry_title);
  m_entry_title.show();
  box_label->show();
  vbox->pack_start(*box_label);

#ifndef GLOM_ENABLE_CLIENT_ONLY
  m_radiobutton_server_selfhosted.set_label(_("Create database in its own folder, to be hosted by this computer."));
  vbox->pack_start(m_radiobutton_server_selfhosted);
  m_radiobutton_server_selfhosted.show();

  m_radiobutton_server_central.set_label(_("Create database on an external database server, to be specified in the next step."));
  Gtk::RadioButton::Group group = m_radiobutton_server_selfhosted.get_group();
  m_radiobutton_server_central.set_group(group);
  vbox->pack_start(m_radiobutton_server_central);
  m_radiobutton_server_central.show();
#endif // !GLOM_ENABLE_CLIENT_ONLY


  m_extra_widget.pack_start(*frame);

  set_extra_widget(m_extra_widget);
  m_extra_widget.show();
}

void FileChooserDialog::set_extra_newdb_title(const Glib::ustring& title)
{
  m_entry_title.set_text(title);
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void FileChooserDialog::set_extra_newdb_self_hosted(bool self_hosted)
{
  m_radiobutton_server_selfhosted.set_active(self_hosted);
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

Glib::ustring FileChooserDialog::get_extra_newdb_title() const
{
  return m_entry_title.get_text();
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
bool FileChooserDialog::get_extra_newdb_self_hosted() const
{
  return m_radiobutton_server_selfhosted.get_active();
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

} //namespace Glom


