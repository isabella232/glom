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

#include "config.h" // For GLOM_ENABLE_MAEMO

#include <libglom/utils.h>
#include <libglom/connectionpool.h>
#include <libglom/data_structure/layout/report_parts/layoutitem_fieldsummary.h>
#include <libglom/data_structure/glomconversions.h>

#include <glibmm/i18n.h>
#include <gtkmm/messagedialog.h>

#include <gio/gio.h> // For g_app_info_launch_default_for_uri

#ifdef GLOM_ENABLE_MAEMO
#include <hildonmm/note.h>
#endif

#include <string.h> // for strchr
#include <sstream> //For stringstream

#include <iostream>
#include <fstream>

#include <locale>     // for locale, time_put
#include <ctime>     // for struct tm
#include <iostream>   // for cout, endl
#include <iomanip>

#include <stack>

#ifdef GLOM_ENABLE_MAEMO
//We use different spacings on Maemo because the screen is smaller:
const unsigned int Glom::Utils::DEFAULT_SPACING_LARGE =  1;
const unsigned int Glom::Utils::DEFAULT_SPACING_SMALL =  1;
#else
const unsigned int Glom::Utils::DEFAULT_SPACING_LARGE = 12;
const unsigned int Glom::Utils::DEFAULT_SPACING_SMALL =  6;
#endif //GLOM_ENABLE_MAEMO

namespace
{

// Basically copied from libgnome (gnome-help.c, Copyright (C) 2001 Sid Vicious
// Copyright (C) 2001 Jonathan Blandford <jrb@alum.mit.edu>), but C++ified
std::string locate_help_file(const std::string& path, const std::string& doc_name)
{
  // g_get_language_names seems not to be wrapped by glibmm
  const char* const* lang_list = g_get_language_names ();

  for(unsigned int j = 0; lang_list[j] != NULL; ++j)
  {
    const char* lang = lang_list[j];

    /* This has to be a valid language AND a language with
     * no encoding postfix.  The language will come up without
     * encoding next. */
    if(lang == NULL || strchr(lang, '.') != NULL)
      continue;

    const char* exts[] = { "", ".xml", ".docbook", ".sgml", ".html", NULL };
    for(unsigned i = 0; exts[i] != NULL; ++i)
    {
      std::string name = doc_name + exts[i];
      std::string full = Glib::build_filename(path, Glib::build_filename(lang, name));

      if(Glib::file_test(full, Glib::FILE_TEST_EXISTS))
        return full;
    }
  }

  return std::string();
}

}

namespace Glom
{

template<class T_Element>
class predicate_UsesRelationshipHasRelationships
{
public:
  predicate_UsesRelationshipHasRelationships(const Glib::ustring& relationship_name, const Glib::ustring& related_relationship_name = Glib::ustring())
  : m_relationship_name(relationship_name),
    m_related_relationship_name(related_relationship_name)
  {
  }

  predicate_UsesRelationshipHasRelationships(const sharedptr<const UsesRelationship> uses_relationship_name, bool first_level_only = false)
  : m_relationship_name(uses_relationship_name->get_relationship_name()),
    m_related_relationship_name(uses_relationship_name->get_related_relationship_name())
  {
    //If first_level_only, search for relationships that have the same top-level relationship, but have no related relationship.
    if(first_level_only)
      m_related_relationship_name = Glib::ustring();
  }

  virtual ~predicate_UsesRelationshipHasRelationships()
  {
  }

  bool operator() (const T_Element& element)
  {
    return (element.get_relationship_name() == m_relationship_name) && (element.get_related_relationship_name() == m_related_relationship_name);
  }

  bool operator() (const sharedptr<T_Element>& element)
  {
    return (element->get_relationship_name() == m_relationship_name) && (element->get_related_relationship_name() == m_related_relationship_name);
  }

  bool operator() (const sharedptr<const T_Element>& element)
  {
    return (element->get_relationship_name() == m_relationship_name) && (element->get_related_relationship_name() == m_related_relationship_name);
  }

private:
  Glib::ustring m_relationship_name, m_related_relationship_name;
};


Glib::ustring Utils::trim_whitespace(const Glib::ustring& text)
{
  //TODO_Performance:

  Glib::ustring result = text;

  //Find non-whitespace from front:
  Glib::ustring::size_type posFront = Glib::ustring::npos;
  Glib::ustring::size_type pos = 0;
  for(Glib::ustring::iterator iter = result.begin(); iter != result.end(); ++iter)
  {
    if(!Glib::Unicode::isspace(*iter))
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
  for(Glib::ustring::reverse_iterator iter = result.rbegin(); iter != result.rend(); ++iter)
  {
    if(!Glib::Unicode::isspace(*iter))
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

Glib::ustring Utils::string_replace(const Glib::ustring& src, const Glib::ustring search_for, const Glib::ustring& replace_with)
{
  std::string result = src;

  std::string::size_type pos = 0;
  std::string::size_type len= search_for.size();

  while((pos = result.find(search_for)) != std::string::npos)
  {
   result.replace(pos, len, replace_with);
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
    const bool found = (src.find(search_for.c_str(), src_index) == src_index);
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


Glib::ustring Utils::build_sql_select_with_where_clause(const Glib::ustring& table_name, const type_vecLayoutFields& fieldsToGet, const Glib::ustring& where_clause, const Glib::ustring& extra_join, const type_sort_clause& sort_clause, const Glib::ustring& extra_group_by)
{
  //TODO_Performance:
  type_vecConstLayoutFields constFieldsToGet;
  for(type_vecLayoutFields::const_iterator iter = fieldsToGet.begin(); iter != fieldsToGet.end(); ++iter)
  {
    constFieldsToGet.push_back(*iter);
  }

  return build_sql_select_with_where_clause(table_name, constFieldsToGet, where_clause, extra_join, sort_clause, extra_group_by);
}


typedef std::list< sharedptr<const UsesRelationship> > type_list_relationships;

static void add_to_relationships_list(type_list_relationships& list_relationships, const sharedptr<const LayoutItem_Field>& layout_item)
{
  if(!(layout_item->get_has_relationship_name()))
    return;

  //If this is a related relationship, add the first-level relationship too, so that the related relationship can be defined in terms of it:
  type_list_relationships::const_iterator iterFind = std::find_if(list_relationships.begin(), list_relationships.end(), predicate_UsesRelationshipHasRelationships<UsesRelationship>(layout_item, true /* top_level_only */) );
  if(iterFind == list_relationships.end()) //If the table is not yet in the list:
  {
    sharedptr<UsesRelationship> uses_rel = sharedptr<UsesRelationship>::create();
    uses_rel->set_relationship(layout_item->get_relationship());
    list_relationships.push_front(uses_rel); //These need to be at the front, so that related relationships can use them later in the SQL statement.
  }

  //Add the relationship to the list:
  iterFind = std::find_if(list_relationships.begin(), list_relationships.end(), predicate_UsesRelationshipHasRelationships<UsesRelationship>(layout_item) );
  if(iterFind == list_relationships.end()) //If the table is not yet in the list:
  {
    sharedptr<UsesRelationship> uses_rel = sharedptr<UsesRelationship>::create();
    uses_rel->set_relationship(layout_item->get_relationship());
    uses_rel->set_related_relationship(layout_item->get_related_relationship());
    list_relationships.push_back(uses_rel);
  }

 
}

Glib::ustring Utils::build_sql_select_with_where_clause(const Glib::ustring& table_name, const type_vecConstLayoutFields& fieldsToGet, const Glib::ustring& where_clause, const Glib::ustring& extra_join, const type_sort_clause& sort_clause, const Glib::ustring& extra_group_by)
{
  Glib::ustring result;

  //Get all relationships used in the query:
  typedef std::list< sharedptr<const UsesRelationship> > type_list_relationships;
  type_list_relationships list_relationships;

  for(type_vecConstLayoutFields::const_iterator iter = fieldsToGet.begin(); iter != fieldsToGet.end(); ++iter)
  {
    sharedptr<const LayoutItem_Field> layout_item = *iter;
    add_to_relationships_list(list_relationships, layout_item);
  }

  for(type_sort_clause::const_iterator iter = sort_clause.begin(); iter != sort_clause.end(); ++iter)
  {
    sharedptr<const LayoutItem_Field> layout_item = iter->first;
    add_to_relationships_list(list_relationships, layout_item);
  }


  Glib::ustring sql_part_fields;
  Glib::ustring sql_part_from;

  for(type_vecConstLayoutFields::const_iterator iter = fieldsToGet.begin(); iter != fieldsToGet.end(); ++iter)
  {
    Glib::ustring one_sql_part;

    sharedptr<const LayoutItem_Field> layout_item = *iter;

    bool is_summary = false;
    const LayoutItem_FieldSummary* fieldsummary = dynamic_cast<const LayoutItem_FieldSummary*>(layout_item.obj());
    if(fieldsummary)
      is_summary = true;

    //Add, for instance, "SUM(":
    if(is_summary)
      one_sql_part += fieldsummary->get_summary_type_sql() + "(";

    one_sql_part += layout_item->get_sql_name(table_name);

    //Close the summary bracket if necessary.
    if(is_summary)
      one_sql_part +=  ")";

    //Append it to the big string of fields:
    if(!one_sql_part.empty())
    {
      if(!sql_part_fields.empty())
        sql_part_fields += ", ";

      sql_part_fields += one_sql_part;
    }
  }

  if(sql_part_fields.empty())
  {
    std::cerr << "Utils::build_sql_select_with_where_clause(): sql_part_fields.empty(): fieldsToGet.size()=" << fieldsToGet.size() << std::endl;
    return result;
  }
  else
  {
    result =  "SELECT " + sql_part_fields +
      " FROM \"" + table_name + "\"";
  }

  //LEFT OUTER JOIN will get the field values from the other tables, and give us our fields for this table even if there is no corresponding value in the other table.
  Glib::ustring sql_part_leftouterjoin; 
  for(type_list_relationships::const_iterator iter = list_relationships.begin(); iter != list_relationships.end(); ++iter)
  {
    sharedptr<const UsesRelationship> uses_relationship = *iter;
    sharedptr<const Relationship> relationship = uses_relationship->get_relationship();
    if(relationship->get_has_fields()) //TODO: Handle related_record has_fields.
    {
      sql_part_leftouterjoin += uses_relationship->get_sql_join_alias_definition();
    }
    else if(relationship->get_has_to_table())
    {
      //It is a relationship that only specifies the table, without specifying linking fields:
      if(!(sql_part_from.empty()))
        sql_part_from += ", ";

      sql_part_from += relationship->get_to_table();
    }
  }

  if(!extra_join.empty())
  {
    sql_part_leftouterjoin += (" " + extra_join + " ");
  }

  if(!sql_part_from.empty())
    result += ("," + sql_part_from);

  if(!sql_part_leftouterjoin.empty())
    result += (" " + sql_part_leftouterjoin);

  if(!where_clause.empty())
    result += " WHERE " + where_clause;

  //Extra GROUP_BY clause for doubly-related records. This must be before the ORDER BY sort clause:
  if(!extra_group_by.empty())
  {
    result += (" " + extra_group_by + " ");
  }

  //Sort clause:
  if(!sort_clause.empty())
  {
    Glib::ustring str_sort_clause;
    for(type_sort_clause::const_iterator iter = sort_clause.begin(); iter != sort_clause.end(); ++iter)
    {
      sharedptr<const LayoutItem_Field> layout_item = iter->first;
      if(layout_item)
      {
        const bool ascending = iter->second;

        if(!str_sort_clause.empty())
          str_sort_clause += ", ";

        str_sort_clause += "\"" + layout_item->get_sql_table_or_join_alias_name(table_name) + "\".\"" + layout_item->get_name() + "\" " + (ascending ? "ASC" : "DESC");
      }
    }

    if(!str_sort_clause.empty())
      result += " ORDER BY " + str_sort_clause;
  }

  return result;
}


Glib::ustring Utils::build_sql_select_with_key(const Glib::ustring& table_name, const type_vecLayoutFields& fieldsToGet, const sharedptr<const Field>& key_field, const Gnome::Gda::Value& key_value)
{
  //TODO_Performance:
  type_vecConstLayoutFields constFieldsToGet;
  for(type_vecLayoutFields::const_iterator iter = fieldsToGet.begin(); iter != fieldsToGet.end(); ++iter)
  {
    constFieldsToGet.push_back(*iter);
  }

  return build_sql_select_with_key(table_name, constFieldsToGet, key_field, key_value);


}

Glib::ustring Utils::build_sql_select_with_key(const Glib::ustring& table_name, const type_vecConstLayoutFields& fieldsToGet, const sharedptr<const Field>& key_field, const Gnome::Gda::Value& key_value)
{
  if(!Conversions::value_is_empty(key_value)) //If there is a record to show:
  {
    //TODO: Use a SQL parameter instead of using sql():
    const Glib::ustring where_clause = "\"" + table_name + "\".\"" + key_field->get_name() + "\" = " + key_field->sql(key_value);
    return Utils::build_sql_select_with_where_clause(table_name, fieldsToGet, where_clause);
  }

  return Glib::ustring();
}

Utils::type_list_values_with_second Utils::get_choice_values(const sharedptr<const LayoutItem_Field>& field)
{
  type_list_values_with_second list_values;

  sharedptr<Relationship> choice_relationship;
  Glib::ustring choice_field, choice_second;
  field->get_formatting_used().get_choices(choice_relationship, choice_field, choice_second);
  if(!choice_relationship)
  {
    //std::cout <<" debug: field has no choices: " << field->get_name() << std::endl;
    return list_values;
  }

  const Glib::ustring to_table = choice_relationship->get_to_table();
  if(to_table.empty())
  {
    g_warning("get_choice_values(): table_name is null. relationship name = %s", glom_get_sharedptr_name(choice_relationship).c_str());
    return list_values;
  }

  const bool with_second = !choice_second.empty();
  const Glib::ustring sql_second = "\"" + to_table + "\".\"" + choice_second + "\"";

  //Get possible values from database, sorted by the first column.
  Glib::ustring sql_query = "SELECT \"" + to_table + "\".\"" + choice_field + "\"";
  if(with_second)
    sql_query += ", " + sql_second;

  sql_query += " FROM \"" + choice_relationship->get_to_table() + "\" ORDER BY \"" + to_table + "\".\"" + choice_field + "\"";

  //std::cout << "debug: get_choice_values(): query: " << sql_query << std::endl;
  //Connect to database:
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  sharedptr<SharedConnection> connection = ConnectionPool::get_instance()->connect();
#else
  std::auto_ptr<ExceptionConnection> conn_error;
  sharedptr<SharedConnection> connection = ConnectionPool::get_instance()->connect(conn_error);
  if(conn_error.get() != NULL) return list_values;
#endif

  //std::cout << "get_choice_values: Executing SQL: " << sql_query << std::endl;
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  Glib::RefPtr<Gnome::Gda::DataModel> datamodel = connection->get_gda_connection()->statement_execute_select(sql_query);
#else
  std::auto_ptr<Glib::Error> error;
  Glib::RefPtr<Gnome::Gda::DataModel> datamodel = connection->get_gda_connection()->statement_execute_select(sql_query, error);
#endif

  if(datamodel)
  {
    const guint count = datamodel->get_n_rows();
    //std::cout << "  result: count=" << count << std::endl;
    for(guint row = 0; row < count; ++row)
    {
      std::pair<Gnome::Gda::Value, Gnome::Gda::Value> itempair;
      itempair.first = datamodel->get_value_at(0, row);

      if(with_second)
        itempair.second = datamodel->get_value_at(1, row);

      list_values.push_back(itempair);
    }
  }
  else
  {
      std::cerr << "Glom  get_choice_values(): Error while executing SQL" << std::endl <<
                   "  " <<  sql_query << std::endl;
  }

  return list_values;
}


Glib::ustring Utils::create_name_from_title(const Glib::ustring& title)
{
  Glib::ustring result = string_replace(title, " ", "");
  return result.lowercase(); //TODO: Maybe they need to be ASCII (not UTF8)?
}

Glib::ustring Utils::string_escape_underscores(const Glib::ustring& text)
{
  Glib::ustring result;
  for(Glib::ustring::const_iterator iter = text.begin(); iter != text.end(); ++iter)
  {
    if(*iter == '_')
      result += "__";
    else
      result += *iter;
  }

  return result;
}

/** Get just the first part of a locale, such as de_DE, 
 * ignoring, for instance, .UTF-8 or @euro at the end.
 */
Glib::ustring Utils::locale_simplify(const Glib::ustring& locale_id)
{
  Glib::ustring result = locale_id;

  //Get everything before the .:
  Glib::ustring::size_type posDot = locale_id.find(".");
  if(posDot != Glib::ustring::npos)
  {
    result = result.substr(0, posDot);
  }

  //Get everything before the @:
  const Glib::ustring::size_type posAt = locale_id.find("@");
  if(posAt != Glib::ustring::npos)
  {
    result = result.substr(0, posAt);
  }

  return result;
}

Glib::ustring Utils::locale_language_id(const Glib::ustring& locale_id)
{
  Glib::ustring result;

  const Glib::ustring::size_type posUnderscore = locale_id.find("_");
  if(posUnderscore != Glib::ustring::npos)
  {
    result = locale_id.substr(0, posUnderscore);
  }

  return result;
}

Glib::ustring Utils::create_local_image_uri(const Gnome::Gda::Value& value)
{
  static guint m_temp_image_uri_number = 0;

  Glib::ustring result;

  if(value.get_value_type() == GDA_TYPE_BINARY)
  {
    glong size = 0;
    gconstpointer pData = value.get_binary(size);
    if(size && pData)
    {
      // Note that this is regular binary data, not escaped text representing the data:

      //Save the image to a temporary file and provide the file URI.
      char pchExtraNum[10];
      sprintf(pchExtraNum, "%d", m_temp_image_uri_number);
      result = ("/tmp/glom_report_image_" + Glib::ustring(pchExtraNum) + ".png");
      ++m_temp_image_uri_number;

      std::fstream the_stream(result.c_str(), std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
      if(the_stream)
      {
        the_stream.write((char*)pData, size);
      }
    }
    else
       std::cerr << "Utils::create_local_image_uri(): binary GdaValue contains no data." << std::endl;
  }
  //else
  //  std::cerr << "Utils::create_local_image_uri(): type != BINARY" << std::endl;

  if(result.empty())
    result = "/tmp/glom_report_image_invalid.png";

  return ("file://" + result);
}

Glib::ustring Utils::string_from_decimal(guint decimal)
{
  //TODO_Performance:

  std::stringstream stream;
  stream << decimal;

  Glib::ustring result;
  stream >> result;

  return result;
}

Glib::ustring Utils::title_from_string(const Glib::ustring& text)
{
  Glib::ustring result;

  bool capitalise_next_char = true;
  for(Glib::ustring::const_iterator iter = text.begin(); iter != text.end(); ++iter)
  {
    const gunichar& ch = *iter;
    if(ch == '_') //Replace _ with space.
    {
      capitalise_next_char = true; //Capitalise all words.
      result += " ";
    }
    else
    {
      if(capitalise_next_char)
        result += Glib::Unicode::toupper(*iter);
      else
        result += *iter;

      capitalise_next_char = false;
    }
  }

  return result;
}

Utils::type_vecStrings Utils::string_separate(const Glib::ustring& str, const Glib::ustring& separator, bool ignore_quoted_separator)
{
  //std::cout << "Utils::string_separate(): separator=" << separator << std::endl;

  type_vecStrings result;

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
          //std::cout << "  continue" << std::endl;
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
              //std::cout << "   popping quote" << std::endl;
              //Yes, so remove that quote from our stack, because we found the closing quote:
              m_current_quotes.pop();
            }
            else
            {
              //std::cout << "   pushing quote" << std::endl;
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
        //std::cout << "!in_quotes" << std::endl;

        //Store this item, and start the next item after it:
        item = str.substr(item_start, posComma - item_start);
        //std::cout << "  ITEM. pos_comma=" << posComma << ", ITEM= " << item << std::endl;
        item_start = posComma + size_separator;
      }
      else
      {
        //std::cout << "in quotes." << std::endl;
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
    result.push_back(item);
  } //while

  return result;
}

Glib::ustring Utils::string_trim(const Glib::ustring& str, const Glib::ustring& to_remove)
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

Glib::ustring Utils::string_remove_suffix(const Glib::ustring& str, const Glib::ustring& suffix, bool case_sensitive)
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

/* Run dialog and response on Help if appropriate */

int Utils::dialog_run_with_help(Gtk::Dialog* dialog, const Glib::ustring& id)
{
  int result = dialog->run();
  while (result == Gtk::RESPONSE_HELP)
  {
    show_help(id);
    result = dialog->run();
  }

  dialog->hide();
  return result;
}

/*
 * Help::show_help(const std::string& id)
 *
 * Launch a help browser with the glom help and load the given id if given
 * If the help cannot be found an error dialog will be shown
 */

void Utils::show_help(const Glib::ustring& id)
{
  // TODO_maemo: Show help on maemo by some other means
#ifndef GLOM_ENABLE_MAEMO
  GError* err = 0;
  const gchar* pId;
  if(id.length())
  {
    pId = id.c_str();
  }
  else
  {
    pId = 0;
  }

  try
  {
    const char* path = DATADIR "/gnome/help/glom";
    std::string help_file = locate_help_file(path, "glom.xml");
    if(help_file.empty())
    {
      throw std::runtime_error(_("No help file available"));
    }
    else
    {
      std::string uri = "ghelp:" + help_file;
      if(pId) { uri += "?"; uri += pId; }

      // g_app_info_launch_default_for_uri seems not to be wrapped by giomm
      if(!g_app_info_launch_default_for_uri(uri.c_str(), NULL, &err))
      {
        std::string message(err->message);
        g_error_free(err);
        throw std::runtime_error(message);
      }
    }
  }
  catch(const std::exception& ex)
  {
    const Glib::ustring message = _("Could not display help: ") + Glib::ustring(ex.what());
    Gtk::MessageDialog dialog(message, false, Gtk::MESSAGE_ERROR);
    dialog.run();
  }
#endif
}

void Utils::show_ok_dialog(const Glib::ustring& title, const Glib::ustring& message, Gtk::Window* parent, Gtk::MessageType message_type)
{
#ifdef GLOM_ENABLE_MAEMO
  // TODO_maemo: Map message_type to a senseful stock_id?
  Hildon::Note dialog(Hildon::NOTE_TYPE_INFORMATION, parent, message);
#else
  Gtk::MessageDialog dialog("<b>" + title + "</b>", true /* markup */, message_type, Gtk::BUTTONS_OK);
  dialog.set_secondary_text(message);
  if(parent)
    dialog.set_transient_for(*parent);
#endif

  dialog.run();
}

void Utils::show_ok_dialog(const Glib::ustring& title, const Glib::ustring& message, Gtk::Window& parent, Gtk::MessageType message_type)
{
  show_ok_dialog(title, message, &parent, message_type);
}

namespace
{

static void on_window_hide(Glib::RefPtr<Glib::MainLoop> main_loop, sigc::connection handler_connection)
{
  handler_connection.disconnect(); //This should release a main_loop reference.
  main_loop->quit();

  //main_loop should be destroyed soon, because nothing else is using it.
}

} //anonymous namespace.

void Utils::show_window_until_hide(Gtk::Window* window)
{
  if(!window)
    return;

  Glib::RefPtr<Glib::MainLoop> main_loop = Glib::MainLoop::create(false /* not running */);

  //Stop the main_loop when the window is hidden:
  sigc::connection handler_connection; //TODO: There seems to be a crash if this is on the same line.
  handler_connection = window->signal_hide().connect( 
    sigc::bind(
      sigc::ptr_fun(&on_window_hide),
      main_loop, handler_connection
    ) );
  
  window->show();
  main_loop->run(); //Run and block until it is stopped by the hide signal handler.
}


} //namespace Glom
