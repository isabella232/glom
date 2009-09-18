/* Glom
 *
 * Copyright (C) 2001-2004 Murray Cumming
 * Copyright (C) 2009 Openismus GmbH
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

#ifndef GLOM_IMPORT_CSV_H
#define GLOM_IMPORT_CSV_H

#include "base_db.h"

#include <memory>
#include <giomm/asyncresult.h>
#include <giomm/file.h>
#include <giomm/inputstream.h>
#include <gtkmm/liststore.h>

namespace Glom
{

// We use the low-level Glib::IConv routines to progressively convert the
// input data in an idle handler.

class CsvParser
{
public:
  explicit CsvParser(const char* encoding);

  ~CsvParser();

  enum State {
    STATE_NONE, 
    STATE_PARSING,  /**< Parsing is in progress. */
    STATE_ENCODING_ERROR, /**< An error happened while parsing. */
    STATE_PARSED /**< Finished parsing. */
  };

  static const gunichar DELIMITER = ',';
  static const gunichar QUOTE = '\"';

  static bool next_char_is_quote(const Glib::ustring::const_iterator& iter, const Glib::ustring::const_iterator& end);
  static Glib::ustring::const_iterator advance_field(const Glib::ustring::const_iterator& iter, const Glib::ustring::const_iterator& end, Glib::ustring& field);

  // Signals:
  typedef sigc::signal<void> type_signal_encoding_error;
  type_signal_encoding_error signal_encoding_error() const;

  typedef sigc::signal<void, std::string, unsigned int> type_signal_line_scanned;
  type_signal_line_scanned signal_line_scanned() const;

  /// Make parser object reusable.
  void clear();

  // In order to not make the UI feel sluggish during larger imports we parse
  // on chunk at a time in the idle handler.
  bool on_idle_parse();

  void set_encoding(const char* encoding);

//For now, everything's public. Until we know how our interface will look like.
//private:
  // The raw data in the original encoding. We keep this so we can convert
  // from the user-selected encoding to UTF-8 every time the user changes
  // the encoding.
  std::vector<char> m_raw;

  std::string m_encoding;
  std::vector<char>::size_type m_input_position;
  std::string m_current_line;
  sigc::connection m_idle_connection;
  unsigned int m_line_number;
  State m_state;
  Glib::RefPtr<Gio::FileInputStream> m_stream;

  // Parsed data:
  typedef std::vector<Glib::ustring> type_row_strings;
  typedef std::vector<type_row_strings> type_rows;
  type_rows m_rows;

  type_signal_encoding_error m_signal_encoding_error;
  type_signal_line_scanned m_signal_line_scanned;
};

} //namespace Glom

#endif //GLOM_IMPORT_CSV_H


