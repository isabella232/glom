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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#ifndef GLOM_UTILS_UI_H
#define GLOM_UTILS_UI_H

#include "config.h"
#include <gtkmm/dialog.h>
#include <gtkmm/treeview.h>
#include <gtkmm/cssprovider.h>
#include <libglom/data_structure/field.h>
#include <libglom/data_structure/numeric_format.h>

#include <libglom/data_structure/layout/layoutitem_field.h>

#include <gtkmm/messagedialog.h>

namespace Glom
{

namespace UiUtils
{

enum DefaultSpacings
{
  DEFAULT_SPACING_LARGE = 12,
  DEFAULT_SPACING_SMALL = 6
};

/**
 * Show the dialog, blocking until there is a non-help response,
 * showing the appropriate help page if the help button is clicked.
 */
int dialog_run_with_help(Gtk::Dialog* dialog, const Glib::ustring& id = Glib::ustring());

/**
 * Show the dialog, blocking until there is a non-help response,
 * showing the appropriate help page if the help button is clicked.
 *  This requires the dialog class to have a static
 * glade_id member variable, which we reuse as the help ID.
 */
template<class T_Dialog>
int dialog_run_with_help(T_Dialog* dialog)
{
  return dialog_run_with_help(dialog, T_Dialog::glade_id);
}

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

/** Get the width required for typical data of this type in the current font.
 *
 * @widget The widget whose font should be used.
 * @field_layout The layout item whose data type should be used.
 * @or_title If true, check the width of the item's title too, returning the larger of the two values.
 * @result The width in pixels.
 */
int get_suitable_field_width_for_widget(Gtk::Widget& widget, const sharedptr<const LayoutItem_Field>& field_layout, bool or_title = false, bool for_treeview = false);

/// Add the @a extension if no extension is there already:
std::string get_filepath_with_extension(const std::string& filepath, const std::string& extension);

Glib::RefPtr<Gdk::Pixbuf> image_scale_keeping_ratio(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf, int target_height, int target_width);

///@result Whether the user would like to find again.
bool show_warning_no_records_found(Gtk::Window& transient_for);

void show_report_in_browser(const std::string& filepath, Gtk::Window* parent_window);

std::string get_icon_path(const Glib::ustring& filename);

/** Runs libglom's Utils::script_check_for_pygtk2() and shows
 * a warning dialog if necessary.
 */
bool script_check_for_pygtk2_with_warning(const Glib::ustring& script, Gtk::Window* parent_window);

void treeview_delete_all_columns(Gtk::TreeView* treeview);

void container_remove_all(Gtk::Container& container);

void load_font_into_css_provider(Gtk::Widget& widget, const Glib::ustring& font);
void load_color_into_css_provider(Gtk::Widget& widget, const Glib::ustring& color);
void load_background_color_into_css_provider(Gtk::Widget& widget, const Glib::ustring& color);

} //namespace UiUtils

} //namespace Glom

#endif //GLOM_UTILS_UI_H
