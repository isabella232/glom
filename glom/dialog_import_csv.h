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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef GLOM_DIALOG_IMPORT_CSV_H
#define GLOM_DIALOG_IMPORT_CSV_H

#include "base_db.h"

#include <memory>
#include <giomm/asyncresult.h>
#include <giomm/file.h>
#include <giomm/inputstream.h>
#include <gtkmm/dialog.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/combobox.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/builder.h>
#include <libgdamm/datamodelimport.h>


namespace Glom
{

class Dialog_Import_CSV
  : public Gtk::Dialog,
    public Base_DB
{
public:
  typedef sigc::signal<void> SignalStateChanged;

  enum State {
    NONE,
    PARSING,
    ENCODING_ERROR,
    PARSED
  };

  Dialog_Import_CSV(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

  void import(const Glib::ustring& uri, const Glib::ustring& into_table);
  
  State get_state() const { return m_state; }
  Glib::ustring get_target_table_name() const { return m_target_table->get_text(); }
  const Glib::ustring& get_filename() const { return m_filename; }

  unsigned int get_row_count() const;
  unsigned int get_column_count() const;
  const sharedptr<Field>& get_field_for_column(unsigned int col);
  const Glib::ustring& get_data(unsigned int row, unsigned int col);

  SignalStateChanged signal_state_changed() const { return m_signal_state_changed; }

private:
  void clear();
  void show_error_dialog(const Glib::ustring& primary, const Glib::ustring& secondary);

  void encoding_data_func(const Gtk::TreeModel::iterator& iter, Gtk::CellRendererText& renderer);
  bool row_separator_func(const Glib::RefPtr<Gtk::TreeModel>& model, const Gtk::TreeModel::iterator& iter) const;

  void on_query_info(const Glib::RefPtr<Gio::AsyncResult>& result);
  void on_file_read(const Glib::RefPtr<Gio::AsyncResult>& result);
  void on_stream_read(const Glib::RefPtr<Gio::AsyncResult>& result);

  void on_encoding_changed();
  void on_first_line_as_title_toggled();
  void on_sample_rows_changed();

  Glib::ustring get_current_encoding() const;
  void begin_parse();
  void encoding_error();

  bool on_idle_parse();
  void handle_line(const Glib::ustring& line, unsigned int line_number);

  void line_data_func(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter);
  void field_data_func(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter, unsigned int column_number);
  void on_field_edited(const Glib::ustring& path, const Glib::ustring& new_text, unsigned int column_number);

  void set_state(State state);
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
    Gtk::TreeModelColumn<sharedptr<Field> > m_col_field;
  };

  class SampleColumns: public Gtk::TreeModelColumnRecord
  {
  public:
    SampleColumns() { add(m_col_row); }

    Gtk::TreeModelColumn<int> m_col_row;
  };

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

  Glib::RefPtr<Gio::File> m_file;
  Glib::RefPtr<Gio::FileInputStream> m_stream;
  Glib::ustring m_filename;

  // The raw data in the original encoding. We keep this so we can convert
  // from the user-selected encoding to UTF-8 every time the user changes
  // the encoding.
  std::vector<char> m_raw;

  struct Buffer {
    char buf[1024];
  };
  std::auto_ptr<Buffer> m_buffer;

  // Index into the ENCODINGS array (see dialog_import_csv.cc) for the
  // encoding that we currently try to read the data with, or -1 if
  // auto-detection is disabled.
  int m_auto_detect_encoding;

  // We use the low-level Glib::IConv routines to progressively convert the
  // input data in an idle handler.
  struct Parser {
    Glib::IConv conv;
    std::vector<char>::size_type input_position;
    std::string current_line;
    sigc::connection idle_connection;
    unsigned int line_number;

    Parser(const char* encoding): conv("UTF-8", encoding), input_position(0), line_number(0) {}
    ~Parser() { idle_connection.disconnect(); }
  };

  std::auto_ptr<Parser> m_parser;
  State m_state;

  // Parsed data:
  std::vector<std::vector<Glib::ustring> > m_rows;

  // The fields into which to import the data:
  typedef std::vector< sharedptr<Field> > type_vec_fields;
  type_vec_fields m_fields;

  SignalStateChanged m_signal_state_changed;
};

} //namespace Glom

#endif //GLOM_DIALOG_IMPORT_CSV_H

