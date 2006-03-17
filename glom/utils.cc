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

#include "utils.h"
#include "connectionpool.h"
#include "data_structure/layout/report_parts/layoutitem_fieldsummary.h"
#include "data_structure/glomconversions.h"
#include <libxml++/libxml++.h>
#include <libxslt/transform.h>
#include <libgnomevfsmm.h>
#include <libgnome/gnome-url.h>
#include <sstream> //For stringstream

#include <locale>     // for locale, time_put
#include <ctime>     // for struct tm
#include <iostream>   // for cout, endl
#include <iomanip>

template<class T_Element>
class predicate_UsesRelationshipHasRelationships
{
public:
  predicate_UsesRelationshipHasRelationships(const Glib::ustring& relationship_name, const Glib::ustring& related_relationship_name = Glib::ustring())
  : m_relationship_name(relationship_name),
    m_related_relationship_name(related_relationship_name)
  {
  }

  predicate_UsesRelationshipHasRelationships(const sharedptr<const UsesRelationship> uses_relationship_name)
  : m_relationship_name(uses_relationship_name->get_relationship_name()),
    m_related_relationship_name(uses_relationship_name->get_related_relationship_name())
  {
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

protected:
  Glib::ustring m_relationship_name, m_related_relationship_name;
};


Glib::ustring GlomUtils::trim_whitespace(const Glib::ustring& text)
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

Glib::ustring GlomUtils::string_replace(const Glib::ustring& src, const Glib::ustring search_for, const Glib::ustring& replace_with)
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



Glib::ustring GlomUtils::build_sql_select_with_where_clause(const Glib::ustring& table_name, const type_vecLayoutFields& fieldsToGet, const Glib::ustring& where_clause, const Glib::ustring& sort_clause)
{
  Glib::ustring result;

  Glib::ustring sql_part_fields;
  Glib::ustring sql_part_from;

  typedef std::list< sharedptr<const UsesRelationship> > type_list_relationships;
  type_list_relationships list_relationships;


  for(type_vecLayoutFields::const_iterator iter =  fieldsToGet.begin(); iter != fieldsToGet.end(); ++iter)
  {
    Glib::ustring one_sql_part;

    sharedptr<LayoutItem_Field> layout_item = *iter;

    bool is_summary = false;
    LayoutItem_FieldSummary* fieldsummary = dynamic_cast<LayoutItem_FieldSummary*>(layout_item.obj());
    if(fieldsummary)
      is_summary = true;

    //Add, for instance, "SUM(":
    if(is_summary)
      one_sql_part += fieldsummary->get_summary_type_sql() + "(";

    one_sql_part += ( "\"" + layout_item->get_sql_table_or_join_alias_name(table_name) + "\"." );
    if(layout_item->get_has_relationship_name())
    {
      //Add the relationship to the list:
      type_list_relationships::const_iterator iterFind = std::find_if(list_relationships.begin(), list_relationships.end(), predicate_UsesRelationshipHasRelationships<UsesRelationship>(layout_item) );
      if(iterFind == list_relationships.end()) //If the table is not yet in the list:
      {
        sharedptr<UsesRelationship> uses_rel = sharedptr<UsesRelationship>::create();
        uses_rel->set_relationship(layout_item->get_relationship());
        uses_rel->set_related_relationship(layout_item->get_related_relationship());
        list_relationships.push_back(uses_rel);
      }
    }

    const Glib::ustring name = layout_item->get_name();
    if(!name.empty())
    {
      one_sql_part += ("\"" + name + "\"");

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

  }

  if(sql_part_fields.empty())
  {
    std::cerr << "GlomUtils::build_sql_select_with_where_clause(): sql_part_fields.empty(): fieldsToGet.size()=" << fieldsToGet.size() << std::endl;
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

  if(!sql_part_from.empty())
    result += ("," + sql_part_from);

  if(!sql_part_leftouterjoin.empty())
    result += (" " + sql_part_leftouterjoin);

  if(!where_clause.empty())
    result += " WHERE " + where_clause;

  if(!sort_clause.empty())
    result += " " + sort_clause;

  return result;
}

Glib::ustring GlomUtils::build_sql_select_with_key(const Glib::ustring& table_name, const type_vecLayoutFields& fieldsToGet, const sharedptr<const Field>& key_field, const Gnome::Gda::Value& key_value)
{
  if(!GlomConversions::value_is_empty(key_value)) //If there is a record to show:
  {
    const Glib::ustring where_clause = "\"" + table_name + "\".\"" + key_field->get_name() + "\" = " + key_field->sql(key_value);
    return GlomUtils::build_sql_select_with_where_clause(table_name, fieldsToGet, where_clause);
  }

  return Glib::ustring();
}

GlomUtils::type_list_values_with_second GlomUtils::get_choice_values(const sharedptr<const LayoutItem_Field>& field)
{
  type_list_values_with_second list_values;

  sharedptr<Relationship> choice_relationship;
  Glib::ustring choice_field, choice_second;
  field->get_formatting_used().get_choices(choice_relationship, choice_field, choice_second);
  if(!choice_relationship)
    return list_values;

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

  //Connect to database:
  sharedptr<SharedConnection> connection = ConnectionPool::get_instance()->connect();


  //std::cout << "get_choice_values: Executing SQL: " << sql_query << std::endl;
  Glib::RefPtr<Gnome::Gda::DataModel> datamodel = connection->get_gda_connection()->execute_single_command(sql_query);

  if(datamodel)
  {
    guint count = datamodel->get_n_rows();
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


void GlomUtils::transform_and_open(const xmlpp::Document& xml_document, const Glib::ustring& xsl_file_path, Gtk::Window* parent_window)
{
  //Use libxslt to convert the XML to HTML:
  Glib::ustring result = xslt_process(xml_document, GLOM_XSLTDIR + xsl_file_path);
  std::cout << "After xslt: " << result << std::endl;

  //Save it to a temporary file and show it in a browser:
  //TODO: This actually shows it in gedit.
  const Glib::ustring temp_uri = "file:///tmp/glom_printout.html";
  std::cout << "temp_uri=" << temp_uri << std::endl;

  Gnome::Vfs::Handle write_handle;

  try
  {
    //0660 means "this user and his group can read and write this non-executable file".
    //The 0 prefix means that this is octal.
    write_handle.create(temp_uri, Gnome::Vfs::OPEN_WRITE, false, 0660 /* leading zero means octal */);
  }
  catch(const Gnome::Vfs::exception& ex)
  {
    // If the operation was not successful, print the error and abort
    return; // print_error(ex, output_uri_string);
  }

  try
  {
    //Write the data to the output uri
    /* GnomeVFSFileSize bytes_written = */ write_handle.write(result.data(), result.bytes());
  }
  catch(const Gnome::Vfs::exception& ex)
  {
    // If the operation was not successful, print the error and abort
    return; //print_error(ex, output_uri_string);
  }

  //Give the user a clue, in case the web browser opens in the background, for instance in a new tab:
  if(parent_window)
    Frame_Glom::show_ok_dialog(_("Report Finished"), _("The report will now be opened in your web browser."), *parent_window, Gtk::MESSAGE_INFO);

  //Use the GNOME browser:
  GError* error = 0;
  gnome_url_show(temp_uri.c_str(), &error); //This is in libgnome.
}

Glib::ustring GlomUtils::create_name_from_title(const Glib::ustring& title)
{
  Glib::ustring result = string_replace(title, " ", "");
  return result.lowercase(); //Database titles must be lowercase, it seems. TODO: Maybe they need to be ASCII (not UTF8)?
}

Glib::ustring GlomUtils::xslt_process(const xmlpp::Document& xml_document, const std::string& filepath_xslt)
{
  //Debug output:
  std::cout << "XML before XSLT processing: " << std::endl;
  std::cout << "  ";
  xmlpp::Document& nonconst = const_cast<xmlpp::Document&>(xml_document);
  nonconst.write_to_stream_formatted(std::cout);
  std::cout << std::endl;

  Glib::ustring  result;

  //Use libxslt to transform the XML:
  xmlDocPtr style = xmlReadFile(filepath_xslt.c_str(), 0, 0);
  if(style)
  {
    //Parse the stylesheet:
    xsltStylesheetPtr cur = xsltParseStylesheetDoc(style);
    if(cur)
    {
      //Use the parsed stylesheet on the XML:
      xmlDocPtr pDocOutput = xsltApplyStylesheet(cur, const_cast<xmlDoc*>(xml_document.cobj()), 0);
      xsltFreeStylesheet(cur);

      //Get the output text:
      xmlChar* buffer = 0;
      int length = 0;
      xmlIndentTreeOutput = 1; //Format the output with extra white space. TODO: Is there a better way than this global variable?
      xmlDocDumpFormatMemoryEnc(pDocOutput, &buffer, &length, 0, 0);

      if(buffer)
      {
        // Create a Glib::ustring copy of the buffer

        // Here we force the use of Glib::ustring::ustring( InputIterator begin, InputIterator end )
        // instead of Glib::ustring::ustring( const char*, size_type ) because it
        // expects the length of the string in characters, not in bytes.
        result = Glib::ustring( reinterpret_cast<const char *>(buffer), reinterpret_cast<const char *>(buffer + length) );

        // Deletes the original buffer
        xmlFree(buffer);
      }

      xmlFreeDoc(pDocOutput);
    }
  }

  return result;
}

Glib::ustring GlomUtils::string_escape_underscores(const Glib::ustring& text)
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
Glib::ustring GlomUtils::locale_simplify(const Glib::ustring& locale_id)
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

Glib::ustring GlomUtils::locale_language_id(const Glib::ustring& locale_id)
{
  Glib::ustring result;

  const Glib::ustring::size_type posUnderscore = locale_id.find("_");
  if(posUnderscore != Glib::ustring::npos)
  {
    result = locale_id.substr(0, posUnderscore);
  }

  return result;
}
