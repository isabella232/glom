/* Glom
 *
 * Copyright (C) 2001-2016 Murray Cumming
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

#include "string_utils.h"
#include <libglom/data_structure/field.h>
#include <libglom/document/document.h>
#include <giomm/file.h>
#include <giomm/resource.h>
#include <glibmm/convert.h>
#include <glibmm/fileutils.h>
#include <glibmm/miscutils.h>
#include <stack>
#include <iostream>

namespace Glom
{

namespace Utils
{

Glib::ustring trim_whitespace(const Glib::ustring& text)
{
  //TODO_Performance:

  Glib::ustring result = text;

  //Find non-whitespace from front:
  Glib::ustring::size_type posFront = Glib::ustring::npos;
  Glib::ustring::size_type pos = 0;
  for (const auto& item : result)
  {
    if (!Glib::Unicode::isspace(item))
    {
      posFront = pos;
      break;
    }

    ++pos;
  }

  //Remove the white space from the front:
  result = result.substr(posFront);


  //Find non-whitespace from back:
  Glib::ustring::size_type posBack = Glib::ustring::npos;
  pos = 0;
  for (auto iter = result.rbegin(); iter != result.rend(); ++iter)
  {
    if (!Glib::Unicode::isspace(*iter))
    {
      posBack = pos;
      break;
    }

    ++pos;
  }

  //Remove the white space from the front:
  result = result.substr(0, result.size() - posBack);

  return result;
}

Glib::ustring string_replace(const Glib::ustring& src, const Glib::ustring& search_for, const Glib::ustring& replace_with)
{
  if(search_for.empty())
  {
    std::cerr << G_STRFUNC << ": search_for was empty.\n";
    return src;
  }

  //std::cout << "debug: " << G_STRFUNC << ": src=" << src << ", search_for=" << search_for << ", replace_with=" << replace_with << std::endl;

  std::string result = src;

  std::string::size_type pos = 0;
  const auto len_search = search_for.size();
  const auto len_replace = replace_with.size();

  std::string::size_type pos_after_prev = 0;
  while((pos = result.find(search_for, pos_after_prev)) != std::string::npos)
  {
    //std::cout << "  debug: pos=" << pos << ", found=" << search_for << ", in string: " << result.substr(pos_after_prev, 20) << std::endl;
    //std::cout << "  debug: before: result =" << result << ", pos_after_prev=pos_after_prev\n";
    result.replace(pos, len_search, replace_with);
    //std::cout << "  after: before: result = result\n";
    pos_after_prev = pos + len_replace;
  }

  return result;

/*
  //TODO_Performance:

  Glib::ustring result;
  const size_t src_length = src.size();
  const size_t search_for_length = search_for.size();
  //const size_t replace_with_length = replace_with.size();

  size_t src_index = 0;
  size_t src_index_section_start = 0;
  while(src_index < src_length)
  {
    const bool found = (src.find(search_for, src_index) == src_index);
    if(found)
    {
      result += src.substr(src_index_section_start, src_index - src_index_section_start);
      result += replace_with;

      src_index_section_start = src_index + search_for_length;
      src_index = src_index_section_start;
    }
    else
      ++src_index;
  }

  if(src_index_section_start < src_length)
  {
    result += src.substr(src_index_section_start);
  }

  return result;
*/
}

Glib::ustring string_clean_for_xml(const Glib::ustring& src)
{
  // The form feed character may not be in XML, even if escaped.
  // So lets just lose it.
  // Other unusual characters, such as &, are escaped by libxml later.
  // TODO_Performance: Find a quicker way to do this.
  return string_replace(src, "\f", Glib::ustring());
}

Glib::ustring create_name_from_title(const Glib::ustring& title)
{
  Glib::ustring result = string_replace(title, " ", "");
  return result.lowercase(); //TODO: Maybe they need to be ASCII (not UTF8)?
}

Glib::ustring string_escape_underscores(const Glib::ustring& text)
{
  Glib::ustring result;
  for(const auto& item : text)
  {
    if(item == '_')
      result += "__";
    else
      result += item;
  }

  return result;
}

Glib::ustring string_trim(const Glib::ustring& str, const Glib::ustring& to_remove)
{
  Glib::ustring result = str;

  //Remove from the start:
  Glib::ustring::size_type posOpenBracket = result.find(to_remove);
  if(posOpenBracket == 0)
  {
    result = result.substr(to_remove.size());
  }

  //Remove from the end:
  Glib::ustring::size_type posCloseBracket = result.rfind(to_remove);
  if(posCloseBracket == (result.size() - to_remove.size()))
  {
    result = result.substr(0, posCloseBracket);
  }

  return result;
}

Glib::ustring string_remove_suffix(const Glib::ustring& str, const Glib::ustring& suffix, bool case_sensitive)
{
  //There is also g_string_has_suffix(), but I assume that is case sensitive. murrayc.

  const Glib::ustring::size_type size = str.size();
  const Glib::ustring::size_type suffix_size = suffix.size();
  if(size < suffix_size)
    return str;

  const Glib::ustring possible_suffix = str.substr(size - suffix_size);

  if(case_sensitive)
  {
    if(possible_suffix == suffix)
      return str.substr(0, size - suffix_size);
  }
  else
  {
    if(g_ascii_strcasecmp(possible_suffix.c_str(), suffix.c_str()) == 0) //TODO: I don't understand the warnings about using this function in the glib documentation. murrayc.
      return str.substr(0, size - suffix_size);
  }

  return str;
}

Glib::ustring title_from_string(const Glib::ustring& text)
{
  Glib::ustring result;

  bool capitalise_next_char = true;
  for(const auto& ch : text)
  {
    if(ch == '_') //Replace _ with space.
    {
      capitalise_next_char = true; //Capitalise all words.
      result += " ";
    }
    else
    {
      if(capitalise_next_char)
        result += Glib::Unicode::toupper(ch);
      else
        result += ch;

      capitalise_next_char = false;
    }
  }

  return result;
}

Glib::ustring string_from_decimal(guint decimal)
{
  //TODO_Performance:

  std::stringstream stream;
  stream.imbue(std::locale("")); //Use the user's current locale.
  stream << decimal;

  Glib::ustring result;
  stream >> result;

  return result;
}

type_vec_strings string_separate(const Glib::ustring& str, const Glib::ustring& separator, bool ignore_quoted_separator)
{
  //std::cout << "debug: " << G_STRFUNC << ": separator=" << separator << std::endl;

  type_vec_strings result;

  const Glib::ustring::size_type size = str.size();
  const Glib::ustring::size_type size_separator = separator.size();

  //A stack of quotes, so that we can handle nested quotes, whether they are " or ':
  typedef std::stack<Glib::ustring> type_queue_quotes;
  type_queue_quotes m_current_quotes;

  Glib::ustring::size_type unprocessed_start = 0;
  Glib::ustring::size_type item_start = 0;
  while(unprocessed_start < size)
  {
    //std::cout << "while unprocessed: un_processed_start=" << unprocessed_start << std::endl;
    Glib::ustring::size_type posComma = str.find(separator, unprocessed_start);

    Glib::ustring item;
    if(posComma != Glib::ustring::npos)
    {
      //Check that the separator was not in quotes:
      bool in_quotes = false;

      if(ignore_quoted_separator)
      {
        //std::cout << "  debug: attempting to ignore quoted separators: " << separator << std::endl;

        Glib::ustring::size_type posLastQuote = unprocessed_start;

        //std::cout << "    debug: posLastQuote=" << posLastQuote << std::endl;
        //std::cout << "    debug: posComma=" << posComma << std::endl;


        bool bContinue = true;
        while(bContinue && (posLastQuote < posComma))
        {
          //std::cout << "  continue\n";
          Glib::ustring closing_quote;
          if(!m_current_quotes.empty())
            closing_quote = m_current_quotes.top();

          //std::cout << "   posLastQuote=" << posLastQuote << std::endl;
          const Glib::ustring::size_type posSingleQuote = str.find("'", posLastQuote);
          const Glib::ustring::size_type posDoubleQuote = str.find("\"", posLastQuote);

          // std::cout << "   posSingleQuote=" << posSingleQuote << "posDoubleQuote=" << posDoubleQuote << std::endl;

          //Which quote, if any, is first:
          Glib::ustring::size_type posFirstQuote = posSingleQuote;
          if( (posDoubleQuote != Glib::ustring::npos) && (posDoubleQuote < posFirstQuote) )
            posFirstQuote = posDoubleQuote;

          //Ignore quotes that are _after_ the separator:
          if( posFirstQuote >= posComma)
            posFirstQuote = Glib::ustring::npos;

          //std::cout << "   posFirstQuote=" << posFirstQuote << std::endl;

          //If any quote character was found:
          if(posFirstQuote != Glib::ustring::npos)
          {
            //std::cout << "quote found: posFirstQuote=" << posFirstQuote << std::endl;

            //Which quote was it?
            const Glib::ustring first_quote =  (posFirstQuote == posSingleQuote ? "'" : "\"");
            //std::cout << "   first_quote=" << first_quote << std::endl;

            //Was it an expected closing quote, if we expected any:
            if(first_quote == closing_quote)
            {
              //std::cout << "   popping quote\n";
              //Yes, so remove that quote from our stack, because we found the closing quote:
              m_current_quotes.pop();
            }
            else
            {
              //std::cout << "   pushing quote\n";
              //This must be an opening quote, so remember it:
              m_current_quotes.push(first_quote);
            }

            posLastQuote = posFirstQuote + 1; //Do the next find after the quote.
          }
          else
          {
            //There were no quotes, or no closing quotes:
            bContinue = false;
          }
        } //while(bContinue)

        //If there were any unclosed quotes then this separator must have been in quotes:
        if(!m_current_quotes.empty())
          in_quotes = true;
      } //If ignore_quoted_separator

      if(!in_quotes) //or if we don't care about quotes.
      {
        //std::cout << "!in_quotes\n";

        //Store this item, and start the next item after it:
        item = str.substr(item_start, posComma - item_start);
        //std::cout << "  ITEM. pos_comma=" << posComma << ", ITEM= " << item << std::endl;
        item_start = posComma + size_separator;
      }
      else
      {
        //std::cout << "in quotes.\n";
        // Continue behind separator
        unprocessed_start = posComma + size_separator;
        // Do not add this item to the result, because it was quoted.
        continue;
      }

      unprocessed_start = posComma + size_separator; //The while loops stops when this is empty.
    }
    else //if no separator found:
    {
      item = str.substr(item_start);
      unprocessed_start = size; //Stop.
    }

    item = string_trim(item, " ");
    result.emplace_back(item);
  } //while

  return result;
}

} //namespace Utils

} //namespace Glom