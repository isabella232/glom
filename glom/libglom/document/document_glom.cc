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

#include <glom/libglom/document/document_glom.h>
#include <glom/libglom/utils.h>
#include <glom/libglom/data_structure/glomconversions.h>
#include <glom/libglom/data_structure/layout/report_parts/layoutitem_summary.h>
#include <glom/libglom/data_structure/layout/report_parts/layoutitem_fieldsummary.h>
#include <glom/libglom/data_structure/layout/report_parts/layoutitem_verticalgroup.h>
#include <glom/libglom/data_structure/layout/report_parts/layoutitem_header.h>
#include <glom/libglom/data_structure/layout/report_parts/layoutitem_footer.h>
#include <glom/libglom/data_structure/layout/layoutitem_button.h>
#include <glom/libglom/data_structure/layout/layoutitem_text.h>
#include <glom/libglom/data_structure/layout/layoutitem_image.h>
#include <glom/libglom/standard_table_prefs_fields.h>
#include <libgnomevfsmm/uri.h>
#include <bakery/Utilities/BusyCursor.h>

#include <glom/libglom/connectionpool.h>

#include <gtk/gtkpagesetup.h> //TODO: Remove this when we can use the C++ constructor.

#include <glibmm/i18n.h>
//#include "config.h" //To get GLOM_DTD_INSTALL_DIR - dependent on configure prefix.
#include <algorithm> //For std::find_if().
#include <sstream> //For stringstream

namespace Glom
{

#define GLOM_NODE_CONNECTION "connection"
#define GLOM_ATTRIBUTE_CONNECTION_SELF_HOSTED "self_hosted"
#define GLOM_ATTRIBUTE_CONNECTION_SERVER "server"
#define GLOM_ATTRIBUTE_CONNECTION_USER "user"
#define GLOM_ATTRIBUTE_CONNECTION_DATABASE "database"

#define GLOM_NODE_DATA_LAYOUT_GROUPS "data_layout_groups"
#define GLOM_NODE_DATA_LAYOUT_GROUP "data_layout_group"
#define GLOM_ATTRIBUTE_COLUMNS_COUNT "columns_count"
#define GLOM_ATTRIBUTE_BORDER_WIDTH "border_width"

#define GLOM_NODE_DATA_LAYOUTS "data_layouts"
#define GLOM_NODE_DATA_LAYOUT "data_layout"
#define GLOM_ATTRIBUTE_PARENT_TABLE_NAME "parent_table"

#define GLOM_NODE_DATA_LAYOUT_NOTEBOOK "data_layout_notebook"
#define GLOM_NODE_DATA_LAYOUT_PORTAL "data_layout_portal"
#define GLOM_NODE_DATA_LAYOUT_PORTAL_NAVIGATIONRELATIONSHIP "portal_navigation_relationship"
#define GLOM_ATTRIBUTE_PORTAL_NAVIGATIONRELATIONSHIP_MAIN "navigation_main"
#define GLOM_NODE_DATA_LAYOUT_ITEM "data_layout_item" //A field.
#define GLOM_NODE_LAYOUT_ITEM_CUSTOM_TITLE "title_custom"
#define GLOM_ATTRIBUTE_LAYOUT_ITEM_CUSTOM_TITLE_USE "use_custom"
#define GLOM_NODE_DATA_LAYOUT_BUTTON "data_layout_button"
#define GLOM_NODE_DATA_LAYOUT_TEXTOBJECT "data_layout_text"
#define GLOM_NODE_DATA_LAYOUT_TEXTOBJECT_TEXT "text"
#define GLOM_NODE_DATA_LAYOUT_IMAGEOBJECT "data_layout_image"
#define GLOM_ATTRIBUTE_DATA_LAYOUT_IMAGEOBJECT_IMAGE "text"
#define GLOM_ATTRIBUTE_DATA_LAYOUT_ITEM_FIELD_USE_DEFAULT_FORMATTING "use_default_formatting"
#define GLOM_NODE_DATA_LAYOUT_ITEM_GROUPBY "data_layout_item_groupby"
#define GLOM_NODE_DATA_LAYOUT_GROUP_SECONDARYFIELDS "secondary_fields"
#define GLOM_NODE_DATA_LAYOUT_ITEM_VERTICALGROUP "data_layout_item_verticalgroup"
#define GLOM_NODE_DATA_LAYOUT_ITEM_SUMMARY "data_layout_item_summary"
#define GLOM_NODE_DATA_LAYOUT_ITEM_FIELDSUMMARY "data_layout_item_fieldsummary"
#define GLOM_NODE_DATA_LAYOUT_ITEM_HEADER "data_layout_item_header"
#define GLOM_NODE_DATA_LAYOUT_ITEM_FOOTER "data_layout_item_footer"
#define GLOM_NODE_TABLE "table"
#define GLOM_NODE_FIELDS "fields"
#define GLOM_NODE_FIELD "field"
#define GLOM_ATTRIBUTE_PRIMARY_KEY "primary_key"
#define GLOM_ATTRIBUTE_DEFAULT_VALUE "default_value"
#define GLOM_ATTRIBUTE_UNIQUE "unique"
#define GLOM_ATTRIBUTE_AUTOINCREMENT "auto_increment"
#define GLOM_DEPRECATED_ATTRIBUTE_CALCULATION "calculation"
#define GLOM_NODE_CALCULATION "calculation"
#define GLOM_ATTRIBUTE_TYPE "type"

#define GLOM_NODE_FIELD_LOOKUP "field_lookup"
#define GLOM_NODE_RELATIONSHIPS "relationships"
#define GLOM_NODE_RELATIONSHIP "relationship"
#define GLOM_ATTRIBUTE_KEY "key"
#define GLOM_ATTRIBUTE_OTHER_TABLE "other_table"
#define GLOM_ATTRIBUTE_OTHER_KEY "other_key"
#define GLOM_ATTRIBUTE_AUTO_CREATE "auto_create"
#define GLOM_ATTRIBUTE_ALLOW_EDIT "allow_edit"

#define GLOM_NODE_GROUPS "groups"
#define GLOM_NODE_GROUP "group"
#define GLOM_ATTRIBUTE_DEVELOPER "developer"
#define GLOM_NODE_TABLE_PRIVS "table_privs"
#define GLOM_ATTRIBUTE_TABLE_NAME "table_name"
#define GLOM_ATTRIBUTE_PRIV_VIEW "priv_view"
#define GLOM_ATTRIBUTE_PRIV_EDIT "priv_edit"
#define GLOM_ATTRIBUTE_PRIV_CREATE "priv_create"
#define GLOM_ATTRIBUTE_PRIV_DELETE "priv_delete"

#define GLOM_ATTRIBUTE_FORMAT_VERSION "format_version"
#define GLOM_ATTRIBUTE_IS_EXAMPLE "is_example"
#define GLOM_ATTRIBUTE_CONNECTION_DATABASE_TITLE "database_title"
#define GLOM_ATTRIBUTE_TRANSLATION_ORIGINAL_LOCALE "translation_original_locale"
#define GLOM_ATTRIBUTE_NAME "name"
#define GLOM_ATTRIBUTE_TITLE "title"
#define GLOM_ATTRIBUTE_SEQUENCE "sequence"
#define GLOM_ATTRIBUTE_HIDDEN "hidden"
#define GLOM_ATTRIBUTE_DEFAULT "default"
#define GLOM_ATTRIBUTE_OVERVIEW_X "overview_x"
#define GLOM_ATTRIBUTE_OVERVIEW_Y "overview_y"
#define GLOM_ATTRIBUTE_FIELD "field"
#define GLOM_ATTRIBUTE_EDITABLE "editable"
#define GLOM_DEPRECATED_ATTRIBUTE_EXAMPLE_ROWS "example_rows"
#define GLOM_NODE_EXAMPLE_ROWS "example_rows"
#define GLOM_NODE_EXAMPLE_ROW "example_row"
#define GLOM_NODE_VALUE "value"
#define GLOM_ATTRIBUTE_COLUMN "column"
#define GLOM_DEPRECATED_ATTRIBUTE_BUTTON_SCRIPT "script"
#define GLOM_NODE_BUTTON_SCRIPT "script"
#define GLOM_ATTRIBUTE_SORT_ASCENDING "sort_ascending"



#define GLOM_ATTRIBUTE_RELATIONSHIP_NAME "relationship"
#define GLOM_ATTRIBUTE_RELATED_RELATIONSHIP_NAME "related_relationship"

#define GLOM_NODE_REPORTS "reports"
#define GLOM_NODE_REPORT "report"
#define GLOM_ATTRIBUTE_REPORT_SHOW_TABLE_TITLE "show_table_title"
#define GLOM_NODE_REPORT_ITEM_GROUPBY_GROUPBY "groupby"
#define GLOM_NODE_REPORT_ITEM_GROUPBY_SORTBY "sortby"
#define GLOM_ATTRIBUTE_LAYOUT_ITEM_FIELDSUMMARY_SUMMARYTYPE "summarytype"

#define GLOM_NODE_PRINT_LAYOUTS "print_layouts"
#define GLOM_NODE_PRINT_LAYOUT "print_layout"

#define GLOM_NODE_FORMAT "formatting"
#define GLOM_ATTRIBUTE_FORMAT_THOUSANDS_SEPARATOR "format_thousands_separator"
#define GLOM_ATTRIBUTE_FORMAT_DECIMAL_PLACES_RESTRICTED "format_decimal_places_restricted"
#define GLOM_ATTRIBUTE_FORMAT_DECIMAL_PLACES "format_decimal_places"
#define GLOM_ATTRIBUTE_FORMAT_CURRENCY_SYMBOL "format_currency_symbol"

#define GLOM_ATTRIBUTE_FORMAT_TEXT_MULTILINE "format_text_multiline"
#define GLOM_ATTRIBUTE_FORMAT_TEXT_MULTILINE_HEIGHT_LINES "format_text_multiline_height_lines"
#define GLOM_ATTRIBUTE_FORMAT_TEXT_FONT "font"
#define GLOM_ATTRIBUTE_FORMAT_TEXT_COLOR_FOREGROUND "color_fg"
#define GLOM_ATTRIBUTE_FORMAT_TEXT_COLOR_BACKGROUND "color_bg"

#define GLOM_ATTRIBUTE_FORMAT_CHOICES_RESTRICTED "choices_restricted"
#define GLOM_ATTRIBUTE_FORMAT_CHOICES_CUSTOM "choices_custom"
#define GLOM_ATTRIBUTE_FORMAT_CHOICES_CUSTOM_LIST "custom_choice_list"
#define GLOM_NODE_FORMAT_CUSTOM_CHOICE "custom_choice"
#define GLOM_ATTRIBUTE_VALUE "value"
#define GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED "choices_related"
#define GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_RELATIONSHIP "choices_related_relationship"
#define GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_FIELD "choices_related_field"
#define GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_SECOND "choices_related_second"

#define GLOM_NODE_TRANSLATIONS_SET "trans_set"
#define GLOM_NODE_TRANSLATION "trans"
#define GLOM_ATTRIBUTE_TRANSLATION_LOCALE "loc"
#define GLOM_ATTRIBUTE_TRANSLATION_VALUE "val"

#define GLOM_NODE_POSITION "position"
#define GLOM_ATTRIBUTE_POSITION_X "x"
#define GLOM_ATTRIBUTE_POSITION_Y "y"
#define GLOM_ATTRIBUTE_POSITION_WIDTH "width"
#define GLOM_ATTRIBUTE_POSITION_HEIGHT "height"

#define GLOM_NODE_PAGE_SETUP "page_setup" //It's text child is the keyfile for a GtkPageSetup

#define GLOM_NODE_LIBRARY_MODULES "library_modules"
#define GLOM_NODE_LIBRARY_MODULE "module"
#define GLOM_ATTRIBUTE_LIBRARY_MODULE_NAME "name"
#define GLOM_ATTRIBUTE_LIBRARY_MODULE_SCRIPT "script"

//A built-in relationship that is available for every table:
#define GLOM_RELATIONSHIP_NAME_SYSTEM_PROPERTIES "system_properties"

Document_Glom::Document_Glom()
:
#ifndef GLOM_ENABLE_CLIENT_ONLY
  m_connection_is_self_hosted(false),
#endif // !GLOM_ENABLE_CLIENT_ONLY
  m_block_cache_update(false),
  m_block_modified_set(false),
#ifndef GLOM_ENABLE_CLIENT_ONLY
  m_allow_auto_save(true), //Save all changes immediately, by default.
#endif // !GLOM_ENABLE_CLIENT_ONLY
  m_is_example(false),
  m_parent_window(0)
{
  m_document_format_version = get_latest_known_document_format_version(); //Default to this for new documents.

  //Conscious use of virtual methods in a constructor:
  set_file_extension("glom");

  set_dtd_name("glom_document.dtd");
  //set_DTD_Location(GLOM_DTD_INSTALL_DIR); //Determined at configure time. It still looks in the working directory first.

  set_dtd_root_node_name("glom_document");

  //Don't add newlines automatically, because we have text nodes that this might affect:
  //It doesn't seem to work anyway.
  //set_write_formatted(); //Make the output more human-readable, just in case.

  //Set default database name:
  //This is also the XML attribute default value,
  //but that isn't available for new documents.
  if(get_connection_server().empty())
    set_connection_server("localhost");

  set_translation_original_locale(TranslatableItem::get_current_locale()); //By default, we assume that the original is in the current locale. We must do this here so that TranslatableItem::set/get_title() knows.

  m_app_state.signal_userlevel_changed().connect( sigc::mem_fun(*this, &Document_Glom::on_app_state_userlevel_changed) );
}

Document_Glom::~Document_Glom()
{
#ifndef GLOM_ENABLE_CLIENT_ONLY
  //It would be better to do this in a Application::on_document_closed() virtual method,
  //but that would need an ABI break in Bakery:
  if(get_connection_is_self_hosted())
  {
    ConnectionPool* connection_pool = ConnectionPool::get_instance();
    if(!connection_pool)
      return;

    connection_pool->stop_self_hosting();
  }
#endif // !GLOM_ENABLE_CLIENT_ONLY
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
bool Document_Glom::get_connection_is_self_hosted() const
{
  return m_connection_is_self_hosted;
}

std::string Document_Glom::get_connection_self_hosted_directory_uri() const
{
  const std::string uri_file = get_file_uri();
  if(uri_file.empty())
  {
    g_warning("Document_Glom::get_connection_self_hosted_directory_uri(): file_uri is empty.");
    return std::string();
  }
  else
  {
    Glib::RefPtr<Gnome::Vfs::Uri> vfsuri = Gnome::Vfs::Uri::create(uri_file);

    Glib::RefPtr<Gnome::Vfs::Uri> vfsuri_parent = vfsuri->get_parent();
    if(vfsuri_parent)
    {
      Glib::RefPtr<Gnome::Vfs::Uri> datadir = vfsuri_parent->append_string("glom_postgres_data");
      return datadir->to_string();
    }
    else
    {
      g_warning("Document_Glom::get_connection_self_hosted_directory_uri(): get_parent() returned empty.");
      return std::string();
    }
  }
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

Glib::ustring Document_Glom::get_connection_user() const
{
  return m_connection_user;
}

Glib::ustring Document_Glom::get_connection_server() const
{
  return m_connection_server;
}

Glib::ustring Document_Glom::get_connection_database() const
{
  return m_connection_database;
}

void Document_Glom::set_connection_user(const Glib::ustring& strVal)
{
  if(strVal != m_connection_user)
  {
    m_connection_user = strVal;
    set_modified();
  }
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void Document_Glom::set_connection_is_self_hosted(bool self_hosted)
{
  if(self_hosted != m_connection_is_self_hosted)
  {
    m_connection_is_self_hosted = self_hosted;
    set_modified();
  }
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

void Document_Glom::set_connection_server(const Glib::ustring& strVal)
{
  if(strVal != m_connection_server)
  {
    m_connection_server = strVal;
    set_modified();
  }
}

void Document_Glom::set_connection_database(const Glib::ustring& strVal)
{
  if(strVal != m_connection_database)
  {
    m_connection_database = strVal;
    set_modified();
  }
}

void Document_Glom::set_relationship(const Glib::ustring& table_name, const sharedptr<Relationship>& relationship)
{
  //Find the existing relationship:
  type_tables::iterator iterFind = m_tables.find(table_name);
  if(iterFind != m_tables.end())
  {
    DocumentTableInfo& info = iterFind->second;

    //Look for the relationship with this name:
    bool existing = false;
    const Glib::ustring relationship_name = glom_get_sharedptr_name(relationship);

    for(type_vecRelationships::iterator iter = info.m_relationships.begin(); iter != info.m_relationships.end(); ++iter)
    {
      if((*iter)->get_name() == relationship_name)
      {
        *iter = relationship; //Changes the relationship. All references (sharedptrs) to the relationship will get the informatin too, because it is shared.
        existing = true;
      } 
    }

    if(!existing)
    {
      //Add a new one if it's not there.
      info.m_relationships.push_back(relationship);
    }
  }
}

sharedptr<Relationship> Document_Glom::create_relationship_system_preferences(const Glib::ustring& table_name)
{
  sharedptr<Relationship> relationship = sharedptr<Relationship>::create();
  relationship->set_name(GLOM_RELATIONSHIP_NAME_SYSTEM_PROPERTIES);
  relationship->set_title(_("System Preferences"));
  relationship->set_from_table(table_name);
  relationship->set_to_table(GLOM_STANDARD_TABLE_PREFS_TABLE_NAME);
  relationship->set_allow_edit(false);

  return relationship;
}

sharedptr<TableInfo> Document_Glom::create_table_system_preferences()
{
  type_vecFields fields_ignored;
  return create_table_system_preferences(fields_ignored);
}

sharedptr<TableInfo> Document_Glom::create_table_system_preferences(type_vecFields& fields)
{
  sharedptr<TableInfo> prefs_table_info = sharedptr<TableInfo>::create();
  prefs_table_info->set_name(GLOM_STANDARD_TABLE_PREFS_TABLE_NAME);
  prefs_table_info->set_title(_("System Preferences"));
  prefs_table_info->m_hidden = true;


  fields.clear();

  sharedptr<Field> primary_key(new Field()); //It's not used, because there's only one record, but we must have one.
  primary_key->set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_ID);
  primary_key->set_glom_type(Field::TYPE_NUMERIC);
  fields.push_back(primary_key);

  sharedptr<Field> field_name(new Field());
  field_name->set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_NAME);
  field_name->set_title(_("System Name"));
  field_name->set_glom_type(Field::TYPE_TEXT);
  fields.push_back(field_name);

  sharedptr<Field> field_org_name(new Field());
  field_org_name->set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_NAME);
  field_org_name->set_title(_("Organisation Name"));
  field_org_name->set_glom_type(Field::TYPE_TEXT);
  fields.push_back(field_org_name);

  sharedptr<Field> field_org_logo(new Field());
  field_org_logo->set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_LOGO);
  field_org_logo->set_title(_("Organisation Logo"));
  field_org_logo->set_glom_type(Field::TYPE_IMAGE);
  fields.push_back(field_org_logo);

  sharedptr<Field> field_org_address_street(new Field());
  field_org_address_street->set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_STREET);
  field_org_address_street->set_title(_("Street"));
  field_org_address_street->set_glom_type(Field::TYPE_TEXT);
  fields.push_back(field_org_address_street);

  sharedptr<Field> field_org_address_street2(new Field());
  field_org_address_street2->set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_STREET2);
  field_org_address_street2->set_title(_("Street (line 2)"));
  field_org_address_street2->set_glom_type(Field::TYPE_TEXT);
  fields.push_back(field_org_address_street2);

  sharedptr<Field> field_org_address_town(new Field());
  field_org_address_town->set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_TOWN);
  field_org_address_town->set_title(_("City"));
  field_org_address_town->set_glom_type(Field::TYPE_TEXT);
  fields.push_back(field_org_address_town);

  sharedptr<Field> field_org_address_county(new Field());
  field_org_address_county->set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_COUNTY);
  field_org_address_county->set_title(_("State"));
  field_org_address_county->set_glom_type(Field::TYPE_TEXT);
  fields.push_back(field_org_address_county);

  sharedptr<Field> field_org_address_country(new Field());
  field_org_address_country->set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_COUNTRY);
  field_org_address_country->set_title(_("Country"));
  field_org_address_country->set_glom_type(Field::TYPE_TEXT);
  fields.push_back(field_org_address_country);

  sharedptr<Field> field_org_address_postcode(new Field());
  field_org_address_postcode->set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_POSTCODE);
  field_org_address_postcode->set_title(_("Zip Code"));
  field_org_address_postcode->set_glom_type(Field::TYPE_TEXT);
  fields.push_back(field_org_address_postcode);

  return prefs_table_info;
}

bool Document_Glom::get_relationship_is_system_properties(const sharedptr<const Relationship>& relationship)
{
  return relationship->get_name() == GLOM_RELATIONSHIP_NAME_SYSTEM_PROPERTIES;
}

sharedptr<Relationship> Document_Glom::get_relationship(const Glib::ustring& table_name, const Glib::ustring& relationship_name) const
{
  sharedptr<Relationship> result;

  if(relationship_name == GLOM_RELATIONSHIP_NAME_SYSTEM_PROPERTIES)
  {
    return create_relationship_system_preferences(table_name);
  }

  type_tables::const_iterator iterFind = m_tables.find(table_name);
  if(iterFind != m_tables.end())
  {
    const DocumentTableInfo& info = iterFind->second;

    //Look for the relationship with this name:
    for(type_vecRelationships::const_iterator iter = info.m_relationships.begin(); iter != info.m_relationships.end(); ++iter)
    {
      if(*iter && ((*iter)->get_name() == relationship_name))
      {
        result = *iter;
      }
    }
  }

  return result;
}

Document_Glom::type_vecRelationships Document_Glom::get_relationships(const Glib::ustring& table_name, bool plus_system_prefs) const
{
  type_tables::const_iterator iterFind = m_tables.find(table_name);
  if(iterFind != m_tables.end())
  {
    type_vecRelationships result = iterFind->second.m_relationships;

    //Add the system properties if necessary:
    if(plus_system_prefs)
    {
        if(std::find_if(result.begin(), result.end(), predicate_FieldHasName<Relationship>(GLOM_RELATIONSHIP_NAME_SYSTEM_PROPERTIES)) == result.end())
          result.push_back(create_relationship_system_preferences(table_name));
    }

    return result;
  }
  else
    return type_vecRelationships(); 
}

void Document_Glom::set_relationships(const Glib::ustring& table_name, const type_vecRelationships& vecRelationships) //TODO_shared_relationships
{
  if(!table_name.empty())
  {
    DocumentTableInfo& info = get_table_info_with_add(table_name);
    info.m_relationships = vecRelationships;

    set_modified();
  }
}

void Document_Glom::remove_relationship(const sharedptr<const Relationship>& relationship)
{
  //Get the table that this relationship is part of:
  type_tables::iterator iter = m_tables.find(relationship->get_from_table());
  if(iter != m_tables.end())
  {
    DocumentTableInfo& info = iter->second;

    const Glib::ustring relationship_name = glom_get_sharedptr_name(relationship);

    //Find the relationship and remove it:
    type_vecRelationships::iterator iterRel = std::find_if(info.m_relationships.begin(), info.m_relationships.end(), predicate_FieldHasName<Relationship>(relationship_name));
    if(iterRel != info.m_relationships.end())
    {
      info.m_relationships.erase(iterRel);

      set_modified();
    }

    //Remove relationship from any layouts:
    DocumentTableInfo::type_layouts::iterator iterLayouts = info.m_layouts.begin();
    while(iterLayouts != info.m_layouts.end())
    {
      LayoutInfo& layout_info = *iterLayouts;

      type_mapLayoutGroupSequence::iterator iterGroups = layout_info.m_layout_groups.begin(); 
      while(iterGroups != layout_info.m_layout_groups.end())
      {
        //Remove any layout parts that use this relationship:
        sharedptr<LayoutGroup> group = iterGroups->second;
        sharedptr<UsesRelationship> uses_rel = sharedptr<UsesRelationship>::cast_dynamic(group);
        if(uses_rel && uses_rel->get_has_relationship_name())
        {
          if(*(uses_rel->get_relationship()) == *relationship) //TODO_Performance: Slow when there are many translations.
          {
            layout_info.m_layout_groups.erase(iterGroups);
            iterGroups = layout_info.m_layout_groups.begin(); //Start again because we changed the structure.
            continue;
          }
        }

        if(group)
          group->remove_relationship(relationship);

        ++iterGroups;
      }

       ++iterLayouts;
    }

     //Remove relationshp from any reports:
    for(DocumentTableInfo::type_reports::iterator iterReports = info.m_reports.begin(); iterReports != info.m_reports.end(); ++iterReports)
    {
      sharedptr<Report> report = iterReports->second;
      sharedptr<LayoutGroup> group = report->m_layout_group;

      //Remove the field wherever it is a related field:
      group->remove_relationship(relationship);
    }
  }
}

void Document_Glom::remove_field(const Glib::ustring& table_name, const Glib::ustring& field_name)
{
  //Remove the field itself:
  type_tables::iterator iterFindTable = m_tables.find(table_name);
  if(iterFindTable != m_tables.end())
  {
    type_vecFields& vecFields = iterFindTable->second.m_fields;
    type_vecFields::iterator iterFind = std::find_if( vecFields.begin(), vecFields.end(), predicate_FieldHasName<Field>(field_name) );
    if(iterFind != vecFields.end()) //If it was found:
    {
      //Remove it:
      vecFields.erase(iterFind);

      set_modified();
    }
  }

  //Remove any relationships that use this field:
  for(type_tables::iterator iter = m_tables.begin(); iter != m_tables.end(); ++iter)
  {
    DocumentTableInfo& info = iter->second;

    if(!(info.m_relationships.empty()))
    {
      type_vecRelationships::iterator iterRel = info.m_relationships.begin();
      bool something_changed = true;
      while(something_changed && !info.m_relationships.empty())
      {
        sharedptr<Relationship> relationship = *iterRel;

        if( ((relationship->get_from_table() == table_name) && (relationship->get_from_field() == field_name))
          || ((relationship->get_to_table() == table_name) && (relationship->get_to_field() == field_name)) )
        {
          //Loop again, because we have changed the structure:
          remove_relationship(relationship); //Also removes anything that uses the relationship.

          something_changed = true;
          iterRel = info.m_relationships.begin();
        }
        else
        {
          ++iterRel;

          if(iterRel == info.m_relationships.end())
            something_changed = false; //We've looked at them all, without changing things.
        }
      }
    }

    //Remove field from any layouts:
    for(DocumentTableInfo::type_layouts::iterator iterLayouts = info.m_layouts.begin(); iterLayouts != info.m_layouts.end(); ++iterLayouts)
    {
      LayoutInfo& layout_info = *iterLayouts;
      for(type_mapLayoutGroupSequence::iterator iter = layout_info.m_layout_groups.begin(); iter != layout_info.m_layout_groups.end(); ++iter)
      {
        //Remove regular fields if the field is in this layout's table:
        if(info.m_info->get_name() == table_name)
          iter->second->remove_field(field_name);

        //Remove the field wherever it is a related field:
        iter->second->remove_field(table_name, field_name);
      }
    }

     //Remove field from any reports:
    for(DocumentTableInfo::type_reports::iterator iterReports = info.m_reports.begin(); iterReports != info.m_reports.end(); ++iterReports)
    {
      sharedptr<Report> report = iterReports->second;
      sharedptr<LayoutGroup> group = report->m_layout_group;

      //Remove regular fields if the field is in this layout's table:
      if(info.m_info->get_name() == table_name)
        group->remove_field(field_name);

      //Remove the field wherever it is a related field:
      group->remove_field(table_name, field_name);
    }
  }
}

void Document_Glom::remove_table(const Glib::ustring& table_name)
{
  type_tables::iterator iter = m_tables.find(table_name);
  if(iter != m_tables.end())
  {
    m_tables.erase(iter);
    set_modified();
  }

  //Remove any relationships that use this table:
  for(type_tables::iterator iter = m_tables.begin(); iter != m_tables.end(); ++iter)
  {
    DocumentTableInfo& info = iter->second;

    if(!(info.m_relationships.empty()))
    {
      type_vecRelationships::iterator iterRel = info.m_relationships.begin();
      bool something_changed = true;
      while(something_changed && !info.m_relationships.empty())
      {
        sharedptr<Relationship> relationship = *iterRel;

        if(relationship->get_to_table() == table_name)
        {
          //Loop again, because we have changed the structure:
          remove_relationship(relationship); //Also removes anything that uses the relationship.

          something_changed = true;
          iterRel = info.m_relationships.begin();
        }
        else
        {
          ++iterRel;

          if(iterRel == info.m_relationships.end())
            something_changed = false; //We've looked at them all, without changing things.
        }
      }
    }

  }
}


Document_Glom::type_vecFields Document_Glom::get_table_fields(const Glib::ustring& table_name) const
{
  type_vecFields result;

  if(!table_name.empty())
  {
    type_tables::const_iterator iterFind = m_tables.find(table_name);
    if(iterFind != m_tables.end())
    {
      if(iterFind->second.m_fields.empty())
      {
         g_warning("Document_Glom::get_table_fields: table found, but m_fields is empty");
      }

      return iterFind->second.m_fields;
    }
    else
    {
      //It's a standard table, not saved in the document:
      if(table_name == GLOM_STANDARD_TABLE_PREFS_TABLE_NAME)
      {
        type_vecFields fields;
        sharedptr<TableInfo> temp = create_table_system_preferences(fields);
        result = fields;
      }
      else
      {
        //g_warning("Document_Glom::get_table_fields: table not found in document: %s", table_name.c_str());
      }
    }
  }
  else
  {
    //g_warning("Document_Glom::get_table_fields: table name is empty.");
  }

  //Hide any system fields:
  type_vecFields::iterator iterFind = std::find_if(result.begin(), result.end(), predicate_FieldHasName<Field>(GLOM_STANDARD_FIELD_LOCK));
  if(iterFind != result.end())
    result.erase(iterFind);

  return result;
}

void Document_Glom::set_table_fields(const Glib::ustring& table_name, const type_vecFields& vecFields)
{
  if(!table_name.empty())
  {
    if(vecFields.empty())
    {
      g_warning("Document_Glom::set_table_fields(): vecFields is empty: table_name=%s", table_name.c_str());
    }

    DocumentTableInfo& info = get_table_info_with_add(table_name);
    const bool will_change = (info.m_fields == vecFields); //TODO: Does this do a deep comparison?
    info.m_fields = vecFields;

    set_modified(will_change);
  }
}

sharedptr<Field> Document_Glom::get_field(const Glib::ustring& table_name, const Glib::ustring& strFieldName) const
{
  type_vecFields vecFields = get_table_fields(table_name);
  type_vecFields::iterator iterFind = std::find_if( vecFields.begin(), vecFields.end(), predicate_FieldHasName<Field>(strFieldName) );
  if(iterFind != vecFields.end()) //If it was found:
  {
    return  *iterFind; //A reference, not a copy.
  }

  return sharedptr<Field>();
}


void Document_Glom::change_field_name(const Glib::ustring& table_name, const Glib::ustring& strFieldNameOld, const Glib::ustring& strFieldNameNew)
{
  type_tables::iterator iterFindTable = m_tables.find(table_name);
  if(iterFindTable != m_tables.end())
  {
    //Fields:
    type_vecFields& vecFields = iterFindTable->second.m_fields;
    type_vecFields::iterator iterFind = std::find_if( vecFields.begin(), vecFields.end(), predicate_FieldHasName<Field>(strFieldNameOld) );
    if(iterFind != vecFields.end()) //If it was found:
    {
      //Change it:
      (*iterFind)->set_name(strFieldNameNew);
    }


    //Find any relationships, layouts, or formatting that use this field
    //Look at each table:
    for(type_tables::iterator iter = m_tables.begin(); iter != m_tables.end(); ++iter)
    {
      //Look at each relationship in the table:
      for(type_vecRelationships::iterator iterRels = iter->second.m_relationships.begin(); iterRels != iter->second.m_relationships.end(); ++iterRels)
      {
        sharedptr<Relationship> relationship = *iterRels;

        if(relationship->get_from_table() == table_name)
        {
          if(relationship->get_from_field() == strFieldNameOld)
          {
            //Change it:
            relationship->set_from_field(strFieldNameNew);
          }
        }

        if(relationship->get_to_table() == table_name)
        {
          if(relationship->get_to_field() == strFieldNameOld)
          {
            //Change it:
            relationship->set_to_field(strFieldNameNew);
          }
        }
      }

      //Look at all field formatting:
      for(type_vecFields::iterator iterFields = iter->second.m_fields.begin(); iterFields != iter->second.m_fields.end(); ++iterFields)
      {
        (*iterFields)->m_default_formatting.change_field_name(table_name, strFieldNameOld, strFieldNameNew);
      }


      const bool is_parent_table = (iter->second.m_info->get_name() == table_name);

      //Look at each layout:
      for(DocumentTableInfo::type_layouts::iterator iterLayouts = iter->second.m_layouts.begin(); iterLayouts != iter->second.m_layouts.end(); ++iterLayouts)
      {

        //Look at each group:
        for(type_mapLayoutGroupSequence::iterator iterGroup = iterLayouts->m_layout_groups.begin(); iterGroup != iterLayouts->m_layout_groups.end(); ++iterGroup)
        {
          sharedptr<LayoutGroup> group = iterGroup->second;
          if(group)
          {
            //Change the field if it is in this group:
            if(is_parent_table)
              group->change_field_item_name(table_name, strFieldNameOld, strFieldNameNew);
            else
              group->change_related_field_item_name(table_name, strFieldNameOld, strFieldNameNew);
          }
        }
      }


      //Look at each report:
      for(DocumentTableInfo::type_reports::iterator iterReports = iter->second.m_reports.begin(); iterReports != iter->second.m_reports.end(); ++iterReports)
      {
        //Change the field if it is in this group:
        sharedptr<Report> report = iterReports->second;
        if(report)
        {
          if(is_parent_table)
            report->m_layout_group->change_field_item_name(table_name, strFieldNameOld, strFieldNameNew);
          else
            report->m_layout_group->change_related_field_item_name(table_name, strFieldNameOld, strFieldNameNew);
        }
      }

    }

    set_modified();
  }
}

void Document_Glom::change_table_name(const Glib::ustring& table_name_old, const Glib::ustring& table_name_new)
{
  type_tables::iterator iterFindTable = m_tables.find(table_name_old);
  if(iterFindTable != m_tables.end())
  {
    //Change it:
    //We can't just change the key of the iterator (I think),
    //so we copy the whole thing and put it back in the map under a different key:

    //iterFindTable->first = table_name_new;
    DocumentTableInfo doctableinfo = iterFindTable->second;
    m_tables.erase(iterFindTable);

    doctableinfo.m_info->set_name(table_name_new); 
    m_tables[table_name_new] = doctableinfo; 

    //Find any relationships or layouts that use this table
    //Look at each table:
    for(type_tables::iterator iter = m_tables.begin(); iter != m_tables.end(); ++iter)
    {
      //Look at each relationship in the table:
      for(type_vecRelationships::iterator iterRels = iter->second.m_relationships.begin(); iterRels != iter->second.m_relationships.end(); ++iterRels)
      {
        sharedptr<Relationship> relationship = *iterRels;

        if(relationship->get_from_table() == table_name_old)
        {
          //Change it:
           relationship->set_from_table(table_name_new);
        }

        if(relationship->get_to_table() == table_name_old)
        {
          //Change it:
           relationship->set_to_table(table_name_new);
        }
      }
    }

    //TODO: Remember to change it in layouts when we add the ability to show fields from other tables.

    set_modified();
  }
}

void Document_Glom::change_relationship_name(const Glib::ustring& table_name, const Glib::ustring& name, const Glib::ustring& name_new)
{
  type_tables::iterator iterFindTable = m_tables.find(table_name);
  if(iterFindTable != m_tables.end())
  {
    //Change the relationship name:
    type_vecRelationships::iterator iterRelFind = std::find_if( iterFindTable->second.m_relationships.begin(), iterFindTable->second.m_relationships.end(), predicate_FieldHasName<Relationship>(name) );
    if(iterRelFind != iterFindTable->second.m_relationships.end())
      (*iterRelFind)->set_name(name_new);


    //Any layouts, reports, etc that use this relationship will already have the new name via the sharedptr<Relationship>.


    //Look at each table:
    for(type_tables::iterator iter = m_tables.begin(); iter != m_tables.end(); ++iter)
    {
      /*
       //Look at all field formatting:
      for(type_vecFields::iterator iterFields = iter->second.m_fields.begin(); iterFields != iter->second.m_fields.end(); ++iterFields)
      {
        (*iterFields)->m_default_formatting.change_relationship_name(table_name, name, name_new);
      }

      const bool is_parent_table = (iter->second.m_info->get_name() == table_name);

      //Look at each layout:
      for(DocumentTableInfo::type_layouts::iterator iterLayouts = iter->second.m_layouts.begin(); iterLayouts != iter->second.m_layouts.end(); ++iterLayouts)
      {
        //Look at each group:
        for(type_mapLayoutGroupSequence::iterator iterGroup = iterLayouts->m_layout_groups.begin(); iterGroup != iterLayouts->m_layout_groups.end(); ++iterGroup)
        {
          //Change the field if it is in this group:
          if(is_parent_table)
            iterGroup->second.change_relationship_name(table_name, name, name_new);
          else
            iterGroup->second.change_related_relationship_name(table_name, name, name_new);
        }
      }


      //Look at each report:
      for(DocumentTableInfo::type_reports::iterator iterReports = iter->second.m_reports.begin(); iterReports != iter->second.m_reports.end(); ++iterReports)
      {
        //Change the field if it is in this group:
        if(is_parent_table)
          iterReports->second->m_layout_group->change_relationship_name(table_name, name, name_new);
        else
          iterReports->second->m_layout_group->change_related_relationship_name(table_name, name, name_new);
      }
      */

     //TODO_SharedRelationshipCheck lookups.

    }

    set_modified();
  }
 }


bool Document_Glom::get_node_attribute_value_as_bool(const xmlpp::Element* node, const Glib::ustring& strAttributeName)
{
  Glib::ustring strValue = get_node_attribute_value(node, strAttributeName);
  return strValue == "true";
}

Glib::ustring Document_Glom::get_child_text_node(const xmlpp::Element* node, const Glib::ustring& child_node_name) const
{
  const xmlpp::Element* child = get_node_child_named(node, child_node_name);
  if(child)
  {
     const xmlpp::TextNode* text_child = child->get_child_text();
     if(text_child)
       return text_child->get_content();
  }

  return Glib::ustring();
}

void Document_Glom::set_child_text_node(xmlpp::Element* node, const Glib::ustring& child_node_name, const Glib::ustring& text)
{
  xmlpp::Element* child = get_node_child_named(node, child_node_name);
  if(!child)
    child = node->add_child(child_node_name);

  xmlpp::TextNode* text_child = child->get_child_text();
  if(!text_child)
    child->add_child_text(text);
  else
    text_child->set_content(text);
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void Document_Glom::set_node_attribute_value_as_bool(xmlpp::Element* node, const Glib::ustring& strAttributeName, bool value)
{
  if(!value && !node->get_attribute(strAttributeName))
    return; //Use the non-existance of an attribute to mean false, to save space.

  Glib::ustring strValue = (value ? "true" : "false");
  set_node_attribute_value(node, strAttributeName, strValue);
}

void Document_Glom::set_node_attribute_value_as_decimal(xmlpp::Element* node, const Glib::ustring& strAttributeName, int value)
{
  if(!value && !node->get_attribute(strAttributeName))
    return; //Use the non-existance of an attribute to mean zero, to save space.

  //Get text representation of int:
  std::stringstream thestream;
  thestream.imbue( std::locale::classic() ); //The C locale.
  thestream << value;
  const Glib::ustring sequence_string = thestream.str();

  set_node_attribute_value(node, strAttributeName, sequence_string);
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

void Document_Glom::set_node_attribute_value_as_decimal_double(xmlpp::Element* node, const Glib::ustring& strAttributeName, double value)
{
  if(!value && !node->get_attribute(strAttributeName))
    return; //Use the non-existance of an attribute to mean zero, to save space.

  //Get text representation of int:
  std::stringstream thestream;
  thestream.imbue( std::locale::classic() ); //The C locale.
  thestream << value;
  const Glib::ustring sequence_string = thestream.str();

  set_node_attribute_value(node, strAttributeName, sequence_string);
}

guint Document_Glom::get_node_attribute_value_as_decimal(const xmlpp::Element* node, const Glib::ustring& strAttributeName)
{
  guint result = 0;
  const Glib::ustring value_string = get_node_attribute_value(node, strAttributeName);

  //Get number for string:
  if(!value_string.empty())
  {
    //Visible fields, with sequence:
    std::stringstream thestream;
    thestream.imbue( std::locale::classic() ); //The C locale.
    thestream.str(value_string);
    thestream >> result;
  }

  return result;
}

double Document_Glom::get_node_attribute_value_as_decimal_double(const xmlpp::Element* node, const Glib::ustring& strAttributeName)
{
  double result = 0;
  const Glib::ustring value_string = get_node_attribute_value(node, strAttributeName);

  //Get number for string:
  if(!value_string.empty())
  {
    //Visible fields, with sequence:
    std::stringstream thestream;
    thestream.imbue( std::locale::classic() ); //The C locale.
    thestream.str(value_string);
    thestream >> result;
  }

  return result;
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void Document_Glom::set_node_attribute_value_as_float(xmlpp::Element* node, const Glib::ustring& strAttributeName, float value)
{
    if(value == std::numeric_limits<float>::infinity() && !node->get_attribute(strAttributeName))
    return; //Use the non-existance of an attribute to mean "invalid"/infinity, to save space.
  
  //Get text representation of float:
  std::stringstream thestream;
  thestream.imbue( std::locale::classic() ); //The C locale.
  thestream << value;
  const Glib::ustring sequence_string = thestream.str();

  set_node_attribute_value(node, strAttributeName, sequence_string);
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

float Document_Glom::get_node_attribute_value_as_float(const xmlpp::Element* node, const Glib::ustring& strAttributeName)
{
  float result = std::numeric_limits<float>::infinity();
  const Glib::ustring value_string = get_node_attribute_value(node, strAttributeName);

  //Get number for string:
  if(!value_string.empty())
  {
    //Visible fields, with sequence:
    std::stringstream thestream;
    thestream.imbue( std::locale::classic() ); //The C locale.
    thestream.str(value_string);
    thestream >> result;
  }

  return result;
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void Document_Glom::set_node_attribute_value_as_value(xmlpp::Element* node, const Glib::ustring& strAttributeName, const Gnome::Gda::Value& value,  Field::glom_field_type field_type)
{
  NumericFormat format_ignored; //Because we use ISO format.
  const Glib::ustring value_as_text = Conversions::get_text_for_gda_value(field_type, value, std::locale() /* Use the C locale */, format_ignored, true /* ISO standard */);

  set_node_attribute_value(node, strAttributeName, value_as_text);
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

Gnome::Gda::Value Document_Glom::get_node_attribute_value_as_value(const xmlpp::Element* node, const Glib::ustring& strAttributeName, Field::glom_field_type field_type)
{
  const Glib::ustring value_string = get_node_attribute_value(node, strAttributeName);

  bool success = false;
  Gnome::Gda::Value  result = Conversions::parse_value(field_type, value_string, success, true /* iso_format */);
  if(success)
    return result;
  else 
    return Gnome::Gda::Value();
}

void Document_Glom::append_newline(xmlpp::Element* parent_node)
{
  parent_node->add_child_text("\n");
}

Document_Glom::type_listTableInfo Document_Glom::get_tables(bool plus_system_prefs) const
{
  type_listTableInfo result;

  for(type_tables::const_iterator iter = m_tables.begin(); iter != m_tables.end(); ++iter)
  {
    result.push_back(iter->second.m_info);
    //std::cout << "Document_Glom::get_tables(): title=" << iter->second.m_info->get_title() << std::endl;
  }

  //Add the system properties if necessary:
  if(plus_system_prefs)
  {
      if(std::find_if(result.begin(), result.end(), predicate_FieldHasName<TableInfo>(GLOM_STANDARD_TABLE_PREFS_TABLE_NAME)) == result.end())
        result.push_back(create_table_system_preferences());
  }

  return result;
}

std::vector<Glib::ustring> Document_Glom::get_table_names(bool plus_system_prefs) const
{
  type_listTableInfo list_full = get_tables(plus_system_prefs);
  std::vector<Glib::ustring> result;
  for(type_listTableInfo::iterator iter = list_full.begin(); iter != list_full.end(); ++iter)
  {
    sharedptr<TableInfo> info = *iter;
    if(info)
      result.push_back(info->get_name());
  }

  return result;
}

sharedptr<TableInfo> Document_Glom::get_table(const Glib::ustring& table_name) const
{
  type_tables::const_iterator iterfind = m_tables.find(table_name);
  if(iterfind != m_tables.end())
    return iterfind->second.m_info;
  else
    return sharedptr<TableInfo>();
}

void Document_Glom::add_table(const sharedptr<TableInfo>& table_info)
{
  if(!table_info)
    return;

  type_tables::const_iterator iterfind = m_tables.find(table_info->get_name());
  if(iterfind == m_tables.end())
  {
    DocumentTableInfo item;
    item.m_info = table_info;
    m_tables[table_info->get_name()] = item;
    set_modified();
  }
}

bool Document_Glom::get_table_overview_position ( const Glib::ustring &table_name,
                                                  float &x, float &y ) const
{
  type_tables::const_iterator it = m_tables.find(table_name);
  if(it != m_tables.end())
  {
    if ( it->second.m_overviewx == std::numeric_limits<float>::infinity() ||
         it->second.m_overviewy == std::numeric_limits<float>::infinity() )
    {
      return false;
    }
    x = it->second.m_overviewx;
    y = it->second.m_overviewy;
    return true;
  }
  else
  {
    return false;
  }
}
    
void Document_Glom::set_table_overview_position ( const Glib::ustring &table_name,
                                                  float x, float y )
{
  type_tables::iterator it = m_tables.find(table_name);
  if(it != m_tables.end())
  {
    it->second.m_overviewx = x;
    it->second.m_overviewy = y;
  }
}
    
void Document_Glom::set_tables(const type_listTableInfo& tables)
{
  //TODO: Avoid adding information about tables that we don't know about - that should be done explicitly.
  //Look at each "table".

  bool something_changed = false;
  for(type_tables::iterator iter = m_tables.begin(); iter != m_tables.end(); iter++)
  {
    const DocumentTableInfo& doctableinfo = iter->second;

    const Glib::ustring table_name = doctableinfo.m_info->get_name();

    type_listTableInfo::const_iterator iterfind = std::find_if(tables.begin(), tables.end(), predicate_FieldHasName<TableInfo>(table_name));
    if(iterfind != tables.end())
    {
      sharedptr<TableInfo> info = doctableinfo.m_info;

      sharedptr<TableInfo> infoFound = *iterfind;
      *info = *infoFound;

      something_changed = true;
    }
  }

  if(something_changed)
    set_modified();
}

void Document_Glom::fill_layout_field_details(const Glib::ustring& parent_table_name, const sharedptr<LayoutGroup>& layout_group) const
{
  //Get the full field information for the LayoutItem_Fields in this group:

  for(LayoutGroup::type_map_items::iterator iter = layout_group->m_map_items.begin(); iter != layout_group->m_map_items.end(); ++iter)
  {
    sharedptr<LayoutItem> layout_item = iter->second;

    sharedptr<LayoutItem_Field> layout_field = sharedptr<LayoutItem_Field>::cast_dynamic(layout_item);
    if(layout_field)
    {
      layout_field->set_full_field_details( get_field(layout_field->get_table_used(parent_table_name), layout_field->get_name()) );
    }
    else
    {
      sharedptr<LayoutItem_Portal> layout_portal_child = sharedptr<LayoutItem_Portal>::cast_dynamic(layout_item);
      if(layout_portal_child)
        fill_layout_field_details(layout_portal_child->get_table_used(parent_table_name), layout_portal_child); //recurse
      else
      {
        sharedptr<LayoutGroup> layout_group_child = sharedptr<LayoutGroup>::cast_dynamic(layout_item);
        if(layout_group_child)
          fill_layout_field_details(parent_table_name, layout_group_child); //recurse
      }
    }
  }
}

void Document_Glom::fill_layout_field_details(const Glib::ustring& parent_table_name, type_mapLayoutGroupSequence& sequence) const
{
  for(type_mapLayoutGroupSequence::iterator iterGroups = sequence.begin(); iterGroups != sequence.end(); ++iterGroups)
  {
    sharedptr<LayoutGroup> group = iterGroups->second;
    if(group)
      fill_layout_field_details(parent_table_name, group);
  }
}

Document_Glom::type_mapLayoutGroupSequence Document_Glom::get_data_layout_groups_default(const Glib::ustring& layout_name, const Glib::ustring& parent_table_name) const
{
  //std::cout << "debug: Document_Glom::get_data_layout_groups_default(): table_name = " << parent_table_name << std::endl;

  type_mapLayoutGroupSequence result;

  //Add one if necessary:
  sharedptr<LayoutGroup> pTopLevel;
  sharedptr<LayoutGroup> pOverview;
  sharedptr<LayoutGroup> pDetails;
  if(!pTopLevel)
  {
    sharedptr<LayoutGroup> group = sharedptr<LayoutGroup>::create();
    group->set_name("main");
    group->m_sequence = 1;
    group->m_columns_count = 1;
    result[1] = group;
    pTopLevel = group;

    if(layout_name == "details") //The Details default layut is a bit more complicated.
    {
      sharedptr<LayoutGroup> overview = sharedptr<LayoutGroup>::create();;
      overview->set_name("overview");
      overview->set_title_original("Overview"); //Don't translate this, but TODO: add standard translations.
      overview->m_columns_count = 2;
      pOverview = sharedptr<LayoutGroup>::cast_dynamic(pTopLevel->add_item(overview));

      sharedptr<LayoutGroup> details = sharedptr<LayoutGroup>::create();
      details->set_name("details");
      details->set_title_original("Details"); //Don't translate this, but TODO: add standard translations.
      details->m_columns_count = 2;
      pDetails = sharedptr<LayoutGroup>::cast_dynamic(pTopLevel->add_item(details));
    }
  }

  //If, for some reason, we didn't create the-subgroups, add everything to the top level group:
  if(!pOverview)
    pOverview = pTopLevel;

  if(!pDetails)
    pDetails = pTopLevel;


  //Discover new fields, and add them:
  type_vecFields all_fields = get_table_fields(parent_table_name);
  for(type_vecFields::const_iterator iter = all_fields.begin(); iter != all_fields.end(); ++iter)
  {
    const Glib::ustring field_name = (*iter)->get_name();
    if(!field_name.empty())
    {
      //See whether it's already in the result:
      //TODO_Performance: There is a lot of iterating and comparison here:
      bool found = false; //TODO: This is horrible.
      for(type_mapLayoutGroupSequence::const_iterator iterFind = result.begin(); iterFind != result.end(); ++iterFind)
      {
        if(iterFind->second->has_field(field_name))
        {
          found = true;
          break;
        }
      }

      if(!found)
      {
        sharedptr<LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::create();
        layout_item->set_full_field_details(*iter);
        //layout_item.set_table_name(child_table_name); //TODO: Allow viewing of fields through relationships.
        //layout_item.m_sequence = sequence;  add_item() will fill this.

        //std::cout << "  debug: add_item(): " << layout_item.get_name() << std::endl;
        if(layout_item->get_full_field_details()->get_primary_key())
          pOverview->add_item(layout_item);
        else
          pDetails->add_item(layout_item);
      }
    }
  }

  return result;
}

Document_Glom::type_mapLayoutGroupSequence Document_Glom::get_data_layout_groups_plus_new_fields(const Glib::ustring& layout_name, const Glib::ustring& parent_table_name) const
{
  type_mapLayoutGroupSequence result = get_data_layout_groups(layout_name, parent_table_name);

  //If there are no fields in the layout, then add a default:
  bool create_default = false;
  if(result.empty())
    create_default = true;
  //TODO: Also set create_default if all groups have no fields.

  if(create_default)
  {
    result = get_data_layout_groups_default(layout_name, parent_table_name);
    
    //Store this so we don't have to recreate it next time:
    Document_Glom* nonconst_this = const_cast<Document_Glom*>(this); //TODO: This is not ideal.
    nonconst_this->set_data_layout_groups(layout_name, parent_table_name, result);
    nonconst_this->set_modified(false); //This might have happened in operator mode, but in that case we don't really need to save it, or mark the document as unsaved.
  }

  return result;  
}

Document_Glom::type_mapLayoutGroupSequence Document_Glom::get_data_layout_groups(const Glib::ustring& layout_name, const Glib::ustring& parent_table_name) const
{
  type_tables::const_iterator iterFind = m_tables.find(parent_table_name);
  if(iterFind != m_tables.end())
  {
    const DocumentTableInfo& info = iterFind->second;

    //Look for the layout with this name:
    DocumentTableInfo::type_layouts::const_iterator iter = std::find_if(info.m_layouts.begin(), info.m_layouts.end(), predicate_Layout<LayoutInfo>(parent_table_name, layout_name));
    if(iter != info.m_layouts.end())
    {
      return iter->m_layout_groups; //found
    }
  }

  return type_mapLayoutGroupSequence(); //not found
}

void Document_Glom::set_data_layout_groups(const Glib::ustring& layout_name, const Glib::ustring& parent_table_name, const type_mapLayoutGroupSequence& groups)
{
  const Glib::ustring child_table_name = parent_table_name; //TODO: Remove this cruft.

  //g_warning("Document_Glom::set_data_layout_groups(): ADDING layout for table %s (child_table=%s), for layout %s", parent_table_name.c_str(), child_table_name.c_str(), layout_name.c_str());


  if(!parent_table_name.empty())
  {
    DocumentTableInfo& info = get_table_info_with_add(parent_table_name);

    LayoutInfo layout_info;
    layout_info.m_parent_table = child_table_name;
    layout_info.m_layout_name = layout_name;
    layout_info.m_layout_groups = groups;

    DocumentTableInfo::type_layouts::iterator iter = std::find_if(info.m_layouts.begin(), info.m_layouts.end(), predicate_Layout<LayoutInfo>(child_table_name, layout_name));
    if(iter == info.m_layouts.end())
      info.m_layouts.push_back(layout_info);
    else
      *iter = layout_info;

    set_modified();
  }
}

Document_Glom::DocumentTableInfo& Document_Glom::get_table_info_with_add(const Glib::ustring& table_name)
{
  type_tables::iterator iterFind = m_tables.find(table_name);
  if(iterFind != m_tables.end())
  {
    return iterFind->second;
  }
  else
  {
    m_tables[table_name] = DocumentTableInfo();
    m_tables[table_name].m_info->set_name(table_name);
    return get_table_info_with_add(table_name);
  }
}

Glib::ustring Document_Glom::get_table_title(const Glib::ustring& table_name) const
{
  type_tables::const_iterator iterFind = m_tables.find(table_name);
  if(iterFind != m_tables.end())
    return iterFind->second.m_info->get_title();
  else
    return Glib::ustring();
}

void Document_Glom::set_table_title(const Glib::ustring& table_name, const Glib::ustring& value)
{
  //std::cout << "debug: Document_Glom::set_table_title(): table_name=" << table_name << ", value=" << value << std::endl;
  if(!table_name.empty())
  {
    DocumentTableInfo& info = get_table_info_with_add(table_name);
    if(info.m_info->get_title() != value)
    {
      info.m_info->set_title(value);
      set_modified();
    }
  }
}

void Document_Glom::set_table_example_data(const Glib::ustring& table_name, const Glib::ustring& rows)
{
 if(!table_name.empty())
  {
    DocumentTableInfo& info = get_table_info_with_add(table_name);
    if(info.m_example_rows != rows)
    {
      info.m_example_rows = rows;
      set_modified();
    }
  }
}

Glib::ustring Document_Glom::get_table_example_data(const Glib::ustring& table_name) const
{
  type_tables::const_iterator iterFind = m_tables.find(table_name);
  if(iterFind != m_tables.end())
    return iterFind->second.m_example_rows;
  else
    return Glib::ustring();
}

bool Document_Glom::get_table_is_known(const Glib::ustring& table_name) const
{
  type_tables::const_iterator iterFind = m_tables.find(table_name);
  return (iterFind != m_tables.end());
}

bool Document_Glom::get_table_is_hidden(const Glib::ustring& table_name) const
{
  type_tables::const_iterator iterFind = m_tables.find(table_name);
  if(iterFind != m_tables.end())
    return iterFind->second.m_info->m_hidden;
  else
    return false; //It's not even known.
}

AppState::userlevels Document_Glom::get_userlevel() const
{
  userLevelReason reason;
  return get_userlevel(reason);
}

AppState::userlevels Document_Glom::get_userlevel(userLevelReason& reason) const
{
  //Initialize output parameter:
  reason = USER_LEVEL_REASON_UNKNOWN;

  if(get_read_only())
  {
    reason = USER_LEVEL_REASON_FILE_READ_ONLY;
    return AppState::USERLEVEL_OPERATOR; //A read-only document cannot be changed, so there's no point in being in developer mode. This is one way to control the user level on purpose.
  }
  else if(m_file_uri.empty()) //If it has never been saved then this is a new default document, so the user created it, so the user can be a developer.
  {
#ifdef GLOM_ENABLE_CLIENT_ONLY
    // Client only mode doesn't support developer mode, though.
    return AppState::USERLEVEL_OPERATOR;
#else
    return AppState::USERLEVEL_DEVELOPER;
#endif
  }
  else
  {
    return m_app_state.get_userlevel();
  }
}

Document_Glom::type_signal_userlevel_changed Document_Glom::signal_userlevel_changed()
{
  return m_signal_userlevel_changed;
}

void Document_Glom::on_app_state_userlevel_changed(AppState::userlevels userlevel)
{
  m_signal_userlevel_changed.emit(userlevel);
}

bool Document_Glom::set_userlevel(AppState::userlevels userlevel)
{
#ifndef GLOM_ENABLE_CLIENT_ONLY
  //Prevent incorrect user level:
  if((userlevel == AppState::USERLEVEL_DEVELOPER) && get_read_only())
  {
    std::cout << "DEBUG: Document_Glom::set_userlevel(): Developer mode denied because get_read_only() returned true." << std::endl;
    std::cout << "  DEBUG: get_read_only()=" << get_read_only() << std::endl;
    std::cout << "  DEBUG: get_file_uri()=" << get_file_uri() << std::endl;

    m_app_state.set_userlevel(AppState::USERLEVEL_OPERATOR);
    return false;
  }
  else
#endif
  {
    m_app_state.set_userlevel(userlevel);
    return true;
  }
}

void Document_Glom::emit_userlevel_changed()
{
  m_signal_userlevel_changed.emit(m_app_state.get_userlevel());
}

Glib::ustring Document_Glom::get_default_table() const
{
  for(type_tables::const_iterator iter = m_tables.begin(); iter != m_tables.end(); ++iter)
  {
    if(iter->second.m_info->m_default)
      return iter->second.m_info->get_name();
  }

  //If there is only one table then pretend that is the default:
  if(m_tables.size() == 1)
  {
    type_tables::const_iterator iter = m_tables.begin();
    return iter->second.m_info->get_name();
  }

  return Glib::ustring();
}

Glib::ustring Document_Glom::get_first_table() const
{
  if(m_tables.empty())
    return Glib::ustring();

  type_tables::const_iterator iter = m_tables.begin();
  return iter->second.m_info->get_name();
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void Document_Glom::set_allow_autosave(bool value)
{
  if(m_allow_auto_save == value)
    return;

  m_allow_auto_save = value;

  //Save changes that have been waiting for us to call this function:
  if(m_allow_auto_save && get_modified())
  {
    save_changes();
  }
}

void Document_Glom::save_changes()
{
  //Save changes automatically
  //(when in developer mode - no changes should even be possible when not in developer mode)
  if(get_userlevel() == AppState::USERLEVEL_DEVELOPER)
  {
    //This rebuilds the whole XML DOM and saves the whole document,
    //so we need to be careful not to call set_modified() too often.

      bool test = save_before();
      if(test)
      {
        //std::cout << "Document_Glom::save_changes(): calling write_to_disk()." << std::endl;
        test = write_to_disk();
        if(test)
        {
          set_modified(false);
        }
      }
  }
  else
  {
    //std::cout << "Document_Glom::save_changes(): Not saving, because not AppState::USERLEVEL_DEVELOPER" << std::endl;
  }
}

void Document_Glom::set_modified(bool value)
{
  //std::cout << "Document_Glom::set_modified()" << std::endl;

  if(value && m_block_modified_set) //For instance, don't save changes while loading.
  {
    //std::cout << "  Document_Glom::set_modified() m_block_modified_set" << std::endl;
    return;
  }

  if (get_userlevel() != AppState::USERLEVEL_DEVELOPER)
  {
    //Some things can be legitimately changed by the user, 
    //such as field information from the server,
    //but only for the duration of the session.
    //There's no way that save_changes() can work for the user,
    //so we don't use set_modified().
    return;
  }


  //if(value != get_modified()) //Prevent endless loops
  //{
    Bakery::Document_XML::set_modified(value);

    if(value)
    {
      //std::cout << "  Document_Glom::set_modified() save_changes" << std::endl;
      save_changes();
    }
  //}
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

void Document_Glom::load_after_layout_item_field_formatting(const xmlpp::Element* element, FieldFormatting& format, Field::glom_field_type field_type, const Glib::ustring& table_name, const Glib::ustring& field_name)
{
  //Numeric formatting:
  format.m_numeric_format.m_use_thousands_separator = get_node_attribute_value_as_bool(element, GLOM_ATTRIBUTE_FORMAT_THOUSANDS_SEPARATOR);
  format.m_numeric_format.m_decimal_places_restricted = get_node_attribute_value_as_bool(element, GLOM_ATTRIBUTE_FORMAT_DECIMAL_PLACES_RESTRICTED);
  format.m_numeric_format.m_decimal_places = get_node_attribute_value_as_decimal(element, GLOM_ATTRIBUTE_FORMAT_DECIMAL_PLACES);
  format.m_numeric_format.m_currency_symbol = get_node_attribute_value(element, GLOM_ATTRIBUTE_FORMAT_CURRENCY_SYMBOL);


  //Text formatting:
  if(field_type == Field::TYPE_TEXT)
  {
    format.set_text_format_multiline( get_node_attribute_value_as_bool(element, GLOM_ATTRIBUTE_FORMAT_TEXT_MULTILINE) );
    format.set_text_format_multiline_height_lines( get_node_attribute_value_as_decimal(element, GLOM_ATTRIBUTE_FORMAT_TEXT_MULTILINE_HEIGHT_LINES) );
  }

  format.set_text_format_font( get_node_attribute_value (element, GLOM_ATTRIBUTE_FORMAT_TEXT_FONT) );
  format.set_text_format_color_foreground( get_node_attribute_value (element, GLOM_ATTRIBUTE_FORMAT_TEXT_COLOR_FOREGROUND) );
  format.set_text_format_color_background( get_node_attribute_value (element, GLOM_ATTRIBUTE_FORMAT_TEXT_COLOR_BACKGROUND) );


  //Choices:
  format.set_choices_restricted( get_node_attribute_value_as_bool(element, GLOM_ATTRIBUTE_FORMAT_CHOICES_RESTRICTED) );
  format.set_has_custom_choices( get_node_attribute_value_as_bool(element, GLOM_ATTRIBUTE_FORMAT_CHOICES_CUSTOM) );

  if(format.get_has_custom_choices())
  {
    const xmlpp::Element* nodeChoiceList = get_node_child_named(element, GLOM_ATTRIBUTE_FORMAT_CHOICES_CUSTOM_LIST);
    if(nodeChoiceList)
    {
      FieldFormatting::type_list_values list_values;

      xmlpp::Node::NodeList listNodesCustomChoices = nodeChoiceList->get_children(GLOM_NODE_FORMAT_CUSTOM_CHOICE);
      for(xmlpp::Node::NodeList::iterator iter = listNodesCustomChoices.begin(); iter != listNodesCustomChoices.end(); ++iter)
      {
        const xmlpp::Element* element = dynamic_cast<const xmlpp::Element*>(*iter);
        if(element)
        {
          if(field_type == Field::TYPE_INVALID)
          {
            //Discover the field type, so we can interpret the text as a value.
            //Not all calling functions know this.
            //TODO_Performance.
            sharedptr<const Field> field_temp = get_field(table_name, field_name);
            if(field_temp)
              field_type = field_temp->get_glom_type();
          }

          const Gnome::Gda::Value value = get_node_attribute_value_as_value(element, GLOM_ATTRIBUTE_VALUE, field_type);
          list_values.push_back(value);
        }
      }

      format.set_choices_custom(list_values);
    }
  }

  format.set_has_related_choices( get_node_attribute_value_as_bool(element, GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED) );

  const Glib::ustring relationship_name = get_node_attribute_value(element, GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_RELATIONSHIP);
  if(!relationship_name.empty())
  {
    sharedptr<Relationship> relationship = get_relationship(table_name, relationship_name);
    format.set_choices(relationship,
      get_node_attribute_value(element, GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_FIELD),
      get_node_attribute_value(element, GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_SECOND) );
    //Full details are updated in filled-in ().
  }
}

void Document_Glom::load_after_layout_item_usesrelationship(const xmlpp::Element* element, const Glib::ustring& table_name, const sharedptr<UsesRelationship>& item)
{
  if(!element || !item)
    return;

  const Glib::ustring relationship_name = get_node_attribute_value(element, GLOM_ATTRIBUTE_RELATIONSHIP_NAME);
  sharedptr<Relationship> relationship;
  if(!relationship_name.empty())
  {
    //std::cout << "  debug in : table_name=" << table_name << ", relationship_name=" << relationship_name << std::endl; 
    relationship = get_relationship(table_name, relationship_name);
    item->set_relationship(relationship); 

    if(!relationship)
    {
      std::cerr << "Document_Glom::load_after_layout_item_usesrelationship(): relationship not found: " << relationship_name << ", in table:" << table_name << std::endl;
    }
  }

  const Glib::ustring related_relationship_name = get_node_attribute_value(element, GLOM_ATTRIBUTE_RELATED_RELATIONSHIP_NAME);
  if(!related_relationship_name.empty() && relationship)
  {
    sharedptr<Relationship> related_relationship = get_relationship(relationship->get_to_table(), related_relationship_name);
    if(!related_relationship)
      std::cerr << "Document_Glom::load_after_layout_item_field(): related relationship not found in table=" << relationship->get_to_table() << ",  name=" << related_relationship_name << std::endl;

    item->set_related_relationship(related_relationship); 
  }
}

void Document_Glom::load_after_layout_item_field(const xmlpp::Element* element, const Glib::ustring& table_name, const sharedptr<LayoutItem_Field>& item)
{
  const Glib::ustring name = get_node_attribute_value(element, GLOM_ATTRIBUTE_NAME);
  item->set_name(name);

  load_after_layout_item_usesrelationship(element, table_name, item);

  item->set_editable( get_node_attribute_value_as_bool(element, GLOM_ATTRIBUTE_EDITABLE) );

  const xmlpp::Element* elementFormatting = get_node_child_named(element, GLOM_NODE_FORMAT);
  if(elementFormatting)
  {
    //TODO: Provide the name of the relationship's table if there is a relationship:
    load_after_layout_item_field_formatting(elementFormatting, item->m_formatting, item->get_glom_type(), table_name, name);
  }

  item->set_formatting_use_default( get_node_attribute_value_as_bool(element, GLOM_ATTRIBUTE_DATA_LAYOUT_ITEM_FIELD_USE_DEFAULT_FORMATTING) );


  const xmlpp::Element* nodeCustomTitle = get_node_child_named(element, GLOM_NODE_LAYOUT_ITEM_CUSTOM_TITLE);
  if(nodeCustomTitle)
  {
    sharedptr<CustomTitle> custom_title = sharedptr<CustomTitle>::create();
    custom_title->set_use_custom_title( get_node_attribute_value_as_bool(nodeCustomTitle, GLOM_ATTRIBUTE_LAYOUT_ITEM_CUSTOM_TITLE_USE) );

    load_after_translations(nodeCustomTitle, *custom_title);
    item->set_title_custom(custom_title);
  }
}

void Document_Glom::load_after_sort_by(const xmlpp::Element* node, const Glib::ustring& table_name, LayoutItem_GroupBy::type_list_sort_fields& list_fields)
{
  list_fields.clear();

  if(!node)
    return;

  xmlpp::Node::NodeList listNodes = node->get_children(GLOM_NODE_DATA_LAYOUT_ITEM);
  for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
  {
    const xmlpp::Element* element = dynamic_cast<const xmlpp::Element*>(*iter);
    if(element)
    {
      sharedptr<LayoutItem_Field> item = sharedptr<LayoutItem_Field>::create();
      //item.set_full_field_details_empty();
      load_after_layout_item_field(element, table_name, item);
      item->set_full_field_details( get_field(item->get_table_used(table_name), item->get_name()) );

      const guint sequence = get_node_attribute_value_as_decimal(element, GLOM_ATTRIBUTE_SEQUENCE);
      item->m_sequence = sequence;

      const bool ascending = get_node_attribute_value_as_bool(element, GLOM_ATTRIBUTE_SORT_ASCENDING);

      list_fields.push_back( LayoutItem_GroupBy::type_pair_sort_field(item, ascending) );
    }
  }
}

void Document_Glom::load_after_layout_group(const xmlpp::Element* node, const Glib::ustring& table_name, const sharedptr<LayoutGroup>& group, bool with_print_layout_positions)
{
  if(!node || !group)
  {
    //g_warning("Document_Glom::load_after_layout_group(): node is NULL");
    return;
  }

  //Get the group details:
  group->set_name( get_node_attribute_value(node, GLOM_ATTRIBUTE_NAME) );
  group->set_title( get_node_attribute_value(node, GLOM_ATTRIBUTE_TITLE) );
  group->m_columns_count = get_node_attribute_value_as_decimal(node, GLOM_ATTRIBUTE_COLUMNS_COUNT);
  group->set_border_width( get_node_attribute_value_as_decimal_double(node, GLOM_ATTRIBUTE_BORDER_WIDTH) );

  group->m_sequence = get_node_attribute_value_as_decimal(node, GLOM_ATTRIBUTE_SEQUENCE);



  //Translations:
  sharedptr<LayoutGroup> temp = group;
  load_after_translations(node, *temp);

  //Get the child items:
  xmlpp::Node::NodeList listNodes = node->get_children();
  for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
  {
    sharedptr<LayoutItem> item_added;
    int item_added_sequence = -1;

    //Create the layout item:
    const xmlpp::Element* element = dynamic_cast<const xmlpp::Element*>(*iter);
    if(element)
    {
      const guint sequence = get_node_attribute_value_as_decimal(element, GLOM_ATTRIBUTE_SEQUENCE);
      if(element->get_name() == GLOM_NODE_DATA_LAYOUT_ITEM)
      {
        sharedptr<LayoutItem_Field> item = sharedptr<LayoutItem_Field>::create();
        //item.set_full_field_details_empty();
        load_after_layout_item_field(element, table_name, item);

        item->m_sequence = sequence;
        item_added_sequence = sequence;

        item_added = item; 
      }
      else if(element->get_name() == GLOM_NODE_DATA_LAYOUT_BUTTON)
      {
        sharedptr<LayoutItem_Button> item = sharedptr<LayoutItem_Button>::create();

        item->set_script( get_child_text_node(element, GLOM_NODE_BUTTON_SCRIPT) );
        if(!(item->get_has_script())) //Try the deprecated attribute instead
           item->set_script( get_node_attribute_value(element, GLOM_DEPRECATED_ATTRIBUTE_BUTTON_SCRIPT) );

        load_after_translations(element, *item);

        item->m_sequence = sequence;
        item_added_sequence = sequence;

        item_added = item; 
      }
      else if(element->get_name() == GLOM_NODE_DATA_LAYOUT_TEXTOBJECT)
      {
        sharedptr<LayoutItem_Text> item = sharedptr<LayoutItem_Text>::create();
        load_after_translations(element, *item);

        //The text can be translated too, so it has its own node:
        const xmlpp::Element* element_text = get_node_child_named(element, GLOM_NODE_DATA_LAYOUT_TEXTOBJECT_TEXT);
        if(element_text)
        {
          sharedptr<TranslatableItem> translatable_text = sharedptr<TranslatableItem>::create();
          load_after_translations(element_text, *translatable_text);
          item->m_text = translatable_text;
        }

        item->m_sequence = sequence;
        item_added_sequence = sequence;

        item_added = item; 
      }
      else if(element->get_name() == GLOM_NODE_DATA_LAYOUT_IMAGEOBJECT)
      {
        sharedptr<LayoutItem_Image> item = sharedptr<LayoutItem_Image>::create();
        load_after_translations(element, *item);

        item->set_image(get_node_attribute_value_as_value(element, GLOM_ATTRIBUTE_DATA_LAYOUT_IMAGEOBJECT_IMAGE, Field::TYPE_IMAGE));

        item->m_sequence = sequence;
        item_added_sequence = sequence;

        item_added = item; 
      }
      else if(element->get_name() == GLOM_NODE_DATA_LAYOUT_ITEM_FIELDSUMMARY)
      {
        sharedptr<LayoutItem_FieldSummary> item = sharedptr<LayoutItem_FieldSummary>::create();
        //item.set_full_field_details_empty();
        load_after_layout_item_field(element, table_name, item);
        item->set_full_field_details( get_field(item->get_table_used(table_name), item->get_name()) );
        item->set_summary_type_from_sql( get_node_attribute_value(element, GLOM_ATTRIBUTE_LAYOUT_ITEM_FIELDSUMMARY_SUMMARYTYPE) );

        item->m_sequence = sequence;
        item_added_sequence = sequence;
        item_added = item; 
      }
      else if(element->get_name() == GLOM_NODE_DATA_LAYOUT_ITEM_HEADER)
      {
        sharedptr<LayoutItem_Header> child_group = sharedptr<LayoutItem_Header>::create();
        //Recurse:
        load_after_layout_group(element, table_name, child_group, with_print_layout_positions);
        item_added = child_group; 
      }
      else if(element->get_name() == GLOM_NODE_DATA_LAYOUT_ITEM_FOOTER)
      {
        sharedptr<LayoutItem_Footer> child_group = sharedptr<LayoutItem_Footer>::create();
        //Recurse:
        load_after_layout_group(element, table_name, child_group, with_print_layout_positions);
        item_added = child_group; 
      }
      else if(element->get_name() == GLOM_NODE_DATA_LAYOUT_GROUP)
      {
        sharedptr<LayoutGroup> child_group = sharedptr<LayoutGroup>::create();
        //Recurse:
        load_after_layout_group(element, table_name, child_group, with_print_layout_positions);
        item_added = child_group; 
      }
      else if(element->get_name() == GLOM_NODE_DATA_LAYOUT_NOTEBOOK)
      {
        sharedptr<LayoutItem_Notebook> notebook = sharedptr<LayoutItem_Notebook>::create();
        load_after_layout_group(element, table_name, notebook, with_print_layout_positions);
        item_added = notebook; 
      }
      else if(element->get_name() == GLOM_NODE_DATA_LAYOUT_PORTAL)
      {
        sharedptr<LayoutItem_Portal> portal = sharedptr<LayoutItem_Portal>::create();

        load_after_layout_item_usesrelationship(element, table_name, portal); 
       
        sharedptr<UsesRelationship> relationship_navigation_specific;
        bool relationship_navigation_specific_main = false;
        xmlpp::Element* elementNavigationRelationshipSpecific = get_node_child_named(element, GLOM_NODE_DATA_LAYOUT_PORTAL_NAVIGATIONRELATIONSHIP);
        if(elementNavigationRelationshipSpecific)
        {
           //std::cout << "debug: loading GLOM_NODE_DATA_LAYOUT_PORTAL_NAVIGATIONRELATIONSHIP" << std::endl;

           relationship_navigation_specific = sharedptr<UsesRelationship>::create();
           load_after_layout_item_usesrelationship(elementNavigationRelationshipSpecific, portal->get_table_used(table_name), relationship_navigation_specific);
           relationship_navigation_specific_main = get_node_attribute_value_as_bool(elementNavigationRelationshipSpecific, GLOM_ATTRIBUTE_PORTAL_NAVIGATIONRELATIONSHIP_MAIN);

           if(relationship_navigation_specific)
           {
            std::cout << "  debug: relationship_navigation_specific->relationship=" << relationship_navigation_specific->get_relationship_name() << std::endl;

           }
        }

        portal->set_navigation_relationship_specific(relationship_navigation_specific_main, relationship_navigation_specific);

        load_after_layout_group(element, portal->get_table_used(table_name), portal, with_print_layout_positions);
        item_added = portal; 
      }
      else if(element->get_name() == GLOM_NODE_DATA_LAYOUT_ITEM_GROUPBY)
      {
        sharedptr<LayoutItem_GroupBy> child_group = sharedptr<LayoutItem_GroupBy>::create();
        //Recurse:
        load_after_layout_group(element, table_name, child_group, with_print_layout_positions);

        //Group-By field:
        sharedptr<LayoutItem_Field> field_groupby = sharedptr<LayoutItem_Field>::create();
        xmlpp::Element* elementGroupBy = get_node_child_named(element, GLOM_NODE_REPORT_ITEM_GROUPBY_GROUPBY);
        if(elementGroupBy)
        {
          load_after_layout_item_field(elementGroupBy, table_name, field_groupby);
          field_groupby->set_full_field_details( get_field(field_groupby->get_table_used(table_name), field_groupby->get_name()) );
        }
        child_group->set_field_group_by(field_groupby);


        //field_groupby.set_full_field_details_empty();

        //Sort fields:
        xmlpp::Element* elementSortBy = get_node_child_named(element, GLOM_NODE_REPORT_ITEM_GROUPBY_SORTBY);
        if(elementSortBy)
        {
          LayoutItem_GroupBy::type_list_sort_fields sort_fields;
          load_after_sort_by(elementSortBy, table_name, sort_fields);
          child_group->set_fields_sort_by(sort_fields);
        }

        //Secondary fields:
        xmlpp::Element* elementSecondary = get_node_child_named(element, GLOM_NODE_DATA_LAYOUT_GROUP_SECONDARYFIELDS);
        if(elementSecondary)
        {
          xmlpp::Element* elementGroup = get_node_child_named(elementSecondary, GLOM_NODE_DATA_LAYOUT_GROUP);
          if(elementGroup)
          {
            load_after_layout_group(elementGroup, table_name, child_group->m_group_secondary_fields, with_print_layout_positions);
            fill_layout_field_details(table_name, child_group->m_group_secondary_fields); //Get full field details from the field names.
          }
        }

        item_added = child_group; 
      }
      else if(element->get_name() == GLOM_NODE_DATA_LAYOUT_ITEM_VERTICALGROUP)
      {
        sharedptr<LayoutItem_VerticalGroup> child_group = sharedptr<LayoutItem_VerticalGroup>::create();
        //Recurse:
        load_after_layout_group(element, table_name, child_group, with_print_layout_positions);

        item_added = child_group; 
      }
      else if(element->get_name() == GLOM_NODE_DATA_LAYOUT_ITEM_SUMMARY)
      {
        sharedptr<LayoutItem_Summary> child_group = sharedptr<LayoutItem_Summary>::create();
        //Recurse:
        load_after_layout_group(element, table_name, child_group, with_print_layout_positions);

        item_added = child_group; 
      }
    }

    //Add the new layout item to the group:
    if(item_added)
    {
      if(item_added_sequence != -1)
        group->add_item(item_added, item_added_sequence);
      else
        group->add_item(item_added);

      if(with_print_layout_positions)
        load_after_print_layout_position(element, item_added);
    }
  } //for
}

void Document_Glom::load_after_translations(const xmlpp::Element* element, TranslatableItem& item)
{
  if(!element)
    return;

  item.set_title_original( get_node_attribute_value(element, GLOM_ATTRIBUTE_TITLE) );

  const xmlpp::Element* nodeTranslations = get_node_child_named(element, GLOM_NODE_TRANSLATIONS_SET);
  if(nodeTranslations)
  {
    xmlpp::Node::NodeList listNodesTranslations = nodeTranslations->get_children(GLOM_NODE_TRANSLATION);
    for(xmlpp::Node::NodeList::iterator iter = listNodesTranslations.begin(); iter != listNodesTranslations.end(); ++iter)
    {
      const xmlpp::Element* element = dynamic_cast<const xmlpp::Element*>(*iter);
      if(element)
      {
        const Glib::ustring locale = get_node_attribute_value(element, GLOM_ATTRIBUTE_TRANSLATION_LOCALE);
        const Glib::ustring translation = get_node_attribute_value(element, GLOM_ATTRIBUTE_TRANSLATION_VALUE);
        item.set_translation(locale, translation);
      }
    }
  }
}

void Document_Glom::load_after_print_layout_position(const xmlpp::Element* nodeItem, const sharedptr<LayoutItem>& item)
{
  if(!nodeItem)
    return;

  const xmlpp::Element* child = get_node_child_named(nodeItem, GLOM_NODE_POSITION);
  if(child)
  {
    const double x = get_node_attribute_value_as_decimal_double(child, GLOM_ATTRIBUTE_POSITION_X);
    const double y = get_node_attribute_value_as_decimal_double(child, GLOM_ATTRIBUTE_POSITION_Y);
    const double width = get_node_attribute_value_as_decimal_double(child, GLOM_ATTRIBUTE_POSITION_WIDTH);
    const double height = get_node_attribute_value_as_decimal_double(child, GLOM_ATTRIBUTE_POSITION_HEIGHT);
    item->set_print_layout_position(x, y, width, height);
  }
}

bool Document_Glom::load_after()
{
  Bakery::BusyCursor busy_cursor(m_parent_window);

  m_block_modified_set = true; //Prevent the set_ functions from trigerring a save.

  bool result = Bakery::Document_XML::load_after();  

  m_block_cache_update = true; //Don't waste time repeatedly updating this until we have finished.

  if(result)
  {
    const xmlpp::Element* nodeRoot = get_node_document();
    if(nodeRoot)
    {
      m_document_format_version = get_node_attribute_value_as_decimal(nodeRoot, GLOM_ATTRIBUTE_FORMAT_VERSION);

      if(m_document_format_version > get_latest_known_document_format_version())
      {
        std::cerr << "Document_Glom::load_after(): Loading failed because format_version=" << m_document_format_version << ", but latest known format version is " << get_latest_known_document_format_version() << std::endl;
        return false; //TODO: Provide more information so the application (or Bakery) can say exactly why loading failed.
      }
      m_is_example = get_node_attribute_value_as_bool(nodeRoot, GLOM_ATTRIBUTE_IS_EXAMPLE);
      m_database_title = get_node_attribute_value(nodeRoot, GLOM_ATTRIBUTE_CONNECTION_DATABASE_TITLE);

      m_translation_original_locale = get_node_attribute_value(nodeRoot, GLOM_ATTRIBUTE_TRANSLATION_ORIGINAL_LOCALE);
      TranslatableItem::set_original_locale(m_translation_original_locale);

      const xmlpp::Element* nodeConnection = get_node_child_named(nodeRoot, GLOM_NODE_CONNECTION);
      if(nodeConnection)
      {
        //Connection information:
        bool self_hosted = get_node_attribute_value_as_bool(nodeConnection, GLOM_ATTRIBUTE_CONNECTION_SELF_HOSTED);
        m_connection_server = get_node_attribute_value(nodeConnection, GLOM_ATTRIBUTE_CONNECTION_SERVER);
        m_connection_user = get_node_attribute_value(nodeConnection, GLOM_ATTRIBUTE_CONNECTION_USER);
        m_connection_database = get_node_attribute_value(nodeConnection, GLOM_ATTRIBUTE_CONNECTION_DATABASE);

#ifdef GLOM_ENABLE_CLIENT_ONLY
        if(self_hosted)
        {
          std::cerr << "Document_Glom::load_after(): Loading failed because the document needs to be self-hosted, but self-hosting is not supported in client only mode" << std::endl;
          return false; //TODO: Provide more information so the application (or Bakery) can say exactly why loading failed.
        }
#else
        m_connection_is_self_hosted = self_hosted;
#endif
      }

      //Tables:
      m_tables.clear();

      //Look at each "table" node.
      xmlpp::Node::NodeList listNodes = nodeRoot->get_children(GLOM_NODE_TABLE);
      for(xmlpp::Node::NodeList::const_iterator iter = listNodes.begin(); iter != listNodes.end(); iter++)
      {
        xmlpp::Element* nodeTable = dynamic_cast<xmlpp::Element*>(*iter);
        if(nodeTable)
        {
          const Glib::ustring table_name = get_node_attribute_value(nodeTable, GLOM_ATTRIBUTE_NAME);

          m_tables[table_name] = DocumentTableInfo();
          DocumentTableInfo& doctableinfo = m_tables[table_name]; //Setting stuff directly in the reference is more efficient than copying it later:

          sharedptr<TableInfo> table_info(new TableInfo());
          table_info->set_name(table_name);
          table_info->m_hidden = get_node_attribute_value_as_bool(nodeTable, GLOM_ATTRIBUTE_HIDDEN);
          table_info->m_default = get_node_attribute_value_as_bool(nodeTable, GLOM_ATTRIBUTE_DEFAULT);

          doctableinfo.m_info = table_info;

          doctableinfo.m_overviewx = get_node_attribute_value_as_float(nodeTable, GLOM_ATTRIBUTE_OVERVIEW_X);
          doctableinfo.m_overviewy = get_node_attribute_value_as_float(nodeTable, GLOM_ATTRIBUTE_OVERVIEW_Y);

          //Translations:
          load_after_translations(nodeTable, *(doctableinfo.m_info));

          //Relationships:
          //These should be loaded before the fields, because the fields use them.
          const xmlpp::Element* nodeRelationships = get_node_child_named(nodeTable, GLOM_NODE_RELATIONSHIPS);
          if(nodeRelationships)
          {
            const xmlpp::Node::NodeList listNodes = nodeRelationships->get_children(GLOM_NODE_RELATIONSHIP);
            for(xmlpp::Node::NodeList::const_iterator iter = listNodes.begin(); iter != listNodes.end(); iter++)
            {
              const xmlpp::Element* nodeChild = dynamic_cast<xmlpp::Element*>(*iter);
              if(nodeChild)
              {
                sharedptr<Relationship> relationship = sharedptr<Relationship>::create();
                const Glib::ustring relationship_name = get_node_attribute_value(nodeChild, GLOM_ATTRIBUTE_NAME);

                relationship->set_from_table(table_name);
                relationship->set_name(relationship_name);;

                relationship->set_from_field( get_node_attribute_value(nodeChild, GLOM_ATTRIBUTE_KEY) );
                relationship->set_to_table( get_node_attribute_value(nodeChild, GLOM_ATTRIBUTE_OTHER_TABLE) );
                relationship->set_to_field( get_node_attribute_value(nodeChild, GLOM_ATTRIBUTE_OTHER_KEY) );
                relationship->set_auto_create( get_node_attribute_value_as_bool(nodeChild, GLOM_ATTRIBUTE_AUTO_CREATE) );
                relationship->set_allow_edit( get_node_attribute_value_as_bool(nodeChild, GLOM_ATTRIBUTE_ALLOW_EDIT) );

                //Translations:
                load_after_translations(nodeChild, *relationship);

                doctableinfo.m_relationships.push_back(relationship);
              }
            }
          }

          //Fields:
          const xmlpp::Element* nodeFields = get_node_child_named(nodeTable, GLOM_NODE_FIELDS);
          if(nodeFields)
          {
            const Field::type_map_type_names type_names = Field::get_type_names();

            //Loop through Field child nodes:
            xmlpp::Node::NodeList listNodes = nodeFields->get_children(GLOM_NODE_FIELD);
            for(xmlpp::Node::NodeList::const_iterator iter = listNodes.begin(); iter != listNodes.end(); iter++)
            {
              const xmlpp::Element* nodeChild = dynamic_cast<xmlpp::Element*>(*iter);
              if(nodeChild)
              {
                sharedptr<Field> field(new Field());

                const Glib::ustring strName = get_node_attribute_value(nodeChild, GLOM_ATTRIBUTE_NAME);
                field->set_name( strName );

                field->set_primary_key( get_node_attribute_value_as_bool(nodeChild, GLOM_ATTRIBUTE_PRIMARY_KEY) );
                field->set_unique_key( get_node_attribute_value_as_bool(nodeChild, GLOM_ATTRIBUTE_UNIQUE) );
                field->set_auto_increment( get_node_attribute_value_as_bool(nodeChild, GLOM_ATTRIBUTE_AUTOINCREMENT) );

                //Get lookup information, if present.
                xmlpp::Element* nodeLookup = get_node_child_named(nodeChild, GLOM_NODE_FIELD_LOOKUP);
                if(nodeLookup)
                {
                  const Glib::ustring lookup_relationship_name = get_node_attribute_value(nodeLookup, GLOM_ATTRIBUTE_RELATIONSHIP_NAME);
                  sharedptr<Relationship> lookup_relationship = get_relationship(table_name, lookup_relationship_name);
                  field->set_lookup_relationship(lookup_relationship);

                  field->set_lookup_field( get_node_attribute_value(nodeLookup, GLOM_ATTRIBUTE_FIELD) );
                }

                field->set_calculation( get_child_text_node(nodeChild, GLOM_NODE_CALCULATION) );
                if(!(field->get_has_calculation())) //Try the deprecated attribute instead
                  field->set_calculation( get_node_attribute_value(nodeChild, GLOM_DEPRECATED_ATTRIBUTE_CALCULATION) );

                //Field Type:
                const Glib::ustring field_type = get_node_attribute_value(nodeChild, GLOM_ATTRIBUTE_TYPE);

                //Get the type enum for this string representation of the type:
                Field::glom_field_type field_type_enum = Field::TYPE_INVALID;
                for(Field::type_map_type_names::const_iterator iter = type_names.begin(); iter !=type_names.end(); ++iter)
                {
                  if(iter->second == field_type)
                  {
                    field_type_enum = iter->first;
                    break;
                  }
                }

                field->set_default_value( get_node_attribute_value_as_value(nodeChild, GLOM_ATTRIBUTE_DEFAULT_VALUE, field_type_enum) );

                //We set this after set_field_info(), because that gets a glom type from the (not-specified) gdatype. Yes, that's strange, and should probably be more explicit.
                field->set_glom_type(field_type_enum);

                //Default Formatting:
                const xmlpp::Element* elementFormatting = get_node_child_named(nodeChild, GLOM_NODE_FORMAT);
                if(elementFormatting)
                  load_after_layout_item_field_formatting(elementFormatting, field->m_default_formatting, field_type_enum, table_name, strName);

                //Translations:
                load_after_translations(nodeChild, *field);

                doctableinfo.m_fields.push_back(field);
              }
            }
          } //Fields

          // Load Example Rows after fields have been loaded, because they
	  // need the fields to be able to associate a value to a named field.
          const xmlpp::Element* nodeExampleRows = get_node_child_named(nodeTable, GLOM_NODE_EXAMPLE_ROWS);
          if(nodeExampleRows)
          {
            //Loop through example_row child nodes:
            xmlpp::Node::NodeList listExampleRows = nodeExampleRows->get_children(GLOM_NODE_EXAMPLE_ROW);
            for(xmlpp::Node::NodeList::const_iterator iter = listExampleRows.begin(); iter != listExampleRows.end(); ++ iter)
            {
              const xmlpp::Element* nodeChild = dynamic_cast<xmlpp::Element*>(*iter);
              if(nodeChild)
              {
                typedef std::vector<Glib::ustring> type_vecStrings; // TODO: Put this into class declaration?
                type_vecStrings field_values(doctableinfo.m_fields.size(), "''");
                //Loop through value child nodes
                xmlpp::Node::NodeList listNodes = nodeChild->get_children(GLOM_NODE_VALUE);
                for(xmlpp::Node::NodeList::const_iterator iter = listNodes.begin(); iter != listNodes.end(); ++ iter)
                {
                  const xmlpp::Element* nodeChild = dynamic_cast<xmlpp::Element*>(*iter);
                  if(nodeChild)
                  {
                    const xmlpp::Attribute* column_name = nodeChild->get_attribute(GLOM_ATTRIBUTE_COLUMN);
                    if(column_name)
                    {
                      // TODO_Performance: If it's too much rows we could
                      // consider a map to find the column more quickly.
                      for(unsigned int i = 0; i < doctableinfo.m_fields.size(); ++ i)
                      {
                        if(doctableinfo.m_fields[i]->get_name() == column_name->get_value())
                        {
			  if(nodeChild->has_child_text())
                            field_values[i] = nodeChild->get_child_text()->get_content();
                          break;
                        }
                      }
                    }
                  }
                }

                // Append line to doctableinfo.m_example_rows
                if(!doctableinfo.m_example_rows.empty())
                  doctableinfo.m_example_rows += "\n";

                for(type_vecStrings::const_iterator field_iter = field_values.begin(); field_iter != field_values.end(); ++ field_iter)
                {
                  if(field_iter != field_values.begin())
                    doctableinfo.m_example_rows += ",";
                  doctableinfo.m_example_rows += *field_iter;
                }

                // TODO_Performance: doctableinfo.m_example_rows should perhaps be a vector or list of strings (a string for each row) instead of one big string
              }
            }
          } // Example Rows

          if(doctableinfo.m_example_rows.empty()) //Try the deprecated format (all rows in a single node) instead
            doctableinfo.m_example_rows = get_child_text_node(nodeTable, GLOM_NODE_EXAMPLE_ROWS);
          if(doctableinfo.m_example_rows.empty()) //Try the deprecated attribute instead
            doctableinfo.m_example_rows = get_node_attribute_value(nodeTable, GLOM_DEPRECATED_ATTRIBUTE_EXAMPLE_ROWS);

          //std::cout << "  debug: loading: table=" << table_name << ", m_example_rows.size()=" << doctableinfo.m_example_rows.size() << std::endl;

        } //if(table)
      } //Tables.

      //Look at each "table" node.
      //We do load the layouts separately, because we needed to load all the tables' relationships and tables 
      //before we can load layouts that can use them.
      for(xmlpp::Node::NodeList::const_iterator iter = listNodes.begin(); iter != listNodes.end(); iter++)
      {
        xmlpp::Element* nodeTable = dynamic_cast<xmlpp::Element*>(*iter);
        if(nodeTable)
        {
          const Glib::ustring table_name = get_node_attribute_value(nodeTable, GLOM_ATTRIBUTE_NAME);
          DocumentTableInfo& doctableinfo = m_tables[table_name]; //Setting stuff directly in the reference is more efficient than copying it later:

          //Layouts:
          const xmlpp::Element* nodeDataLayouts = get_node_child_named(nodeTable, GLOM_NODE_DATA_LAYOUTS);
          if(nodeDataLayouts)
          {
            xmlpp::Node::NodeList listNodes = nodeDataLayouts->get_children(GLOM_NODE_DATA_LAYOUT);
            for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
            {
              xmlpp::Element* node = dynamic_cast<xmlpp::Element*>(*iter);
              if(node)
              {
                const Glib::ustring layout_name = get_node_attribute_value(node, GLOM_ATTRIBUTE_NAME);
                Glib::ustring parent_table = get_node_attribute_value(node, GLOM_ATTRIBUTE_PARENT_TABLE_NAME);
                if(parent_table.empty())
                  parent_table = table_name; //Deal with the earlier file format that did not include this.

                type_mapLayoutGroupSequence layout_groups;

                const xmlpp::Element* nodeGroups = get_node_child_named(node, GLOM_NODE_DATA_LAYOUT_GROUPS);
                if(nodeGroups)
                {
                  //Look at all its children:
                  xmlpp::Node::NodeList listNodes = nodeGroups->get_children(GLOM_NODE_DATA_LAYOUT_GROUP);
                  for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
                  {
                    const xmlpp::Element* node = dynamic_cast<const xmlpp::Element*>(*iter);
                    if(node)
                    {
                      const Glib::ustring group_name = get_node_attribute_value(node, GLOM_ATTRIBUTE_NAME);
                      if(!group_name.empty())
                      {
                        sharedptr<LayoutGroup> group(new LayoutGroup());
                        load_after_layout_group(node, table_name, group);

                        layout_groups[group->m_sequence] = group;
                      }
                    }
                  }
                }

                LayoutInfo layout_info;
                layout_info.m_parent_table = parent_table;
                layout_info.m_layout_name = layout_name;
                layout_info.m_layout_groups = layout_groups;
                doctableinfo.m_layouts.push_back(layout_info);
              }
            }
          } //if(nodeDataLayouts)


          //Reports:
          const xmlpp::Element* nodeReports = get_node_child_named(nodeTable, GLOM_NODE_REPORTS);
          if(nodeReports)
          {
            xmlpp::Node::NodeList listNodes = nodeReports->get_children(GLOM_NODE_REPORT);
            for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
            {
              xmlpp::Element* node = dynamic_cast<xmlpp::Element*>(*iter);
              if(node)
              {
                const Glib::ustring report_name = get_node_attribute_value(node, GLOM_ATTRIBUTE_NAME);
                const bool show_table_title = get_node_attribute_value_as_bool(node, GLOM_ATTRIBUTE_REPORT_SHOW_TABLE_TITLE);

                //type_mapLayoutGroupSequence layout_groups;

                sharedptr<Report> report(new Report());
                report->set_name(report_name);
                report->set_show_table_title(show_table_title);

                const xmlpp::Element* nodeGroups = get_node_child_named(node, GLOM_NODE_DATA_LAYOUT_GROUPS);
                if(nodeGroups)
                {
                  //Look at all its children:
                  xmlpp::Node::NodeList listNodes = nodeGroups->get_children(GLOM_NODE_DATA_LAYOUT_GROUP);
                  for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
                  {
                    const xmlpp::Element* node = dynamic_cast<const xmlpp::Element*>(*iter);
                    if(node)
                    {
                      sharedptr<LayoutGroup> group = sharedptr<LayoutGroup>::create();
                      load_after_layout_group(node, table_name, group);

                      //layout_groups[group.m_sequence] = group;
                      report->m_layout_group = group; //TODO: Get rid of the for loop here.

                      fill_layout_field_details(table_name, report->m_layout_group); //Get full field details from the field names.
                    }
                  }
                }

                //Translations:
                load_after_translations(node, *report);

                doctableinfo.m_reports[report->get_name()] = report;
              }
            }
          } //if(nodeReports)


          //Print Layouts:
          const xmlpp::Element* nodePrintLayouts = get_node_child_named(nodeTable, GLOM_NODE_PRINT_LAYOUTS);
          if(nodePrintLayouts)
          {
            xmlpp::Node::NodeList listNodes = nodePrintLayouts->get_children(GLOM_NODE_PRINT_LAYOUT);
            for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
            {
              xmlpp::Element* node = dynamic_cast<xmlpp::Element*>(*iter);
              if(node)
              {
                const Glib::ustring name = get_node_attribute_value(node, GLOM_ATTRIBUTE_NAME);
                const bool show_table_title = get_node_attribute_value_as_bool(node, GLOM_ATTRIBUTE_REPORT_SHOW_TABLE_TITLE);

                sharedptr<PrintLayout> print_layout(new PrintLayout());
                print_layout->set_name(name);
                print_layout->set_show_table_title(show_table_title);

                //Page Setup:
                Glib::RefPtr<Gtk::PageSetup> page_setup;
                const Glib::ustring key_file_text = get_child_text_node(node, GLOM_NODE_PAGE_SETUP);
                if(!key_file_text.empty())
                {
                  Glib::KeyFile key_file;
                  key_file.load_from_data(key_file_text);
                  //TODO: Use this when gtkmm and GTK+ have been fixed: page_setup = Gtk::PageSetup::create(key_file);
                  page_setup = Glib::wrap(gtk_page_setup_new_from_key_file(key_file.gobj(), NULL, NULL));
                }
                print_layout->set_page_setup(page_setup);

                //Layout Groups:
                const xmlpp::Element* nodeGroups = get_node_child_named(node, GLOM_NODE_DATA_LAYOUT_GROUPS);
                if(nodeGroups)
                {
                  //Look at all its children:
                  xmlpp::Node::NodeList listNodes = nodeGroups->get_children(GLOM_NODE_DATA_LAYOUT_GROUP);
                  for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
                  {
                    const xmlpp::Element* node = dynamic_cast<const xmlpp::Element*>(*iter);
                    if(node)
                    {
                      sharedptr<LayoutGroup> group = sharedptr<LayoutGroup>::create();
                      load_after_layout_group(node, table_name, group, true /* load positions too. */);

                      //layout_groups[group.m_sequence] = group;
                      print_layout->m_layout_group = group; //TODO: Get rid of the for loop here.

                      fill_layout_field_details(table_name, print_layout->m_layout_group); //Get full field details from the field names.
                    }
                  }
                }

                //Translations:
                load_after_translations(node, *print_layout);

                doctableinfo.m_print_layouts[print_layout->get_name()] = print_layout;
              }
            }
          } //if(nodePrintLayouts)


          //Groups:
          m_groups.clear();

          const xmlpp::Element* nodeGroups = get_node_child_named(nodeRoot, GLOM_NODE_GROUPS);
          if(nodeGroups)
          {
            xmlpp::Node::NodeList listNodes = nodeGroups->get_children(GLOM_NODE_GROUP);
            for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
            {
              xmlpp::Element* node = dynamic_cast<xmlpp::Element*>(*iter);
              if(node)
              {
                GroupInfo group_info;

                group_info.set_name( get_node_attribute_value(node, GLOM_ATTRIBUTE_NAME) );
                group_info.m_developer = get_node_attribute_value_as_bool(node, GLOM_ATTRIBUTE_DEVELOPER);

                xmlpp::Node::NodeList listTablePrivs = nodeGroups->get_children(GLOM_NODE_TABLE_PRIVS);
                for(xmlpp::Node::NodeList::iterator iter = listTablePrivs.begin(); iter != listTablePrivs.end(); ++iter)
                {
                  xmlpp::Element* node = dynamic_cast<xmlpp::Element*>(*iter);
                  if(node)
                  {
                    const Glib::ustring table_name = get_node_attribute_value(node, GLOM_ATTRIBUTE_TABLE_NAME);

                    Privileges privs;
                    privs.m_view = get_node_attribute_value_as_bool(node, GLOM_ATTRIBUTE_PRIV_VIEW);
                    privs.m_edit = get_node_attribute_value_as_bool(node, GLOM_ATTRIBUTE_PRIV_EDIT);
                    privs.m_create = get_node_attribute_value_as_bool(node, GLOM_ATTRIBUTE_PRIV_CREATE);
                    privs.m_delete = get_node_attribute_value_as_bool(node, GLOM_ATTRIBUTE_PRIV_DELETE);

                    group_info.m_map_privileges[table_name] = privs;
                  }
                }

                m_groups[group_info.get_name()] = group_info;
              }
            }
          }


          //Library Modules:
          m_map_library_scripts.clear();

          const xmlpp::Element* nodeModules = get_node_child_named(nodeRoot, GLOM_NODE_LIBRARY_MODULES);
          if(nodeModules)
          {
            xmlpp::Node::NodeList listNodes = nodeModules->get_children(GLOM_NODE_LIBRARY_MODULE);
            for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
            {
              xmlpp::Element* node = dynamic_cast<xmlpp::Element*>(*iter);
              if(node)
              {
                const Glib::ustring table_name = get_node_attribute_value(node, GLOM_ATTRIBUTE_LIBRARY_MODULE_NAME);
                const Glib::ustring script = get_node_attribute_value(node, GLOM_ATTRIBUTE_LIBRARY_MODULE_SCRIPT);

                m_map_library_scripts[table_name] = script;
              }
            }
          }

        } //root
      }
    }
  }

  m_block_cache_update = false;

  m_block_modified_set = false;

  return result;
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void Document_Glom::save_before_layout_item_field_formatting(xmlpp::Element* nodeItem, const FieldFormatting& format, Field::glom_field_type field_type)
{
  //Numeric format:
  set_node_attribute_value_as_bool(nodeItem, GLOM_ATTRIBUTE_FORMAT_THOUSANDS_SEPARATOR,  format.m_numeric_format.m_use_thousands_separator);
  set_node_attribute_value_as_bool(nodeItem, GLOM_ATTRIBUTE_FORMAT_DECIMAL_PLACES_RESTRICTED, format.m_numeric_format.m_decimal_places_restricted);
  set_node_attribute_value_as_decimal(nodeItem, GLOM_ATTRIBUTE_FORMAT_DECIMAL_PLACES, format.m_numeric_format.m_decimal_places);
  set_node_attribute_value(nodeItem, GLOM_ATTRIBUTE_FORMAT_CURRENCY_SYMBOL, format.m_numeric_format.m_currency_symbol);

  set_node_attribute_value_as_bool(nodeItem, GLOM_ATTRIBUTE_FORMAT_CHOICES_RESTRICTED, format.get_choices_restricted());
  set_node_attribute_value_as_bool(nodeItem, GLOM_ATTRIBUTE_FORMAT_CHOICES_CUSTOM, format.get_has_custom_choices());

  //Text formatting:
  if(field_type == Field::TYPE_TEXT)
  {
    set_node_attribute_value_as_bool(nodeItem, GLOM_ATTRIBUTE_FORMAT_TEXT_MULTILINE, format.get_text_format_multiline());
    set_node_attribute_value_as_decimal(nodeItem, GLOM_ATTRIBUTE_FORMAT_TEXT_MULTILINE_HEIGHT_LINES, format.get_text_format_multiline_height_lines());

    set_node_attribute_value(nodeItem, GLOM_ATTRIBUTE_FORMAT_TEXT_FONT, format.get_text_format_font());
    set_node_attribute_value(nodeItem, GLOM_ATTRIBUTE_FORMAT_TEXT_COLOR_FOREGROUND, format.get_text_format_color_foreground());
    set_node_attribute_value(nodeItem, GLOM_ATTRIBUTE_FORMAT_TEXT_COLOR_BACKGROUND, format.get_text_format_color_background());
  }

  //Choices:
  if(format.get_has_custom_choices())
  {
    xmlpp::Element* child = nodeItem->add_child(GLOM_ATTRIBUTE_FORMAT_CHOICES_CUSTOM_LIST);

    const FieldFormatting::type_list_values list_values = format.get_choices_custom();
    for(FieldFormatting::type_list_values::const_iterator iter = list_values.begin(); iter != list_values.end(); ++iter)
    {
      xmlpp::Element* childChoice = child->add_child(GLOM_NODE_FORMAT_CUSTOM_CHOICE);
      set_node_attribute_value_as_value(childChoice, GLOM_ATTRIBUTE_VALUE, *iter, field_type);
    }
  }

  set_node_attribute_value_as_bool(nodeItem, GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED, format.get_has_related_choices() );

  sharedptr<Relationship> choice_relationship;
  Glib::ustring choice_field, choice_second;
  format.get_choices(choice_relationship, choice_field, choice_second);

  set_node_attribute_value(nodeItem, GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_RELATIONSHIP, glom_get_sharedptr_name(choice_relationship));
  set_node_attribute_value(nodeItem, GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_FIELD, choice_field);
  set_node_attribute_value(nodeItem, GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_SECOND, choice_second);
}

void Document_Glom::save_before_layout_item_usesrelationship(xmlpp::Element* nodeItem, const sharedptr<const UsesRelationship>& item)
{
  if(!item)
    return;

  set_node_attribute_value(nodeItem, GLOM_ATTRIBUTE_RELATIONSHIP_NAME, item->get_relationship_name());
  set_node_attribute_value(nodeItem, GLOM_ATTRIBUTE_RELATED_RELATIONSHIP_NAME, item->get_related_relationship_name());
}

void Document_Glom::save_before_layout_item_field(xmlpp::Element* nodeItem, const sharedptr<const LayoutItem_Field>& field)
{
  if(!field)
    return;

  set_node_attribute_value(nodeItem, GLOM_ATTRIBUTE_NAME, field->get_name());
  save_before_layout_item_usesrelationship(nodeItem, field);
  set_node_attribute_value_as_bool(nodeItem, GLOM_ATTRIBUTE_EDITABLE, field->get_editable());

  xmlpp::Element* elementFormat = nodeItem->add_child(GLOM_NODE_FORMAT);
  save_before_layout_item_field_formatting(elementFormat, field->m_formatting, field->get_glom_type());

  set_node_attribute_value_as_bool(nodeItem, GLOM_ATTRIBUTE_DATA_LAYOUT_ITEM_FIELD_USE_DEFAULT_FORMATTING, field->get_formatting_use_default());

  sharedptr<const CustomTitle> custom_title = field->get_title_custom();
  if(custom_title)
  {
    xmlpp::Element* elementCustomTitle = nodeItem->add_child(GLOM_NODE_LAYOUT_ITEM_CUSTOM_TITLE);
    set_node_attribute_value_as_bool(elementCustomTitle, GLOM_ATTRIBUTE_LAYOUT_ITEM_CUSTOM_TITLE_USE, custom_title->get_use_custom_title());

    save_before_translations(elementCustomTitle, *custom_title);
  }

  set_node_attribute_value_as_decimal(nodeItem, GLOM_ATTRIBUTE_SEQUENCE, field->m_sequence);
}

void Document_Glom::save_before_sort_by(xmlpp::Element* node, const LayoutItem_GroupBy::type_list_sort_fields& list_fields)
{
  if(!node)
    return;

  for(LayoutItem_GroupBy::type_list_sort_fields::const_iterator iter = list_fields.begin(); iter != list_fields.end(); ++iter)
  {
    sharedptr<const LayoutItem_Field> field = iter->first;

    xmlpp::Element* nodeChild = node->add_child(GLOM_NODE_DATA_LAYOUT_ITEM);
    save_before_layout_item_field(nodeChild, field);

    set_node_attribute_value_as_bool(nodeChild, GLOM_ATTRIBUTE_SORT_ASCENDING, iter->second);
  }
}

void Document_Glom::save_before_layout_group(xmlpp::Element* node, const sharedptr<const LayoutGroup>& group, bool with_print_layout_positions)
{
  if(!node || !group)
    return;

  //g_warning("save_before_layout_group");

  xmlpp::Element* child = 0;

  sharedptr<const LayoutItem_GroupBy> group_by = sharedptr<const LayoutItem_GroupBy>::cast_dynamic(group);
  if(group_by) //If it is a GroupBy report part.
  {
    child = node->add_child(GLOM_NODE_DATA_LAYOUT_ITEM_GROUPBY);

    if(group_by->get_has_field_group_by())
    {
      xmlpp::Element* nodeGroupBy = child->add_child(GLOM_NODE_REPORT_ITEM_GROUPBY_GROUPBY);
      save_before_layout_item_field(nodeGroupBy, group_by->get_field_group_by());
    }

    //Sort fields:
    if(group_by->get_has_fields_sort_by())
    {
      xmlpp::Element* nodeSortBy = child->add_child(GLOM_NODE_REPORT_ITEM_GROUPBY_SORTBY);
      save_before_sort_by(nodeSortBy, group_by->get_fields_sort_by());
    }

    //Secondary fields:
    if(!group_by->m_group_secondary_fields->m_map_items.empty())
    {
      xmlpp::Element* secondary_fields = child->add_child(GLOM_NODE_DATA_LAYOUT_GROUP_SECONDARYFIELDS);
      save_before_layout_group(secondary_fields, group_by->m_group_secondary_fields, with_print_layout_positions);
    }
  }
  else
  {
    sharedptr<const LayoutItem_Summary> summary = sharedptr<const LayoutItem_Summary>::cast_dynamic(group);
    if(summary) //If it is a GroupBy report part.
    {
      child = node->add_child(GLOM_NODE_DATA_LAYOUT_ITEM_SUMMARY);
      //TODO: summary_type.
    }
    else
    {
      sharedptr<const LayoutItem_VerticalGroup> verticalgroup = sharedptr<const LayoutItem_VerticalGroup>::cast_dynamic(group);
      if(verticalgroup) //If it is a GroupBy report part.
      {
        child = node->add_child(GLOM_NODE_DATA_LAYOUT_ITEM_VERTICALGROUP);
      }
      else
      {
        sharedptr<const LayoutItem_Header> headerGroup = sharedptr<const LayoutItem_Header>::cast_dynamic(group);
        if(headerGroup) //If it is a GroupBy report part.
        {
          child = node->add_child(GLOM_NODE_DATA_LAYOUT_ITEM_HEADER);
        }
        else
        {
          sharedptr<const LayoutItem_Footer> footerGroup = sharedptr<const LayoutItem_Footer>::cast_dynamic(group);
          if(footerGroup) //If it is a GroupBy report part.
          {
            child = node->add_child(GLOM_NODE_DATA_LAYOUT_ITEM_FOOTER);
          }
          else
          {
            sharedptr<const LayoutItem_Portal> portal = sharedptr<const LayoutItem_Portal>::cast_dynamic(group);
            if(portal) //If it is a related records portal
            {
              child = node->add_child(GLOM_NODE_DATA_LAYOUT_PORTAL);
              save_before_layout_item_usesrelationship(child, portal);

              //Portal navigation details:
              bool navigation_specific_main = false;
              sharedptr<const UsesRelationship> relationship_navigation_specific = portal->get_navigation_relationship_specific(navigation_specific_main);
              if(navigation_specific_main || relationship_navigation_specific) //Avoid wasting a node if these are not even specified.
              {
                xmlpp::Element* child_navigation_relationship_specific = child->add_child(GLOM_NODE_DATA_LAYOUT_PORTAL_NAVIGATIONRELATIONSHIP);

                save_before_layout_item_usesrelationship(child_navigation_relationship_specific, relationship_navigation_specific);
                set_node_attribute_value_as_bool(child_navigation_relationship_specific, GLOM_ATTRIBUTE_PORTAL_NAVIGATIONRELATIONSHIP_MAIN, navigation_specific_main);
              }
            }
            else
            {
              sharedptr<const LayoutItem_Notebook> notebook = sharedptr<const LayoutItem_Notebook>::cast_dynamic(group);
              if(notebook) //If it is a related records portal
              {
                child = node->add_child(GLOM_NODE_DATA_LAYOUT_NOTEBOOK);
              }
              else if(group)
              {
                child = node->add_child(GLOM_NODE_DATA_LAYOUT_GROUP);
              }
            }
          }
        }
      }
    }
  }

  if(!child)
    return;

  child->set_attribute(GLOM_ATTRIBUTE_NAME, group->get_name());
  set_node_attribute_value_as_decimal(child, GLOM_ATTRIBUTE_COLUMNS_COUNT, group->m_columns_count);

  set_node_attribute_value_as_decimal(child, GLOM_ATTRIBUTE_SEQUENCE, group->m_sequence);

  set_node_attribute_value_as_decimal_double(child, GLOM_ATTRIBUTE_BORDER_WIDTH, group->get_border_width());

  //Translations:
  save_before_translations(child, *group);

  //Print layout position:
  if(with_print_layout_positions)
    save_before_print_layout_position(child, group);

  //Add the child items:
  LayoutGroup::type_map_const_items items = group->get_items();
  for(LayoutGroup::type_map_const_items::const_iterator iterItems = items.begin(); iterItems != items.end(); ++iterItems)
  {
    sharedptr<const LayoutItem> item = iterItems->second;
    //g_warning("save_before_layout_group: child part type=%s", item->get_part_type_name().c_str());

    sharedptr<const LayoutGroup> child_group = sharedptr<const LayoutGroup>::cast_dynamic(item);
    if(child_group) //If it is a group, portal, summary, or groupby.
    {
      //recurse:
      save_before_layout_group(child, child_group, with_print_layout_positions);
    }
    else
    {
      xmlpp::Element* nodeItem = 0;

      sharedptr<const LayoutItem_FieldSummary> fieldsummary = sharedptr<const LayoutItem_FieldSummary>::cast_dynamic(item);
      if(fieldsummary) //If it is a summaryfield
      {
        nodeItem = child->add_child(GLOM_NODE_DATA_LAYOUT_ITEM_FIELDSUMMARY);
        save_before_layout_item_field(nodeItem, fieldsummary);
        set_node_attribute_value(nodeItem, GLOM_ATTRIBUTE_LAYOUT_ITEM_FIELDSUMMARY_SUMMARYTYPE, fieldsummary->get_summary_type_sql()); //The SQL name is as good as anything as an identifier for the summary function.
      }
      else
      {
        sharedptr<const LayoutItem_Field> field = sharedptr<const LayoutItem_Field>::cast_dynamic(item);
        if(field) //If it is a field
        {
          nodeItem = child->add_child(GLOM_NODE_DATA_LAYOUT_ITEM);
          save_before_layout_item_field(nodeItem, field);
        }
        else
        {
          sharedptr<const LayoutItem_Button> button = sharedptr<const LayoutItem_Button>::cast_dynamic(item);
          if(button) //If it is a button
          {
            nodeItem = child->add_child(GLOM_NODE_DATA_LAYOUT_BUTTON);
            set_child_text_node(nodeItem, GLOM_NODE_BUTTON_SCRIPT, button->get_script());
            save_before_translations(nodeItem, *button);
          }
          else
          {
            sharedptr<const LayoutItem_Text> textobject = sharedptr<const LayoutItem_Text>::cast_dynamic(item);
            if(textobject) //If it is a text object.
            {
              nodeItem = child->add_child(GLOM_NODE_DATA_LAYOUT_TEXTOBJECT);
              save_before_translations(nodeItem, *textobject);

              //The text is translatable too, so we use a node for it:
              xmlpp::Element* element_text = nodeItem->add_child(GLOM_NODE_DATA_LAYOUT_TEXTOBJECT_TEXT);
              save_before_translations(element_text, *(textobject->m_text));
            }
            else
            {
              sharedptr<const LayoutItem_Image> imageobject = sharedptr<const LayoutItem_Image>::cast_dynamic(item);
              if(imageobject) //If it is an image object.
              {
                nodeItem = child->add_child(GLOM_NODE_DATA_LAYOUT_IMAGEOBJECT);
                save_before_translations(nodeItem, *imageobject);

                set_node_attribute_value_as_value(nodeItem, GLOM_ATTRIBUTE_DATA_LAYOUT_IMAGEOBJECT_IMAGE, imageobject->get_image(), Field::TYPE_IMAGE);
              }
            }
          }
        }
      }

      if(nodeItem)
      {
        set_node_attribute_value_as_decimal(nodeItem, GLOM_ATTRIBUTE_SEQUENCE, item->m_sequence);

        if(with_print_layout_positions)
          save_before_print_layout_position(nodeItem, item);
      }
    }

    //g_warning("save_before_layout_group: after child part type=%s", item->get_part_type_name().c_str());
  } 
}

void Document_Glom::save_before_translations(xmlpp::Element* element, const TranslatableItem& item)
{
  if(!element)
    return;

  set_node_attribute_value(element, GLOM_ATTRIBUTE_TITLE, item.get_title_original());


  if(!item.get_has_translations())
    return;

  xmlpp::Element* child = element->add_child(GLOM_NODE_TRANSLATIONS_SET);

  const TranslatableItem::type_map_locale_to_translations& map_translations = item._get_translations_map();
  for(TranslatableItem::type_map_locale_to_translations::const_iterator iter = map_translations.begin(); iter != map_translations.end(); ++iter)
  {
    xmlpp::Element* childItem = child->add_child(GLOM_NODE_TRANSLATION);
    set_node_attribute_value(childItem, GLOM_ATTRIBUTE_TRANSLATION_LOCALE, iter->first);
    set_node_attribute_value(childItem, GLOM_ATTRIBUTE_TRANSLATION_VALUE, iter->second);
  }
}

void Document_Glom::save_before_print_layout_position(xmlpp::Element* nodeItem, const sharedptr<const LayoutItem>& item)
{
  xmlpp::Element* child = nodeItem->add_child(GLOM_NODE_POSITION);
 
  double x = 0;
  double y = 0;
  double width = 0;
  double height = 0;
  item->get_print_layout_position(x, y, width, height);

  set_node_attribute_value_as_decimal_double(child, GLOM_ATTRIBUTE_POSITION_X, x);
  set_node_attribute_value_as_decimal_double(child, GLOM_ATTRIBUTE_POSITION_Y, y);
  set_node_attribute_value_as_decimal_double(child, GLOM_ATTRIBUTE_POSITION_WIDTH, width);
  set_node_attribute_value_as_decimal_double(child, GLOM_ATTRIBUTE_POSITION_HEIGHT, height);
}

bool Document_Glom::save_before()
{
  //std::cout << "debug: save_before(): uri=" << get_file_uri() << std::endl;
 
  Bakery::BusyCursor busy_cursor(m_parent_window);

  xmlpp::Element* nodeRoot = get_node_document();
  if(nodeRoot)
  {
    //Always save as the latest format,
    //possibly making it impossible to open this document in older versions of Glom:
    m_document_format_version = get_latest_known_document_format_version();
    set_node_attribute_value_as_decimal(nodeRoot, GLOM_ATTRIBUTE_FORMAT_VERSION, m_document_format_version);

    set_node_attribute_value_as_bool(nodeRoot, GLOM_ATTRIBUTE_IS_EXAMPLE, m_is_example);
    set_node_attribute_value(nodeRoot, GLOM_ATTRIBUTE_CONNECTION_DATABASE_TITLE, m_database_title);

    //Assume that the first language used is the original locale.
    //It can be identified as a translation later.
    if(m_translation_original_locale.empty())
      m_translation_original_locale = TranslatableItem::get_current_locale();

    set_node_attribute_value(nodeRoot, GLOM_ATTRIBUTE_TRANSLATION_ORIGINAL_LOCALE, m_translation_original_locale);

    xmlpp::Element* nodeConnection = get_node_child_named_with_add(nodeRoot, GLOM_NODE_CONNECTION);
#ifdef GLOM_ENABLE_CLIENT_ONLY
    set_node_attribute_value_as_bool(nodeConnection, GLOM_ATTRIBUTE_CONNECTION_SELF_HOSTED, false);
#else // GLOM_ENABLE_CLIENT_ONLY
    set_node_attribute_value_as_bool(nodeConnection, GLOM_ATTRIBUTE_CONNECTION_SELF_HOSTED, m_connection_is_self_hosted);
#endif // !GLOM_ENABLE_CLIENT_ONLY

    set_node_attribute_value(nodeConnection, GLOM_ATTRIBUTE_CONNECTION_SERVER, m_connection_server);
    set_node_attribute_value(nodeConnection, GLOM_ATTRIBUTE_CONNECTION_USER, m_connection_user);
    set_node_attribute_value(nodeConnection, GLOM_ATTRIBUTE_CONNECTION_DATABASE, m_connection_database);
    append_newline(nodeRoot);

    //Remove existing tables:
    xmlpp::Node::NodeList listNodes = nodeRoot->get_children(GLOM_NODE_TABLE);
    for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
      nodeRoot->remove_child(*iter);

    //Add tables:
    for(type_tables::const_iterator iter = m_tables.begin(); iter != m_tables.end(); ++iter)
    {
      const DocumentTableInfo& doctableinfo = iter->second;

      const Glib::ustring table_name = doctableinfo.m_info->get_name();
      if(table_name.empty())
        g_warning("Document_Glom::save_before(): table name is empty.");

      if(!table_name.empty())
      {
        xmlpp::Element* nodeTable = nodeRoot->add_child(GLOM_NODE_TABLE);
        set_node_attribute_value(nodeTable, GLOM_ATTRIBUTE_NAME, table_name);
        set_node_attribute_value_as_bool(nodeTable, GLOM_ATTRIBUTE_HIDDEN, doctableinfo.m_info->m_hidden);
        set_node_attribute_value_as_bool(nodeTable, GLOM_ATTRIBUTE_DEFAULT, doctableinfo.m_info->m_default);

        set_node_attribute_value_as_float(nodeTable, GLOM_ATTRIBUTE_OVERVIEW_X, doctableinfo.m_overviewx);
        set_node_attribute_value_as_float(nodeTable, GLOM_ATTRIBUTE_OVERVIEW_Y, doctableinfo.m_overviewy);
        
        if(m_is_example) //The example data is useless to non-example files (and is big):
	{
	  xmlpp::Element* nodeExampleRows = nodeTable->add_child(GLOM_NODE_EXAMPLE_ROWS);
          //set_child_text_node(nodeTable, GLOM_NODE_EXAMPLE_ROWS, doctableinfo.m_example_rows);
	  //TODO: This is merely copied from Base_DB::insert_example_data(). Instead, we should probably save the example rows in a more intelligent manner, like a list of vector of strings instead of just one big string.
          typedef std::vector<Glib::ustring> type_vecStrings; // TODO: Put this into class declaration?
	  const type_vecStrings vec_rows = Utils::string_separate(doctableinfo.m_example_rows, "\n", false);
	  for(type_vecStrings::const_iterator iter = vec_rows.begin(); iter != vec_rows.end(); ++iter)
	  {
	    xmlpp::Element* nodeExampleRow = nodeExampleRows->add_child(GLOM_NODE_EXAMPLE_ROW);
	    const Glib::ustring& row_data = *iter;
	    if(!row_data.empty())
	    {
	      const type_vecStrings vec_values = Utils::string_separate(row_data, ",", true /* ignore , inside quotes */);
	      for(unsigned int i = 0; i < vec_values.size(); ++i)
	      {
	        xmlpp::Element* nodeField = nodeExampleRow->add_child(GLOM_NODE_VALUE);
		nodeField->set_attribute(GLOM_ATTRIBUTE_COLUMN, doctableinfo.m_fields[i]->get_name());
		nodeField->add_child_text(vec_values[i]);
	      } // for each value
	    } // !row_data.empty
	  } // for each row
        } // m_is_example
        //else
          //TODO: doctableinfo.m_example_rows.clear(); //Make sure we are not keeping this in memory unnecessarily.

        //Translations:
        save_before_translations(nodeTable, *(doctableinfo.m_info));

        //Fields:
        xmlpp::Element* elemFields = nodeTable->add_child(GLOM_NODE_FIELDS);

        const Field::type_map_type_names type_names = Field::get_type_names();

        for(type_vecFields::const_iterator iter = doctableinfo.m_fields.begin(); iter != doctableinfo.m_fields.end(); ++iter)
        {
          sharedptr<const Field> field = *iter;

          xmlpp::Element* elemField = elemFields->add_child(GLOM_NODE_FIELD);
          set_node_attribute_value(elemField, GLOM_ATTRIBUTE_NAME, field->get_name());

          set_node_attribute_value_as_bool(elemField, GLOM_ATTRIBUTE_PRIMARY_KEY, field->get_primary_key());
          set_node_attribute_value_as_bool(elemField, GLOM_ATTRIBUTE_UNIQUE, field->get_unique_key());
          set_node_attribute_value_as_bool(elemField, GLOM_ATTRIBUTE_AUTOINCREMENT, field->get_auto_increment());
          set_node_attribute_value_as_value(elemField, GLOM_ATTRIBUTE_DEFAULT_VALUE, field->get_default_value(), field->get_glom_type());

          set_child_text_node(elemField, GLOM_NODE_CALCULATION, field->get_calculation());
         
          Glib::ustring field_type;
          Field::type_map_type_names::const_iterator iterTypes = type_names.find( field->get_glom_type() );
          if(iterTypes != type_names.end())
            field_type = iterTypes->second;

          set_node_attribute_value(elemField, GLOM_ATTRIBUTE_TYPE, field_type);

          //Add Lookup sub-node:
          if(field->get_is_lookup())
          {
            xmlpp::Element* elemFieldLookup = elemField->add_child(GLOM_NODE_FIELD_LOOKUP);

            sharedptr<Relationship> lookup_relationship = field->get_lookup_relationship();
            set_node_attribute_value(elemFieldLookup, GLOM_ATTRIBUTE_RELATIONSHIP_NAME, glom_get_sharedptr_name(lookup_relationship));

            set_node_attribute_value(elemFieldLookup, GLOM_ATTRIBUTE_FIELD, field->get_lookup_field());
          }

          //Default Formatting:
          xmlpp::Element* elementFormat = elemField->add_child(GLOM_NODE_FORMAT);
          save_before_layout_item_field_formatting(elementFormat, field->m_default_formatting, field->get_glom_type());

          //Translations:
          save_before_translations(elemField, *field);

          append_newline(elemFields);
        } /* fields */

        append_newline(nodeTable);

        //Relationships:
        //Add new <relationships> node:
        xmlpp::Element* elemRelationships = nodeTable->add_child(GLOM_NODE_RELATIONSHIPS);

        //Add each <relationship> node:
        for(type_vecRelationships::const_iterator iter = doctableinfo.m_relationships.begin(); iter != doctableinfo.m_relationships.end(); iter++)
        {
          sharedptr<const Relationship> relationship = *iter;
          if(relationship)
          {
            xmlpp::Element* elemRelationship = elemRelationships->add_child(GLOM_NODE_RELATIONSHIP);
            set_node_attribute_value(elemRelationship, GLOM_ATTRIBUTE_NAME, relationship->get_name());
            set_node_attribute_value(elemRelationship, GLOM_ATTRIBUTE_KEY, relationship->get_from_field());
            set_node_attribute_value(elemRelationship, GLOM_ATTRIBUTE_OTHER_TABLE, relationship->get_to_table());
            set_node_attribute_value(elemRelationship, GLOM_ATTRIBUTE_OTHER_KEY, relationship->get_to_field());
            set_node_attribute_value_as_bool(elemRelationship, GLOM_ATTRIBUTE_AUTO_CREATE, relationship->get_auto_create());
            set_node_attribute_value_as_bool(elemRelationship, GLOM_ATTRIBUTE_ALLOW_EDIT, relationship->get_allow_edit());

            //Translations:
            save_before_translations(elemRelationship, *relationship);

            append_newline(elemRelationships);
          }
        }

        append_newline(nodeTable);


        //Layouts:
        xmlpp::Element* nodeDataLayouts = nodeTable->add_child(GLOM_NODE_DATA_LAYOUTS);

        //Add the groups:
        //Make sure that we always get these _after_ the relationships.
        for(DocumentTableInfo::type_layouts::const_iterator iter = doctableinfo.m_layouts.begin(); iter != doctableinfo.m_layouts.end(); ++iter)
        {
          xmlpp::Element* nodeLayout = nodeDataLayouts->add_child(GLOM_NODE_DATA_LAYOUT);
          nodeLayout->set_attribute(GLOM_ATTRIBUTE_NAME, iter->m_layout_name);
          nodeLayout->set_attribute(GLOM_ATTRIBUTE_PARENT_TABLE_NAME, iter->m_parent_table);

          xmlpp::Element* nodeGroups = nodeLayout->add_child(GLOM_NODE_DATA_LAYOUT_GROUPS);

          const type_mapLayoutGroupSequence& group_sequence = iter->m_layout_groups;
          for(type_mapLayoutGroupSequence::const_iterator iterGroups = group_sequence.begin(); iterGroups != group_sequence.end(); ++iterGroups)
          {
            save_before_layout_group(nodeGroups, iterGroups->second);
            append_newline(nodeGroups);
          }

          append_newline(nodeDataLayouts);
        }

        append_newline(nodeTable);

        //Reports:
        xmlpp::Element* nodeReports = nodeTable->add_child(GLOM_NODE_REPORTS);

        //Add the groups:
        for(DocumentTableInfo::type_reports::const_iterator iter = doctableinfo.m_reports.begin(); iter != doctableinfo.m_reports.end(); ++iter)
        {
          xmlpp::Element* nodeReport = nodeReports->add_child(GLOM_NODE_REPORT);

          sharedptr<const Report> report = iter->second;
          nodeReport->set_attribute(GLOM_ATTRIBUTE_NAME, report->get_name());
          set_node_attribute_value_as_bool(nodeReport, GLOM_ATTRIBUTE_REPORT_SHOW_TABLE_TITLE, report->get_show_table_title());

          xmlpp::Element* nodeGroups = nodeReport->add_child(GLOM_NODE_DATA_LAYOUT_GROUPS);
          if(report->m_layout_group)
          {
            save_before_layout_group(nodeGroups, report->m_layout_group);
            append_newline(nodeGroups);
          }

          //Translations:
          save_before_translations(nodeReport, *report);

          append_newline(nodeReports);
          append_newline(nodeReports);
        }

        //Print Layouts:
        xmlpp::Element* nodePrintLayouts = nodeTable->add_child(GLOM_NODE_PRINT_LAYOUTS);

        //Add the groups:
        for(DocumentTableInfo::type_print_layouts::const_iterator iter = doctableinfo.m_print_layouts.begin(); iter != doctableinfo.m_print_layouts.end(); ++iter)
        {
          xmlpp::Element* nodePrintLayout = nodePrintLayouts->add_child(GLOM_NODE_PRINT_LAYOUT);

          sharedptr<const PrintLayout> print_layout = iter->second;
          nodePrintLayout->set_attribute(GLOM_ATTRIBUTE_NAME, print_layout->get_name());
          set_node_attribute_value_as_bool(nodePrintLayout, GLOM_ATTRIBUTE_REPORT_SHOW_TABLE_TITLE, print_layout->get_show_table_title());

          //Page Setup:
          Glib::RefPtr<const Gtk::PageSetup> page_setup =  print_layout->get_page_setup();
          if(page_setup)
          {
            Glib::KeyFile key_file;
            Glib::RefPtr<Gtk::PageSetup> unconst = Glib::RefPtr<Gtk::PageSetup>::cast_const(page_setup); //TODO: Remove this when using gtkmm 2.13/14.
            unconst->save_to_key_file(key_file);

            xmlpp::Element* child = nodePrintLayout->add_child(GLOM_NODE_PAGE_SETUP);
            child->add_child_text(key_file.to_data());
          }

          xmlpp::Element* nodeGroups = nodePrintLayout->add_child(GLOM_NODE_DATA_LAYOUT_GROUPS);
          if(print_layout->m_layout_group)
          {
            save_before_layout_group(nodeGroups, print_layout->m_layout_group, true /* x,y positions too. */);
            append_newline(nodeGroups);
          }

          //Translations:
          save_before_translations(nodePrintLayout, *print_layout);

          append_newline(nodePrintLayout);
        }

        append_newline(nodeTable);
        append_newline(nodeTable);
      }

      append_newline(nodeRoot);

    } //for m_tables


    //Remove existing groups:
    listNodes = nodeRoot->get_children(GLOM_NODE_GROUPS);
    for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
      nodeRoot->remove_child(*iter);

    //Add groups:
    xmlpp::Element* nodeGroups = nodeRoot->add_child(GLOM_NODE_GROUPS);

    nodeGroups->add_child_comment("These are only used when recreating a database from an example file. The actual access-control is on the server, of course.");

    for(type_map_groups::const_iterator iter = m_groups.begin(); iter != m_groups.end(); ++iter)
    {
      const GroupInfo& group_info = iter->second;

      xmlpp::Element* nodeGroup = nodeGroups->add_child(GLOM_NODE_GROUP);
      nodeGroup->set_attribute(GLOM_ATTRIBUTE_NAME, group_info.get_name());
      set_node_attribute_value_as_bool(nodeGroup, GLOM_ATTRIBUTE_DEVELOPER, group_info.m_developer);

      //The privileges for each table, for this group:
      for(GroupInfo::type_map_table_privileges::const_iterator iter = group_info.m_map_privileges.begin(); iter != group_info.m_map_privileges.end(); ++iter)
      {
        xmlpp::Element* nodeTablePrivs = nodeGroups->add_child(GLOM_NODE_TABLE_PRIVS);

        set_node_attribute_value(nodeTablePrivs, GLOM_ATTRIBUTE_TABLE_NAME, iter->first);

        const Privileges& privs = iter->second;
        set_node_attribute_value_as_bool(nodeTablePrivs, GLOM_ATTRIBUTE_PRIV_VIEW, privs.m_view);
        set_node_attribute_value_as_bool(nodeTablePrivs, GLOM_ATTRIBUTE_PRIV_EDIT, privs.m_edit);
        set_node_attribute_value_as_bool(nodeTablePrivs, GLOM_ATTRIBUTE_PRIV_CREATE, privs.m_create);
        set_node_attribute_value_as_bool(nodeTablePrivs, GLOM_ATTRIBUTE_PRIV_DELETE, privs.m_delete);
      }
    }

    append_newline(nodeRoot);


    //Remove existing library modules::
    listNodes = nodeRoot->get_children(GLOM_NODE_LIBRARY_MODULES);
    for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
      nodeRoot->remove_child(*iter);

    //Add groups:
    xmlpp::Element* nodeModules = nodeRoot->add_child(GLOM_NODE_LIBRARY_MODULES);

    for(type_map_library_scripts::const_iterator iter = m_map_library_scripts.begin(); iter != m_map_library_scripts.end(); ++iter)
    {
      const Glib::ustring& name = iter->first;
      const Glib::ustring& script = iter->second;

      xmlpp::Element* nodeModule = nodeModules->add_child(GLOM_NODE_LIBRARY_MODULE);

      set_node_attribute_value(nodeModule, GLOM_ATTRIBUTE_LIBRARY_MODULE_NAME, name);
      set_node_attribute_value(nodeModule, GLOM_ATTRIBUTE_LIBRARY_MODULE_SCRIPT, script);
    }

    append_newline(nodeRoot);

  }

  return Bakery::Document_XML::save_before();  
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

Glib::ustring Document_Glom::get_database_title() const
{
  return m_database_title;
}

void Document_Glom::set_database_title(const Glib::ustring& title)
{
  if(m_database_title != title)
  {
    m_database_title = title;
    set_modified();
  }
}

Glib::ustring Document_Glom::get_name() const
{
  //Show the database title in the window title bar:
  if(m_database_title.empty())
    return Bakery::Document_XML::get_name();
  else
    return m_database_title;
}

Document_Glom::type_list_groups Document_Glom::get_groups() const
{
  type_list_groups result;
  for(type_map_groups::const_iterator iter = m_groups.begin(); iter != m_groups.end(); ++iter)
  {
    result.push_back(iter->second);
  }

  return result;
}

///This adds the group if necessary.
void Document_Glom::set_group(GroupInfo& group)
{
  const Glib::ustring name = group.get_name();
  type_map_groups::iterator iter = m_groups.find(name);
  if(iter == m_groups.end())
  {
    //Add it if necesary:
    m_groups[name] = group;
    set_modified();
  }
  else
  {
    const GroupInfo this_group = iter->second;
    if(this_group != group)
    {
      iter->second = group;
      set_modified();
    }
  }
}

void Document_Glom::remove_group(const Glib::ustring& group_name)
{
  type_map_groups::iterator iter = m_groups.find(group_name);
  if(iter != m_groups.end())
  {
    m_groups.erase(iter);
    set_modified();
  }
}

Document_Glom::type_listReports Document_Glom::get_report_names(const Glib::ustring& table_name) const
{
  type_tables::const_iterator iterFind = m_tables.find(table_name);
  if(iterFind != m_tables.end())
  {
    type_listReports result;
    for(DocumentTableInfo::type_reports::const_iterator iter = iterFind->second.m_reports.begin(); iter != iterFind->second.m_reports.end(); ++iter)
    {
      result.push_back(iter->second->get_name());
    }

    return result;
  }
  else
    return type_listReports(); 
}

void Document_Glom::remove_all_reports(const Glib::ustring& table_name)
{
  type_tables::iterator iterFind = m_tables.find(table_name);
  if(iterFind != m_tables.end())
  {
    iterFind->second.m_reports.clear();
    set_modified();
  }
}

void Document_Glom::set_report(const Glib::ustring& table_name, const sharedptr<Report>& report)
{
  type_tables::iterator iterFind = m_tables.find(table_name);
  if(iterFind != m_tables.end())
  {
    iterFind->second.m_reports[report->get_name()] = report;
    set_modified();
  }
}

sharedptr<Report> Document_Glom::get_report(const Glib::ustring& table_name, const Glib::ustring& report_name) const
{
  type_tables::const_iterator iterFindTable = m_tables.find(table_name);
  if(iterFindTable != m_tables.end())
  {
    DocumentTableInfo::type_reports::const_iterator iterFindReport = iterFindTable->second.m_reports.find(report_name);
    if(iterFindReport != iterFindTable->second.m_reports.end())
    {
      return iterFindReport->second;
    }
  }

  return sharedptr<Report>();
}

void Document_Glom::remove_report(const Glib::ustring& table_name, const Glib::ustring& report_name)
{
  type_tables::iterator iterFindTable = m_tables.find(table_name);
  if(iterFindTable != m_tables.end())
  {
    DocumentTableInfo::type_reports::iterator iterFindReport = iterFindTable->second.m_reports.find(report_name);
    if(iterFindReport != iterFindTable->second.m_reports.end())
    {
      iterFindTable->second.m_reports.erase(iterFindReport);

      set_modified();
    }
  }
}


Document_Glom::type_listPrintLayouts Document_Glom::get_print_layout_names(const Glib::ustring& table_name) const
{
  type_tables::const_iterator iterFind = m_tables.find(table_name);
  if(iterFind != m_tables.end())
  {
    type_listReports result;
    for(DocumentTableInfo::type_print_layouts::const_iterator iter = iterFind->second.m_print_layouts.begin(); iter != iterFind->second.m_print_layouts.end(); ++iter)
    {
      result.push_back(iter->second->get_name());
    }

    return result;
  }
  else
    return type_listReports(); 
}

void Document_Glom::remove_all_print_layouts(const Glib::ustring& table_name)
{
  type_tables::iterator iterFind = m_tables.find(table_name);
  if(iterFind != m_tables.end())
  {
    iterFind->second.m_print_layouts.clear();
    set_modified();
  }
}

void Document_Glom::set_print_layout(const Glib::ustring& table_name, const sharedptr<PrintLayout>& print_layout)
{
  type_tables::iterator iterFind = m_tables.find(table_name);
  if(iterFind != m_tables.end())
  {
    iterFind->second.m_print_layouts[print_layout->get_name()] = print_layout;
    set_modified();
  }
}

sharedptr<PrintLayout> Document_Glom::get_print_layout(const Glib::ustring& table_name, const Glib::ustring& print_layout_name) const
{
  type_tables::const_iterator iterFindTable = m_tables.find(table_name);
  if(iterFindTable != m_tables.end())
  {
    DocumentTableInfo::type_print_layouts::const_iterator iterFindPrintLayout = iterFindTable->second.m_print_layouts.find(print_layout_name);
    if(iterFindPrintLayout != iterFindTable->second.m_print_layouts.end())
    {
      return iterFindPrintLayout->second;
    }
  }

  return sharedptr<PrintLayout>();
}

void Document_Glom::remove_print_layout(const Glib::ustring& table_name, const Glib::ustring& print_layout_name)
{
  type_tables::iterator iterFindTable = m_tables.find(table_name);
  if(iterFindTable != m_tables.end())
  {
    DocumentTableInfo::type_print_layouts::iterator iterFindPrintLayout = iterFindTable->second.m_print_layouts.find(print_layout_name);
    if(iterFindPrintLayout != iterFindTable->second.m_print_layouts.end())
    {
      iterFindTable->second.m_print_layouts.erase(iterFindPrintLayout);

      set_modified();
    }
  }
}



bool Document_Glom::get_relationship_is_to_one(const Glib::ustring& table_name, const Glib::ustring& relationship_name) const
{
  sharedptr<const Relationship> relationship = get_relationship(table_name, relationship_name);
  if(relationship)
  {
    sharedptr<const Field> field_to = get_field(relationship->get_to_table(), relationship->get_to_field());
    if(field_to)
      return (field_to->get_primary_key() || field_to->get_unique_key());
  }

  return false;
}

sharedptr<Relationship> Document_Glom::get_field_used_in_relationship_to_one(const Glib::ustring& table_name, const Glib::ustring& field_name) const
{
  sharedptr<Relationship> result;

  type_tables::const_iterator iterFind = m_tables.find(table_name);
  if(iterFind != m_tables.end())
  {
    //Look at each relationship:
    for(type_vecRelationships::const_iterator iterRel = iterFind->second.m_relationships.begin(); iterRel != iterFind->second.m_relationships.end(); ++iterRel)
    {
      sharedptr<Relationship> relationship = *iterRel;
      if(relationship)
      {
        //If the relationship uses the field
        if(relationship->get_from_field() == field_name)
        {
          //if the to_table is not hidden:
          if(!get_table_is_hidden(relationship->get_to_table()))
          {
            //TODO_Performance: The use of this convenience method means we get the full relationship information again:
            if(get_relationship_is_to_one(table_name, relationship->get_name()))
            {
              result = relationship;
            }
          }
        }
      }
    }
  }

  return result;
}

void Document_Glom::forget_layout_record_viewed(const Glib::ustring& table_name)
{
  type_tables::iterator iterFind = m_tables.find(table_name);
  if(iterFind != m_tables.end())
  {
    iterFind->second.m_map_current_record.clear();
  }
}

void Document_Glom::set_layout_record_viewed(const Glib::ustring& table_name, const Glib::ustring& layout_name, const Gnome::Gda::Value& primary_key_value)
{
  type_tables::iterator iterFind = m_tables.find(table_name);
  if(iterFind != m_tables.end())
  {
    iterFind->second.m_map_current_record[layout_name] = primary_key_value;
  }
}

Gnome::Gda::Value Document_Glom::get_layout_record_viewed(const Glib::ustring& table_name, const Glib::ustring& layout_name) const
{
  type_tables::const_iterator iterFind = m_tables.find(table_name);
  if(iterFind != m_tables.end())
  {
    const DocumentTableInfo& info = iterFind->second;
    DocumentTableInfo::type_map_layout_primarykeys::const_iterator iterLayoutKeys = info.m_map_current_record.find(layout_name);
    if(iterLayoutKeys != info.m_map_current_record.end())
      return iterLayoutKeys->second;
  }

  return Gnome::Gda::Value(); //not found.
}

void Document_Glom::set_layout_current(const Glib::ustring& table_name, const Glib::ustring& layout_name)
{
  type_tables::iterator iterFind = m_tables.find(table_name);
  if(iterFind != m_tables.end())
  {
    iterFind->second.m_layout_current = layout_name;
  }
}

Glib::ustring Document_Glom::get_layout_current(const Glib::ustring& table_name) const
{
  type_tables::const_iterator iterFind = m_tables.find(table_name);
  if(iterFind != m_tables.end())
  {
    return iterFind->second.m_layout_current;
  }

  return Glib::ustring(); //not found.
}

bool Document_Glom::get_is_example_file() const
{
  return m_is_example;
}

void Document_Glom::set_is_example_file(bool value)
{
  if(m_is_example != value)
  {
    m_is_example = value;
    set_modified();
  }
}


void Document_Glom::set_translation_original_locale(const Glib::ustring& locale)
{
  m_translation_original_locale = locale;
  TranslatableItem::set_original_locale(m_translation_original_locale);
  set_modified();
}


Glib::ustring Document_Glom::get_translation_original_locale() const
{
  return m_translation_original_locale;
}

Document_Glom::type_list_translatables Document_Glom::get_translatable_layout_items(const Glib::ustring& table_name)
{
  type_list_translatables result;

  type_tables::iterator iterFindTable = m_tables.find(table_name);
  if(iterFindTable != m_tables.end())
  {
    //Look at each layout:
    for(DocumentTableInfo::type_layouts::iterator iterLayouts = iterFindTable->second.m_layouts.begin(); iterLayouts != iterFindTable->second.m_layouts.end(); ++iterLayouts)
    {
      //Look at each group:
      for(type_mapLayoutGroupSequence::iterator iterGroup = iterLayouts->m_layout_groups.begin(); iterGroup != iterLayouts->m_layout_groups.end(); ++iterGroup)
      {
        sharedptr<LayoutGroup> group = iterGroup->second;
        if(group)
        {
          fill_translatable_layout_items(group, result);
        }
      }
    }
  }

  return result;
}


Document_Glom::type_list_translatables Document_Glom::get_translatable_report_items(const Glib::ustring& table_name, const Glib::ustring& report_title)
{
  Document_Glom::type_list_translatables the_list;

  sharedptr<Report> report = get_report(table_name, report_title);
  if(report)
    fill_translatable_layout_items(report->m_layout_group, the_list);

  return the_list;
}

void Document_Glom::fill_translatable_layout_items(const sharedptr<LayoutGroup>& group, type_list_translatables& the_list)
{
  the_list.push_back(group);

  //Look at each item:
  LayoutGroup::type_map_items items = group->get_items();
  for(LayoutGroup::type_map_items::const_iterator iterItems = items.begin(); iterItems != items.end(); ++iterItems)
  {
    sharedptr<LayoutItem> item = iterItems->second;

    sharedptr<LayoutGroup> child_group = sharedptr<LayoutGroup>::cast_dynamic(item);
    if(child_group) //If it is a group, portal, summary, or groupby.
    {
      sharedptr<LayoutItem_GroupBy> group_by = sharedptr<LayoutItem_GroupBy>::cast_dynamic(child_group);
      if(group_by)
      {
        sharedptr<LayoutItem_Field> field = group_by->get_field_group_by();
        sharedptr<CustomTitle> custom_title = field->get_title_custom();
        if(custom_title)
        {
          the_list.push_back(custom_title);
        }

        fill_translatable_layout_items(group_by->m_group_secondary_fields, the_list);
      }

      //recurse:
      fill_translatable_layout_items(child_group, the_list);
    }
    else
    {
      //Buttons too:
      sharedptr<LayoutItem_Button> button = sharedptr<LayoutItem_Button>::cast_dynamic(item);
      if(button)
        the_list.push_back(button);
      else
      {
        sharedptr<LayoutItem_Field> layout_field = sharedptr<LayoutItem_Field>::cast_dynamic(item);
        if(layout_field)
        {
          sharedptr<CustomTitle> custom_title = layout_field->get_title_custom();
          if(custom_title)
          {
            the_list.push_back(custom_title);
          }
        }
      }
    }
  }
}

void Document_Glom::set_parent_window(Gtk::Window* window)
{
  m_parent_window = window;
}

void Document_Glom::set_file_uri(const Glib::ustring& file_uri, bool bEnforceFileExtension /* = false */)
{
  //We override this because set_modified() triggers a save (to the old filename) in this derived class.

  //I'm not sure why this is in the base class's method anyway. murrayc:
  //if(file_uri != m_file_uri)
  //  set_modified(); //Ready to save() for a Save As.

  m_file_uri = file_uri;

  //Enforce file extension:
  if(bEnforceFileExtension)
    m_file_uri = get_file_uri_with_extension(m_file_uri);

  //Put this here instead. In the base class it's at the start:
  if(file_uri != m_file_uri)
    set_modified(); //Ready to save() for a Save As.
}

guint Document_Glom::get_document_format_version()
{
  return m_document_format_version;
}

guint Document_Glom::get_latest_known_document_format_version()
{
  // History:
  // Version 0: The first document format. (And the default version number when no version number was saved in the .XML)
  // Version 1: Saved scripts and other multiline text in text nodes instead of attributes. Can open Version 1 documents.

  return 1;
}

std::vector<Glib::ustring> Document_Glom::get_library_module_names() const
{
  std::vector<Glib::ustring> result;
  for(type_map_library_scripts::const_iterator iter = m_map_library_scripts.begin(); iter != m_map_library_scripts.end(); ++iter)
  {
    result.push_back(iter->first);
  }

  return result;
}

void Document_Glom::set_library_module(const Glib::ustring& name, const Glib::ustring& script)
{
  if(name.empty())
    return;

  type_map_library_scripts::iterator iter = m_map_library_scripts.find(name);
  if(iter != m_map_library_scripts.end())
  {
    //Change the existing script, if necessary:
    if(iter->second != script)
    {
      iter->second = script;
      set_modified();
    }
  }
  else
  {
    //Add the script:
    m_map_library_scripts[name] = script;
    set_modified();
  }
}

Glib::ustring Document_Glom::get_library_module(const Glib::ustring& name) const
{
  type_map_library_scripts::const_iterator iter = m_map_library_scripts.find(name);
  if(iter != m_map_library_scripts.end())
  {
    return iter->second;
  }

  return Glib::ustring();
}

void Document_Glom::remove_library_module(const Glib::ustring& name)
{
  type_map_library_scripts::iterator iter = m_map_library_scripts.find(name);
  if(iter != m_map_library_scripts.end())
  {
     m_map_library_scripts.erase(iter);
     set_modified();
  }
}



} //namespace Glom

