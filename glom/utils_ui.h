/* Glom
 *
 * Copyright (C) 2001-2009 Openismus GmbH
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

#ifndef GLOM_UTILS_UI_H
#define GLOM_UTILS_UI_H

#include <libglom/data_structure/field.h>
#include <libglom/data_structure/numeric_format.h>

#include <libglom/data_structure/layout/layoutitem_field.h>

#include <gtkmm/dialog.h>
#include <gtkmm/messagedialog.h>

namespace Glom
{

namespace Utils
{

enum DefaultSpacings
{
  #ifdef GLOM_ENABLE_MAEMO
  //We use different spacings on Maemo because the screen is smaller:
  DEFAULT_SPACING_LARGE = 1,
  DEFAULT_SPACING_SMALL = 1
  #else
  DEFAULT_SPACING_LARGE = 12,
  DEFAULT_SPACING_SMALL = 6
  #endif //GLOM_ENABLE_MAEMO
};

int dialog_run_with_help(Gtk::Dialog* dialog, const Glib::ustring& id = Glib::ustring());

/** This is a replacement for gnome_help_display(), 
 * to avoid the libgnome dependency.
 * TODO: GTK+ should have a function for this soon.
 */
void show_help(const Glib::ustring& id = Glib::ustring());

void show_ok_dialog(const Glib::ustring& title, const Glib::ustring& message, Gtk::Window& parent, Gtk::MessageType message_type);
void show_ok_dialog(const Glib::ustring& title, const Glib::ustring& message, Gtk::Window* parent, Gtk::MessageType message_type);

void show_window_until_hide(Gtk::Window* window);

/// For instance, to create bold primary text for a dialog box, without marking the markup for translation.
Glib::ustring bold_message(const Glib::ustring& message);

Glib::RefPtr<Gdk::Pixbuf> get_pixbuf_for_gda_value(const Gnome::Gda::Value& value);

} //namespace Utils

} //namespace Glom

#endif //GLOM_UTILS_UI_H

