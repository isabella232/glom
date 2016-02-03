/* Glom
 *
 * Copyright (C) 2001-2004 Murray Cumming
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

#ifndef GLOM_MODE_DESIGN_DIALOG_USER_H
#define GLOM_MODE_DESIGN_DIALOG_USER_H

#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/builder.h>
#include "../../utility_widgets/combo_textglade.h"

namespace Glom
{

class Dialog_User : public Gtk::Dialog
{
public:
  static const char* glade_id;
  static const bool glade_developer;

  Dialog_User(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

  bool check_password();

  /**
   * @param layout "list" or "details"
   * @param document The document, so that the dialog can load the previous layout, and save changes.
   * @param table_name The table name.
   * @param table_fields: The actual fields in the table, in case the document does not yet know about them all.
   */
  //virtual void set_document(const Glib::ustring& layout, const std::shared_ptr<Document>& document, const Glib::ustring& table_name, const type_vecLayoutFields& table_fields);

  Gtk::Entry* m_entry_user;
  Combo_TextGlade* m_combo_group;
  Gtk::Entry* m_entry_password;
  Gtk::Entry* m_entry_password_confirm;
};

} //namespace Glom

#endif //GLOM_MODE_DESIGN_DIALOG_USER_H
