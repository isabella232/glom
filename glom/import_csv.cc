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

#include "import_csv.h"

#include <cerrno>

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
  Glib::ustring::const_iterator iter_next = iter;
  ++iter_next;
  if(iter_next != end)
  {
    const gunichar c_next = *iter_next;
    if(c_next == CsvParser::QUOTE)
    {
      return true;
    }
  }

  return false;
}

CsvParser::CsvParser(const char* encoding)
: m_raw(0),
  m_encoding(encoding),
  m_input_position(0),
  m_idle_connection(),
  m_line_number(0),
  m_state(STATE_NONE),
  m_stream(),
  m_rows()
{}

CsvParser::~CsvParser()
{
  m_idle_connection.disconnect();
}

CsvParser::State CsvParser::get_state() const
{
  return m_state;
}

guint CsvParser::get_rows_count() const
{
  return m_rows.size();
}

CsvParser::type_signal_encoding_error CsvParser::signal_encoding_error() const
{
  return m_signal_encoding_error;
}

CsvParser::type_signal_line_scanned CsvParser::signal_line_scanned() const
{
  return m_signal_line_scanned;
}

void CsvParser::set_encoding(const char* encoding)
{
  m_encoding = static_cast<std::string>(encoding);
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

      continue;
    }

    field += c; // Just so that we don't need to iterate through the field again, since there is no Glib::ustring::substr(iter, iter)
  }

  // TODO: Throw error if still inside a quoted string?
  //std::cout << "debug: field=" << field << std::endl;
  return walk;
}

void CsvParser::clear()
{
  //m_stream.reset();
  //m_raw.clear();
  m_rows.clear();
  // Set to current encoding I guess ...
  //m_conv("UTF-8", encoding),
  m_input_position= 0;
  // Disconnect signal handlers, too? Nah, I don't think so ...
  //m_idle_connection.disconnect();
  m_line_number = 0;
  m_state = STATE_NONE;
}

bool CsvParser::on_idle_parse()
{
  Glib::IConv conv("UTF-8", m_encoding);

  // The amount of bytes to process in one pass of the idle handler:
  static const guint CONVERT_BUFFER_SIZE = 1024;

  const char* inbuffer = &m_raw[m_input_position];
  char* inbuf = const_cast<char*>(inbuffer);
  gsize inbytes = m_raw.size() - m_input_position;
  char outbuffer[CONVERT_BUFFER_SIZE];
  char* outbuf = outbuffer;
  gsize outbytes = CONVERT_BUFFER_SIZE;

  const std::size_t result = conv.iconv(&inbuf, &inbytes, &outbuf, &outbytes);
  bool more_to_process = (inbytes != 0);

  if(result == static_cast<size_t>(-1))
  {
    if(errno == EILSEQ)
    {
      // Invalid text in the current encoding.
      signal_encoding_error().emit();
      return false;
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
        signal_encoding_error().emit();
        return false;
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
  bool in_quotes = false;
  while(true)
  {
    // Note that, unlike std::string::find*, std::find* returns an iterator (char*), not a position.
    // It returns outbuf if none is found.
    const char newline_to_find[] = { '\r', '\n', '\0' };
    const char* pos_newline = std::find_first_of<const char*>(prev, outbuf, newline_to_find, newline_to_find + sizeof(newline_to_find));

    const char quote_to_find[] = {(char)QUOTE};
    const char* pos_quote = std::find_first_of<const char*>(prev, outbuf, quote_to_find, quote_to_find + sizeof(quote_to_find));

    // Examine the first character (quote or newline) that was found:
    const char* pos = pos_newline;
    if((pos_quote != outbuf) && pos_quote < pos)
      pos = pos_quote;

    if(pos == outbuf)
      break;

    char ch = *pos;   

    if(ch == '\0')
    {
      // There is a null byte in the conversion. Because normal text files don't
      // contain null bytes this only occurs when converting, for example, a UTF-16
      // file from ISO-8859-1 to UTF-8 (note that the UTF-16 file is valid ISO-8859-1 - 
      // it just contains lots of nullbytes). We therefore produce an error here.
      signal_encoding_error().emit();
      return false;
    }
    else if(in_quotes)
    {
      // Ignore newlines inside quotes.

      // End quote:
      if(ch == (char)QUOTE)
        in_quotes = false;

      prev = pos + 1;
      continue;
    }
    else
    {
      // Start quote:
      if(ch == (char)QUOTE)
      {
        in_quotes = true;
        prev = pos + 1;
        continue;
      }

      // Found a newline (outside of quotes) that marks the end of the line:
      m_current_line.append(prev_line_end, pos - prev_line_end);
      ++m_line_number;

      if(!m_current_line.empty())
      {
        // Emitting a signal frees the parser of having to know the line
        // handling object instance (inversion of control using the observer
        // pattern).
        signal_line_scanned().emit(m_current_line, m_line_number);
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

  // Append last chunk of this line
  m_current_line.append(prev, outbuf - prev);
  if(!m_stream && m_raw.size() == m_input_position)
  {
    ++m_line_number;

    // Handle last line, if nonempty
    if(!m_current_line.empty())
    {
      signal_line_scanned().emit(m_current_line, m_line_number);
    }

    // We have parsed the whole file. We have finished.
    m_state = STATE_PARSED;
  }

  // Continue if there are more bytes to process
  return more_to_process;
}

} // namespace Glom
