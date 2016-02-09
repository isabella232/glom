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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#include "csv_parser.h"
#include <glibmm/convert.h>
#include <glibmm/main.h>

#include <cerrno>
#include <iostream>

// On Windows, "iconv" seems to be a define for "libiconv", breaking the Glib::IConv::iconv() call.
#ifdef iconv
#undef iconv
#endif

namespace Glom
{

bool CsvParser::next_char_is_quote(const Glib::ustring::const_iterator& iter, const Glib::ustring::const_iterator& end)
{
  if(iter == end)
    return false;

  // Look at the next character to see if it's really "" (an escaped "):
  auto iter_next = iter;
  ++iter_next;
  if(iter_next != end)
  {
    const auto c_next = *iter_next;
    if(c_next == CsvParser::QUOTE)
    {
      return true;
    }
  }

  return false;
}

CsvParser::CsvParser(const std::string& encoding_charset)
: m_encoding(encoding_charset),
  m_input_position(0),
  m_in_quotes(false),
  m_line_number(0),
  m_state(State::NONE),
  m_row_index(0)
{
}

void CsvParser::set_file_and_start_parsing(const std::string& file_uri)
{
  // TODO: Check URI validity?
  g_return_if_fail(!file_uri.empty());

  auto file = Gio::File::create_for_uri(file_uri);

  set_state(CsvParser::State::PARSING);
  
  //TODO Get a rough estimate of the number of rows (for showing progress),
  //by counting the number of lines:
  //guint lines_count = 0;

  // Query the display name of the file to set in the title:
  file->query_info_async(sigc::bind(sigc::mem_fun(*this, &CsvParser::on_file_query_info), file),
                         G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME);

  file->read_async(sigc::bind(sigc::mem_fun(*this, &CsvParser::on_file_read), file));
}


CsvParser::~CsvParser()
{
  m_idle_connection.disconnect();
}

CsvParser::State CsvParser::get_state() const
{
  return m_state;
}

bool CsvParser::get_rows_empty() const
{
  return m_rows.empty();
}

const Glib::ustring& CsvParser::get_data(guint row, guint col) const
{
  static Glib::ustring empty_result;

  // TODO: Why do we complain here? We cannot (and don't need to) restrict
  // our clients to only call this method with valid rows & cols, it falls in
  // our responsibility to check the access, which we do. Handing out an
  // empty result is perfectly fine!

  if(row >= m_rows.size())
  {
    //std::cerr << G_STRFUNC << ": get_data(): row out of range.\n";
    return empty_result;
  }

  const auto row_data = m_rows[row];
  if(col >= row_data.size())
  {
    //std::cerr << G_STRFUNC << ": get_data(): col out of range.\n";
    return empty_result;
  }

  return row_data[col];
}

CsvParser::type_row_strings CsvParser::fetch_next_row()
{
  g_return_val_if_fail(m_state == CsvParser::State::PARSING, fetch_next_row());
  g_return_val_if_fail(m_state == CsvParser::State::PARSED, fetch_next_row());

  // We cannot fetch the next row, but since we are still parsing we might just have to parse a bit more!
  if(m_state == CsvParser::State::PARSING && m_row_index >= m_rows.size())
  {
    on_idle_parse();
    // The final recursion guard is m_state == CsvParser::State::PARSING
    return fetch_next_row();
  }

  if(m_state == CsvParser::State::PARSED && m_row_index >= m_rows.size())
  {
    return type_row_strings();
  }

  if(m_row_index < m_rows.size())
  {
    return m_rows[m_row_index++];
  }

  return type_row_strings();
}

CsvParser::type_signal_file_read_error CsvParser::signal_file_read_error() const
{
  return m_signal_file_read_error;
}

CsvParser::type_signal_file_read_error CsvParser::signal_have_display_name() const
{
  return m_signal_have_display_name;
}

CsvParser::type_signal_encoding_error CsvParser::signal_encoding_error() const
{
  return m_signal_encoding_error;
}

CsvParser::type_signal_finished_parsing CsvParser::signal_finished_parsing() const
{
  return m_finished_parsing;
}

CsvParser::type_signal_line_scanned CsvParser::signal_line_scanned() const
{
  return m_signal_line_scanned;
}

CsvParser::type_signal_state_changed CsvParser::signal_state_changed() const
{
  return m_signal_state_changed;
}

void CsvParser::set_encoding(const Glib::ustring& encoding_charset)
{
  if(m_encoding == encoding_charset)
    return;

  m_encoding = encoding_charset;

  //Stop parsing if the encoding changes.
  //The caller should restart the parsing when wanted.
  clear();
  set_state(State::NONE);
}

// Parse the field in a comma-separated line, returning the field including the quotes:
// (But can it operate on non-UTF, read: binary, data?)
Glib::ustring::const_iterator CsvParser::advance_field(const Glib::ustring::const_iterator& iter, const Glib::ustring::const_iterator& end, Glib::ustring& field)
{
  bool inside_quotes = false;
  //bool string_finished = false; //Ignore anything after "something", such as "something"else,

  field.clear();

  Glib::ustring::const_iterator walk;
  for(walk = iter; walk != end; ++walk)
  {
    const gunichar c = *walk;

    //if(string_finished)
    //  continue;

    if(inside_quotes)
    {
      // End of quoted string?
      if(c == CsvParser::QUOTE)
      {
        if(CsvParser::next_char_is_quote(walk, end))
        {
          // This is "" so it's not an end quote. Just add one quote:
          field += c;
          ++walk; //Skip the second ".
        }
        else
        {
          inside_quotes = false;
          //string_finished = true; //Ignore anything else before the next comma.
        }

        continue;
      }
    }
    else
    {
      // Start of quoted string:
      if((c == CsvParser::QUOTE))
      {
        inside_quotes = true;
        continue;
      }
      // End of field:
      else if(!inside_quotes && c == CsvParser::DELIMITER)
      {
        break;
      }
    }

    field += c; // Just so that we don't need to iterate through the field again, since there is no Glib::ustring::substr(iter, iter)
  }

  // TODO: Throw error if still inside a quoted string?
  //std::cout << "debug: field=" << field << std::endl;
  return walk;
}

void CsvParser::clear()
{
  m_buffer.reset();

  //m_stream.reset();
  //m_raw.clear();
  m_rows.clear();
  m_in_quotes = false;

  // Set to current encoding I guess ...
  //m_conv("UTF-8", encoding),
  m_input_position= 0;
  // Disconnect signal handlers, too.
  m_idle_connection.disconnect();
  m_line_number = 0;
  set_state(State::NONE);
}

bool CsvParser::on_idle_parse()
{
  Glib::IConv conv("UTF-8", m_encoding);

  // The amount of bytes to process in one pass of the idle handler:
  static const guint CONVERT_BUFFER_SIZE = 1024;

  const char* inbuffer = &m_raw[m_input_position];
  auto inbuf = const_cast<char*>(inbuffer);

  g_return_val_if_fail(m_input_position <= m_raw.size(), true);
  auto inbytes = m_raw.size() - m_input_position;

  char outbuffer[CONVERT_BUFFER_SIZE];
  char* outbuf = outbuffer;
  gsize outbytes = CONVERT_BUFFER_SIZE;

  const auto result = conv.iconv(&inbuf, &inbytes, &outbuf, &outbytes);
  bool more_to_process = (inbytes != 0);

  if(result == static_cast<size_t>(-1))
  {
    if(errno == EILSEQ)
    {
      // Invalid text in the current encoding.
      set_state(State::ENCODING_ERROR);
      signal_encoding_error().emit();
      return false; //Stop calling the idle handler.
    }

    // If EINVAL is set, this means that an incomplete multibyte sequence was at
    // the end of the input. We might have some more bytes, but those do not make
    // up a whole character, so we need to wait for more input.
    if(errno == EINVAL)
    {
      if(!m_stream)
      {
        // This means that we already reached the end of the file. The file
        // should not end with an incomplete multibyte sequence.
        set_state(State::ENCODING_ERROR);
        signal_encoding_error().emit();
        return false; //Stop calling the idle handler.
      }
      else
      {
        more_to_process = false;
      }
    }
  }

  m_input_position += (inbuf - inbuffer);

  // We now have outbuf - outbuffer bytes of valid UTF-8 in outbuffer.
  const char* prev_line_end = outbuffer;
  const char* prev = prev_line_end;

  // Identify the record rows in the .csv file.
  // We can't just search for newlines because they may be inside quotes too.
  // TODO: Use a regex instead, to more easily handle quotes?
  while(true)
  {
    //std::cout << "debug: checking start: " << std::string(prev, 10) << std::endl;

    // Note that, unlike std::string::find*, std::find* returns an iterator (char*), not a position.
    // It returns outbuf if none is found.
    const char newline_to_find[] = { '\r', '\n', '\0' };
    const auto pos_newline = std::find_first_of<const char*>(prev, outbuf, newline_to_find, newline_to_find + sizeof(newline_to_find));

    const char quote_to_find[] = {(char)QUOTE};
    const auto pos_quote = std::find_first_of<const char*>(prev, outbuf, quote_to_find, quote_to_find + sizeof(quote_to_find));

    // Examine the first character (quote or newline) that was found:
    const char* pos = pos_newline;
    if((pos_quote != outbuf) && pos_quote < pos)
      pos = pos_quote;

    if(pos == outbuf)
    {
      //std::cout << "debug: not found. stopping\n";
      break;
    }

    char ch = *pos;
    //std::cout << "debug: ch=START" << ch << "END\n";

    if(ch == '\0')
    {
      // There is a null byte in the conversion. Because normal text files don't
      // contain null bytes this only occurs when converting, for example, a UTF-16
      // file from ISO-8859-1 to UTF-8 (note that the UTF-16 file is valid ISO-8859-1 -
      // it just contains lots of nullbytes). We therefore produce an error here.
      //std::cerr << G_STRFUNC << ": on_idle_parse(): Encoding error\n";
      set_state(State::ENCODING_ERROR);
      signal_encoding_error().emit();
      return false;  //Stop calling the idle handler.
    }
    else if(m_in_quotes)
    {
      // Ignore newlines inside quotes.

      // End quote:
      if(ch == (char)QUOTE)
      {
        m_in_quotes = false;

        /*
        const size_t len = pos - prev;
        std::string quoted_text;
        if(len)
          quoted_text = std::string(prev, len);
        std::cout << "DEBUG: Quoted=" << quoted_text << std::endl;
        */
      }
      //else
      //  std::cout << "Ignoring a newline in quotes.\n";

      prev = pos + 1;
      continue;
    }
    else
    {
      // Start quote:
      if(ch == (char)QUOTE)
      {
        m_in_quotes = true;
        prev = pos + 1;
        continue;
      }

      // Found a newline (outside of quotes) that marks the end of the line:
      m_current_line.append(prev_line_end, pos - prev_line_end);
      ++m_line_number;

      if(!m_current_line.empty())
      {
        //std::cout << "debug: intermediate chunk\n";
        do_line_scanned(m_current_line, m_line_number);
      }

      m_current_line.clear();

      // Skip linebreak
      prev = pos + 1;

      // Skip DOS-style linebreak (\r\n)
      if(ch == '\r'
         && prev != outbuf && *prev == '\n')
      {
         ++prev;
      }

      prev_line_end = prev;
    }
  }

  // We reached the end of buffer (instead of ending with a newline):
  m_current_line.append(prev_line_end, outbuf - prev_line_end);

  if(!m_stream && m_raw.size() == m_input_position)
  {
    ++m_line_number;

    // Handle last line, if nonempty
    if(!m_current_line.empty())
    {
      //std::cout << "debug: last chunk\n";
      do_line_scanned(m_current_line, m_line_number);
    }

    // We have parsed the whole file. We have finished.
    // TODO: To only emit signal_finished_parsing here is *not* enough.
    set_state(State::PARSED);
    signal_finished_parsing().emit();
  }
  else
  {
    //TODO: Make in_quotes static, so that quotes work across calls to this chunk parser.
    //std::cout << "Waiting for next read: so far size=" << m_current_line.size() << ", start=" << m_current_line.substr(0, 40) << std::endl;
  }

  // Continue if there are more bytes to process
  return more_to_process; //false means stop calling the idle handler.
}

void CsvParser::do_line_scanned(const Glib::ustring& line, guint line_number)
{
  //std::cout << "debug: on_line_scanned=" << line_number << ", line start=" << line.substr(0, 40) << std::endl;
  if(line.empty())
   return;

  m_rows.emplace_back(CsvParser::type_row_strings());
  auto& row = m_rows.back();

  Glib::ustring field;
  //Gtk::TreeModelColumnRecord record;

  // Parse first field:
  auto line_iter = CsvParser::advance_field(line.begin(), line.end(), field);
  row.emplace_back(field);

  // Parse more fields:
  while(line_iter != line.end())
  {
    // Skip delimiter:
    ++line_iter;

    // Read field:
    line_iter = advance_field(line_iter, line.end(), field);

    // Add field to current row:
    row.emplace_back(field);
  }

  signal_line_scanned().emit(row, line_number);
}

void CsvParser::ensure_idle_handler_connection()
{
  if(!m_idle_connection.connected())
  {
    m_idle_connection = Glib::signal_idle().connect(
      sigc::mem_fun(*this, &CsvParser::on_idle_parse),
      Glib::PRIORITY_DEFAULT_IDLE - 20);
  }
}

void CsvParser::on_file_read(const Glib::RefPtr<Gio::AsyncResult>& result, const Glib::RefPtr<Gio::File>& source)
{
  ensure_idle_handler_connection();

  try
  {
    m_stream = source->read_finish(result);

    m_buffer = std::make_unique<Buffer>();
    m_stream->read_async(m_buffer->buf, sizeof(m_buffer->buf), sigc::mem_fun(*this, &CsvParser::on_buffer_read));
  }
  catch(const Glib::Exception& ex)
  {
    signal_file_read_error().emit(ex.what());
    clear();
  }
}

void CsvParser::copy_buffer_and_continue_reading(gssize size)
{
  if(size > 0)
  {
    m_raw.insert(m_raw.end(), m_buffer->buf, m_buffer->buf + size);

    m_buffer = std::make_unique<Buffer>();
    m_stream->read_async(m_buffer->buf, sizeof(m_buffer->buf), sigc::mem_fun(*this, &CsvParser::on_buffer_read));
  }
  else // When size == 0 we finished reading.
  {
    //TODO: put in proper data reset method?
    m_buffer.reset();
    m_stream.reset();
  }

  //In case it has been disconnected too early due to timing problems:
  ensure_idle_handler_connection();
}

void CsvParser::on_buffer_read(const Glib::RefPtr<Gio::AsyncResult>& result)
{
  try
  {
    const auto size = m_stream->read_finish(result);
    copy_buffer_and_continue_reading(size);
  }
  catch(const Glib::Exception& ex)
  {
    signal_file_read_error().emit(ex.what());
    clear();
  }
}

void CsvParser::on_file_query_info(const Glib::RefPtr<Gio::AsyncResult>& result, const Glib::RefPtr<Gio::File>& source) const
{
  try
  {
    auto info = source->query_info_finish(result);
    if(info)
      signal_have_display_name().emit(info->get_display_name());
  }
  catch(const Glib::Exception& ex)
  {
    std::cerr << G_STRFUNC << "Failed to fetch display name of uri " << source->get_uri() << ": " << ex.what() << std::endl;
  }
}

void CsvParser::set_state(State state)
{
  if(m_state == state)
    return;

  m_state = state;
  signal_state_changed().emit();
}


} // namespace Glom
