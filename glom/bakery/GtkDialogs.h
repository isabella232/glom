/*
 * Copyright 2000 Murray Cumming
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef BAKERY_APP_GTKDIALOGS_H
#define BAKERY_APP_GTKDIALOGS_H

#include <glom/bakery/App_WithDoc.h>

namespace GlomBakery
{

/** This class implements some gtkmm UI abstractions.
 */
class GtkDialogs
{
public:
  static void ui_warning(App& app, const Glib::ustring& text, const Glib::ustring& secondary_text);
  static Glib::ustring ui_file_select_open(App& app, const Glib::ustring& starting_folder_uri = Glib::ustring());

  ///Ask the user for a filename, and ask for confirmation if the file exists already.
  static Glib::ustring ui_file_select_save(App& app, const Glib::ustring& old_file_uri);

  static void ui_show_modification_status();
  static App_WithDoc::enumSaveChanges ui_offer_to_save_changes(App& app, const Glib::ustring& file_uri);
};

} //namespace

#endif //BAKERY_APP_GTKDIALOGS_H
