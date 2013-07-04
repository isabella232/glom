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

#ifndef GLOM_DIALOG_IMPORT_CSV_H
#define GLOM_DIALOG_IMPORT_CSV_H

#include <glom/import_csv/csv_parser.h>
#include <glom/base_db.h>

#include <memory>
#include <giomm/asyncresult.h>
#include <giomm/file.h>
#include <gtkmm/dialog.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/combobox.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/builder.h>
#include <gtkmm/cellrenderercombo.h>
#include <gtkmm/treemodelsort.h>
//#include <libgdamm/datamodelimport.h>


namespace Glom
{

class Dialog_Import_CSV
  : public Gtk::Dialog,
    public Base_DB
{
public:
  static const char* glade_id;
  static const bool glade_developer;

  Dialog_Import_CSV(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

  void import(const Glib::ustring& uri, const Glib::ustring& into_table);

  CsvParser::State get_parser_state() const;
  Glib::ustring get_target_table_name() const;
  const Glib::ustring& get_file_uri() const;


  std::shared_ptr<const Field> get_field_for_column(unsigned int col) const;
  const Glib::ustring& get_data(unsigned int row, unsigned int col);

  // TODO: perhaps it would be safer to just wrap the needed parser API here.
  CsvParser& get_parser();

  typedef sigc::signal<void> type_signal_state_changed;

  /** This signal will be emitted when the parser's state changes.
   */
  type_signal_state_changed signal_state_changed() const;


private:
  void clear();
  void show_error_dialog(const Glib::ustring& primary, const Glib::ustring& secondary);

  Glib::ustring get_current_encoding() const;
  void begin_parse();

  void setup_sample_model(const CsvParser::type_row_strings& row);
  Gtk::TreeViewColumn* create_sample_column(const Glib::ustring& title, guint index);
  Gtk::CellRendererCombo* create_sample_cell(guint index);

  //CellRenderer cell_data_func callbacks:
  void line_data_func(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter);
  void field_data_func(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter, unsigned int column_number);
  void on_field_edited(const Glib::ustring& path, const Glib::ustring& new_text, unsigned int column_number);

  void encoding_data_func(const Gtk::TreeModel::iterator& iter, Gtk::CellRendererText& renderer);
  bool row_separator_func(const Glib::RefPtr<Gtk::TreeModel>& model, const Gtk::TreeModel::iterator& iter) const;

  void on_parser_file_read_error(const Glib::ustring& error_message);
  void on_parser_have_display_name(const Glib::ustring& display_name);
  void on_parser_encoding_error();
  void on_parser_line_scanned(CsvParser::type_row_strings row, unsigned int row_number);
  void on_parser_state_changed();

  void on_combo_encoding_changed();
  void on_first_line_as_title_toggled();
  void on_sample_rows_changed();

  void validate_primary_key();

  class EncodingColumns: public Gtk::TreeModelColumnRecord
  {
  public:
    EncodingColumns() { add(m_col_name); add(m_col_charset); }

    Gtk::TreeModelColumn<Glib::ustring> m_col_name;
    Gtk::TreeModelColumn<Glib::ustring> m_col_charset;
  };

  class FieldColumns: public Gtk::TreeModelColumnRecord
  {
  public:
    FieldColumns() { add(m_col_field_name); add(m_col_field); }

    Gtk::TreeModelColumn<Glib::ustring> m_col_field_name;
    Gtk::TreeModelColumn<std::shared_ptr<Field> > m_col_field;
  };

  class SampleColumns: public Gtk::TreeModelColumnRecord
  {
  public:
    SampleColumns() { add(m_col_row); }

    Gtk::TreeModelColumn<int> m_col_row;
  };

  std::shared_ptr<CsvParser> m_parser;

  EncodingColumns m_encoding_columns;
  Glib::RefPtr<Gtk::ListStore> m_encoding_model;

  FieldColumns m_field_columns;
  Glib::RefPtr<Gtk::ListStore> m_field_model;
  Glib::RefPtr<Gtk::TreeModelSort> m_field_model_sorted;

  SampleColumns m_sample_columns;
  Glib::RefPtr<Gtk::ListStore> m_sample_model;
  Gtk::TreeView* m_sample_view;
  Gtk::Label* m_target_table;
  Gtk::ComboBox* m_encoding_combo;
  Gtk::Label* m_encoding_info;
  Gtk::CheckButton* m_first_line_as_title;
  Gtk::SpinButton* m_sample_rows;
  Gtk::Label* m_advice_label;
  Gtk::Label* m_error_label;

  Glib::ustring m_file_uri;

  // Index into the ENCODINGS array (see dialog_import_csv.cc) for the
  // encoding that we currently try to read the data with, or -1 if
  // auto-detection is disabled.
  int m_auto_detect_encoding;

  // The first row decides the amount of columns in our model, during  model
  // setup. We implicitly fill up every row that is shorter, and cut longer
  // rows.
  guint m_cols_count;

  // The fields into which to import the data:
  typedef std::vector< std::shared_ptr<Field> > type_vec_fields;
  type_vec_fields m_fields;

  type_signal_state_changed m_signal_state_changed;
};

} //namespace Glom

#endif //GLOM_DIALOG_IMPORT_CSV_H

