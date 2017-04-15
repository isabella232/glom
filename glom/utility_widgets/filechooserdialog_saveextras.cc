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

#include "config.h"
#include <glom/utility_widgets/filechooserdialog_saveextras.h>
#include <glom/utils_ui.h>
#include <gtkmm/alignment.h>
#include <libglom/utils.h> //For bold_message()).
#include <glibmm/i18n.h>

namespace Glom
{

FileChooserDialog_SaveExtras::FileChooserDialog_SaveExtras(const Glib::ustring& title, Gtk::FileChooserAction action)
: Gtk::FileChooserDialog(title, action),
  m_extra_widget(Gtk::Orientation::VERTICAL)
{
  create_child_widgets();
}

FileChooserDialog_SaveExtras::FileChooserDialog_SaveExtras(Gtk::Window& parent, const Glib::ustring& title, Gtk::FileChooserAction action)
: Gtk::FileChooserDialog(parent, title, action),
  m_extra_widget(Gtk::Orientation::VERTICAL)
{
  create_child_widgets();
}

void FileChooserDialog_SaveExtras::set_extra_message(const Glib::ustring& message)
{
  m_label_extra_message.set_text(message);

  if(!message.empty()) {
    m_label_extra_message.show();
  } else {
    m_label_extra_message.hide();
  }
}

void FileChooserDialog_SaveExtras::create_child_widgets()
{
  //m_extra_widget.pack_start(m_label_extra_message);
  m_label_extra_message.set_halign(Gtk::Align::START);
  m_label_extra_message.set_valign(Gtk::Align::CENTER);

  auto frame = Gtk::manage(new Gtk::Frame());
  auto frame_label = Gtk::manage(new Gtk::Label());
  frame_label->set_markup(UiUtils::bold_message(_("New Database")));
  frame_label->show();
  frame->set_label_widget(*frame_label);
  frame->set_shadow_type(Gtk::ShadowType::NONE);
  frame->show();

  auto vbox = Gtk::manage(new Gtk::Box(Gtk::Orientation::VERTICAL, Utils::to_utype(UiUtils::DefaultSpacings::SMALL)));
  vbox->set_margin_start(Utils::to_utype(UiUtils::DefaultSpacings::LARGE));
  vbox->set_margin_top(Utils::to_utype(UiUtils::DefaultSpacings::SMALL));
  frame->add(*vbox);
  vbox->show();

  vbox->pack_start(m_label_extra_message); /* For instance, an extra hint when saving from an example, saying that a new file must be saved. */

  auto label_newdb = Gtk::manage(new Gtk::Label(_("Please choose a human-readable title for the new database. You can change this later in the database properties. It may contain any characters.")));
  vbox->pack_start(*label_newdb);
  label_newdb->set_halign(Gtk::Align::START);
  label_newdb->set_valign(Gtk::Align::CENTER);
  label_newdb->show();

  auto box_label = Gtk::manage(new Gtk::Box(Gtk::Orientation::HORIZONTAL, Utils::to_utype(UiUtils::DefaultSpacings::LARGE)));
  auto label_title = Gtk::manage(new Gtk::Label(_("_Title:"), true));
  box_label->pack_start(*label_title, Gtk::PackOptions::PACK_SHRINK);
  label_title->show();
  box_label->pack_start(m_entry_title);
  m_entry_title.get_accessible()->set_name(_("Title"));
  m_entry_title.show();
  box_label->show();
  vbox->pack_start(*box_label);

#ifndef GLOM_ENABLE_CLIENT_ONLY

#ifdef GLOM_ENABLE_POSTGRESQL

#if defined(GLOM_ENABLE_SQLITE) || defined(GLOM_ENABLE_MYSQL)
  //Use titles that show the distinction between PostgreSQL and the alternatives:
  const auto postgresql_selfhost_label = _("Create PostgreSQL database in its own folder, to be hosted by this computer.");
  const auto postgresql_central_label = _("Create database on an external PostgreSQL database server, to be specified in the next step.");
#else
  const auto postgresql_selfhost_label = _("Create database in its own folder, to be hosted by this computer.");
  const auto postgresql_central_label = _("Create database on an external database server, to be specified in the next step.");
#endif

  m_radiobutton_server_postgres_selfhosted.set_label(postgresql_selfhost_label);
  vbox->pack_start(m_radiobutton_server_postgres_selfhosted);
  m_radiobutton_server_postgres_selfhosted.show();

  m_radiobutton_server_postgres_central.set_label(postgresql_central_label);
  Gtk::RadioButton::Group group = m_radiobutton_server_postgres_selfhosted.get_group();
  m_radiobutton_server_postgres_central.set_group(group);
  vbox->pack_start(m_radiobutton_server_postgres_central);
  m_radiobutton_server_postgres_central.show();

  m_radiobutton_server_postgres_selfhosted.set_active(true); // Default
#endif

#ifdef GLOM_ENABLE_SQLITE
  m_radiobutton_server_sqlite.set_label(_("Create SQLite database in its own folder, to be hosted by this computer."));
  m_radiobutton_server_sqlite.set_tooltip_text(_("SQLite does not support authentication or remote access but is suitable for embedded devices."));
  m_radiobutton_server_sqlite.set_group(group);
  vbox->pack_start(m_radiobutton_server_sqlite);
  m_radiobutton_server_sqlite.show();
#endif

#ifdef GLOM_ENABLE_MYSQL
  m_radiobutton_server_mysql_selfhosted.set_label(_("Create MySQL database in its own folder, to be hosted by this computer."));
  m_radiobutton_server_mysql_selfhosted.set_tooltip_text(_("MySQL support in Glom is experimental and unlikely to work properly."));
  m_radiobutton_server_mysql_selfhosted.set_group(group);
  vbox->pack_start(m_radiobutton_server_mysql_selfhosted);
  m_radiobutton_server_mysql_selfhosted.show();

  m_radiobutton_server_mysql_central.set_label(_("Create database on an external MySQL database server, to be specified in the next step."));
  m_radiobutton_server_mysql_central.set_tooltip_text(_("MySQL support in Glom is experimental and unlikely to work properly."));
  m_radiobutton_server_mysql_central.set_group(group);
  vbox->pack_start(m_radiobutton_server_mysql_central);
  m_radiobutton_server_mysql_central.show();
#endif

#endif // !GLOM_ENABLE_CLIENT_ONLY


  m_extra_widget.pack_start(*frame);

  set_extra_widget(m_extra_widget);
  m_extra_widget.show();
}

void FileChooserDialog_SaveExtras::set_extra_newdb_title(const Glib::ustring& title)
{
  m_entry_title.set_text(title);
}

void FileChooserDialog_SaveExtras::set_extra_newdb_hosting_mode(Document::HostingMode mode)
{
  switch(mode)
  {
#ifdef GLOM_ENABLE_POSTGRESQL
  case Document::HostingMode::POSTGRES_CENTRAL:
    m_radiobutton_server_postgres_central.set_active();
    break;
  case Document::HostingMode::POSTGRES_SELF:
    m_radiobutton_server_postgres_selfhosted.set_active();
    break;
#endif //GLOM_ENABLE_POSTGRESQL

#ifdef GLOM_ENABLE_SQLITE
  case Document::HostingMode::SQLITE:
    m_radiobutton_server_sqlite.set_active();
    break;
#endif //GLOM_ENABLE_SQLITE

#ifdef GLOM_ENABLE_MYSQL
  case Document::HostingMode::MYSQL_CENTRAL:
    m_radiobutton_server_mysql_central.set_active();
    break;
  case Document::HostingMode::MYSQL_SELF:
    m_radiobutton_server_mysql_selfhosted.set_active();
    break;
#endif //GLOM_ENABLE_SQLITE

  default:
    g_assert_not_reached();
    break;
  }
}

Glib::ustring FileChooserDialog_SaveExtras::get_extra_newdb_title() const
{
  return m_entry_title.get_text();
}

Document::HostingMode FileChooserDialog_SaveExtras::get_extra_newdb_hosting_mode() const
{
#ifdef GLOM_ENABLE_POSTGRESQL
  if(m_radiobutton_server_postgres_central.get_active())
    return Document::HostingMode::POSTGRES_CENTRAL;
  else if(m_radiobutton_server_postgres_selfhosted.get_active())
    return Document::HostingMode::POSTGRES_SELF;
#endif //GLOM_ENABLE_POSTGRESQL

#ifdef GLOM_ENABLE_SQLITE
  if(m_radiobutton_server_sqlite.get_active())
    return Document::HostingMode::SQLITE;
#endif //GLOM_ENABLE_SQLITE

#ifdef GLOM_ENABLE_MYSQL
  if(m_radiobutton_server_mysql_central.get_active())
    return Document::HostingMode::MYSQL_CENTRAL;
  else if(m_radiobutton_server_mysql_selfhosted.get_active())
    return Document::HostingMode::MYSQL_SELF;
#endif //GLOM_ENABLE_MYSQL

  g_assert_not_reached();

#ifdef GLOM_ENABLE_SQLITE
  return Document::HostingMode::SQLITE; //Arbitrary
#else
  return Document::HostingMode::POSTGRES_SELF; //Arbitrary.
#endif //GLOM_ENABLE_SQLITE
}

} //namespace Glom
