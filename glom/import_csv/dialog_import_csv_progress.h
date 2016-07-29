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

#ifndef GLOM_DIALOG_IMPORT_CSV_PROGRESS_H
#define GLOM_DIALOG_IMPORT_CSV_PROGRESS_H

#include "base_db_table_data.h"

#include "dialog_import_csv.h"
#include <gtkmm/progressbar.h>
#include <gtkmm/textview.h>

namespace Glom
{

class Dialog_Import_CSV_Progress
  : public Gtk::Dialog,
    public Base_DB_Table_Data
{
public:
  static const char* glade_id;
  static const bool glade_developer;

  Dialog_Import_CSV_Progress(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

  bool init_db_details(const Glib::ustring& table_name);

  // Reads the data from the Dialog_Import_CSV. We might want to wrap the
  // parsed data within a separate class.
  void import(Dialog_Import_CSV& data_source);

private:
  void clear();
  void add_text(const Glib::ustring& text);

  void begin_import();

  void on_data_source_state_changed();
  bool on_idle_import();

  void on_response(int response_id) override; // From Gtk::Dialog

  Gnome::Gda::Value get_entered_field_data(const LayoutItem_Field& field) const override; // from Base_DB_Table_Data
  void set_entered_field_data(const LayoutItem_Field& field, const Gnome::Gda::Value&  value) override; // from Base_DB

  std::shared_ptr<Field> get_field_primary_key() const override; // from Base_DB_Table_Data
  Gnome::Gda::Value get_primary_key_value_selected() const override; // from Base_DB_Table_Data
  void set_primary_key_value(const Gtk::TreeModel::iterator& row, const Gnome::Gda::Value& value) override; // from Base_DB_Table_Data
  Gnome::Gda::Value get_primary_key_value(const Gtk::TreeModel::iterator& row) const override; // from Base_DB_Table_Data

  std::shared_ptr<Field> m_field_primary_key;
  Dialog_Import_CSV* m_data_source;
  unsigned int m_current_row;

  // We use this for implementing get_entered_field_data and
  // set_entered_field_data, required by Base_DB_Table_Data::record_new().
  // It just holds the values for the fields in the current row.
  typedef std::map<Glib::ustring, Gnome::Gda::Value> type_mapValues;
  type_mapValues m_current_row_values;

  Gtk::ProgressBar* m_progress_bar;
  Gtk::TextView* m_text_view;

  sigc::connection m_progress_connection;
  sigc::connection m_ready_connection;
};

} //namespace Glom

#endif //GLOM_DIALOG_IMPORT_CSV_PROGRESS_H

