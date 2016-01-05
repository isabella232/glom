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

#ifndef GLOM_DIALOG_ADDRELATEDTABLE_H
#define GLOM_DIALOG_ADDRELATEDTABLE_H

#include <gtkmm/dialog.h>
#include <gtkmm/builder.h>
#include <gtkmm/entry.h>
#include <gtkmm/comboboxtext.h>
#include <glom/base_db.h>

namespace Glom
{

class Dialog_AddRelatedTable
  : public Gtk::Dialog,
    public Base_DB
{
public:
  static const char* glade_id;
  static const bool glade_developer;

  Dialog_AddRelatedTable(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

  void set_fields(const Glib::ustring& table_name);

  //Get the user input:
  void get_input(Glib::ustring& table_name, Glib::ustring& relationship_name, Glib::ustring& from_key_name);

  typedef sigc::signal<void> type_signal_request_edit_fields;
  type_signal_request_edit_fields signal_request_edit_fields();

private:

  void on_entry_table_name();
  void on_combo_field_name();
  void on_button_edit_fields();

  Gtk::Entry* m_entry_table_name;
  Gtk::Entry* m_entry_relationship_name;
  Gtk::ComboBoxText* m_combo_from_field;
  Gtk::Button* m_button_edit_fields;
  Gtk::Button* m_button_ok;

  Glib::ustring m_table_name;

  type_signal_request_edit_fields m_signal_request_edit_fields;
};

} //namespace Glom

#endif //GLOM_DIALOG_ADDRELATEDTABLE_H
