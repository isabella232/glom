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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef GLOM_APPLICATION_H
#define GLOM_APPLICATION_H


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
  virtual void on_activate();
  virtual void on_open(const Gio::Application::type_vec_files& files,
    const Glib::ustring& hint);

private:
  void create_window(const Glib::RefPtr<Gio::File>& file = Glib::RefPtr<Gio::File>());

  void on_window_hide(Gtk::Window* window);
};

} //namespace Glom

#endif /* GTKMM_APPLICATION_H */
