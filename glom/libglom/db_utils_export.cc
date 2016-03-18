/* Glom
 *
 * Copyright (C) 2001-2010 Murray Cumming
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


#include <libglom/db_utils_export.h>
#include <libglom/db_utils.h>
#include <libglom/layout_utils.h>
#include <libglom/utils.h>
#include <libglom/sql_utils.h>
#include <libglom/algorithms_utils.h>
#include <libgdamm/metastore.h>
#include <iostream>

namespace Glom
{

namespace DbUtils
{


//TODO: Reduce copy/pasting in these export_data_to_*() methods:
void export_data_to_vector(const std::shared_ptr<Document>& document, Document::type_example_rows& the_vector, const FoundSet& found_set, const Document::type_list_const_layout_groups& sequence)
{
  auto fieldsSequence = Utils::get_table_fields_to_show_for_sequence(document, found_set.m_table_name, sequence);

  if(fieldsSequence.empty())
  {
    std::cerr << G_STRFUNC << ": No fields in sequence.\n";
    return;
  }

  auto query = SqlUtils::build_sql_select_with_where_clause(found_set.m_table_name, fieldsSequence, found_set.m_where_clause, found_set.m_extra_join, found_set.m_sort_clause);

  //TODO: Lock the database (prevent changes) during export.
  auto result = DbUtils::query_execute_select(query);

  guint rows_count = 0;
  if(result)
    rows_count = result->get_n_rows();

  if(rows_count)
  {
    const guint columns_count = result->get_n_columns();

    for(guint row_index = 0; row_index < rows_count; ++row_index)
    {
        Document::type_row_data row_data;

        for(guint col_index = 0; col_index < columns_count; ++col_index)
        {
          const auto value = result->get_value_at(col_index, row_index);

          auto layout_item = fieldsSequence[col_index];
          //if(layout_item->m_field.get_glom_type() != Field::glom_field_type::IMAGE) //This is too much data.
          //{

            //Output data in canonical SQL format, ignoring the user's locale, and ignoring the layout formatting:
            row_data.emplace_back(value);  //TODO_Performance: reserve the size.

            //if(layout_item->m_field.get_glom_type() == Field::glom_field_type::IMAGE) //This is too much data.
            //{
             //std::cout << "  field name=" << layout_item->get_name() << ", value=" << layout_item->m_field.sql(value) << std::endl;
            //}
        }

        //std::cout << " row_string=" << row_string << std::endl;
        the_vector.emplace_back(row_data); //TODO_Performance: Reserve the size.
    }
  }
}

void export_data_to_stream(const std::shared_ptr<Document>& document, std::ostream& the_stream, const FoundSet& found_set, const Document::type_list_const_layout_groups& sequence)
{
  auto fieldsSequence = Utils::get_table_fields_to_show_for_sequence(document, found_set.m_table_name, sequence);

  if(fieldsSequence.empty())
  {
    std::cerr << G_STRFUNC << ": No fields in sequence.\n";
    return;
  }

  auto query = SqlUtils::build_sql_select_with_where_clause(found_set.m_table_name, fieldsSequence, found_set.m_where_clause, found_set.m_extra_join, found_set.m_sort_clause);

  //TODO: Lock the database (prevent changes) during export.
  auto result = DbUtils::query_execute_select(query);

  guint rows_count = 0;
  if(result)
    rows_count = result->get_n_rows();

  if(rows_count)
  {
    const guint columns_count = result->get_n_columns();

    for(guint row_index = 0; row_index < rows_count; ++row_index)
    {
        std::string row_string;

        for(guint col_index = 0; col_index < columns_count; ++col_index)
        {
          const auto value = result->get_value_at(col_index, row_index);

          auto layout_item = fieldsSequence[col_index];
          //if(layout_item->m_field.get_glom_type() != Field::glom_field_type::IMAGE) //This is too much data.
          //{
            if(!row_string.empty())
              row_string += ",";

            //Output data in canonical SQL format, ignoring the user's locale, and ignoring the layout formatting:
            auto field = layout_item->get_full_field_details();
            if(!field)
            {
              std::cerr << G_STRFUNC << ": A field was null.\n";
              return;
            }

            const auto field_text = field->to_file_format(value);

            if(layout_item->get_glom_type() == Field::glom_field_type::IMAGE) //This is too much data.
            {
              // Some extra debug checks,
              // though we believe that all these problems are now fixed in File::to_file_format():

              const char* newline_to_find = "\r\n";
              size_t pos = field_text.find_first_of(newline_to_find);
              if(pos != std::string::npos)
              {
                std::cerr << G_STRFUNC << ": export: binary data field text contains an unexpected newline: " << field_text << std::endl;
                continue;
              }

              const char* quote_to_find = "\"";
              pos = field_text.find_first_of(quote_to_find);
              if(pos != std::string::npos)
              {
                std::cerr << G_STRFUNC << ": export: binary data field text contains an unexpected quote: " << field_text << std::endl;
                continue;
              }
            }

            if(layout_item->get_glom_type() == Field::glom_field_type::TEXT)
            {
              //The CSV RFC says text may be quoted and should be if it has newlines:
              //TODO: Escape the text?
              row_string += ("\"" + field_text + "\"");
            }
            else
              row_string += field_text;


            //std::cout << "  field name=" << layout_item->get_name() << ", value=" << layout_item->m_field.sql(value) << std::endl;
          //}
        }

        //std::cout << " row_string=" << row_string << std::endl;
        the_stream << row_string << std::endl;
    }
  }
}

} //namespace DbUtils

} //namespace Glom
