/* Glom
 *
 * Copyright (C) 2001-2006 Murray Cumming
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

#include <libglom/report_builder.h>
#include <libglom/utils.h>
#include <libglom/data_structure/glomconversions.h>
#include <libglom/db_utils.h>
#include <libglom/xsl_utils.h>
#include <iostream>
#include <glibmm/miscutils.h>
#include <glibmm/i18n.h>

namespace Glom
{

ReportBuilder::ReportBuilder()
: m_document(0)
{
}

ReportBuilder::~ReportBuilder()
{
}

void ReportBuilder::report_build_headerfooter(const FoundSet& found_set, xmlpp::Element& parent_node, const sharedptr<LayoutGroup>& group)
{
  //Add XML node:
  xmlpp::Element* node = parent_node.add_child(group->get_report_part_id());

  //Add child parts:
  type_vecLayoutItems itemsToGet;
  for(LayoutGroup::type_list_items::iterator iterChildren = group->m_list_items.begin(); iterChildren != group->m_list_items.end(); ++iterChildren)
  {
    sharedptr<LayoutItem> item = *iterChildren;

    sharedptr<LayoutItem_Text> item_text = sharedptr<LayoutItem_Text>::cast_dynamic(item);
    if(item_text)
    {
      report_build_records_text(found_set, *node, item_text);
    }
    else
    {
      sharedptr<LayoutItem_Image> item_image = sharedptr<LayoutItem_Image>::cast_dynamic(item);
      if(item_image)
      {
        report_build_records_image(found_set, *node, item_image);
      }
      else
      {
        sharedptr<LayoutItem_Field> pField = sharedptr<LayoutItem_Field>::cast_dynamic(item);
        if(pField)
        {
          guint col_index = 0; //ignored.
          report_build_records_field(found_set, *node, pField, Glib::RefPtr<Gnome::Gda::DataModel>(), 0, col_index);
        }
        else
        {
          sharedptr<LayoutItem_VerticalGroup> vertical_group = sharedptr<LayoutItem_VerticalGroup>::cast_dynamic(item);
          if(vertical_group)
          {
            //Reuse (a bit hacky) this function for the header and footer:
            guint col_index = 0; //Ignored, because the model is null.
            report_build_records_vertical_group(found_set, *node, vertical_group, Glib::RefPtr<Gnome::Gda::DataModel>(), 0, col_index);
          }
        }
      }
    }
  }

}

void ReportBuilder::report_build_summary(const FoundSet& found_set, xmlpp::Element& parent_node, const sharedptr<LayoutItem_Summary>& summary)
{
  //Add XML node:
  xmlpp::Element* node = parent_node.add_child(summary->get_report_part_id());

  //Get fields
  type_vecLayoutItems itemsToGet;
  for(LayoutGroup::type_list_items::iterator iterChildren = summary->m_list_items.begin(); iterChildren != summary->m_list_items.end(); ++iterChildren)
  {
    sharedptr<LayoutItem> item = *iterChildren;

    sharedptr<LayoutItem_GroupBy> pGroupBy = sharedptr<LayoutItem_GroupBy>::cast_dynamic(item);
    if(pGroupBy)
    {
      //Recurse, adding a sub-groupby block:
      report_build_groupby(found_set, *node, pGroupBy);
    }
    else
    {
      sharedptr<LayoutItem_Summary> pSummary = sharedptr<LayoutItem_Summary>::cast_dynamic(item);
      if(pSummary)
      {
        //Recurse, adding a summary block:
        report_build_summary(found_set, *node, pSummary);
      }
      else
      {
        itemsToGet.push_back( glom_sharedptr_clone(item) );
      }
    }
  }

  if(!itemsToGet.empty())
  {
    //Rows, with data:
    report_build_records(found_set, *node, itemsToGet);
  }
}



void ReportBuilder::report_build_groupby_children(const FoundSet& found_set, xmlpp::Element& node, const sharedptr<LayoutItem_GroupBy>& group_by)
{
  //Get data and add child rows:
  type_vecLayoutItems itemsToGet;
  for(LayoutGroup::type_list_items::iterator iterChildren = group_by->m_list_items.begin(); iterChildren != group_by->m_list_items.end(); ++iterChildren)
  {
    sharedptr<LayoutItem> item = *iterChildren;

    sharedptr<LayoutItem_GroupBy> pGroupBy = sharedptr<LayoutItem_GroupBy>::cast_dynamic(item);
    if(pGroupBy)
    {
      //Recurse, adding a sub-groupby block:
      report_build_groupby(found_set, node, pGroupBy);
    }
    else
    {
      sharedptr<LayoutItem_Summary> pSummary = sharedptr<LayoutItem_Summary>::cast_dynamic(item);
      if(pSummary)
      {
        //Recurse, adding a summary block:
        report_build_summary(found_set, node, pSummary);
      }
      else
      {
        itemsToGet.push_back( glom_sharedptr_clone(item) );
      }
    }
  }

  if(!itemsToGet.empty())
  {
    //Rows, with data:
    FoundSet found_set_records = found_set;
    found_set_records.m_sort_clause = group_by->get_fields_sort_by();
    report_build_records(found_set_records, node, itemsToGet);
  }
}

void ReportBuilder::report_build_groupby(const FoundSet& found_set_parent, xmlpp::Element& parent_node, const sharedptr<LayoutItem_GroupBy>& group_by)
{
  //Get the possible heading values.
  if(group_by->get_has_field_group_by())
  {
    sharedptr<LayoutItem_Field> field_group_by = group_by->get_field_group_by();
    DbUtils::layout_item_fill_field_details(get_document(), found_set_parent.m_table_name, field_group_by);

    //Get the possible group values, ignoring repeats by using GROUP BY.
    const Glib::ustring group_field_table_name = field_group_by->get_table_used(found_set_parent.m_table_name);

    Glib::RefPtr<Gnome::Gda::SqlBuilder> builder =
      Gnome::Gda::SqlBuilder::create(Gnome::Gda::SQL_STATEMENT_SELECT);
    builder->select_add_field(field_group_by->get_name(), group_field_table_name);
    builder->select_add_target(group_field_table_name);

    if(!found_set_parent.m_where_clause.empty())
    {
      builder->set_where( builder->import_expression(found_set_parent.m_where_clause) );
    }

    builder->select_group_by( builder->add_field_id(field_group_by->get_name(), group_field_table_name) ); //TODO: And restrict to the current found set.

    Glib::RefPtr<Gnome::Gda::DataModel> datamodel = DbUtils::query_execute_select(builder);
    if(datamodel)
    {
      guint rows_count = datamodel->get_n_rows();
      for(guint row = 0; row < rows_count; ++row)
      {
        const Gnome::Gda::Value group_value = datamodel->get_value_at(0 /* col*/, row); //TODO: Catch exceptions.


        //Add XML node:
        xmlpp::Element* nodeGroupBy = parent_node.add_child(group_by->get_report_part_id());
        Document::set_node_attribute_value_as_decimal_double(nodeGroupBy, "border_width", group_by->get_border_width());

        nodeGroupBy->set_attribute("group_field", field_group_by->get_title_or_name());
        nodeGroupBy->set_attribute("group_value",
          Conversions::get_text_for_gda_value(field_group_by->get_glom_type(), group_value, field_group_by->get_formatting_used().m_numeric_format) );

        //TODO: Use a SQL parameter instead of using sql().
        Gnome::Gda::SqlExpr where_clause =
          Utils::build_simple_where_expression(group_field_table_name, field_group_by->get_full_field_details(), group_value);

        if(!found_set_parent.m_where_clause.empty())
        {
          where_clause = Utils::build_combined_where_expression(where_clause, found_set_parent.m_where_clause,
            Gnome::Gda::SQL_OPERATOR_TYPE_AND);
        }

        FoundSet found_set_records = found_set_parent;
        found_set_records.m_where_clause = where_clause;

        //Secondary fields. For instance, the Contact Name, in addition to the Contact ID that we group by.
        if(!(group_by->m_group_secondary_fields->m_list_items.empty()))
        {
          xmlpp::Element* nodeSecondaryFields = nodeGroupBy->add_child("secondary_fields");

          type_vecLayoutItems itemsToGet;
          for(LayoutGroup::type_list_items::iterator iterChildren = group_by->m_group_secondary_fields->m_list_items.begin(); iterChildren != group_by->m_group_secondary_fields->m_list_items.end(); ++iterChildren)
          {
            sharedptr<LayoutItem> item = *iterChildren;
            itemsToGet.push_back( glom_sharedptr_clone(item) );
          }

          if(!itemsToGet.empty())
          {
            report_build_records(found_set_records, *nodeSecondaryFields, itemsToGet, true /* one record only */);
          }
        }

        //Get data and add child rows:
        report_build_groupby_children(found_set_records, *nodeGroupBy, group_by);
      }
    }
  }
  else
  {
    //There is no group-by field, so ouput all the found records.
    //For instance, the user could use the GroupBy part just to specify a sort, though that would be a bit of a hack:
    xmlpp::Element* nodeGroupBy = parent_node.add_child(group_by->get_report_part_id()); //We need this to create the HTML table.
    Document::set_node_attribute_value_as_decimal_double(nodeGroupBy, "border_width", group_by->get_border_width());
    report_build_groupby_children(found_set_parent, *nodeGroupBy, group_by);
  }
}

void ReportBuilder::report_build_records_get_fields(const FoundSet& found_set, const sharedptr<LayoutGroup>& group, type_vecLayoutFields& items)
{
  for(LayoutGroup::type_list_items::iterator iterChildren = group->m_list_items.begin(); iterChildren != group->m_list_items.end(); ++iterChildren)
  {
    sharedptr<LayoutItem> item = *iterChildren;

    sharedptr<LayoutItem_VerticalGroup> pVerticalGroup = sharedptr<LayoutItem_VerticalGroup>::cast_dynamic(item);
    if(pVerticalGroup)
    {
      report_build_records_get_fields(found_set, pVerticalGroup, items);
    }
    else
    {
      sharedptr<LayoutItem_Field> pField = sharedptr<LayoutItem_Field>::cast_dynamic(item);
      if(pField)
        items.push_back(pField);
    }
  }
}

void ReportBuilder::report_build_records(const FoundSet& found_set, xmlpp::Element& parent_node, const type_vecLayoutItems& items, bool one_record_only)
{
  if(!items.empty())
  {
    //Add Field headings:
    for(type_vecLayoutItems::const_iterator iter = items.begin(); iter != items.end(); ++iter)
    {
      sharedptr<LayoutItem> layout_item = *iter;
      sharedptr<LayoutItem_Field> layoutitem_field = sharedptr<LayoutItem_Field>::cast_dynamic(layout_item);

      //This adds a field heading (and therefore, column) for fields, or for a vertical group.
      xmlpp::Element* nodeFieldHeading = parent_node.add_child("field_heading");
      if(layoutitem_field && layoutitem_field->get_glom_type() == Field::TYPE_NUMERIC)
        nodeFieldHeading->set_attribute("field_type", "numeric"); //TODO: More sophisticated formatting.

      nodeFieldHeading->set_attribute("name", layout_item->get_name()); //Not really necessary, but maybe useful.
      nodeFieldHeading->set_attribute("title", layout_item->get_title_or_name());
    }

    //Get list of fields to get from the database.
    Utils::type_vecLayoutFields fieldsToGet;
    for(type_vecLayoutItems::const_iterator iter = items.begin(); iter != items.end(); ++iter)
    {
      sharedptr<LayoutItem> layout_item = *iter;
      sharedptr<LayoutItem_Field> layoutitem_field = sharedptr<LayoutItem_Field>::cast_dynamic(layout_item);
      if(layoutitem_field)
        fieldsToGet.push_back(layoutitem_field);
      else
      {
        sharedptr<LayoutItem_VerticalGroup> vertical_group = sharedptr<LayoutItem_VerticalGroup>::cast_dynamic(layout_item);
        if(vertical_group)
        {
          //Get all the fields in this group:
          report_build_records_get_fields(found_set, vertical_group, fieldsToGet);
        }
      }
    }

    //For instance, when we just want to get a name corresponding to a contact ID, and want to ignore duplicates.
    guint limit = 0; //no limit.
    if(one_record_only)
      limit = 1;

    Glib::RefPtr<Gnome::Gda::SqlBuilder> sql_query = Utils::build_sql_select_with_where_clause(found_set.m_table_name,
      fieldsToGet,
      found_set.m_where_clause, sharedptr<const Relationship>() /* extra_join */, found_set.m_sort_clause,
      limit);

    Glib::RefPtr<Gnome::Gda::DataModel> datamodel = DbUtils::query_execute_select(sql_query);
    if(datamodel)
    {
      const guint rows_count = datamodel->get_n_rows();

      for(guint row = 0; row < rows_count; ++row)
      {
        xmlpp::Element* nodeRow = parent_node.add_child("row");

        guint colField = 0;
        for(type_vecLayoutItems::const_iterator iter = items.begin(); iter != items.end(); ++iter)
        {
          xmlpp::Element* nodeField = 0;

          sharedptr<LayoutItem> item = *iter;
          sharedptr<LayoutItem_Field> field = sharedptr<LayoutItem_Field>::cast_dynamic(item);
          if(field)
          {
            report_build_records_field(found_set, *nodeRow, field, datamodel, row, colField);
          }
          else
          {
            sharedptr<LayoutItem_Text> item_text = sharedptr<LayoutItem_Text>::cast_dynamic(item);
            if(item_text)
            {
              report_build_records_text(found_set, *nodeRow, item_text);
            }
            else
            {
              sharedptr<LayoutItem_VerticalGroup> item_verticalgroup = sharedptr<LayoutItem_VerticalGroup>::cast_dynamic(item);
              if(item_verticalgroup)
              {
                report_build_records_vertical_group(found_set, *nodeRow, item_verticalgroup, datamodel, row, colField);
                //TODO
              }
            }
          }

          if(nodeField)
            nodeField->set_attribute("name", item->get_name()); //Not really necessary, but maybe useful.
        }
      }

    }

    //If there are no records, show zero
    //if(!rows_found && show_null_row)
  }
}

void ReportBuilder::report_build_records_field(const FoundSet& found_set, xmlpp::Element& nodeParent, const sharedptr<const LayoutItem_Field>& field, const Glib::RefPtr<Gnome::Gda::DataModel>& datamodel, guint row, guint& colField, bool vertical)
{
  const Field::glom_field_type field_type = field->get_glom_type();

  xmlpp::Element* nodeField = nodeParent.add_child(field->get_report_part_id());
  if(field_type == Field::TYPE_NUMERIC)
    nodeField->set_attribute("field_type", "numeric"); //TODO: More sophisticated formatting.

  if(vertical)
    nodeField->set_attribute("vertical", "true");

  Gnome::Gda::Value value;
  Glib::ustring text_value;

  if(!datamodel) //We call this for headers and footers too.
  {
    //In this case it can only be a system preferences field.
    //So let's get that data here:
    Glib::RefPtr<Gnome::Gda::SqlBuilder> builder = Gnome::Gda::SqlBuilder::create(Gnome::Gda::SQL_STATEMENT_SELECT);
    builder->set_table(field->get_table_used(found_set.m_table_name));
    builder->select_add_field(field->get_name(), found_set.m_table_name);
    builder->select_set_limit(1);
    Glib::RefPtr<Gnome::Gda::DataModel> datamodel = DbUtils::query_execute_select(builder);

    if(!datamodel)
      return;

    value = datamodel->get_value_at(colField, row); //TODO: Catch exceptions.
    colField = 0;
    row = 0;
  }
  else
  {
    value = datamodel->get_value_at(colField, row); //TODO: Catch exceptions.
  }

  nodeField->set_attribute("title", field->get_title_or_name()); //Not always used, but useful.

  //Handle the value:
  if(field_type == Field::TYPE_IMAGE)
     nodeField->set_attribute("image_uri", Utils::create_local_image_uri(value));
  else
  {
    Glib::ustring text_value = Conversions::get_text_for_gda_value(field_type, value, field->get_formatting_used().m_numeric_format);

    //The Postgres summary functions return NULL when summarising NULL records, but 0 is more sensible:
    if(text_value.empty() && sharedptr<const LayoutItem_FieldSummary>::cast_dynamic(field) && (field_type == Field::TYPE_NUMERIC))
    {
      //Use get_text_for_gda_value() instead of "0" so we get the correct numerical formatting:
      const Gnome::Gda::Value value = Conversions::parse_value(0);
      text_value = Conversions::get_text_for_gda_value(field_type, value, field->get_formatting_used().m_numeric_format);
    }

    nodeField->set_attribute("value", text_value);
  }

  ++colField;
}

void ReportBuilder::report_build_records_text(const FoundSet& /* found_set */, xmlpp::Element& nodeParent, const sharedptr<const LayoutItem_Text>& textobject, bool vertical)
{
  //Text object:
  xmlpp::Element* nodeField = nodeParent.add_child(textobject->get_report_part_id()); //We reuse this node type for text objects.
  nodeField->set_attribute("value", textobject->get_text());

  if(vertical)
    nodeField->set_attribute("vertical", "true");
}

void ReportBuilder::report_build_records_image(const FoundSet& /* found_set */, xmlpp::Element& nodeParent, const sharedptr<const LayoutItem_Image>& imageobject, bool vertical)
{
  //Text object:
  xmlpp::Element* nodeImage = nodeParent.add_child(imageobject->get_report_part_id()); //We reuse this node type for text objects.
  nodeImage->set_attribute("image_uri", imageobject->create_local_image_uri());

  if(vertical)
    nodeImage->set_attribute("vertical", "true");
}

void ReportBuilder::report_build_records_vertical_group(const FoundSet& found_set, xmlpp::Element& parentNode, const sharedptr<LayoutItem_VerticalGroup>& group, const Glib::RefPtr<Gnome::Gda::DataModel>& datamodel, guint row, guint& field_index)
{
  xmlpp::Element* nodeGroupVertical = parentNode.add_child(group->get_report_part_id());

  for(LayoutGroup::type_list_items::iterator iterChildren = group->m_list_items.begin(); iterChildren != group->m_list_items.end(); ++iterChildren)
  {
    sharedptr<LayoutItem> item = *iterChildren;

    sharedptr<LayoutItem_VerticalGroup> pVerticalGroup = sharedptr<LayoutItem_VerticalGroup>::cast_dynamic(item);
    if(pVerticalGroup)
    {
      report_build_records_vertical_group(found_set, *nodeGroupVertical, pVerticalGroup, datamodel, row, field_index);
    }
    else
    {
      sharedptr<LayoutItem_Field> pField = sharedptr<LayoutItem_Field>::cast_dynamic(item);
      if(pField)
      {
        report_build_records_field(found_set, *nodeGroupVertical, pField, datamodel, row, field_index, true /* vertical, so we get a row for each field too. */);
      }
      else
      {
        sharedptr<LayoutItem_Text> pText = sharedptr<LayoutItem_Text>::cast_dynamic(item);
        if(pText)
        {
          report_build_records_text(found_set, *nodeGroupVertical, pText, true);
        }
      }
    }
  }
}

//TODO: Return a URI
std::string ReportBuilder::report_build_and_save(const FoundSet& found_set, const sharedptr<const Report>& report)
{
  const Glib::ustring contents = report_build(found_set, report);

 //Save it to a temporary file and show it in a browser:
  const Glib::ustring temp_uri = Utils::get_temp_file_uri("glom_printout", "html");
  std::cout << G_STRFUNC << ": temp_uri=" << temp_uri << std::endl;

  Glib::RefPtr<Gio::File> file = Gio::File::create_for_uri(temp_uri);
  Glib::RefPtr<Gio::FileOutputStream> stream;

  //Create the file if it does not already exist:
  try
  {
    if(file->query_exists())
    {
      stream = file->replace(); //Instead of append_to().
    }
    else
    {
      //By default files created are generally readable by everyone, but if we pass FILE_CREATE_PRIVATE in flags the file will be made readable only to the current user, to the level that is supported on the target filesystem.
      //TODO: Do we want to specify 0660 exactly? (means "this user and his group can read and write this non-executable file".)
      stream = file->create_file();
    }
  }
  catch(const Gio::Error& ex)
  {
    // If the operation was not successful, print the error and abort
    return std::string(); // print_error(ex, output_uri_string);
  }

  //Write the data to the output uri
  gssize bytes_written = 0;
  const Glib::ustring::size_type result_bytes = contents.bytes();
  try
  {
    bytes_written = stream->write(contents.data(), result_bytes);
  }
  catch(const Gio::Error& ex)
  {
    // If the operation was not successful, print the error and abort
    return std::string(); // false; //print_error(ex, output_uri_string);
  }

  if(bytes_written != (gssize)result_bytes)
    return std::string(); //false

  return file->get_path();
}
 

Glib::ustring ReportBuilder::report_build(const FoundSet& found_set, const sharedptr<const Report>& report)
{
  //Create a DOM Document with the XML:
  xmlpp::DomParser dom_parser;;

  xmlpp::Document* pDocument = dom_parser.get_document();
  xmlpp::Element* nodeRoot = pDocument->get_root_node();
  if(!nodeRoot)
  {
    //Add it if it isn't there already:
    nodeRoot = pDocument->create_root_node("report_print");
  }

  Glib::ustring table_title = get_document()->get_table_title(found_set.m_table_name);
  if(table_title.empty())
    table_title = found_set.m_table_name;

  nodeRoot->set_attribute("table", table_title);
  if(report->get_show_table_title())
    nodeRoot->set_attribute("show_table_title", "true");


  //The groups:
  xmlpp::Element* nodeParent = nodeRoot;


  nodeRoot->set_attribute("title", report->get_title_or_name());

  type_vecLayoutItems itemsToGet_TopLevel;

  for(LayoutGroup::type_list_items::const_iterator iter = report->m_layout_group->m_list_items.begin(); iter != report->m_layout_group->m_list_items.end(); ++iter)
  {
    sharedptr<LayoutItem> pPart = *iter;

    //The Group, and the details for each record in the group:
    sharedptr<LayoutItem_GroupBy> pGroupBy = sharedptr<LayoutItem_GroupBy>::cast_dynamic(pPart);
    if(pGroupBy)
      report_build_groupby(found_set, *nodeParent, pGroupBy);
    else
    {
      sharedptr<LayoutItem_Summary> pSummary = sharedptr<LayoutItem_Summary>::cast_dynamic(pPart);
      if(pSummary)
      {
        //Recurse, adding a summary block:
        report_build_summary(found_set, *nodeParent, pSummary);
      }
      else
      {
        sharedptr<LayoutGroup> pGroup = sharedptr<LayoutGroup>::cast_dynamic(pPart);
        if(pGroup)
        {
          sharedptr<LayoutItem_Header> pHeader = sharedptr<LayoutItem_Header>::cast_dynamic(pPart);
          sharedptr<LayoutItem_Footer> pFooter = sharedptr<LayoutItem_Footer>::cast_dynamic(pPart);
          if(pHeader || pFooter)
          {
            //Recurse, adding a summary block:
            report_build_headerfooter(found_set, *nodeParent, pGroup);
          }
        }
        else
          itemsToGet_TopLevel.push_back( glom_sharedptr_clone(pPart) );
      }
    }
  }

  //Add top-level records, outside of any groupby or summary, if fields have been specified:
  if(!itemsToGet_TopLevel.empty())
  {
    xmlpp::Element* nodeGroupBy = nodeParent->add_child("ungrouped_records");
    report_build_records(found_set, *nodeGroupBy, itemsToGet_TopLevel);
  }

  return GlomXslUtils::transform(*pDocument, "print_report_to_html.xsl");
}

static void fill_standard_list_report_fill(const sharedptr<Report>& report, const sharedptr<const LayoutGroup>& layout_group)
{
  if(!report)
    return;

  if(!layout_group)
    return;

  for(LayoutGroup::type_list_items::const_iterator iter = layout_group->m_list_items.begin(); iter != layout_group->m_list_items.end(); ++iter)
  {
    const sharedptr<const LayoutItem> item = *iter;
    if(!item)
      continue;

    const sharedptr<LayoutItem> unconst = sharedptr<LayoutItem>::cast_const(item); //TODO: Avoid this?
    report->m_layout_group->add_item(unconst);
  }
}

sharedptr<Report> ReportBuilder::create_standard_list_report(const Document* document, const Glib::ustring& table_name)
{
  sharedptr<Report> result(new Report());
  result->set_name("list");
  //Translators: This is a noun. It is the title of a report.
  result->set_title(_("List"));

  const Document::type_list_layout_groups layout_groups = 
    document->get_data_layout_groups("list", table_name); //TODO: layout_platform.
  for(Document::type_list_layout_groups::const_iterator iter = layout_groups.begin(); iter != layout_groups.end(); ++iter)
  {
    const sharedptr<const LayoutGroup> group = *iter;
    if(group)
      fill_standard_list_report_fill(result, group);
  }

  return result;
}

void ReportBuilder::set_document(Document* document)
{
  m_document = document;
}

Document* ReportBuilder::get_document()
{
  return m_document;
}


} //namespace Glom
