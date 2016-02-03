/* Glom
 *
 * Copyright (C) 2001-2011 Openismus GmbH
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

#ifndef GLOM_PRINT_LAYOUT_UTILS_H
#define GLOM_PRINT_LAYOUT_UTILS_H

#include "config.h"
#include <glom/print_layout/canvas_layout_item.h>
#include <libglom/data_structure/print_layout.h>
#include <libglom/document/document.h>
#include <gtkmm/pagesetup.h>

namespace Glom
{

namespace PrintLayoutUtils
{

//TODO: Add get_grid_gap(units) instead.
const double GRID_GAP = 6.0f; //Roughly the right height for 12 point text.

//Base the default item sizes on the grid gap, instead of being arbitrary:
const double ITEM_HEIGHT = GRID_GAP;
const double ITEM_WIDTH_WIDE = GRID_GAP * 10;

//TODO: Move this into libglom, by replacing Gtk::PageSetup with a custom class.
//However, this also uses goocanvas, which would need to have its GTK+ widget split away too.
/** Create a print layout based on the on-screen details layout.
 * @param avoid_page_margins If true then do skip page margins.
 */
std::shared_ptr<PrintLayout> create_standard(const Glib::RefPtr<const Gtk::PageSetup>& page_setup, const Glib::ustring& table_name, const std::shared_ptr<const Document>& document, bool avoid_page_margins);

void do_print_layout(const std::shared_ptr<const PrintLayout>& print_layout, const FoundSet& found_set, bool preview, const std::shared_ptr<const Document>& document, bool avoid_page_margins, Gtk::Window* transient_for);

double get_page_height(const Glib::RefPtr<const Gtk::PageSetup>& page_setup, Gtk::Unit units);
double get_page_height(const Glib::RefPtr<const Gtk::PageSetup>& page_setup, Gtk::Unit units, double& margin_top, double& margin_bottom);

/** Discover what page the y position is on:
 */
guint get_page_for_y(const Glib::RefPtr<const Gtk::PageSetup>& page_setup, Gtk::Unit units, double y);

/** See if the item needs to move to the start of a page, past the top margin,
 * or if it is currently in the bottom margin of a page, or in the top margin of a page.
 *
 * @result Whether the item needs to be moved.
 */
bool needs_move_fully_to_page(const Glib::RefPtr<const Gtk::PageSetup>& page_setup, Gtk::Unit units, const Glib::RefPtr<const CanvasItemMovable>& item);

double get_offset_to_move_fully_to_next_page(const Glib::RefPtr<const Gtk::PageSetup>& page_setup, Gtk::Unit units, double y, double height);

} //namespace PrintLayoutUtils

} //namespace Glom

#endif //GLOM_PRINT_LAYOUT_UTILS_H
