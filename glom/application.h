/* Glom
 *
 * Copyright (C) 2012 Murray Cumming
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

#ifndef GLOM_APPLICATION_H
#define GLOM_APPLICATION_H

#include <glom/main_remote_options.h>
#include <gtkmm/application.h>

namespace Glom
{

class Application: public Gtk::Application
{
protected:
  Application();

public:
  static Glib::RefPtr<Application> create();

protected:
  //Overrides of default signal handlers:
  void on_activate() override;
  void on_startup() override;
  void on_open(const Gio::Application::type_vec_files& files,
    const Glib::ustring& hint) override;
  int on_command_line(const Glib::RefPtr<Gio::ApplicationCommandLine>& command_line) override;

private:
  void create_window(const Glib::RefPtr<Gio::File>& file = Glib::RefPtr<Gio::File>());

  void on_window_hide(Gtk::Window* window);

  RemoteOptionGroup m_remote_option_group;
};

} //namespace Glom

#endif /* GTKMM_APPLICATION_H */
