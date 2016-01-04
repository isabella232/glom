/* Glom
 *
 * Copyright (C) 2001-2013 Openismus GmbH
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

#include <libglom/document/document.h>
#include <libglom/xml_utils.h>
#include <libglom/utils.h>
//#include <libglom/data_structure/glomconversions.h>
#include <libglom/data_structure/layout/report_parts/layoutitem_summary.h>
#include <libglom/data_structure/layout/report_parts/layoutitem_fieldsummary.h>
#include <libglom/data_structure/layout/report_parts/layoutitem_verticalgroup.h>
#include <libglom/data_structure/layout/report_parts/layoutitem_header.h>
#include <libglom/data_structure/layout/report_parts/layoutitem_footer.h>
#include <libglom/data_structure/layout/layoutitem_button.h>
#include <libglom/data_structure/layout/layoutitem_text.h>
#include <libglom/data_structure/layout/layoutitem_image.h>
#include <libglom/data_structure/layout/layoutitem_calendarportal.h>
#include <libglom/data_structure/layout/layoutitem_line.h>
#include <libglom/standard_table_prefs_fields.h>
#include <libglom/spawn_with_feedback.h>
#include <libglom/translations_po.h>
#include <giomm/file.h>
#include <glibmm/miscutils.h>
#include <glibmm/fileutils.h>
#include <glibmm/convert.h>

#include <libglom/connectionpool.h>

//libarchive:
#include <archive.h>
#include <archive_entry.h>

#include <glibmm/i18n.h>
//#include <libglom/libglom_config.h> //To get GLOM_DTD_INSTALL_DIR - dependent on configure prefix.
#include <algorithm> //For std::find_if().
#include <sstream> //For stringstream
#include <iostream>

namespace Glom
{

static const char GLOM_NODE_CONNECTION[] = "connection";
static const char GLOM_ATTRIBUTE_CONNECTION_SELF_HOSTED[] = "self_hosted"; //deprecated.
static const char GLOM_ATTRIBUTE_CONNECTION_HOSTING_MODE[] = "hosting_mode";
static const char GLOM_ATTRIBUTE_CONNECTION_HOSTING_POSTGRES_CENTRAL[] = "postgres_central";
static const char GLOM_ATTRIBUTE_CONNECTION_HOSTING_POSTGRES_SELF[] = "postgres_self";
static const char GLOM_ATTRIBUTE_CONNECTION_HOSTING_SQLITE[] = "sqlite";
static const char GLOM_ATTRIBUTE_CONNECTION_HOSTING_MYSQL_CENTRAL[] = "mysql_central";
static const char GLOM_ATTRIBUTE_CONNECTION_HOSTING_MYSQL_SELF[] = "mysql_self";
static const char GLOM_ATTRIBUTE_CONNECTION_NETWORK_SHARED[] = "network_shared";
static const char GLOM_ATTRIBUTE_CONNECTION_SERVER[] = "server";
static const char GLOM_ATTRIBUTE_CONNECTION_PORT[] = "port";
static const char GLOM_ATTRIBUTE_CONNECTION_TRY_OTHER_PORTS[] = "try_other_ports";
static const char GLOM_ATTRIBUTE_CONNECTION_USER[] = "user";
static const char GLOM_ATTRIBUTE_CONNECTION_DATABASE[] = "database";

static const char GLOM_NODE_DATA_LAYOUT_GROUPS[] = "data_layout_groups";
static const char GLOM_NODE_DATA_LAYOUT_GROUP[] = "data_layout_group";
static const char GLOM_ATTRIBUTE_COLUMNS_COUNT[] = "columns_count";
static const char GLOM_ATTRIBUTE_BORDER_WIDTH[] = "border_width";

static const char GLOM_NODE_DATA_LAYOUTS[] = "data_layouts";
static const char GLOM_NODE_DATA_LAYOUT[] = "data_layout";
static const char GLOM_ATTRIBUTE_LAYOUT_PLATFORM[] = "platform";

static const char GLOM_NODE_DATA_LAYOUT_NOTEBOOK[] = "data_layout_notebook";

static const char GLOM_NODE_DATA_LAYOUT_PORTAL[] = "data_layout_portal";
static const char GLOM_NODE_DATA_LAYOUT_PORTAL_NAVIGATIONRELATIONSHIP[] = "portal_navigation_relationship";
static const char GLOM_ATTRIBUTE_PORTAL_ROWS_COUNT_MIN[] = "portal_rows_count_min";
static const char GLOM_ATTRIBUTE_PORTAL_ROWS_COUNT_MAX[] = "portal_rows_count_max";
static const char GLOM_ATTRIBUTE_PORTAL_NAVIGATION_TYPE[] = "navigation_type";
static const char GLOM_ATTRIBUTE_PORTAL_NAVIGATION_TYPE_AUTOMATIC[] = "automatic";
static const char GLOM_ATTRIBUTE_PORTAL_NAVIGATION_TYPE_SPECIFIC[] = "specific";
static const char GLOM_ATTRIBUTE_PORTAL_NAVIGATION_TYPE_NONE[] = "none";

static const char GLOM_NODE_DATA_LAYOUT_CALENDAR_PORTAL[] = "data_layout_calendar_portal";
static const char GLOM_ATTRIBUTE_PORTAL_PRINT_LAYOUT_ROW_HEIGHT[] = "row_height";
static const char GLOM_ATTRIBUTE_PORTAL_PRINT_LAYOUT_ROW_LINE_WIDTH[] = "row_line_width";
static const char GLOM_ATTRIBUTE_PORTAL_PRINT_LAYOUT_ROW_LINE_COLOR[] = "column_line_color";
static const char GLOM_ATTRIBUTE_PORTAL_PRINT_LAYOUT_COLUMN_LINE_WIDTH[] = "row_line_width";
static const char GLOM_ATTRIBUTE_PORTAL_PRINT_LAYOUT_LINE_COLOR[] = "line_color";
static const char GLOM_ATTRIBUTE_PORTAL_CALENDAR_DATE_FIELD[] = "date_field";
static const char GLOM_NODE_DATA_LAYOUT_ITEM_FIELD[] = "data_layout_item"; //A field.
static const char GLOM_NODE_LAYOUT_ITEM_CUSTOM_TITLE[] = "title_custom";
static const char GLOM_NODE_TABLE_TITLE_SINGULAR[] = "title_singular"; //such as "Customer" instead of "Customers".
static const char GLOM_ATTRIBUTE_LAYOUT_ITEM_CUSTOM_TITLE_USE[] = "use_custom";
static const char GLOM_ATTRIBUTE_LAYOUT_ITEM_COLUMN_WIDTH[] = "column_width";
static const char GLOM_NODE_DATA_LAYOUT_BUTTON[] = "data_layout_button";
static const char GLOM_NODE_DATA_LAYOUT_TEXTOBJECT[] = "data_layout_text";
static const char GLOM_NODE_DATA_LAYOUT_TEXTOBJECT_TEXT[] = "text";
static const char GLOM_NODE_DATA_LAYOUT_IMAGEOBJECT[] = "data_layout_image";
static const char GLOM_ATTRIBUTE_DATA_LAYOUT_IMAGEOBJECT_IMAGE[] = "text"; //Was gda-formatted image data. Deprecated in favour of a child text node containing base64-formatted image data.
static const char GLOM_NODE_DATA_LAYOUT_LINE[] = "data_layout_line";
static const char GLOM_ATTRIBUTE_DATA_LAYOUT_LINE_START_X[] = "start_x";
static const char GLOM_ATTRIBUTE_DATA_LAYOUT_LINE_START_Y[] = "start_y";
static const char GLOM_ATTRIBUTE_DATA_LAYOUT_LINE_END_X[] = "end_x";
static const char GLOM_ATTRIBUTE_DATA_LAYOUT_LINE_END_Y[] = "end_y";
static const char GLOM_ATTRIBUTE_DATA_LAYOUT_LINE_WIDTH[] = "line_width";
static const char GLOM_ATTRIBUTE_DATA_LAYOUT_LINE_COLOR[] = "color";
static const char GLOM_ATTRIBUTE_DATA_LAYOUT_ITEM_FIELD_USE_DEFAULT_FORMATTING[] = "use_default_formatting";
static const char GLOM_NODE_DATA_LAYOUT_ITEM_GROUPBY[] = "data_layout_item_groupby";
static const char GLOM_NODE_DATA_LAYOUT_GROUP_SECONDARYFIELDS[] = "secondary_fields";
static const char GLOM_NODE_DATA_LAYOUT_ITEM_VERTICALGROUP[] = "data_layout_item_verticalgroup";
static const char GLOM_NODE_DATA_LAYOUT_ITEM_SUMMARY[] = "data_layout_item_summary";
static const char GLOM_NODE_DATA_LAYOUT_ITEM_FIELDSUMMARY[] = "data_layout_item_fieldsummary";
static const char GLOM_NODE_DATA_LAYOUT_ITEM_HEADER[] = "data_layout_item_header";
static const char GLOM_NODE_DATA_LAYOUT_ITEM_FOOTER[] = "data_layout_item_footer";
static const char GLOM_NODE_TABLE[] = "table";
static const char GLOM_NODE_FIELDS[] = "fields";
static const char GLOM_NODE_FIELD[] = "field";
static const char GLOM_ATTRIBUTE_PRIMARY_KEY[] = "primary_key";
static const char GLOM_ATTRIBUTE_DEFAULT_VALUE[] = "default_value";
static const char GLOM_ATTRIBUTE_UNIQUE[] = "unique";
static const char GLOM_ATTRIBUTE_AUTOINCREMENT[] = "auto_increment";
static const char GLOM_DEPRECATED_ATTRIBUTE_CALCULATION[] = "calculation";
static const char GLOM_NODE_CALCULATION[] = "calculation";
static const char GLOM_ATTRIBUTE_TYPE[] = "type";

static const char GLOM_NODE_FIELD_LOOKUP[] = "field_lookup";
static const char GLOM_NODE_RELATIONSHIPS[] = "relationships";
static const char GLOM_NODE_RELATIONSHIP[] = "relationship";
static const char GLOM_ATTRIBUTE_KEY[] = "key";
static const char GLOM_ATTRIBUTE_OTHER_TABLE[] = "other_table";
static const char GLOM_ATTRIBUTE_OTHER_KEY[] = "other_key";
static const char GLOM_ATTRIBUTE_AUTO_CREATE[] = "auto_create";
static const char GLOM_ATTRIBUTE_ALLOW_EDIT[] = "allow_edit";

static const char GLOM_NODE_GROUPS[] = "groups";
static const char GLOM_NODE_GROUP[] = "group";
static const char GLOM_ATTRIBUTE_DEVELOPER[] = "developer";
static const char GLOM_NODE_TABLE_PRIVS[] = "table_privs";
static const char GLOM_ATTRIBUTE_TABLE_NAME[] = "table_name";
static const char GLOM_ATTRIBUTE_PRIV_VIEW[] = "priv_view";
static const char GLOM_ATTRIBUTE_PRIV_EDIT[] = "priv_edit";
static const char GLOM_ATTRIBUTE_PRIV_CREATE[] = "priv_create";
static const char GLOM_ATTRIBUTE_PRIV_DELETE[] = "priv_delete";

static const char GLOM_ATTRIBUTE_FORMAT_VERSION[] = "format_version";
static const char GLOM_ATTRIBUTE_IS_EXAMPLE[] = "is_example";
static const char GLOM_ATTRIBUTE_IS_BACKUP[] = "is_backup";
static const char GLOM_DEPRECATED_ATTRIBUTE_CONNECTION_DATABASE_TITLE[] = "database_title"; //deprecated in favour of GLOM_ATTRIBUTE_TITLE.
static const char GLOM_NODE_STARTUP_SCRIPT[] = "startup_script";
static const char GLOM_ATTRIBUTE_TRANSLATION_ORIGINAL_LOCALE[] = "translation_original_locale";
static const char GLOM_ATTRIBUTE_NAME[] = "name";
static const char GLOM_ATTRIBUTE_TITLE[] = "title";
static const char GLOM_ATTRIBUTE_HIDDEN[] = "hidden";
static const char GLOM_ATTRIBUTE_DEFAULT[] = "default";
static const char GLOM_ATTRIBUTE_OVERVIEW_X[] = "overview_x";
static const char GLOM_ATTRIBUTE_OVERVIEW_Y[] = "overview_y";
static const char GLOM_ATTRIBUTE_FIELD[] = "field";
static const char GLOM_ATTRIBUTE_EDITABLE[] = "editable";
static const char GLOM_NODE_EXAMPLE_ROWS[] = "example_rows";
static const char GLOM_NODE_EXAMPLE_ROW[] = "example_row";
static const char GLOM_NODE_VALUE[] = "value";
static const char GLOM_ATTRIBUTE_COLUMN[] = "column";
static const char GLOM_DEPRECATED_ATTRIBUTE_BUTTON_SCRIPT[] = "script";
static const char GLOM_NODE_BUTTON_SCRIPT[] = "script";
static const char GLOM_ATTRIBUTE_SORT_ASCENDING[] = "sort_ascending";



static const char GLOM_ATTRIBUTE_RELATIONSHIP_NAME[] = "relationship";
static const char GLOM_ATTRIBUTE_RELATED_RELATIONSHIP_NAME[] = "related_relationship";

static const char GLOM_NODE_REPORTS[] = "reports";
static const char GLOM_NODE_REPORT[] = "report";
static const char GLOM_ATTRIBUTE_REPORT_SHOW_TABLE_TITLE[] = "show_table_title";
static const char GLOM_NODE_REPORT_ITEM_GROUPBY_GROUPBY[] = "groupby";
static const char GLOM_NODE_REPORT_ITEM_GROUPBY_SORTBY[] = "sortby";
static const char GLOM_ATTRIBUTE_LAYOUT_ITEM_FIELDSUMMARY_SUMMARYTYPE[] = "summarytype";

static const char GLOM_NODE_PRINT_LAYOUTS[] = "print_layouts";
static const char GLOM_NODE_PRINT_LAYOUT[] = "print_layout";
static const char GLOM_ATTRIBUTE_PRINT_LAYOUT_SHOW_GRID[] = "show_grid";
static const char GLOM_ATTRIBUTE_PRINT_LAYOUT_SHOW_RULES[] = "show_rules";
static const char GLOM_ATTRIBUTE_PRINT_LAYOUT_SHOW_OUTLINES[] = "show_outlines";
static const char GLOM_ATTRIBUTE_PRINT_LAYOUT_PAGE_COUNT[] = "page_count";
static const char GLOM_NODE_HORIZONTAL_RULE[] = "horizonal_rule";
static const char GLOM_NODE_VERTICAL_RULE[] = "vertical_rule";
static const char GLOM_ATTRIBUTE_RULE_POSITION[] = "position";

static const char GLOM_NODE_FORMAT[] = "formatting";
static const char GLOM_ATTRIBUTE_FORMAT_THOUSANDS_SEPARATOR[] = "format_thousands_separator";
static const char GLOM_ATTRIBUTE_FORMAT_DECIMAL_PLACES_RESTRICTED[] = "format_decimal_places_restricted";
static const char GLOM_ATTRIBUTE_FORMAT_DECIMAL_PLACES[] = "format_decimal_places";
static const char GLOM_ATTRIBUTE_FORMAT_CURRENCY_SYMBOL[] = "format_currency_symbol";
static const char GLOM_ATTRIBUTE_FORMAT_USE_ALT_NEGATIVE_COLOR[] = "format_use_alt_negative_color"; //Just a  bool, not a color.

static const char GLOM_ATTRIBUTE_FORMAT_TEXT_MULTILINE[] = "format_text_multiline";
static const char GLOM_ATTRIBUTE_FORMAT_TEXT_MULTILINE_HEIGHT_LINES[] = "format_text_multiline_height_lines";
static const guint GLOM_ATTRIBUTE_FORMAT_TEXT_MULTILINE_HEIGHT_LINES_DEFAULT = 6;
static const char GLOM_ATTRIBUTE_FORMAT_TEXT_FONT[] = "font";
static const char GLOM_ATTRIBUTE_FORMAT_TEXT_COLOR_FOREGROUND[] = "color_fg";
static const char GLOM_ATTRIBUTE_FORMAT_TEXT_COLOR_BACKGROUND[] = "color_bg";

static const char GLOM_ATTRIBUTE_FORMAT_HORIZONTAL_ALIGNMENT[] = "alignment_horizontal";
static const char GLOM_ATTRIBUTE_FORMAT_HORIZONTAL_ALIGNMENT_AUTO[] = "auto";
static const char GLOM_ATTRIBUTE_FORMAT_HORIZONTAL_ALIGNMENT_LEFT[] = "left";
static const char GLOM_ATTRIBUTE_FORMAT_HORIZONTAL_ALIGNMENT_RIGHT[] = "right";

static const char GLOM_ATTRIBUTE_FORMAT_CHOICES_RESTRICTED[] = "choices_restricted";
static const char GLOM_ATTRIBUTE_FORMAT_CHOICES_RESTRICTED_AS_RADIO_BUTTONS[] = "choices_restricted_radiobuttons";
static const char GLOM_ATTRIBUTE_FORMAT_CHOICES_CUSTOM[] = "choices_custom";
static const char GLOM_ATTRIBUTE_FORMAT_CHOICES_CUSTOM_LIST[] = "custom_choice_list";
static const char GLOM_NODE_FORMAT_CUSTOM_CHOICE[] = "custom_choice";
static const char GLOM_ATTRIBUTE_VALUE[] = "value";
static const char GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED[] = "choices_related";
static const char GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_RELATIONSHIP[] = "choices_related_relationship";
static const char GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_FIELD[] = "choices_related_field";
static const char GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_EXTRA_LAYOUT[] = "choices_related_extra_layout";
static const char GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_SECOND[] = "choices_related_second"; //deprecated
static const char GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_SHOW_ALL[] = "choices_related_show_all";
static const char GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_SORTBY[] = "choices_related_sortby";

static const char GLOM_NODE_TRANSLATIONS_SET[] = "trans_set";
static const char GLOM_NODE_TRANSLATION[] = "trans";
static const char GLOM_ATTRIBUTE_TRANSLATION_LOCALE[] = "loc";
static const char GLOM_ATTRIBUTE_TRANSLATION_VALUE[] = "val";

static const char GLOM_NODE_POSITION[] = "position";
static const char GLOM_ATTRIBUTE_POSITION_X[] = "x";
static const char GLOM_ATTRIBUTE_POSITION_Y[] = "y";
static const char GLOM_ATTRIBUTE_POSITION_WIDTH[] = "width";
static const char GLOM_ATTRIBUTE_POSITION_HEIGHT[] = "height";

static const char GLOM_NODE_PAGE_SETUP[] = "page_setup"; //Its text child is the keyfile for a GtkPageSetup

static const char GLOM_NODE_LIBRARY_MODULES[] = "library_modules";
static const char GLOM_NODE_LIBRARY_MODULE[] = "module";
static const char GLOM_ATTRIBUTE_LIBRARY_MODULE_NAME[] = "name";
static const char GLOM_ATTRIBUTE_LIBRARY_MODULE_SCRIPT[] = "script"; //deprecated

//A built-in relationship that is available for every table:
static const char GLOM_RELATIONSHIP_NAME_SYSTEM_PROPERTIES[] = "system_properties";

/**
 * Find the element in the container with the same parent_table and layout_name.
 */
template
<typename T_Container>
auto find_if_layout(T_Container& container, const Glib::ustring& layout_name, const Glib::ustring& layout_platform) -> decltype(container.begin())
{
  return std::find_if(container.begin(), container.end(),
    [&layout_name, &layout_platform](const auto& element)
    {
      return (element.m_layout_name == layout_name) &&
        (element.m_layout_platform == layout_platform);
    }
  );
}

Document::Document()
: m_hosting_mode(HostingMode::DEFAULT),
  m_network_shared(false),
  m_connection_port(0),
  m_connection_try_other_ports(false),
  m_block_cache_update(false),
  m_block_modified_set(false),
  m_allow_auto_save(true), //Save all changes immediately, by default.
  m_is_example(false),
  m_is_backup(false),
  m_opened_from_browse(false)
{
  m_database_title = std::make_shared<DatabaseTitle>();

  //Prevent autosaving during the constructor:
  set_allow_autosave(false); //Prevent saving while we modify the document.

  m_document_format_version = get_latest_known_document_format_version(); //Default to this for new documents.

  //Conscious use of virtual methods in a constructor:
  set_file_extension("glom");

  set_dtd_name("glom_document.dtd");
  //set_DTD_Location(GLOM_DTD_INSTALL_DIR); //Determined at configure time. It still looks in the working directory first.

  //The xmlns URI does not need to be something that actually exists.
  //I think it is just a unique ID. murrayc.
  //It helps the MIME-type system to recognize the file type.
  set_dtd_root_node_name("glom_document",
    "http://glom.org/glom_document" /* xmlns ID */);

  //We don't use set_write_formatted() because it doesn't handle text nodes well.
  //We use add_indenting_white_space_to_node() instead later.

  //Set default database name:
  //This is also the XML attribute default value,
  //but that isn't available for new documents.
  if(get_connection_server().empty())
    set_connection_server("localhost");

  m_app_state.signal_userlevel_changed().connect( sigc::mem_fun(*this, &Document::on_app_state_userlevel_changed) );

  set_modified(false);
  set_allow_autosave(true);
}

Document::~Document()
{
}

Document::HostingMode Document::get_hosting_mode() const
{
  return m_hosting_mode;
}

void Document::set_network_shared(bool shared)
{
  if(shared != m_network_shared)
  {
    m_network_shared = shared;
    set_modified();
  }
}

bool Document::get_network_shared() const
{
  bool shared = m_network_shared;

  //Enforce constraints:
  const auto hosting_mode = get_hosting_mode();
  if( (hosting_mode == HostingMode::POSTGRES_CENTRAL) ||
    (hosting_mode == HostingMode::MYSQL_CENTRAL) )
  {
    shared = true; //Central hosting means that it must be shared on the network.
  }
  else if(hosting_mode == HostingMode::SQLITE)
    shared = false; //sqlite does not allow network sharing.

  return shared;
}

std::string Document::get_connection_self_hosted_directory_uri() const
{
  const auto uri_file = get_file_uri();
  if(uri_file.empty())
  {
    std::cerr << G_STRFUNC << ": file_uri is empty." << std::endl;
    return std::string();
  }
  else
  {
    //Use Gio::File API to construct the URI:
    auto file = Gio::File::create_for_uri(uri_file);

    auto parent = file->get_parent();
    Glib::RefPtr<Gio::File> datadir;

    if(parent)
    {
      switch(m_hosting_mode)
      {
      case HostingMode::POSTGRES_SELF:
        datadir = parent->get_child("glom_postgres_data");
        break;
      case HostingMode::POSTGRES_CENTRAL:
        datadir = parent;
        break;
      case HostingMode::SQLITE:
        datadir = parent;
        break;
      case HostingMode::MYSQL_SELF:
        datadir = parent->get_child("glom_mysql_data");
        break;
      case HostingMode::MYSQL_CENTRAL:
        datadir = parent;
        break;
      default:
        g_assert_not_reached();
        break;
      }

      if(datadir)
        return datadir->get_uri();
    }
  }

  std::cerr << G_STRFUNC << ": returning empty string." << std::endl;
  return std::string();
}

Glib::ustring Document::get_connection_user() const
{
  return m_connection_user;
}

Glib::ustring Document::get_connection_server() const
{
  return m_connection_server;
}

Glib::ustring Document::get_connection_database() const
{
  return m_connection_database;
}

unsigned int Document::get_connection_port() const
{
  return m_connection_port;
}

bool Document::get_connection_try_other_ports() const
{
  return m_connection_try_other_ports;
}

void Document::set_connection_user(const Glib::ustring& strVal)
{
  if(strVal != m_connection_user)
  {
    m_connection_user = strVal;

    //We don't call set_modified(), because this is not saved in the document: set_modified();
  }
}

void Document::set_hosting_mode(HostingMode mode)
{
  if(mode != m_hosting_mode)
  {
    m_hosting_mode = mode;
    set_modified();
  }
}

void Document::set_connection_server(const Glib::ustring& strVal)
{
  if(strVal != m_connection_server)
  {
    m_connection_server = strVal;
    set_modified();
  }
}

void Document::set_connection_database(const Glib::ustring& strVal)
{
  if(strVal != m_connection_database)
  {
    m_connection_database = strVal;
    set_modified();
  }
}

void Document::set_connection_port(unsigned int port_number)
{
  if(port_number != m_connection_port)
  {
    m_connection_port = port_number;
    set_modified();
  }
}

void Document::set_connection_try_other_ports(bool val)
{
  if(val != m_connection_try_other_ports)
  {
    m_connection_try_other_ports = val;
    set_modified();
  }
}


void Document::set_relationship(const Glib::ustring& table_name, const std::shared_ptr<Relationship>& relationship)
{
  //Find the existing relationship:
  const auto info = get_table_info(table_name);
  if(!info)
    return;
   
  //Look for the relationship with this name:
  bool existing = false;
  const auto relationship_name = glom_get_sharedptr_name(relationship);
  for(auto& item : info->m_relationships)
  {
    if(item->get_name() == relationship_name)
    {
      item = relationship; //Changes the relationship. All references (std::shared_ptrs) to the relationship will get the informatin too, because it is shared.
      existing = true;
    }
  }

  if(!existing)
  {
    //Add a new one if it's not there.
    info->m_relationships.push_back(relationship);
  }
}

std::shared_ptr<Relationship> Document::create_relationship_system_preferences(const Glib::ustring& table_name)
{
  auto relationship = std::make_shared<Relationship>();
  relationship->set_name(GLOM_RELATIONSHIP_NAME_SYSTEM_PROPERTIES);
  relationship->set_title_original(_("System Preferences"));
  relationship->set_from_table(table_name);
  relationship->set_to_table(GLOM_STANDARD_TABLE_PREFS_TABLE_NAME);
  relationship->set_allow_edit(false);

  return relationship;
}

std::shared_ptr<TableInfo> Document::create_table_system_preferences()
{
  type_vec_fields fields_ignored;
  return create_table_system_preferences(fields_ignored);
}

std::shared_ptr<TableInfo> Document::create_table_system_preferences(type_vec_fields& fields)
{
  auto prefs_table_info = std::make_shared<TableInfo>();
  prefs_table_info->set_name(GLOM_STANDARD_TABLE_PREFS_TABLE_NAME);
  prefs_table_info->set_title_original(_("System Preferences"));
  prefs_table_info->set_hidden(true);


  fields.clear();

  auto primary_key = std::make_shared<Field>(); //It's not used, because there's only one record, but we must have one.
  primary_key->set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_ID);
  primary_key->set_glom_type(Field::glom_field_type::NUMERIC);
  fields.push_back(primary_key);

  auto field_name = std::make_shared<Field>();
  field_name->set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_NAME);
  field_name->set_title_original(_("System Name"));
  field_name->set_glom_type(Field::glom_field_type::TEXT);
  fields.push_back(field_name);

  auto field_org_name = std::make_shared<Field>();
  field_org_name->set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_NAME);
  field_org_name->set_title_original(_("Organisation Name"));
  field_org_name->set_glom_type(Field::glom_field_type::TEXT);
  fields.push_back(field_org_name);

  auto field_org_logo = std::make_shared<Field>();
  field_org_logo->set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_LOGO);
  field_org_logo->set_title_original(_("Organisation Logo"));
  field_org_logo->set_glom_type(Field::glom_field_type::IMAGE);
  fields.push_back(field_org_logo);

  auto field_org_address_street = std::make_shared<Field>();
  field_org_address_street->set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_STREET);
  field_org_address_street->set_title_original(_("Street"));
  field_org_address_street->set_glom_type(Field::glom_field_type::TEXT);
  fields.push_back(field_org_address_street);

  auto field_org_address_street2 = std::make_shared<Field>();
  field_org_address_street2->set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_STREET2);
  field_org_address_street2->set_title_original(_("Street (line 2)"));
  field_org_address_street2->set_glom_type(Field::glom_field_type::TEXT);
  fields.push_back(field_org_address_street2);

  auto field_org_address_town = std::make_shared<Field>();
  field_org_address_town->set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_TOWN);
  field_org_address_town->set_title_original(_("City"));
  field_org_address_town->set_glom_type(Field::glom_field_type::TEXT);
  fields.push_back(field_org_address_town);

  auto field_org_address_county = std::make_shared<Field>();
  field_org_address_county->set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_COUNTY);
  field_org_address_county->set_title_original(_("State"));
  field_org_address_county->set_glom_type(Field::glom_field_type::TEXT);
  fields.push_back(field_org_address_county);

  auto field_org_address_country = std::make_shared<Field>();
  field_org_address_country->set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_COUNTRY);
  field_org_address_country->set_title_original(_("Country"));
  field_org_address_country->set_glom_type(Field::glom_field_type::TEXT);
  fields.push_back(field_org_address_country);

  auto field_org_address_postcode = std::make_shared<Field>();
  field_org_address_postcode->set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_POSTCODE);
  field_org_address_postcode->set_title_original(_("Zip Code"));
  field_org_address_postcode->set_glom_type(Field::glom_field_type::TEXT);
  fields.push_back(field_org_address_postcode);

  return prefs_table_info;
}

bool Document::get_relationship_is_system_properties(const std::shared_ptr<const Relationship>& relationship)
{
  return relationship && (relationship->get_name() == GLOM_RELATIONSHIP_NAME_SYSTEM_PROPERTIES);
}

std::shared_ptr<Relationship> Document::get_relationship(const Glib::ustring& table_name, const Glib::ustring& relationship_name) const
{
  std::shared_ptr<Relationship> result;

  if(relationship_name == GLOM_RELATIONSHIP_NAME_SYSTEM_PROPERTIES)
  {
    return create_relationship_system_preferences(table_name);
  }

  const std::shared_ptr<const DocumentTableInfo> info = get_table_info(table_name);
  if(info)
  {
    //Look for the relationship with this name:
    for(const auto& relationship : info->m_relationships)
    {
      if(relationship && (relationship->get_name() == relationship_name))
      {
        result = relationship;
      }
    }
  }

  return result;
}

Document::type_vec_relationships Document::get_relationships(const Glib::ustring& table_name, bool plus_system_prefs) const
{
  const auto info = get_table_info(table_name);
  if(!info)
    return type_vec_relationships();

  auto result = info->m_relationships;

  //Add the system properties if necessary:
  if(plus_system_prefs)
  {
    if(find_if_same_name(result, GLOM_RELATIONSHIP_NAME_SYSTEM_PROPERTIES) == result.end())
    {
      result.push_back(create_relationship_system_preferences(table_name));
    }
  }

  return result;
}

void Document::set_relationships(const Glib::ustring& table_name, const type_vec_relationships& vecRelationships) //TODO_shared_relationships
{
  if(!table_name.empty())
  {
    const auto info = get_table_info_with_add(table_name);
    if(info)
      info->m_relationships = vecRelationships;

    set_modified();
  }
}

void Document::remove_relationship(const std::shared_ptr<const Relationship>& relationship)
{
  //Get the table that this relationship is part of:
  const auto info = get_table_info(relationship->get_from_table());
  if(!info)
    return;

  const auto relationship_name = glom_get_sharedptr_name(relationship);

  //Find the relationship and remove it:
  auto relationships = info->m_relationships;
  auto iterRel = find_if_same_name(relationships, relationship_name);
  if(iterRel != relationships.end())
  {
    relationships.erase(iterRel);

    set_modified();
  }

  //Remove relationship from any layouts:
  auto layouts = info->m_layouts;
  auto iterLayouts = layouts.begin();
  while(iterLayouts != layouts.end())
  {
    auto layout_info = *iterLayouts;

    auto iterGroups = layout_info.m_layout_groups.begin();
    while(iterGroups != layout_info.m_layout_groups.end())
    {
      //Remove any layout parts that use this relationship:
      auto group = *iterGroups;
      auto uses_rel = std::dynamic_pointer_cast<UsesRelationship>(group);
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
  for(const auto& report_pair : info->m_reports)
  {
    auto report = report_pair.second;
    auto group = report->get_layout_group();

    //Remove the field wherever it is a related field:
    group->remove_relationship(relationship);
  }
}

void Document::remove_field(const Glib::ustring& table_name, const Glib::ustring& field_name)
{
  //Remove the field itself:
  const auto table_info = get_table_info(table_name);
  if(table_info)
  {
    auto vecFields = table_info->m_fields;
    auto iterFind = find_if_same_name(vecFields, field_name);
    if(iterFind != vecFields.end()) //If it was found:
    {
      //Remove it:
      vecFields.erase(iterFind);

      set_modified();
    }
  }

  //Remove any relationships that use this field:
  for(const auto& table_pair : m_tables)
  {
    const auto info = table_pair.second;
    if(!info)
      continue;

    if(!(info->m_relationships.empty()))
    {
      auto iterRel = info->m_relationships.begin();
      bool something_changed = true;
      while(something_changed && !info->m_relationships.empty())
      {
        auto relationship = *iterRel;

        if( ((relationship->get_from_table() == table_name) && (relationship->get_from_field() == field_name))
          || ((relationship->get_to_table() == table_name) && (relationship->get_to_field() == field_name)) )
        {
          //Loop again, because we have changed the structure:
          remove_relationship(relationship); //Also removes anything that uses the relationship.

          something_changed = true;
          iterRel = info->m_relationships.begin();
        }
        else
        {
          ++iterRel;

          if(iterRel == info->m_relationships.end())
            something_changed = false; //We've looked at them all, without changing things.
        }
      }
    }

    //Remove field from any layouts:
    for(const auto& layout_info : info->m_layouts)
    {
      for(const auto& group : layout_info.m_layout_groups)
      {
        if(!group)
          continue;


        //Remove regular field from the layout:
        if(info->m_info)
        {
          const auto layout_table_name = info->m_info->get_name();
          group->remove_field(layout_table_name, table_name, field_name);
        }
      }
    }

    //Remove field from any reports:
    for(const auto& report_pair : info->m_reports)
    {
      auto report = report_pair.second;
      auto group = report->get_layout_group();

      //Remove regular fields if the field is in this layout's table:
      if(info->m_info)
      {
        const auto layout_table_name = info->m_info->get_name();
        group->remove_field(layout_table_name, table_name, field_name);
      }
    }
  }
}

void Document::remove_table(const Glib::ustring& table_name)
{
  auto iter = m_tables.find(table_name);
  if(iter != m_tables.end())
  {
    m_tables.erase(iter);
    set_modified();
  }

  //Remove any relationships that use this table:
  for(const auto& table_pair : m_tables)
  {
    const auto info = table_pair.second;
    if(!info)
      continue;

    if(!(info->m_relationships.empty()))
    {
      auto iterRel = info->m_relationships.begin();
      bool something_changed = true;
      while(something_changed && !info->m_relationships.empty())
      {
        auto relationship = *iterRel;

        if(relationship->get_to_table() == table_name)
        {
          //Loop again, because we have changed the structure:
          remove_relationship(relationship); //Also removes anything that uses the relationship.

          something_changed = true;
          iterRel = info->m_relationships.begin();
        }
        else
        {
          ++iterRel;

          if(iterRel == info->m_relationships.end())
            something_changed = false; //We've looked at them all, without changing things.
        }
      }
    }

  }
}


Document::type_vec_fields Document::get_table_fields(const Glib::ustring& table_name) const
{
  type_vec_fields result;

  if(!table_name.empty())
  {
    const std::shared_ptr<const DocumentTableInfo> info = get_table_info(table_name);
    if(info)
    {
      if(info->m_fields.empty())
      {
         std::cerr << G_STRFUNC << ": table found, but m_fields is empty. table_name=" << table_name << std::endl;
      }

      return info->m_fields;
    }
    else
    {
      //It's a standard table, not saved in the document:
      if(table_name == GLOM_STANDARD_TABLE_PREFS_TABLE_NAME)
      {
        type_vec_fields fields;
        create_table_system_preferences(fields);
        result = fields;
      }
      else
      {
        //std::cerr << G_STRFUNC << ": table not found in document: " << table_name << std::endl;
      }
    }
  }
  else
  {
    //std::cerr << G_STRFUNC << ": table name is empty." << std::endl;
  }

  //Hide any system fields:
  auto iterFind = find_if_same_name(result, GLOM_STANDARD_FIELD_LOCK);
  if(iterFind != result.end())
    result.erase(iterFind);

  return result;
}

void Document::set_table_fields(const Glib::ustring& table_name, const type_vec_fields& vecFields)
{
  if(!table_name.empty())
  {
    if(vecFields.empty())
    {
      std::cerr << G_STRFUNC << ": : vecFields is empty: table_name=" << table_name << std::endl;
    }

    const auto info = get_table_info_with_add(table_name);
    if(!info)
      return;

    const bool will_change = true; //This won't work because we didn't clone the fields before changing them: (info.m_fields != vecFields); //TODO: Does this do a deep comparison?
    info->m_fields = vecFields;

    set_modified(will_change);
  }
}

std::shared_ptr<Field> Document::get_field(const Glib::ustring& table_name, const Glib::ustring& strFieldName) const
{
  auto vecFields = get_table_fields(table_name);
  auto iterFind = find_if_same_name(vecFields, strFieldName);
  if(iterFind != vecFields.end()) //If it was found:
  {
    return  *iterFind; //A reference, not a copy.
  }

  return std::shared_ptr<Field>();
}

std::shared_ptr<Field> Document::get_field_primary_key(const Glib::ustring& table_name) const
{
  for(const auto& field : get_table_fields(table_name))
  {
    if(field && field->get_primary_key())
      return field;
  }

  return std::shared_ptr<Field>();
}

void Document::change_field_name(const Glib::ustring& table_name, const Glib::ustring& strFieldNameOld, const Glib::ustring& strFieldNameNew)
{
  const auto info = get_table_info(table_name);
  if(info)
  {
    //Fields:
    type_vec_fields& vecFields = info->m_fields;
    auto iterFind = find_if_same_name(vecFields, strFieldNameOld);
    if(iterFind != vecFields.end()) //If it was found:
    {
      //Change it:
      (*iterFind)->set_name(strFieldNameNew);
    }

    //Find any relationships, layouts, or formatting that use this field
    //Look at each table:
    for(const auto& table_pair : m_tables)
    {
      const auto infoInner = table_pair.second;
      if(!infoInner)
        continue;

      //Fields:
      for(const auto& field : infoInner->m_fields)
      {
        if(!field)
          continue;

        //Formatting:
        Formatting& formatting = field->m_default_formatting;
        formatting.change_field_item_name(table_name, strFieldNameOld, strFieldNameNew);
      }

      //Look at each relationship in the table:
      for(const auto& relationship : infoInner->m_relationships)
      {
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

      const auto is_parent_table = (infoInner->m_info->get_name() == table_name);

      //Look at each layout:
      for(const auto& item : infoInner->m_layouts)
      {
        //Look at each group:
        for(const auto& group : item.m_layout_groups)
        {
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
      for(const auto& report_pair : infoInner->m_reports)
      {
        //Change the field if it is in this group:
        auto report = report_pair.second;
        if(report)
        {
          if(is_parent_table)
            report->get_layout_group()->change_field_item_name(table_name, strFieldNameOld, strFieldNameNew);
          else
            report->get_layout_group()->change_related_field_item_name(table_name, strFieldNameOld, strFieldNameNew);
        }
      }

    }

    set_modified();
  }
}

void Document::change_table_name(const Glib::ustring& table_name_old, const Glib::ustring& table_name_new)
{
  auto iterFindTable = m_tables.find(table_name_old);
  if(iterFindTable != m_tables.end())
  {
    //Change it:
    //We can't just change the key of the iterator (I think),
    //so we copy the whole thing and put it back in the map under a different key:

    //iterFindTable->first = table_name_new;
    const auto doctableinfo = iterFindTable->second;
    m_tables.erase(iterFindTable);

    if(doctableinfo && doctableinfo->m_info)
      doctableinfo->m_info->set_name(table_name_new);

    m_tables[table_name_new] = doctableinfo;

    //Find any relationships or layouts that use this table
    //Look at each table:
    for(const auto& table_pair : m_tables)
    {
      //Look at each relationship in the table:
      const auto doctableinfo_inner = table_pair.second;
      if(!doctableinfo_inner)
        continue;

      for(const auto& relationship : doctableinfo_inner->m_relationships)
      {
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

void Document::change_relationship_name(const Glib::ustring& table_name, const Glib::ustring& name, const Glib::ustring& name_new)
{
  const auto doctableinfo = get_table_info(table_name);
  if(doctableinfo)
  {
    type_vec_relationships relationships = doctableinfo->m_relationships;
      
    //Change the relationship name:
    auto iterRelFind = find_if_same_name(relationships, name);
    if(iterRelFind != relationships.end())
      (*iterRelFind)->set_name(name_new);


    //Any layouts, reports, etc that use this relationship will already have the new name via the std::shared_ptr<Relationship>.


    //Look at each table:
      /*
    for(const auto& item : m_tables)
    {

       //Look at all field formatting:
      for(const auto& item : iterFields)
      {
        (*iterFields)->m_default_formatting.change_relationship_name(table_name, name, name_new);
      }

      const auto is_parent_table = (iter->second->m_info->get_name() == table_name);

      //Look at each layout:
      for(const auto& item : iterLayouts)
      {
        //Look at each group:
        for(const auto& item : iterGroup)
        {
          //Change the field if it is in this group:
          if(is_parent_table)
            iterGroup->second.change_relationship_name(table_name, name, name_new);
          else
            iterGroup->second.change_related_relationship_name(table_name, name, name_new);
        }
      }


      //Look at each report:
      for(const auto& item : iterReports)
      {
        //Change the field if it is in this group:
        if(is_parent_table)
          iterReports->second->get_layout_group()->change_relationship_name(table_name, name, name_new);
        else
          iterReports->second->get_layout_group()->change_related_relationship_name(table_name, name, name_new);
      }

     //TODO_SharedRelationshipCheck lookups.

    }
      */

    set_modified();
  }
 }


Document::type_listConstTableInfo Document::get_tables(bool plus_system_prefs) const
{
  //TODO: Avoid the almost-duplicate implementation in the const and non-const methods.

  type_listConstTableInfo result;

  for(const auto& table_pair : m_tables)
  {
    const std::shared_ptr<const DocumentTableInfo> doctableinfo = table_pair.second;
    if(doctableinfo)
      result.push_back(doctableinfo->m_info);

    //std::cout << "debug: " << G_STRFUNC << ": title=" << iter->second->m_info->get_title() << std::endl;
  }

  //Add the system properties if necessary:
  if(plus_system_prefs)
  {
    if(find_if_same_name(result, GLOM_STANDARD_TABLE_PREFS_TABLE_NAME) == result.end())
      result.push_back(create_table_system_preferences());
  }

  return result;
}

Document::type_listTableInfo Document::get_tables(bool plus_system_prefs)
{
  type_listTableInfo result;

  for(const auto& table_pair : m_tables)
  {
    const auto doctableinfo = table_pair.second;
    if(doctableinfo)
      result.push_back(doctableinfo->m_info);

    //std::cout << "debug: " << G_STRFUNC << ": title=" << iter->second->m_info->get_title() << std::endl;
  }

  //Add the system properties if necessary:
  if(plus_system_prefs)
  {
    if(find_if_same_name(result, GLOM_STANDARD_TABLE_PREFS_TABLE_NAME) == result.end())
      result.push_back(create_table_system_preferences());
  }

  return result;
}

std::vector<Glib::ustring> Document::get_table_names(bool plus_system_prefs) const
{
  auto list_full = get_tables(plus_system_prefs);
  std::vector<Glib::ustring> result;
  for (const auto& info : list_full)
  {
    if(info)
      result.push_back(info->get_name());
  }

  return result;
}

std::shared_ptr<TableInfo> Document::get_table(const Glib::ustring& table_name) const
{
  const std::shared_ptr<const DocumentTableInfo> doctableinfo = get_table_info(table_name);
  if(doctableinfo)
    return doctableinfo->m_info;

  return std::shared_ptr<TableInfo>();
}

void Document::add_table(const std::shared_ptr<TableInfo>& table_info)
{
  if(!table_info)
    return;

  auto iterfind = m_tables.find(table_info->get_name());
  if(iterfind == m_tables.end())
  {
    const auto item = std::make_shared<DocumentTableInfo>();
    item->m_info = table_info;
    m_tables[table_info->get_name()] = item;
    set_modified();
  }
}

bool Document::get_table_overview_position(const Glib::ustring& table_name, float& x, float& y) const
{
  const std::shared_ptr<const DocumentTableInfo> doctableinfo = get_table_info(table_name);
  if(doctableinfo)
  {
    if( doctableinfo->m_overviewx == std::numeric_limits<float>::infinity() ||
         doctableinfo->m_overviewy == std::numeric_limits<float>::infinity() )
    {
      return false;
    }

    x = doctableinfo->m_overviewx;
    y = doctableinfo->m_overviewy;
    return true;
  }
  else
  {
    return false;
  }
}

void Document::set_table_overview_position(const Glib::ustring& table_name, float x, float y)
{
  const auto doctableinfo = get_table_info(table_name);
  if(doctableinfo)
  {
    doctableinfo->m_overviewx = x;
    doctableinfo->m_overviewy = y;
  }
}

void Document::set_tables(const type_listTableInfo& tables)
{
  //We avoid adding information about tables that we don't know about - that should be done explicitly.
  //Look at each "table":

  bool something_changed = false;
  for(const auto& table_pair : m_tables)
  {
    const auto doctableinfo = table_pair.second;
    if(!doctableinfo)
      continue;

    std::shared_ptr<TableInfo> info = doctableinfo->m_info;
    if(!info)
      continue;

    const auto table_name = info->get_name();

    //If the table is also in the supplied list:
    auto iterfind = find_if_same_name(tables, table_name);
    if(iterfind != tables.end())
    {
      std::shared_ptr<TableInfo> infoFound = *iterfind;
      if(infoFound && (*infoFound != *info))
      {
        *info = *infoFound;

        something_changed = true;
      }
    }
  }

  if(something_changed)
    set_modified();
}

void Document::fill_sort_field_details(const Glib::ustring& parent_table_name, Formatting::type_list_sort_fields& sort_fields) const
{
  for(const auto& sort_pair : sort_fields)
  {
    std::shared_ptr<const LayoutItem_Field> sort_field = sort_pair.first;
    if(!sort_field)
     continue;
 
    //TODO: Avoid this unconst?
    auto unconst_sort_field = std::const_pointer_cast<LayoutItem_Field>(sort_field);
    auto field = get_field( sort_field->get_table_used(parent_table_name), sort_field->get_name() );
    unconst_sort_field->set_full_field_details(field);
  }
}

void Document::fill_layout_field_details(const Glib::ustring& parent_table_name, const std::shared_ptr<LayoutGroup>& layout_group) const
{
  if(!layout_group)
    return;

  //Get the full field information for the LayoutItem_Fields in this group:

  for(const auto& layout_item : layout_group->m_list_items)
  {
    //Check custom Field Formatting:
    std::shared_ptr<LayoutItem_WithFormatting> layout_withformatting = 
      std::dynamic_pointer_cast<LayoutItem_WithFormatting>(layout_item);
    if(layout_withformatting)
    {
      std::shared_ptr<const Relationship> choice_relationship;
      std::shared_ptr<LayoutItem_Field> choice_layout_first;
      std::shared_ptr<LayoutGroup> choice_extra_layouts;
      Formatting::type_list_sort_fields choice_sort_fields;
      bool choice_show_all = false;
      layout_withformatting->m_formatting.get_choices_related(choice_relationship, choice_layout_first, choice_extra_layouts, choice_sort_fields, choice_show_all);
      
      const auto table_name = (choice_relationship ? choice_relationship->get_to_table() : Glib::ustring());
      if(choice_layout_first)
        choice_layout_first->set_full_field_details( get_field(table_name, choice_layout_first->get_name()) );
      fill_layout_field_details(parent_table_name, choice_extra_layouts); //recurse

      fill_sort_field_details(table_name, choice_sort_fields);
    }

    auto layout_field = std::dynamic_pointer_cast<LayoutItem_Field>(layout_item);
    if(layout_field)
    {
      const auto field = get_field(layout_field->get_table_used(parent_table_name), layout_field->get_name());
      layout_field->set_full_field_details(field);
      if(field)
      {
        //Check default Field Formatting:
        std::shared_ptr<const Relationship> choice_relationship;
        std::shared_ptr<LayoutItem_Field> choice_layout_first;
        std::shared_ptr<LayoutGroup> choice_extra_layouts;
        Formatting::type_list_sort_fields choice_sort_fields;
        bool choice_show_all = false;
        field->m_default_formatting.get_choices_related(choice_relationship, choice_layout_first, choice_extra_layouts, choice_sort_fields, choice_show_all);
        
        const auto table_name = (choice_relationship ? choice_relationship->get_to_table() : Glib::ustring());
        if(choice_layout_first)
          choice_layout_first->set_full_field_details( get_field(table_name, choice_layout_first->get_name()) );
        fill_layout_field_details(parent_table_name, choice_extra_layouts); //recurse

        fill_sort_field_details(table_name, choice_sort_fields);
      }
    }
    else
    {
      auto layout_portal_child = std::dynamic_pointer_cast<LayoutItem_Portal>(layout_item);
      if(layout_portal_child)
        fill_layout_field_details(layout_portal_child->get_table_used(parent_table_name), layout_portal_child); //recurse
      else
      {
        auto layout_group_child = std::dynamic_pointer_cast<LayoutGroup>(layout_item);
        if(layout_group_child)
          fill_layout_field_details(parent_table_name, layout_group_child); //recurse
      }
    }
  }
}

void Document::fill_layout_field_details(const Glib::ustring& parent_table_name, type_list_layout_groups& groups) const
{
  for(const auto& group : groups)
  {
    if(group)
      fill_layout_field_details(parent_table_name, group);
  }
}

Document::type_list_layout_groups Document::get_data_layout_groups_default(const Glib::ustring& layout_name, const Glib::ustring& parent_table_name, const Glib::ustring& /* layout_platform */) const
{
  type_list_layout_groups result;

  //Add one if necessary:
  std::shared_ptr<LayoutGroup> overview;
  std::shared_ptr<LayoutGroup> details;

  if(layout_name == "details") //The Details default layout is a bit more complicated.
  {
    overview = std::make_shared<LayoutGroup>();;
    overview->set_name("overview");
    overview->set_title_original(_("Overview"));
    overview->set_columns_count(2);
    result.push_back(overview);

    details = std::make_shared<LayoutGroup>();
    details->set_name("details");
    details->set_title_original(_("Details"));
    details->set_columns_count(2);
    result.push_back(details);
  }
  
  //If, for some reason, we didn't create the-subgroups, add everything to a top level group:
  if(!overview && !details)
  {
    overview = std::make_shared<LayoutGroup>();
    overview->set_name("main");
    overview->set_columns_count(1);
    result.push_back(overview);
      
    details = overview; //Adding anything to details adds it to the overview, which is the only group.
  }


  //Discover new fields, and add them:
  for(const auto& field : get_table_fields(parent_table_name))
  {
    const auto field_name = field->get_name();
    if(!field_name.empty())
    {
      //See whether it's already in the result:
      //TODO_Performance: There is a lot of iterating and comparison here:
      bool found = false; //TODO: This is horrible.
      for(const auto& group : result)
      {
        if(group && group->has_field(parent_table_name, parent_table_name, field_name))
        {
          found = true;
          break;
        }
      }

      if(!found)
      {
        auto layout_item = std::make_shared<LayoutItem_Field>();
        layout_item->set_full_field_details(field);
        //layout_item.set_table_name(child_table_name); //TODO: Allow viewing of fields through relationships.
        //layout_item.m_sequence = sequence;  add_item() will fill this.

        //std::cout << "debug: " << G_STRFUNC << ": " << layout_item.get_name() << std::endl;
        if(overview && layout_item->get_full_field_details()->get_primary_key())
          overview->add_item(layout_item);
        else if(details)
          details->add_item(layout_item);
      }
    }
  }

  return result;
}

Document::type_list_layout_groups Document::get_data_layout_groups_plus_new_fields(const Glib::ustring& layout_name, const Glib::ustring& parent_table_name, const Glib::ustring& layout_platform) const
{
  auto result = get_data_layout_groups(layout_name, parent_table_name, layout_platform);

  //If there are no fields in the layout, then add a default:
  bool create_default = false;
  if(result.empty() && !layout_name.empty())
  {
    //Fall back to a general layout instead of one for a specific platform:
    result = get_data_layout_groups(layout_name, parent_table_name, Glib::ustring());
  }

  if(result.empty())
  {
    create_default = true;
  }
  //TODO: Also set create_default if all groups have no fields.

  if(create_default)
  {
    std::cout << "debug: " << G_STRFUNC << ": Creating a default layout." << std::endl;
    result = get_data_layout_groups_default(layout_name, parent_table_name, layout_platform);

    /*
    //Make the default layout suitable for the special platform:
    if(layout_platform == "maemo")
    {
      for(const auto& layout_group : result)
      {
        if(!layout_group)
          continue;

        if(layout_name == "list")
        {
          //Don't try to show more than 3 items on the list view:
          if(layout_group->get_items_count() >= 2)
            layout_group->m_list_items.resize(2);
        }

        maemo_restrict_layouts_to_single_column_group(layout_group);

      }
    */

    //Store this so we don't have to recreate it next time:
    auto nonconst_this = const_cast<Document*>(this); //TODO: This is not ideal.
    nonconst_this->set_data_layout_groups(layout_name, parent_table_name, layout_platform, result);
    nonconst_this->set_modified(false); //This might have happened in operator mode, but in that case we don't really need to save it, or mark the document as unsaved.
  }

  return result;
}

Document::type_list_layout_groups Document::get_data_layout_groups(const Glib::ustring& layout_name, const Glib::ustring& parent_table_name, const Glib::ustring& layout_platform) const
{
  //std::cout << "debug: " << G_STRFUNC << ": layout_name=" << layout_name << ", parent_table_name=" << parent_table_name << ", layout_platform=" << layout_platform << std::endl;

  const std::shared_ptr<const DocumentTableInfo> info = get_table_info(parent_table_name);
  if(info)
  {
    const DocumentTableInfo::type_layouts layouts = info->m_layouts;

    //Look for the layout with this name:
    auto iter = find_if_layout(layouts, layout_name, layout_platform);
    if(iter != layouts.end())
    {
      return iter->m_layout_groups; //found
    }
  }

  return type_list_layout_groups(); //not found
}

bool Document::get_data_layout_groups_have_any_fields(const Glib::ustring& layout_name, const Glib::ustring& parent_table_name, const Glib::ustring& layout_platform) const
{
  //TODO_Performance: This could make the response to some button slow, such as the Add button, which does a check for this.
  auto layout_groups = get_data_layout_groups(layout_name, parent_table_name, layout_platform);
  for(const auto& layout_group : layout_groups)
  {
    if(layout_group && layout_group->has_any_fields())
      return true;
  }

  return false;
}

void Document::set_data_layout_groups(const Glib::ustring& layout_name, const Glib::ustring& parent_table_name, const Glib::ustring& layout_platform, const type_list_layout_groups& groups)
{
  //std::cout << "debug: " << G_STRFUNC << ": layout_name=" << layout_name << ", parent_table_name=" << parent_table_name << ", layout_platform=" << layout_platform << std::endl;
  //std::cerr << G_STRFUNC << ": ADDING layout for table " << parent_table_name << " (child_table=" << child_table_name << "), for layout " << layout_name << std::endl;


  if(!parent_table_name.empty())
  {
    const auto info = get_table_info_with_add(parent_table_name);
    if(!info)
      return;

    LayoutInfo layout_info;
    layout_info.m_layout_name = layout_name;
    layout_info.m_layout_groups = groups;

    DocumentTableInfo::type_layouts& layouts = info->m_layouts;
    auto iter = find_if_layout(layouts, layout_name, layout_platform);
    if(iter == layouts.end())
      layouts.push_back(layout_info);
    else
      *iter = layout_info;

    set_modified();
  }
}

std::shared_ptr<Document::DocumentTableInfo> Document::get_table_info(const Glib::ustring& table_name)
{
  auto iterFind = m_tables.find(table_name);
  if(iterFind != m_tables.end())
    return iterFind->second;

  return std::shared_ptr<DocumentTableInfo>();
}

std::shared_ptr<const Document::DocumentTableInfo> Document::get_table_info(const Glib::ustring& table_name) const
{
  auto iterFind = m_tables.find(table_name);
  if(iterFind != m_tables.end())
    return iterFind->second;

  return std::shared_ptr<const DocumentTableInfo>();
}

std::shared_ptr<Document::DocumentTableInfo> Document::get_table_info_with_add(const Glib::ustring& table_name)
{
  std::shared_ptr<DocumentTableInfo>  doctableinfo = get_table_info(table_name);
  if(doctableinfo)
  {
    return doctableinfo;
  }
  else
  {
    doctableinfo = std::make_shared<DocumentTableInfo>();
    doctableinfo->m_info->set_name(table_name);
    m_tables[table_name] = doctableinfo;
    return doctableinfo;
  }
}

Glib::ustring Document::get_table_title(const Glib::ustring& table_name, const Glib::ustring& locale) const
{
  const std::shared_ptr<const DocumentTableInfo> doctableinfo = get_table_info(table_name);
  if(doctableinfo && doctableinfo->m_info)
    return doctableinfo->m_info->get_title(locale);

  return Glib::ustring();
}

Glib::ustring Document::get_table_title_original(const Glib::ustring& table_name) const
{
  const std::shared_ptr<const DocumentTableInfo> doctableinfo = get_table_info(table_name);
  if(doctableinfo && doctableinfo->m_info)
    return doctableinfo->m_info->get_title_original();

  return Glib::ustring();
}

Glib::ustring Document::get_table_title_singular(const Glib::ustring& table_name, const Glib::ustring& locale) const
{
  const std::shared_ptr<const DocumentTableInfo> doctableinfo = get_table_info(table_name);
  if(doctableinfo && doctableinfo->m_info)
    return doctableinfo->m_info->get_title_singular_with_fallback(locale);

  return Glib::ustring();
}

Glib::ustring Document::get_table_title_singular_original(const Glib::ustring& table_name) const
{
  const std::shared_ptr<const DocumentTableInfo> doctableinfo = get_table_info(table_name);
  if(doctableinfo && doctableinfo->m_info)
    return doctableinfo->m_info->get_title_singular_original();

  return Glib::ustring();
}

void Document::set_table_title(const Glib::ustring& table_name, const Glib::ustring& value, const Glib::ustring& locale)
{
  //std::cout << "debug: " << G_STRFUNC << ": table_name=" << table_name << ", value=" << value << std::endl;
  if(!table_name.empty())
  {
    const auto info = get_table_info(table_name);
    if(info && info->m_info && info->m_info->get_title(locale) != value)
    {
      info->m_info->set_title(value, locale);
      set_modified();
    }
  }
}

//TODO: Avoid doing this all in one go, because that leaves all the data in memory at once.
void Document::set_table_example_data(const Glib::ustring& table_name, const type_example_rows& rows)
{
  if(!table_name.empty())
  {
    const auto info = get_table_info_with_add(table_name);
    if(info && info->m_example_rows != rows)
    {
      info->m_example_rows = rows;
      set_modified();
    }
  }
}

//TODO: Avoid doing this all in one go, because that leaves all the data in memory at once.
Document::type_example_rows Document::get_table_example_data(const Glib::ustring& table_name) const
{
  const std::shared_ptr<const DocumentTableInfo> doctableinfo = get_table_info(table_name);
  if(doctableinfo)
   return doctableinfo->m_example_rows;

  return type_example_rows();
}

bool Document::get_table_is_known(const Glib::ustring& table_name) const
{
  auto iterFind = m_tables.find(table_name);
  return (iterFind != m_tables.end());
}

bool Document::get_table_is_hidden(const Glib::ustring& table_name) const
{
  const std::shared_ptr<const DocumentTableInfo> doctableinfo = get_table_info(table_name);
  if(doctableinfo && doctableinfo->m_info)
      return doctableinfo->m_info->get_hidden();

  return false; //It's not even known.
}

AppState::userlevels Document::get_userlevel() const
{
  userLevelReason reason;
  return get_userlevel(reason);
}

AppState::userlevels Document::get_userlevel(userLevelReason& reason) const
{
  //Initialize output parameter:
  reason = userLevelReason::UNKNOWN;

  if(get_read_only())
  {
    reason = userLevelReason::FILE_READ_ONLY;
    return AppState::userlevels::OPERATOR; //A read-only document cannot be changed, so there's no point in being in developer mode. This is one way to control the user level on purpose.
  }
  else if(get_opened_from_browse())
  {
    reason = userLevelReason::OPENED_FROM_BROWSE;
    return AppState::userlevels::OPERATOR; //Developer mode would require changes to the original document.
  }
  else if(m_file_uri.empty()) //If it has never been saved then this is a new default document, so the user created it, so the user can be a developer.
  {
    return AppState::userlevels::DEVELOPER;
  }
  else
  {
    return m_app_state.get_userlevel();
  }
}

Document::type_signal_userlevel_changed Document::signal_userlevel_changed()
{
  return m_signal_userlevel_changed;
}

void Document::on_app_state_userlevel_changed(AppState::userlevels userlevel)
{
  m_signal_userlevel_changed.emit(userlevel);
}

bool Document::set_userlevel(AppState::userlevels userlevel)
{
  //Prevent incorrect user level:
  if((userlevel == AppState::userlevels::DEVELOPER) && get_read_only())
  {
    std::cout << "debug: " << G_STRFUNC << ": Developer mode denied because get_read_only() returned true." << std::endl;
    std::cout << "  DEBUG: get_read_only()=" << get_read_only() << std::endl;
    std::cout << "  DEBUG: get_file_uri()=" << get_file_uri() << std::endl;

    m_app_state.set_userlevel(AppState::userlevels::OPERATOR);
    return false;
  }
  else if(get_opened_from_browse())
  {
    m_app_state.set_userlevel(AppState::userlevels::OPERATOR);
    return false;
  }

  {
    m_app_state.set_userlevel(userlevel);
    return true;
  }
}

void Document::emit_userlevel_changed()
{
  m_signal_userlevel_changed.emit(m_app_state.get_userlevel());
}

Glib::ustring Document::get_active_layout_platform() const
{
  return m_active_layout_platform;
}

void Document::set_active_layout_platform(const Glib::ustring& layout_platform)
{
  m_active_layout_platform = layout_platform;
}

Glib::ustring Document::get_default_table() const
{
  for(const auto& table_pair : m_tables)
  {
    const std::shared_ptr<const DocumentTableInfo> doctableinfo = table_pair.second;
    if(!doctableinfo)
      continue;

    const std::shared_ptr<const TableInfo> info = doctableinfo->m_info;
    if(info && info->get_default())
      return info->get_name();
  }

  //If there is only one table then pretend that is the default:
  if(m_tables.size() == 1)
  {
    auto iter = m_tables.begin();

    const std::shared_ptr<const DocumentTableInfo> doctableinfo = iter->second;
    if(doctableinfo && doctableinfo->m_info)
      return doctableinfo->m_info->get_name();
  }

  return Glib::ustring();
}

Glib::ustring Document::get_first_table() const
{
  if(m_tables.empty())
    return Glib::ustring();

  auto iter = m_tables.begin();
  const std::shared_ptr<const DocumentTableInfo> doctableinfo = iter->second;
  if(doctableinfo && doctableinfo->m_info)
    return doctableinfo->m_info->get_name();

  return Glib::ustring();
}

void Document::set_allow_autosave(bool value)
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

void Document::save_changes()
{
  //Save changes automatically
  //(when in developer mode - no changes should even be possible when not in developer mode)
  if(get_userlevel() == AppState::userlevels::DEVELOPER)
  {
    //This rebuilds the whole XML DOM and saves the whole document,
    //so we need to be careful not to call set_modified() too often.

    auto test = save_before();
    if(test)
    {
      //std::cout << "debug: " << G_STRFUNC << ": calling write_to_disk()." << std::endl;
      test = write_to_disk();
      if(test)
      {
        set_modified(false);
      }
    }
  }
  else
  {
    //std::cout << "debug: " << G_STRFUNC << ": Not saving, because not AppState::userlevels::DEVELOPER" << std::endl;
  }
}

void Document::set_modified(bool value)
{
  //std::cout << "Document::set_modified()" << std::endl;

  if(value && m_block_modified_set) //For instance, don't save changes while loading.
  {
    //std::cout << "  Document::set_modified() m_block_modified_set" << std::endl;
    return;
  }

  if(get_userlevel() != AppState::userlevels::DEVELOPER)
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
    GlomBakery::Document_XML::set_modified(value);

    if(value)
    {
      //std::cout << "  Document::set_modified() save_changes" << std::endl;

      //TODO: Combine m_allow_auto_save and m_block_modified_set?
      if(!m_allow_auto_save) //For instance, don't save changes while making many changes.
        return;

      save_changes();
    }
  //}
}

void Document::load_after_layout_item_formatting(const xmlpp::Element* element, const std::shared_ptr<LayoutItem_WithFormatting>& layout_item, const Glib::ustring& table_name)
{
  if(!layout_item)
    return;

  Formatting& format = layout_item->m_formatting;

  auto field = std::dynamic_pointer_cast<LayoutItem_Field>(layout_item);

  Field::glom_field_type field_type = Field::glom_field_type::INVALID;
  if(field)
    field_type = field->get_glom_type();

  Glib::ustring field_name;
  if(field)
    field_name = field->get_name();

  load_after_layout_item_formatting(element, format, field_type, table_name, field_name);
}

void Document::load_after_layout_item_formatting(const xmlpp::Element* element, Formatting& format, Field::glom_field_type field_type, const Glib::ustring& table_name, const Glib::ustring& field_name)
{
  //Numeric formatting:
  if(!field_name.empty() && (field_type == Field::glom_field_type::NUMERIC))
  {
    format.m_numeric_format.m_use_thousands_separator = XmlUtils::get_node_attribute_value_as_bool(element, GLOM_ATTRIBUTE_FORMAT_THOUSANDS_SEPARATOR);
    format.m_numeric_format.m_decimal_places_restricted = XmlUtils::get_node_attribute_value_as_bool(element, GLOM_ATTRIBUTE_FORMAT_DECIMAL_PLACES_RESTRICTED);
    format.m_numeric_format.m_decimal_places = XmlUtils::get_node_attribute_value_as_decimal(element, GLOM_ATTRIBUTE_FORMAT_DECIMAL_PLACES);
    format.m_numeric_format.m_currency_symbol = XmlUtils::get_node_attribute_value(element, GLOM_ATTRIBUTE_FORMAT_CURRENCY_SYMBOL);
    format.m_numeric_format.m_alt_foreground_color_for_negatives =
      XmlUtils::get_node_attribute_value_as_bool(element, GLOM_ATTRIBUTE_FORMAT_USE_ALT_NEGATIVE_COLOR);
  }

  //Text formatting:
  if(field_type == Field::glom_field_type::TEXT)
  {
    format.set_text_format_multiline( XmlUtils::get_node_attribute_value_as_bool(element, GLOM_ATTRIBUTE_FORMAT_TEXT_MULTILINE) );
    format.set_text_format_multiline_height_lines( XmlUtils::get_node_attribute_value_as_decimal(element, 
      GLOM_ATTRIBUTE_FORMAT_TEXT_MULTILINE_HEIGHT_LINES, GLOM_ATTRIBUTE_FORMAT_TEXT_MULTILINE_HEIGHT_LINES_DEFAULT) );
  }

  format.set_text_format_font( XmlUtils::get_node_attribute_value (element, GLOM_ATTRIBUTE_FORMAT_TEXT_FONT) );
  format.set_text_format_color_foreground( XmlUtils::get_node_attribute_value (element, GLOM_ATTRIBUTE_FORMAT_TEXT_COLOR_FOREGROUND) );
  format.set_text_format_color_background( XmlUtils::get_node_attribute_value (element, GLOM_ATTRIBUTE_FORMAT_TEXT_COLOR_BACKGROUND) );

  //Alignment. Not-specified means auto.
  Formatting::HorizontalAlignment alignment = Formatting::HorizontalAlignment::AUTO;
  const auto alignment_str = XmlUtils::get_node_attribute_value (element, GLOM_ATTRIBUTE_FORMAT_HORIZONTAL_ALIGNMENT);
  if(alignment_str == GLOM_ATTRIBUTE_FORMAT_HORIZONTAL_ALIGNMENT_LEFT)
    alignment = Formatting::HorizontalAlignment::LEFT;
  else if(alignment_str == GLOM_ATTRIBUTE_FORMAT_HORIZONTAL_ALIGNMENT_RIGHT)
    alignment = Formatting::HorizontalAlignment::RIGHT;

  format.set_horizontal_alignment(alignment);

  //Choices:
  if(!field_name.empty())
  {
    format.set_choices_restricted(
      XmlUtils::get_node_attribute_value_as_bool(element, GLOM_ATTRIBUTE_FORMAT_CHOICES_RESTRICTED),
      XmlUtils::get_node_attribute_value_as_bool(element, GLOM_ATTRIBUTE_FORMAT_CHOICES_RESTRICTED_AS_RADIO_BUTTONS) );
    format.set_has_custom_choices( XmlUtils::get_node_attribute_value_as_bool(element, GLOM_ATTRIBUTE_FORMAT_CHOICES_CUSTOM) );

    if(format.get_has_custom_choices())
    {
      const auto nodeChoiceList = XmlUtils::get_node_child_named(element, GLOM_ATTRIBUTE_FORMAT_CHOICES_CUSTOM_LIST);
      if(nodeChoiceList)
      {
        Formatting::type_list_values list_values;

        for(const auto& node_choices : nodeChoiceList->get_children(GLOM_NODE_FORMAT_CUSTOM_CHOICE))
        {
          const auto element_custom_choices = dynamic_cast<const xmlpp::Element*>(node_choices);
          if(element_custom_choices)
          {
            if(field_type == Field::glom_field_type::INVALID)
            {
              //Discover the field type, so we can interpret the text as a value.
              //Not all calling functions know this, so they don't all supply the correct value.
              //TODO_Performance.
              const std::shared_ptr<const Field> field_temp = get_field(table_name, field_name);
              if(field_temp)
                field_type = field_temp->get_glom_type();
            }

            auto value = std::make_shared<ChoiceValue>();
            load_after_choicevalue(element_custom_choices, value, field_type);
            list_values.push_back(value);
          }
        }

        format.set_choices_custom(list_values);
      }
    }

    format.set_has_related_choices( XmlUtils::get_node_attribute_value_as_bool(element, GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED) );

    const auto relationship_name = XmlUtils::get_node_attribute_value(element, GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_RELATIONSHIP);
    if(!relationship_name.empty())
    {
      std::shared_ptr<const Relationship> relationship = get_relationship(table_name, relationship_name);

      auto show_all = XmlUtils::get_node_attribute_value_as_bool(element, GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_SHOW_ALL);
      if(get_document_format_version() < 6)
      {
        show_all = true; //This was the behaviour before this checkbox existed.
      }

      const auto field_first = XmlUtils::get_node_attribute_value(element, GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_FIELD);
      auto layout_field_first = std::make_shared<LayoutItem_Field>();
      layout_field_first->set_name(field_first);

      std::shared_ptr<LayoutGroup> extra_layouts;

      //Previous versions just saved a single extra field name, instead of a whole set of layoutgroups:
      if(m_document_format_version < 6)
      {
        //The deprecated way:
        const auto field_second = XmlUtils::get_node_attribute_value(element, GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_SECOND);
        if(!field_second.empty())
        {
          extra_layouts = std::make_shared<LayoutGroup>();
          auto item = std::make_shared<LayoutItem_Field>();
          item->set_name(field_second);
          extra_layouts->add_item(item);
        }
      }
      else
      {
        //Get the extra layout for related choices:
        auto nodeExtraLayout = XmlUtils::get_node_child_named(element, GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_EXTRA_LAYOUT);
        if(nodeExtraLayout)
        {
          auto nodeGroups = XmlUtils::get_node_child_named(nodeExtraLayout, GLOM_NODE_DATA_LAYOUT_GROUPS);
          if(nodeGroups)
          {

            auto layout_group = std::make_shared<LayoutGroup>();
            load_after_layout_group(nodeGroups, relationship->get_to_table(), layout_group);
            if(layout_group && !(layout_group->m_list_items.empty()))
            {
              //We actually want the sub-group:
              extra_layouts = std::dynamic_pointer_cast<LayoutGroup>( layout_group->m_list_items[0] );
            }
          }
        }
      }

      //Sort fields:
      Formatting::type_list_sort_fields sort_fields;
      auto elementSortBy = XmlUtils::get_node_child_named(element, GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_SORTBY);
      if(elementSortBy)
      {
        load_after_sort_by(elementSortBy, table_name, sort_fields);
      }

      format.set_choices_related(relationship,
        layout_field_first, extra_layouts, sort_fields,
        show_all);

      //Full details are updated in filled-in ().
    }
  }
}

void Document::load_after_layout_item_usesrelationship(const xmlpp::Element* element, const Glib::ustring& table_name, const std::shared_ptr<UsesRelationship>& item)
{
  if(!element || !item)
    return;

  const auto relationship_name = XmlUtils::get_node_attribute_value(element, GLOM_ATTRIBUTE_RELATIONSHIP_NAME);
  std::shared_ptr<Relationship> relationship;
  if(!relationship_name.empty())
  {
    //std::cout << "  debug in : table_name=" << table_name << ", relationship_name=" << relationship_name << std::endl;
    relationship = get_relationship(table_name, relationship_name);
    item->set_relationship(relationship);

    if(!relationship)
    {
      std::cerr << G_STRFUNC << ": relationship not found: " << relationship_name << ", in table:" << table_name << std::endl;
    }
  }

  const auto related_relationship_name = XmlUtils::get_node_attribute_value(element, GLOM_ATTRIBUTE_RELATED_RELATIONSHIP_NAME);
  if(!related_relationship_name.empty() && relationship)
  {
    auto related_relationship = get_relationship(relationship->get_to_table(), related_relationship_name);
    if(!related_relationship)
      std::cerr << G_STRFUNC << ": related relationship not found in table=" << relationship->get_to_table() << ",  name=" << related_relationship_name << std::endl;

    item->set_related_relationship(related_relationship);
  }
}

void Document::load_after_layout_item_field(const xmlpp::Element* element, const Glib::ustring& table_name, const std::shared_ptr<LayoutItem_Field>& item)
{
  const auto name = XmlUtils::get_node_attribute_value(element, GLOM_ATTRIBUTE_NAME);
  item->set_name(name);

  load_after_layout_item_usesrelationship(element, table_name, item);

  //Needed to decide what formatting to load/save:
  const std::shared_ptr<const Field> field = get_field(item->get_table_used(table_name), name);
  
  // This is not unusual, because tables often refer to tables that have not been loaded yet.
  // Code should sometimes check this before returning the layout items.
  //
  //if(!field)
  //{
  //  std::cerr << G_STRFUNC << ": Could not find field details for field=" << name << ", table=" << table_name << std::endl;
  //}
  item->set_full_field_details(field);

  item->set_editable( XmlUtils::get_node_attribute_value_as_bool(element, GLOM_ATTRIBUTE_EDITABLE) );

  item->set_formatting_use_default( XmlUtils::get_node_attribute_value_as_bool(element, GLOM_ATTRIBUTE_DATA_LAYOUT_ITEM_FIELD_USE_DEFAULT_FORMATTING) );


  const auto nodeCustomTitle = XmlUtils::get_node_child_named(element, GLOM_NODE_LAYOUT_ITEM_CUSTOM_TITLE);
  if(nodeCustomTitle)
  {
    auto custom_title = std::make_shared<CustomTitle>();
    custom_title->set_use_custom_title( XmlUtils::get_node_attribute_value_as_bool(nodeCustomTitle, GLOM_ATTRIBUTE_LAYOUT_ITEM_CUSTOM_TITLE_USE) );

    load_after_translations(nodeCustomTitle, custom_title);
    item->set_title_custom(custom_title);
  }
}

void Document::load_after_sort_by(const xmlpp::Element* node, const Glib::ustring& table_name, LayoutItem_GroupBy::type_list_sort_fields& list_fields)
{
  list_fields.clear();

  if(!node)
    return;

  for(const auto& node_item_field : node->get_children(GLOM_NODE_DATA_LAYOUT_ITEM_FIELD))
  {
    const auto element = dynamic_cast<const xmlpp::Element*>(node_item_field);
    if(element)
    {
      auto item = std::make_shared<LayoutItem_Field>();
      //item.set_full_field_details_empty();
      load_after_layout_item_field(element, table_name, item);
      item->set_full_field_details( get_field(item->get_table_used(table_name), item->get_name()) );

      const auto ascending = XmlUtils::get_node_attribute_value_as_bool(element, GLOM_ATTRIBUTE_SORT_ASCENDING);

      list_fields.push_back( LayoutItem_GroupBy::type_pair_sort_field(item, ascending) );
    }
  }
}

void Document::load_after_layout_group(const xmlpp::Element* node, const Glib::ustring& table_name, const std::shared_ptr<LayoutGroup>& group, bool with_print_layout_positions)
{
  if(!node || !group)
  {
    //std::cerr << G_STRFUNC << ": node is NULL" << std::endl;
    return;
  }

  //Get the group details:
  group->set_name( XmlUtils::get_node_attribute_value(node, GLOM_ATTRIBUTE_NAME) );
  group->set_title_original( XmlUtils::get_node_attribute_value(node, GLOM_ATTRIBUTE_TITLE) );
  group->set_columns_count(
    XmlUtils::get_node_attribute_value_as_decimal(node, GLOM_ATTRIBUTE_COLUMNS_COUNT, 1)); //default to 1, because 0 is meaningless.
  group->set_border_width( XmlUtils::get_node_attribute_value_as_decimal_double(node, GLOM_ATTRIBUTE_BORDER_WIDTH) );

  //Translations:
  std::shared_ptr<LayoutGroup> temp = group;
  load_after_translations(node, temp);

  //Get the child items:
  for(const auto& node_child : node->get_children())
  {
    std::shared_ptr<LayoutItem> item_added;

    //Create the layout item:
    const auto element = dynamic_cast<const xmlpp::Element*>(node_child);
    if(element)
    {
      if(element->get_name() == GLOM_NODE_DATA_LAYOUT_ITEM_FIELD)
      {
        auto item = std::make_shared<LayoutItem_Field>();
        //item.set_full_field_details_empty();
        load_after_layout_item_field(element, table_name, item);

        item_added = item;
      }
      else if(element->get_name() == GLOM_NODE_DATA_LAYOUT_BUTTON)
      {
        auto item = std::make_shared<LayoutItem_Button>();

        item->set_script( XmlUtils::get_child_text_node(element, GLOM_NODE_BUTTON_SCRIPT) );
        if(!(item->get_has_script())) //Try the deprecated attribute instead
           item->set_script( XmlUtils::get_node_attribute_value(element, GLOM_DEPRECATED_ATTRIBUTE_BUTTON_SCRIPT) );

        load_after_translations(element, item);

        item_added = item;
      }
      else if(element->get_name() == GLOM_NODE_DATA_LAYOUT_TEXTOBJECT)
      {
        auto item = std::make_shared<LayoutItem_Text>();
        load_after_translations(element, item);

        //The text can be translated too, so it has its own node:
        const auto element_text = XmlUtils::get_node_child_named(element, GLOM_NODE_DATA_LAYOUT_TEXTOBJECT_TEXT);
        if(element_text)
        {
          auto translatable_text = std::make_shared<StaticText>();
          load_after_translations(element_text, translatable_text);
          item->m_text = translatable_text;
          //std::cout << "  DEBUG: text: " << item->m_text->get_title_or_name() << std::endl;
        }

        item_added = item;
      }
      else if(element->get_name() == GLOM_NODE_DATA_LAYOUT_IMAGEOBJECT)
      {
        auto item = std::make_shared<LayoutItem_Image>();
        load_after_translations(element, item);

        Gnome::Gda::Value value_image;
        const auto nodeValue = XmlUtils::get_node_child_named(element, GLOM_NODE_VALUE);
        if(nodeValue)
        {
          value_image = XmlUtils::get_node_text_child_as_value(nodeValue, Field::glom_field_type::IMAGE);
        }

        if(value_image.is_null())
        {
          //Try the deprecated way:
          value_image = XmlUtils::get_node_attribute_value_as_value(element, GLOM_ATTRIBUTE_DATA_LAYOUT_IMAGEOBJECT_IMAGE, Field::glom_field_type::IMAGE);
        }

        item->set_image(value_image);

        item_added = item;
      }
      else if(element->get_name() == GLOM_NODE_DATA_LAYOUT_LINE)
      {
        auto item = std::make_shared<LayoutItem_Line>();
        //Has no translations: load_after_translations(element, item);

        item->set_coordinates(
          XmlUtils::get_node_attribute_value_as_decimal_double(element, GLOM_ATTRIBUTE_DATA_LAYOUT_LINE_START_X),
          XmlUtils::get_node_attribute_value_as_decimal_double(element, GLOM_ATTRIBUTE_DATA_LAYOUT_LINE_START_Y),
          XmlUtils::get_node_attribute_value_as_decimal_double(element, GLOM_ATTRIBUTE_DATA_LAYOUT_LINE_END_X),
          XmlUtils::get_node_attribute_value_as_decimal_double(element, GLOM_ATTRIBUTE_DATA_LAYOUT_LINE_END_Y) );
          
        item->set_line_width(
          XmlUtils::get_node_attribute_value_as_decimal_double(element, GLOM_ATTRIBUTE_DATA_LAYOUT_LINE_WIDTH) );
          
        item->set_line_color(
          XmlUtils::get_node_attribute_value(element, GLOM_ATTRIBUTE_DATA_LAYOUT_LINE_COLOR) );

        item_added = item;
      }
      else if(element->get_name() == GLOM_NODE_DATA_LAYOUT_ITEM_FIELDSUMMARY)
      {
        auto item = std::make_shared<LayoutItem_FieldSummary>();
        //item.set_full_field_details_empty();
        load_after_layout_item_field(element, table_name, item);
        item->set_full_field_details( get_field(item->get_table_used(table_name), item->get_name()) );
        item->set_summary_type_from_sql( XmlUtils::get_node_attribute_value(element, GLOM_ATTRIBUTE_LAYOUT_ITEM_FIELDSUMMARY_SUMMARYTYPE) );

        item_added = item;
      }
      else if(element->get_name() == GLOM_NODE_DATA_LAYOUT_ITEM_HEADER)
      {
        auto child_group = std::make_shared<LayoutItem_Header>();
        //Recurse:
        load_after_layout_group(element, table_name, child_group, with_print_layout_positions);
        item_added = child_group;
      }
      else if(element->get_name() == GLOM_NODE_DATA_LAYOUT_ITEM_FOOTER)
      {
        auto child_group = std::make_shared<LayoutItem_Footer>();
        //Recurse:
        load_after_layout_group(element, table_name, child_group, with_print_layout_positions);
        item_added = child_group;
      }
      else if(element->get_name() == GLOM_NODE_DATA_LAYOUT_GROUP)
      {
        auto child_group = std::make_shared<LayoutGroup>();
        //Recurse:
        load_after_layout_group(element, table_name, child_group, with_print_layout_positions);
        item_added = child_group;
      }
      else if(element->get_name() == GLOM_NODE_DATA_LAYOUT_NOTEBOOK)
      {
        auto notebook = std::make_shared<LayoutItem_Notebook>();
        load_after_layout_group(element, table_name, notebook, with_print_layout_positions);
        item_added = notebook;
      }
      else if( (element->get_name() == GLOM_NODE_DATA_LAYOUT_PORTAL) || (element->get_name() == GLOM_NODE_DATA_LAYOUT_CALENDAR_PORTAL) )
      {
        std::shared_ptr<LayoutItem_Portal> portal;
        std::shared_ptr<LayoutItem_CalendarPortal> calendar_portal;

        if(element->get_name() == GLOM_NODE_DATA_LAYOUT_PORTAL)
          portal = std::make_shared<LayoutItem_Portal>();
        else if(element->get_name() == GLOM_NODE_DATA_LAYOUT_CALENDAR_PORTAL)
        {
          calendar_portal = std::make_shared<LayoutItem_CalendarPortal>();
          portal = calendar_portal;
        }

        load_after_layout_item_usesrelationship(element, table_name, portal);

        auto elementNavigationRelationshipSpecific = XmlUtils::get_node_child_named(element, GLOM_NODE_DATA_LAYOUT_PORTAL_NAVIGATIONRELATIONSHIP);
        if(elementNavigationRelationshipSpecific)
        {
          const Glib::ustring navigation_type_as_string =
            XmlUtils::get_node_attribute_value(elementNavigationRelationshipSpecific,
            GLOM_ATTRIBUTE_PORTAL_NAVIGATION_TYPE);
          if(navigation_type_as_string.empty() ||
             navigation_type_as_string == GLOM_ATTRIBUTE_PORTAL_NAVIGATION_TYPE_AUTOMATIC)
          {
            portal->set_navigation_type(LayoutItem_Portal::navigation_type::AUTOMATIC);
          }
          else if(navigation_type_as_string == GLOM_ATTRIBUTE_PORTAL_NAVIGATION_TYPE_NONE)
          {
            portal->set_navigation_type(LayoutItem_Portal::navigation_type::NONE);
          }
          else if(navigation_type_as_string == GLOM_ATTRIBUTE_PORTAL_NAVIGATION_TYPE_SPECIFIC)
          {
            //Read the specified relationship name:
            auto relationship_navigation_specific = std::make_shared<UsesRelationship>();
            load_after_layout_item_usesrelationship(elementNavigationRelationshipSpecific, portal->get_table_used(table_name), relationship_navigation_specific);
            portal->set_navigation_relationship_specific(relationship_navigation_specific);
          }
        }

        load_after_layout_group(element, portal->get_table_used(table_name), portal, with_print_layout_positions);

        //Get the calendar portal's date field:
        if(calendar_portal)
        {
          const auto date_field_name = XmlUtils::get_node_attribute_value(element, GLOM_ATTRIBUTE_PORTAL_CALENDAR_DATE_FIELD);
          auto date_field = get_field(calendar_portal->get_table_used(table_name), date_field_name);
          calendar_portal->set_date_field(date_field);
        }

        if(!calendar_portal)
        {
          const gulong rows_count_min = 
            XmlUtils::get_node_attribute_value_as_decimal_double(element, 
              GLOM_ATTRIBUTE_PORTAL_ROWS_COUNT_MIN);
          const gulong rows_count_max = 
            XmlUtils::get_node_attribute_value_as_decimal_double(element, 
              GLOM_ATTRIBUTE_PORTAL_ROWS_COUNT_MAX);
          if(rows_count_min || rows_count_max) //Ignore useless 0, 0 values.
            portal->set_rows_count(rows_count_min, rows_count_max);
            
          //Print Layout specific stuff:
          portal->set_print_layout_row_height(
            XmlUtils::get_node_attribute_value_as_decimal(element, 
              GLOM_ATTRIBUTE_PORTAL_PRINT_LAYOUT_ROW_HEIGHT) );
            
          portal->set_print_layout_row_line_width(
            XmlUtils::get_node_attribute_value_as_decimal(element, 
              GLOM_ATTRIBUTE_PORTAL_PRINT_LAYOUT_ROW_LINE_WIDTH) );
          portal->set_print_layout_column_line_width(
            XmlUtils::get_node_attribute_value_as_decimal(element, 
              GLOM_ATTRIBUTE_PORTAL_PRINT_LAYOUT_COLUMN_LINE_WIDTH) );
          portal->set_print_layout_line_color(
            XmlUtils::get_node_attribute_value(element, 
              GLOM_ATTRIBUTE_PORTAL_PRINT_LAYOUT_LINE_COLOR) );
        }
                
        item_added = portal;
      }
      else if(element->get_name() == GLOM_NODE_DATA_LAYOUT_ITEM_GROUPBY)
      {
        auto child_group = std::make_shared<LayoutItem_GroupBy>();
        //Recurse:
        load_after_layout_group(element, table_name, child_group, with_print_layout_positions);

        //Group-By field:
        auto field_groupby = std::make_shared<LayoutItem_Field>();
        auto elementGroupBy = XmlUtils::get_node_child_named(element, GLOM_NODE_REPORT_ITEM_GROUPBY_GROUPBY);
        if(elementGroupBy)
        {
          load_after_layout_item_field(elementGroupBy, table_name, field_groupby);
          field_groupby->set_full_field_details( get_field(field_groupby->get_table_used(table_name), field_groupby->get_name()) );
        }
        child_group->set_field_group_by(field_groupby);


        //field_groupby.set_full_field_details_empty();

        //Sort fields:
        auto elementSortBy = XmlUtils::get_node_child_named(element, GLOM_NODE_REPORT_ITEM_GROUPBY_SORTBY);
        if(elementSortBy)
        {
          LayoutItem_GroupBy::type_list_sort_fields sort_fields;
          load_after_sort_by(elementSortBy, table_name, sort_fields);
          child_group->set_fields_sort_by(sort_fields);
        }

        //Secondary fields:
        auto elementSecondary = XmlUtils::get_node_child_named(element, GLOM_NODE_DATA_LAYOUT_GROUP_SECONDARYFIELDS);
        if(elementSecondary)
        {
          auto elementGroup = XmlUtils::get_node_child_named(elementSecondary, GLOM_NODE_DATA_LAYOUT_GROUP);
          if(elementGroup)
          {
            load_after_layout_group(elementGroup, table_name, child_group->get_secondary_fields(), with_print_layout_positions);
            fill_layout_field_details(table_name, child_group->get_secondary_fields()); //Get full field details from the field names.
          }
        }

        item_added = child_group;
      }
      else if(element->get_name() == GLOM_NODE_DATA_LAYOUT_ITEM_VERTICALGROUP)
      {
        auto child_group = std::make_shared<LayoutItem_VerticalGroup>();
        //Recurse:
        load_after_layout_group(element, table_name, child_group, with_print_layout_positions);

        item_added = child_group;
      }
      else if(element->get_name() == GLOM_NODE_DATA_LAYOUT_ITEM_SUMMARY)
      {
        auto child_group = std::make_shared<LayoutItem_Summary>();
        //Recurse:
        load_after_layout_group(element, table_name, child_group, with_print_layout_positions);

        item_added = child_group;
      }
    }

    //Load formatting for any layout type that uses it:
    auto withformatting = std::dynamic_pointer_cast<LayoutItem_WithFormatting>(item_added);
    if(withformatting)
    {
       const auto elementFormatting = XmlUtils::get_node_child_named(element, GLOM_NODE_FORMAT);
       if(elementFormatting)
       {
         //TODO: Provide the name of the relationship's table if there is a relationship:
         load_after_layout_item_formatting(elementFormatting, withformatting, table_name);
       }
    }

    //Add the new layout item to the group:
    if(item_added)
    {
      group->add_item(item_added);

      //Attributes that all items could have:
      item_added->set_display_width( XmlUtils::get_node_attribute_value_as_decimal(element, GLOM_ATTRIBUTE_LAYOUT_ITEM_COLUMN_WIDTH) );

      if(with_print_layout_positions)
        load_after_print_layout_position(element, item_added);
    }
  } //for
}

void Document::load_after_translations(const xmlpp::Element* element, const std::shared_ptr<TranslatableItem>& item)
{
  if(!element)
    return;

  const auto choicevalue = std::dynamic_pointer_cast<ChoiceValue>(item);
  if(!choicevalue) //This item does not use the title, but uses the title translations to translate its value, if it is of type text.
  {
    item->set_title_original( XmlUtils::get_node_attribute_value(element, GLOM_ATTRIBUTE_TITLE));
  }

  const auto nodeTranslations = XmlUtils::get_node_child_named(element, GLOM_NODE_TRANSLATIONS_SET);
  if(nodeTranslations)
  {
    for(const auto& node_translation : nodeTranslations->get_children(GLOM_NODE_TRANSLATION))
    {
      const auto element_translation = dynamic_cast<const xmlpp::Element*>(node_translation);
      if(element_translation)
      {
        const auto locale = XmlUtils::get_node_attribute_value(element_translation, GLOM_ATTRIBUTE_TRANSLATION_LOCALE);
        const auto translation = XmlUtils::get_node_attribute_value(element_translation, GLOM_ATTRIBUTE_TRANSLATION_VALUE);
        item->set_title(translation, locale);

        //Remember any new translation locales in our cached list:
        if(std::find(m_translation_available_locales.begin(), 
          m_translation_available_locales.end(), locale) == m_translation_available_locales.end())
        {
          m_translation_available_locales.push_back(locale);
        }
      }
    }
  }

  //If it has a singular title, then load that too:
  const std::shared_ptr<HasTitleSingular> has_title_singular =
     std::dynamic_pointer_cast<HasTitleSingular>(item);
  if(has_title_singular)
  {
    const auto nodeTitleSingular = XmlUtils::get_node_child_named(element, GLOM_NODE_TABLE_TITLE_SINGULAR);

    if(!has_title_singular->m_title_singular)
     has_title_singular->m_title_singular = std::make_shared<TranslatableItem>();

    load_after_translations(nodeTitleSingular, has_title_singular->m_title_singular);
  }
}

void Document::load_after_print_layout_position(const xmlpp::Element* nodeItem, const std::shared_ptr<LayoutItem>& item)
{
  if(!nodeItem)
    return;

  const auto child = XmlUtils::get_node_child_named(nodeItem, GLOM_NODE_POSITION);
  if(child)
  {
    const auto x = XmlUtils::get_node_attribute_value_as_decimal_double(child, GLOM_ATTRIBUTE_POSITION_X);
    const auto y = XmlUtils::get_node_attribute_value_as_decimal_double(child, GLOM_ATTRIBUTE_POSITION_Y);
    const auto width = XmlUtils::get_node_attribute_value_as_decimal_double(child, GLOM_ATTRIBUTE_POSITION_WIDTH);
    const auto height = XmlUtils::get_node_attribute_value_as_decimal_double(child, GLOM_ATTRIBUTE_POSITION_HEIGHT);
    item->set_print_layout_position(x, y, width, height);
  }
}

void Document::load_after_choicevalue(const xmlpp::Element* element, const std::shared_ptr<ChoiceValue>& item, Field::glom_field_type field_type)
{
  const auto value = XmlUtils::get_node_attribute_value_as_value(element, GLOM_ATTRIBUTE_VALUE, field_type);
  item->set_value(value);

  std::shared_ptr<ChoiceValue> nonconst_item = item; //TODO: Avoid this.
  load_after_translations(element, nonconst_item);
}

bool Document::load_after(int& failure_code)
{
  //Initialize the output variable:
  failure_code = 0;

  //TODO: Use some callback UI to show a busy cursor?
  /*
  //Use a std::shared_ptr<> to avoid even unncessarily instantiating a BusyCursor,
  //which would require GTK+ to be initialized:
  std::shared_ptr<BusyCursor> auto_cursor;
  if(m_parent_window)
    auto_cursor = std::shared_ptr<BusyCursor>( new BusyCursor(m_parent_window) );
  */

  m_block_modified_set = true; //Prevent the set_ functions from triggering a save.

  auto result = GlomBakery::Document_XML::load_after(failure_code);

  m_block_cache_update = true; //Don't waste time repeatedly updating this until we have finished.

  if(result)
  {
    m_translation_available_locales.clear();

    const auto nodeRoot = get_node_document();
    if(nodeRoot)
    {
      m_document_format_version = XmlUtils::get_node_attribute_value_as_decimal(nodeRoot, GLOM_ATTRIBUTE_FORMAT_VERSION);

      if(m_document_format_version > get_latest_known_document_format_version())
      {
        std::cerr << G_STRFUNC << ": Loading failed because format_version=" << m_document_format_version << ", but latest known format version is " << get_latest_known_document_format_version() << std::endl;
        failure_code = Utils::to_utype(load_failure_codes::FILE_VERSION_TOO_NEW);
        return false;
      }

      m_is_example = XmlUtils::get_node_attribute_value_as_bool(nodeRoot, GLOM_ATTRIBUTE_IS_EXAMPLE);
      m_is_backup = XmlUtils::get_node_attribute_value_as_bool(nodeRoot, GLOM_ATTRIBUTE_IS_BACKUP);

      load_after_translations(nodeRoot, m_database_title);

      //"database_title" is deprecated in favour of "title", loaded in
      //load_after_translations(), but load this from old documents if
      //if it is present, and the only thing present:
      const Glib::ustring database_title_deprecated = 
        XmlUtils::get_node_attribute_value(nodeRoot, GLOM_DEPRECATED_ATTRIBUTE_CONNECTION_DATABASE_TITLE);
      if(!database_title_deprecated.empty() && get_database_title_original().empty())
        m_database_title->set_title_original(database_title_deprecated);

      m_startup_script = XmlUtils::get_child_text_node(nodeRoot, GLOM_NODE_STARTUP_SCRIPT);

      m_translation_original_locale = XmlUtils::get_node_attribute_value(nodeRoot, GLOM_ATTRIBUTE_TRANSLATION_ORIGINAL_LOCALE);
      m_translation_available_locales.push_back(m_translation_original_locale); //Just a cache.

      const auto nodeConnection = XmlUtils::get_node_child_named(nodeRoot, GLOM_NODE_CONNECTION);
      if(nodeConnection)
      {
        //Connection information:
        m_network_shared = XmlUtils::get_node_attribute_value_as_bool(nodeConnection, GLOM_ATTRIBUTE_CONNECTION_NETWORK_SHARED, false /* default */);

        //Older documents always defaulted to network-sharing with self-hosting.
        if(!m_network_shared && !m_is_example && (get_document_format_version() < 4))
        {
          //Otherwise we would assume that the default user already exists,
          //and fail to ask for the user/password:
          m_network_shared = true;
        }

        m_connection_server = XmlUtils::get_node_attribute_value(nodeConnection, GLOM_ATTRIBUTE_CONNECTION_SERVER);
        m_connection_port = XmlUtils::get_node_attribute_value_as_decimal(nodeConnection, GLOM_ATTRIBUTE_CONNECTION_PORT);
        m_connection_try_other_ports = XmlUtils::get_node_attribute_value_as_bool(nodeConnection, GLOM_ATTRIBUTE_CONNECTION_TRY_OTHER_PORTS, true /* default */);
        m_connection_database = XmlUtils::get_node_attribute_value(nodeConnection, GLOM_ATTRIBUTE_CONNECTION_DATABASE);

        const auto attr_mode = XmlUtils::get_node_attribute_value(nodeConnection, GLOM_ATTRIBUTE_CONNECTION_HOSTING_MODE);

        HostingMode mode = HostingMode::DEFAULT;

        if(attr_mode.empty())
        {
          // If no hosting mode is set, then try the self_hosted flag which
          // was used before sqlite support was implemented.
          const auto self_hosted = XmlUtils::get_node_attribute_value_as_bool(nodeConnection, GLOM_ATTRIBUTE_CONNECTION_SELF_HOSTED);
          mode = self_hosted ? HostingMode::POSTGRES_SELF : HostingMode::POSTGRES_CENTRAL;
        }
        else
        {
          if(attr_mode == GLOM_ATTRIBUTE_CONNECTION_HOSTING_POSTGRES_CENTRAL)
            mode = HostingMode::POSTGRES_CENTRAL;
          else if(attr_mode == GLOM_ATTRIBUTE_CONNECTION_HOSTING_POSTGRES_SELF)
            mode = HostingMode::POSTGRES_SELF;
          else if(attr_mode == GLOM_ATTRIBUTE_CONNECTION_HOSTING_SQLITE)
            mode = HostingMode::SQLITE;
          else if(attr_mode == GLOM_ATTRIBUTE_CONNECTION_HOSTING_MYSQL_CENTRAL)
            mode = HostingMode::MYSQL_CENTRAL;
          else if(attr_mode == GLOM_ATTRIBUTE_CONNECTION_HOSTING_MYSQL_SELF)
            mode = HostingMode::MYSQL_SELF;
          else
	  {
            std::cerr << G_STRFUNC << ": Hosting mode " << attr_mode << " is not supported" << std::endl;
            return false; //TODO: Provide more information so the application (or Bakery) can say exactly why loading failed.
	  }
        }

        m_hosting_mode = mode;
      }

      //Tables:
      m_tables.clear();

      //Look at each "table" node.
      auto list_nodes_tables = nodeRoot->get_children(GLOM_NODE_TABLE);
      for(const auto& node_table : list_nodes_tables)
      {
        auto nodeTable = dynamic_cast<xmlpp::Element*>(node_table);
        if(nodeTable)
        {
          const auto table_name = XmlUtils::get_node_attribute_value(nodeTable, GLOM_ATTRIBUTE_NAME);

          const auto doctableinfo = std::make_shared<DocumentTableInfo>();
          m_tables[table_name] = doctableinfo;

          auto table_info = std::make_shared<TableInfo>();
          table_info->set_name(table_name);
          table_info->set_hidden( XmlUtils::get_node_attribute_value_as_bool(nodeTable, GLOM_ATTRIBUTE_HIDDEN) );
          table_info->set_default( XmlUtils::get_node_attribute_value_as_bool(nodeTable, GLOM_ATTRIBUTE_DEFAULT) );

          doctableinfo->m_info = table_info;

          doctableinfo->m_overviewx = XmlUtils::get_node_attribute_value_as_float(nodeTable, GLOM_ATTRIBUTE_OVERVIEW_X);
          doctableinfo->m_overviewy = XmlUtils::get_node_attribute_value_as_float(nodeTable, GLOM_ATTRIBUTE_OVERVIEW_Y);

          //Translations:
          load_after_translations(nodeTable, doctableinfo->m_info);

          //Relationships:
          //These should be loaded before the fields, because the fields use them.
          const auto nodeRelationships = XmlUtils::get_node_child_named(nodeTable, GLOM_NODE_RELATIONSHIPS);
          if(nodeRelationships)
          {
            for(const auto& item_rel : nodeRelationships->get_children(GLOM_NODE_RELATIONSHIP))
            {
              const auto node_rel = dynamic_cast<xmlpp::Element*>(item_rel);
              if(node_rel)
              {
                auto relationship = std::make_shared<Relationship>();
                const auto relationship_name = XmlUtils::get_node_attribute_value(node_rel, GLOM_ATTRIBUTE_NAME);

                relationship->set_from_table(table_name);
                relationship->set_name(relationship_name);;

                relationship->set_from_field( XmlUtils::get_node_attribute_value(node_rel, GLOM_ATTRIBUTE_KEY) );
                relationship->set_to_table( XmlUtils::get_node_attribute_value(node_rel, GLOM_ATTRIBUTE_OTHER_TABLE) );
                relationship->set_to_field( XmlUtils::get_node_attribute_value(node_rel, GLOM_ATTRIBUTE_OTHER_KEY) );
                relationship->set_auto_create( XmlUtils::get_node_attribute_value_as_bool(node_rel, GLOM_ATTRIBUTE_AUTO_CREATE) );
                relationship->set_allow_edit( XmlUtils::get_node_attribute_value_as_bool(node_rel, GLOM_ATTRIBUTE_ALLOW_EDIT) );

                //Translations:
                load_after_translations(node_rel, relationship);

                doctableinfo->m_relationships.push_back(relationship);
              }
            }
          }

          //Fields:
          const auto nodeFields = XmlUtils::get_node_child_named(nodeTable, GLOM_NODE_FIELDS);
          if(nodeFields)
          {
            const auto type_names = Field::get_type_names();

            //Loop through Field child nodes:
            for(const auto& item_field : nodeFields->get_children(GLOM_NODE_FIELD))
            {
              const auto node_field = dynamic_cast<xmlpp::Element*>(item_field);
              if(node_field)
              {
                auto field = std::make_shared<Field>();

                const auto strName = XmlUtils::get_node_attribute_value(node_field, GLOM_ATTRIBUTE_NAME);
                field->set_name( strName );

                field->set_primary_key( XmlUtils::get_node_attribute_value_as_bool(node_field, GLOM_ATTRIBUTE_PRIMARY_KEY) );
                field->set_unique_key( XmlUtils::get_node_attribute_value_as_bool(node_field, GLOM_ATTRIBUTE_UNIQUE) );
                field->set_auto_increment( XmlUtils::get_node_attribute_value_as_bool(node_field, GLOM_ATTRIBUTE_AUTOINCREMENT) );

                //Get lookup information, if present.
                auto nodeLookup = XmlUtils::get_node_child_named(node_field, GLOM_NODE_FIELD_LOOKUP);
                if(nodeLookup)
                {
                  const auto lookup_relationship_name = XmlUtils::get_node_attribute_value(nodeLookup, GLOM_ATTRIBUTE_RELATIONSHIP_NAME);
                  std::shared_ptr<Relationship> lookup_relationship = get_relationship(table_name, lookup_relationship_name);
                  field->set_lookup_relationship(lookup_relationship);

                  field->set_lookup_field( XmlUtils::get_node_attribute_value(nodeLookup, GLOM_ATTRIBUTE_FIELD) );
                }

                field->set_calculation( XmlUtils::get_child_text_node(node_field, GLOM_NODE_CALCULATION) );
                if(!(field->get_has_calculation())) //Try the deprecated attribute instead
                  field->set_calculation( XmlUtils::get_node_attribute_value(node_field, GLOM_DEPRECATED_ATTRIBUTE_CALCULATION) );

                //Field Type:
                const auto field_type = XmlUtils::get_node_attribute_value(node_field, GLOM_ATTRIBUTE_TYPE);

                //Get the type enum for this string representation of the type:
                Field::glom_field_type field_type_enum = Field::glom_field_type::INVALID;
                for(const auto& type_pair : type_names)
                {
                  if(type_pair.second == field_type)
                  {
                    field_type_enum = type_pair.first;
                    break;
                  }
                }


                //We set this after set_field_info(), because that gets a glom type from the (not-specified) gdatype. Yes, that's strange, and should probably be more explicit.
                field->set_glom_type(field_type_enum);

                field->set_default_value( XmlUtils::get_node_attribute_value_as_value(node_field, GLOM_ATTRIBUTE_DEFAULT_VALUE, field_type_enum) );

                //Default Formatting:
                const auto elementFormatting = XmlUtils::get_node_child_named(node_field, GLOM_NODE_FORMAT);
                if(elementFormatting)
                  load_after_layout_item_formatting(elementFormatting, field->m_default_formatting, field_type_enum, table_name, strName);

                //Translations:
                load_after_translations(node_field, field);

                doctableinfo->m_fields.push_back(field);
              }
            }
          } //Fields

          // Load Example Rows after fields have been loaded, because they
          // need the fields to be able to associate a value to a named field.
          // TODO: Allow this to be loaded progressively from disk later,
          // instead of storing in it all in memory?
          const auto nodeExampleRows = XmlUtils::get_node_child_named(nodeTable, GLOM_NODE_EXAMPLE_ROWS);
          if(nodeExampleRows)
          {
            //Loop through example_row child nodes:
            for(const auto& item_row : nodeExampleRows->get_children(GLOM_NODE_EXAMPLE_ROW))
            {
              const auto node_row = dynamic_cast<xmlpp::Element*>(item_row);
              if(node_row)
              {
                type_row_data field_values(doctableinfo->m_fields.size());
                //Loop through value child nodes
                for(const auto& item_value : node_row->get_children(GLOM_NODE_VALUE))
                {
                  const auto node_value = dynamic_cast<xmlpp::Element*>(item_value);
                  if(node_value)
                  {
                    const auto column_name = node_value->get_attribute(GLOM_ATTRIBUTE_COLUMN);
                    if(column_name)
                    {
                      //std::cout << "DEBUG: column_name = " << column_name->get_value() << " fields size=" << doctableinfo->m_fields.size() << std::endl;

                      // TODO_Performance: If it's too many rows we could
                      // consider a map to find the column more quickly.
                      for(unsigned int i = 0; i < doctableinfo->m_fields.size(); ++i)
                      {
                        std::shared_ptr<const Field> field = doctableinfo->m_fields[i];
                        //std::cout << "  DEBUG: searching: field i=" << i << " =" << field->get_name() << std::endl;
                        if(field && (field->get_name() == column_name->get_value()))
                        {
                          field_values[i] = XmlUtils::get_node_text_child_as_value(node_value, field->get_glom_type());
                          //std::cout << "    DEBUG: document example value: field=" << field->get_name() << ", value=" << field_values[i].to_string() << std::endl;
                          break;
                        }
                      }
                    }
                  }
                }

                // Append line to doctableinfo->m_example_rows
                doctableinfo->m_example_rows.push_back(field_values);
              }
            }
          } // Example Rows

          //std::cout << "  debug: loading: table=" << table_name << ", m_example_rows.size()=" << doctableinfo->m_example_rows.size() << std::endl;

        } //if(table)
      } //Tables.

      //Look at each "table" node.
      //We do load the layouts separately, because we needed to load all the tables' relationships and tables
      //before we can load layouts that can use them.
      for(const auto& node_table : list_nodes_tables)
      {
        auto nodeTable = dynamic_cast<xmlpp::Element*>(node_table);
        if(nodeTable)
        {
          const auto table_name = XmlUtils::get_node_attribute_value(nodeTable, GLOM_ATTRIBUTE_NAME);
          const auto doctableinfo = m_tables[table_name];

          //Layouts:
          const auto nodeDataLayouts = XmlUtils::get_node_child_named(nodeTable, GLOM_NODE_DATA_LAYOUTS);
          if(nodeDataLayouts)
          {
            for(const auto& item : nodeDataLayouts->get_children(GLOM_NODE_DATA_LAYOUT))
            {
              auto node_data_layout = dynamic_cast<xmlpp::Element*>(item);
              if(node_data_layout)
              {
                const auto layout_name = XmlUtils::get_node_attribute_value(node_data_layout, GLOM_ATTRIBUTE_NAME);
                const auto layout_platform = XmlUtils::get_node_attribute_value(node_data_layout, GLOM_ATTRIBUTE_LAYOUT_PLATFORM);

                type_list_layout_groups layout_groups;

                const auto node_group = XmlUtils::get_node_child_named(node_data_layout, GLOM_NODE_DATA_LAYOUT_GROUPS);
                if(node_group)
                {
                  //Look at all its children:
                  for(const auto& item_group : node_group->get_children(GLOM_NODE_DATA_LAYOUT_GROUP))
                  {
                    const auto node_layout_group = dynamic_cast<const xmlpp::Element*>(item_group);
                    if(node_layout_group)
                    {
                      const auto group_name = XmlUtils::get_node_attribute_value(node_layout_group, GLOM_ATTRIBUTE_NAME);
                      if(!group_name.empty())
                      {
                        auto group = std::make_shared<LayoutGroup>();
                        load_after_layout_group(node_layout_group, table_name, group);

                        layout_groups.push_back(group);
                      }
                    }
                  }
                }

                LayoutInfo layout_info;
                layout_info.m_layout_name = layout_name;
                layout_info.m_layout_platform = layout_platform;
                layout_info.m_layout_groups = layout_groups;
                doctableinfo->m_layouts.push_back(layout_info);
              }
            }
          } //if(nodeDataLayouts)


          //Reports:
          const auto nodeReports = XmlUtils::get_node_child_named(nodeTable, GLOM_NODE_REPORTS);
          if(nodeReports)
          {
            for(const auto& item_report : nodeReports->get_children(GLOM_NODE_REPORT))
            {
              auto node_report = dynamic_cast<xmlpp::Element*>(item_report);
              if(node_report)
              {
                const auto report_name = XmlUtils::get_node_attribute_value(node_report, GLOM_ATTRIBUTE_NAME);
                const auto show_table_title = XmlUtils::get_node_attribute_value_as_bool(node_report, GLOM_ATTRIBUTE_REPORT_SHOW_TABLE_TITLE);

                //type_list_layout_groups layout_groups;

                auto report = std::make_shared<Report>();
                report->set_name(report_name);
                report->set_show_table_title(show_table_title);

                const auto node_group = XmlUtils::get_node_child_named(node_report, GLOM_NODE_DATA_LAYOUT_GROUPS);
                if(node_group)
                {
                  //Look at all its children:
                  for(const auto& item_group : node_group->get_children(GLOM_NODE_DATA_LAYOUT_GROUP))
                  {
                    const auto node_layout_group = dynamic_cast<const xmlpp::Element*>(item_group);
                    if(node_layout_group)
                    {
                      std::shared_ptr<LayoutGroup> group = report->get_layout_group();
                      group->remove_all_items();
                      load_after_layout_group(node_layout_group, table_name, group);

                      fill_layout_field_details(table_name, group); //Get full field details from the field names.
                    }
                  }
                }

                //Translations:
                load_after_translations(node_report, report);

                doctableinfo->m_reports[report->get_name()] = report;
              }
            }
          } //if(nodeReports)


          //Print Layouts:
          const auto nodePrintLayouts = XmlUtils::get_node_child_named(nodeTable, GLOM_NODE_PRINT_LAYOUTS);
          if(nodePrintLayouts)
          {
            for(const auto& item_print_layout : nodePrintLayouts->get_children(GLOM_NODE_PRINT_LAYOUT))
            {
              auto node_print_layout = dynamic_cast<xmlpp::Element*>(item_print_layout);
              if(node_print_layout)
              {
                const auto name = XmlUtils::get_node_attribute_value(node_print_layout, GLOM_ATTRIBUTE_NAME);
                const auto show_table_title = XmlUtils::get_node_attribute_value_as_bool(node_print_layout, GLOM_ATTRIBUTE_REPORT_SHOW_TABLE_TITLE);

                auto print_layout = std::make_shared<PrintLayout>();
                print_layout->set_name(name);
                print_layout->set_show_table_title(show_table_title);

                print_layout->set_show_grid(
                  XmlUtils::get_node_attribute_value_as_bool(node_print_layout, GLOM_ATTRIBUTE_PRINT_LAYOUT_SHOW_GRID) );
                print_layout->set_show_rules(
                  XmlUtils::get_node_attribute_value_as_bool(node_print_layout, GLOM_ATTRIBUTE_PRINT_LAYOUT_SHOW_RULES) );
                print_layout->set_show_outlines(
                  XmlUtils::get_node_attribute_value_as_bool(node_print_layout, GLOM_ATTRIBUTE_PRINT_LAYOUT_SHOW_OUTLINES) );

                //Get the horizontal and vertical rules:
                PrintLayout::type_vec_doubles vec_rules_h;
                for(const auto& item_rule : node_print_layout->get_children(GLOM_NODE_HORIZONTAL_RULE))
                {
                  const auto node_rule = dynamic_cast<const xmlpp::Element*>(item_rule);
                  if(!node_rule)
                    continue;

                  const auto pos = XmlUtils::get_node_attribute_value_as_decimal(node_rule, GLOM_ATTRIBUTE_RULE_POSITION);
                  vec_rules_h.push_back(pos);
                }
                print_layout->set_horizontal_rules(vec_rules_h);

                PrintLayout::type_vec_doubles vec_rules_v;
                for(const auto& item_rule : node_print_layout->get_children(GLOM_NODE_VERTICAL_RULE))
                {
                  const auto node_rule = dynamic_cast<const xmlpp::Element*>(item_rule);
                  if(!node_rule)
                    continue;

                  const auto pos = XmlUtils::get_node_attribute_value_as_decimal(node_rule, GLOM_ATTRIBUTE_RULE_POSITION);
                  vec_rules_v.push_back(pos);
                }
                print_layout->set_vertical_rules(vec_rules_v);


                //Page Setup:
                const auto key_file_text = XmlUtils::get_child_text_node(node_print_layout, GLOM_NODE_PAGE_SETUP);
                print_layout->set_page_setup(key_file_text);
                
                print_layout->set_page_count(
                  XmlUtils::get_node_attribute_value_as_decimal(node_print_layout, GLOM_ATTRIBUTE_PRINT_LAYOUT_PAGE_COUNT, 1));
                 

                //Layout Groups:
                const auto nodeGroups = XmlUtils::get_node_child_named(node_print_layout, GLOM_NODE_DATA_LAYOUT_GROUPS);
                if(nodeGroups)
                {
                  //Look at all its children:
                  for(const auto& item_group : nodeGroups->get_children(GLOM_NODE_DATA_LAYOUT_GROUP))
                  {
                    const auto node_layout_group = dynamic_cast<const xmlpp::Element*>(item_group);
                    if(node_layout_group)
                    {
                      std::shared_ptr<LayoutGroup> group = print_layout->get_layout_group();
                      group->remove_all_items();
                      load_after_layout_group(node_layout_group, table_name, group, true /* load positions too. */);

                      fill_layout_field_details(table_name, group); //Get full field details from the field names.
                    }
                  }
                }

                //Translations:
                load_after_translations(node_print_layout, print_layout);

                doctableinfo->m_print_layouts[print_layout->get_name()] = print_layout;
              }
            }
          } //if(nodePrintLayouts)


          //Groups:
          //These are only used when recreating the database, for instance from an example file.
          m_groups.clear();

          const auto nodeGroups = XmlUtils::get_node_child_named(nodeRoot, GLOM_NODE_GROUPS);
          if(nodeGroups)
          {
            for(const auto& item_group : nodeGroups->get_children(GLOM_NODE_GROUP))
            {
              auto node_group = dynamic_cast<xmlpp::Element*>(item_group);
              if(node_group)
              {
                GroupInfo group_info;

                group_info.set_name( XmlUtils::get_node_attribute_value(node_group, GLOM_ATTRIBUTE_NAME) );
                group_info.m_developer = XmlUtils::get_node_attribute_value_as_bool(node_group, GLOM_ATTRIBUTE_DEVELOPER);

                for(const auto& item_priv : node_group->get_children(GLOM_NODE_TABLE_PRIVS))
                {
                  auto node_priv = dynamic_cast<xmlpp::Element*>(item_priv);
                  if(node_priv)
                  {
                    const auto priv_table_name = XmlUtils::get_node_attribute_value(node_priv, GLOM_ATTRIBUTE_TABLE_NAME);

                    Privileges privs;
                    privs.m_view = XmlUtils::get_node_attribute_value_as_bool(node_priv, GLOM_ATTRIBUTE_PRIV_VIEW);
                    privs.m_edit = XmlUtils::get_node_attribute_value_as_bool(node_priv, GLOM_ATTRIBUTE_PRIV_EDIT);
                    privs.m_create = XmlUtils::get_node_attribute_value_as_bool(node_priv, GLOM_ATTRIBUTE_PRIV_CREATE);
                    privs.m_delete = XmlUtils::get_node_attribute_value_as_bool(node_priv, GLOM_ATTRIBUTE_PRIV_DELETE);

                    group_info.m_map_privileges[priv_table_name] = privs;
                  }
                }

                m_groups[group_info.get_name()] = group_info;
              }
            }
          }


          //Library Modules:
          m_map_library_scripts.clear();

          const auto nodeModules = XmlUtils::get_node_child_named(nodeRoot, GLOM_NODE_LIBRARY_MODULES);
          if(nodeModules)
          {
            for(const auto& item : nodeModules->get_children(GLOM_NODE_LIBRARY_MODULE))
            {
              auto node_lib_module = dynamic_cast<xmlpp::Element*>(item);
              if(node_lib_module)
              {
                //The name is in an attribute:
                const auto module_name = XmlUtils::get_node_attribute_value(node_lib_module, GLOM_ATTRIBUTE_LIBRARY_MODULE_NAME);

                //The string is in a child text node:
                Glib::ustring script;

                const auto text_child = node_lib_module->get_first_child_text();
                if(text_child)
                  script = text_child->get_content();

                //Fall back to the deprecated attribute:
                if(script.empty())
                  script = XmlUtils::get_node_attribute_value(node_lib_module, GLOM_ATTRIBUTE_LIBRARY_MODULE_SCRIPT);

                m_map_library_scripts[module_name] = script;
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

void Document::save_before_layout_item_formatting(xmlpp::Element* nodeItem, const std::shared_ptr<const LayoutItem_WithFormatting>& layout_item)
{
  if(!layout_item)
    return;

  const Formatting& format = layout_item->m_formatting;

  std::shared_ptr<const LayoutItem_Field> field = std::dynamic_pointer_cast<const LayoutItem_Field>(layout_item);

  Field::glom_field_type field_type = Field::glom_field_type::INVALID;
  if(field)
    field_type = field->get_glom_type();

  save_before_layout_item_formatting(nodeItem, format, field_type);
}

void Document::save_before_layout_item_formatting(xmlpp::Element* nodeItem, const Formatting& format, Field::glom_field_type field_type)
{
  //Numeric format:
  if(field_type != Field::glom_field_type::INVALID)  //These options are only for fields:
  {
    if(field_type == Field::glom_field_type::NUMERIC)
    {
      XmlUtils::set_node_attribute_value_as_bool(nodeItem, GLOM_ATTRIBUTE_FORMAT_THOUSANDS_SEPARATOR,  format.m_numeric_format.m_use_thousands_separator);
      XmlUtils::set_node_attribute_value_as_bool(nodeItem, GLOM_ATTRIBUTE_FORMAT_DECIMAL_PLACES_RESTRICTED, format.m_numeric_format.m_decimal_places_restricted);
      XmlUtils::set_node_attribute_value_as_decimal(nodeItem, GLOM_ATTRIBUTE_FORMAT_DECIMAL_PLACES, format.m_numeric_format.m_decimal_places);
      XmlUtils::set_node_attribute_value(nodeItem, GLOM_ATTRIBUTE_FORMAT_CURRENCY_SYMBOL, format.m_numeric_format.m_currency_symbol);
      XmlUtils::set_node_attribute_value_as_bool(nodeItem, GLOM_ATTRIBUTE_FORMAT_USE_ALT_NEGATIVE_COLOR,
        format.m_numeric_format.m_alt_foreground_color_for_negatives);
    }

    bool as_radio_buttons = false;
    const auto choices_restricted = format.get_choices_restricted(as_radio_buttons);
    XmlUtils::set_node_attribute_value_as_bool(nodeItem, GLOM_ATTRIBUTE_FORMAT_CHOICES_RESTRICTED, choices_restricted);
    XmlUtils::set_node_attribute_value_as_bool(nodeItem, GLOM_ATTRIBUTE_FORMAT_CHOICES_RESTRICTED_AS_RADIO_BUTTONS, as_radio_buttons);
    XmlUtils::set_node_attribute_value_as_bool(nodeItem, GLOM_ATTRIBUTE_FORMAT_CHOICES_CUSTOM, format.get_has_custom_choices());
  }

  //Text formatting:
  if(field_type == Field::glom_field_type::TEXT)
  {
    XmlUtils::set_node_attribute_value_as_bool(nodeItem, GLOM_ATTRIBUTE_FORMAT_TEXT_MULTILINE, format.get_text_format_multiline());
    XmlUtils::set_node_attribute_value_as_decimal(nodeItem, GLOM_ATTRIBUTE_FORMAT_TEXT_MULTILINE_HEIGHT_LINES, 
      format.get_text_format_multiline_height_lines(), GLOM_ATTRIBUTE_FORMAT_TEXT_MULTILINE_HEIGHT_LINES_DEFAULT);
  }

  XmlUtils::set_node_attribute_value(nodeItem, GLOM_ATTRIBUTE_FORMAT_TEXT_FONT, format.get_text_format_font());
  XmlUtils::set_node_attribute_value(nodeItem, GLOM_ATTRIBUTE_FORMAT_TEXT_COLOR_FOREGROUND, format.get_text_format_color_foreground());
  XmlUtils::set_node_attribute_value(nodeItem, GLOM_ATTRIBUTE_FORMAT_TEXT_COLOR_BACKGROUND, format.get_text_format_color_background());

  //Alignment:
  const auto alignment = format.get_horizontal_alignment();
  if(alignment != Formatting::HorizontalAlignment::AUTO) //Save file-size by not even writing this.
  {
    const Glib::ustring alignment_str =
      (alignment == Formatting::HorizontalAlignment::LEFT  ? GLOM_ATTRIBUTE_FORMAT_HORIZONTAL_ALIGNMENT_LEFT : GLOM_ATTRIBUTE_FORMAT_HORIZONTAL_ALIGNMENT_RIGHT);
    XmlUtils::set_node_attribute_value(nodeItem, GLOM_ATTRIBUTE_FORMAT_HORIZONTAL_ALIGNMENT, alignment_str);
  }

  //Choices:
  if(field_type != Field::glom_field_type::INVALID)
  {
    if(format.get_has_custom_choices())
    {
      auto child = nodeItem->add_child_element(GLOM_ATTRIBUTE_FORMAT_CHOICES_CUSTOM_LIST);
      for(const auto& value : format.get_choices_custom())
      {
        auto childChoice = child->add_child_element(GLOM_NODE_FORMAT_CUSTOM_CHOICE);
        save_before_choicevalue(childChoice, value, field_type);
      }
    }

    XmlUtils::set_node_attribute_value_as_bool(nodeItem, GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED, format.get_has_related_choices() );

    std::shared_ptr<const Relationship> choice_relationship;
    std::shared_ptr<const LayoutItem_Field> choice_layout_first;
    std::shared_ptr<const LayoutGroup> choice_extra_layouts;
    Formatting::type_list_sort_fields choice_sort_fields;
    bool choice_show_all = false;
    format.get_choices_related(choice_relationship, choice_layout_first, choice_extra_layouts, choice_sort_fields, choice_show_all);

    if(choice_relationship)
    {
      Glib::ustring choice_field;
      if(choice_layout_first)
        choice_field = choice_layout_first->get_name();

      XmlUtils::set_node_attribute_value(nodeItem, GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_RELATIONSHIP, glom_get_sharedptr_name(choice_relationship));
      XmlUtils::set_node_attribute_value(nodeItem, GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_FIELD, choice_field);
      XmlUtils::set_node_attribute_value_as_bool(nodeItem, GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_SHOW_ALL, choice_show_all);

      //Save the extra fields to show for related choices:
      if(choice_extra_layouts)
      {
        auto nodeExtraLayout = nodeItem->add_child_element(GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_EXTRA_LAYOUT);
        auto nodeGroups = nodeExtraLayout->add_child_element(GLOM_NODE_DATA_LAYOUT_GROUPS);
        save_before_layout_group(nodeGroups, choice_extra_layouts);
      }

      if(!choice_sort_fields.empty())
      {
        auto nodeSortBy = nodeItem->add_child_element(GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_SORTBY);
        save_before_sort_by(nodeSortBy, choice_sort_fields);
      }
    }
  }
}

void Document::save_before_layout_item_usesrelationship(xmlpp::Element* nodeItem, const std::shared_ptr<const UsesRelationship>& item)
{
  if(!item)
    return;

  XmlUtils::set_node_attribute_value(nodeItem, GLOM_ATTRIBUTE_RELATIONSHIP_NAME, item->get_relationship_name());
  XmlUtils::set_node_attribute_value(nodeItem, GLOM_ATTRIBUTE_RELATED_RELATIONSHIP_NAME, item->get_related_relationship_name());
}

void Document::save_before_layout_item_field(xmlpp::Element* nodeItem, const std::shared_ptr<const LayoutItem_Field>& field)
{
  if(!field)
    return;

  XmlUtils::set_node_attribute_value(nodeItem, GLOM_ATTRIBUTE_NAME, field->get_name());
  save_before_layout_item_usesrelationship(nodeItem, field);
  XmlUtils::set_node_attribute_value_as_bool(nodeItem, GLOM_ATTRIBUTE_EDITABLE, field->get_editable());

  XmlUtils::set_node_attribute_value_as_bool(nodeItem, GLOM_ATTRIBUTE_DATA_LAYOUT_ITEM_FIELD_USE_DEFAULT_FORMATTING, field->get_formatting_use_default());

  std::shared_ptr<const CustomTitle> custom_title = field->get_title_custom();
  if(custom_title)
  {
    auto elementCustomTitle = nodeItem->add_child_element(GLOM_NODE_LAYOUT_ITEM_CUSTOM_TITLE);
    XmlUtils::set_node_attribute_value_as_bool(elementCustomTitle, GLOM_ATTRIBUTE_LAYOUT_ITEM_CUSTOM_TITLE_USE, custom_title->get_use_custom_title());

    save_before_translations(elementCustomTitle, custom_title);
  }
}

void Document::save_before_sort_by(xmlpp::Element* node, const LayoutItem_GroupBy::type_list_sort_fields& list_fields)
{
  if(!node)
    return;

  for(const auto& field_pair : list_fields)
  {
    std::shared_ptr<const LayoutItem_Field> field = field_pair.first;

    auto nodeChild = node->add_child_element(GLOM_NODE_DATA_LAYOUT_ITEM_FIELD);
    save_before_layout_item_field(nodeChild, field);

    XmlUtils::set_node_attribute_value_as_bool(nodeChild, GLOM_ATTRIBUTE_SORT_ASCENDING, field_pair.second);
  }
}

void Document::save_before_layout_group(xmlpp::Element* node, const std::shared_ptr<const LayoutGroup>& group, bool with_print_layout_positions)
{
  if(!node || !group)
    return;

  //g_warning("save_before_layout_group");

  xmlpp::Element* child = nullptr;

  std::shared_ptr<const LayoutItem_GroupBy> group_by = std::dynamic_pointer_cast<const LayoutItem_GroupBy>(group);
  if(group_by) //If it is a GroupBy report part.
  {
    child = node->add_child_element(GLOM_NODE_DATA_LAYOUT_ITEM_GROUPBY);

    if(group_by->get_has_field_group_by())
    {
      auto nodeGroupBy = child->add_child_element(GLOM_NODE_REPORT_ITEM_GROUPBY_GROUPBY);
      save_before_layout_item_field(nodeGroupBy, group_by->get_field_group_by());
    }

    //Sort fields:
    if(group_by->get_has_fields_sort_by())
    {
      auto nodeSortBy = child->add_child_element(GLOM_NODE_REPORT_ITEM_GROUPBY_SORTBY);
      save_before_sort_by(nodeSortBy, group_by->get_fields_sort_by());
    }

    //Secondary fields:
    if(!group_by->get_secondary_fields()->m_list_items.empty())
    {
      auto secondary_fields = child->add_child_element(GLOM_NODE_DATA_LAYOUT_GROUP_SECONDARYFIELDS);
      save_before_layout_group(secondary_fields, group_by->get_secondary_fields(), with_print_layout_positions);
    }
  }
  else
  {
    std::shared_ptr<const LayoutItem_Summary> summary = std::dynamic_pointer_cast<const LayoutItem_Summary>(group);
    if(summary) //If it is a GroupBy report part.
    {
      child = node->add_child_element(GLOM_NODE_DATA_LAYOUT_ITEM_SUMMARY);
      //TODO: summary_type.
    }
    else
    {
      std::shared_ptr<const LayoutItem_VerticalGroup> verticalgroup = std::dynamic_pointer_cast<const LayoutItem_VerticalGroup>(group);
      if(verticalgroup) //If it is a GroupBy report part.
      {
        child = node->add_child_element(GLOM_NODE_DATA_LAYOUT_ITEM_VERTICALGROUP);
      }
      else
      {
        std::shared_ptr<const LayoutItem_Header> headerGroup = std::dynamic_pointer_cast<const LayoutItem_Header>(group);
        if(headerGroup) //If it is a GroupBy report part.
        {
          child = node->add_child_element(GLOM_NODE_DATA_LAYOUT_ITEM_HEADER);
        }
        else
        {
          std::shared_ptr<const LayoutItem_Footer> footerGroup = std::dynamic_pointer_cast<const LayoutItem_Footer>(group);
          if(footerGroup) //If it is a GroupBy report part.
          {
            child = node->add_child_element(GLOM_NODE_DATA_LAYOUT_ITEM_FOOTER);
          }
          else
          {
            std::shared_ptr<const LayoutItem_Portal> portal = std::dynamic_pointer_cast<const LayoutItem_Portal>(group);
            if(portal) //If it is a related records portal
            {
              std::shared_ptr<const LayoutItem_CalendarPortal> calendar_portal = std::dynamic_pointer_cast<const LayoutItem_CalendarPortal>(portal);
              if(calendar_portal)
              {
                child = node->add_child_element(GLOM_NODE_DATA_LAYOUT_CALENDAR_PORTAL);
                std::shared_ptr<const Field> date_field = calendar_portal->get_date_field();
                if(date_field)
                  XmlUtils::set_node_attribute_value(child, GLOM_ATTRIBUTE_PORTAL_CALENDAR_DATE_FIELD, date_field->get_name());
              }
              else
                child = node->add_child_element(GLOM_NODE_DATA_LAYOUT_PORTAL);

              save_before_layout_item_usesrelationship(child, portal);

              //Portal navigation details:
              Glib::ustring navigation_type_string;
              std::shared_ptr<const UsesRelationship> relationship_navigation_specific;

              switch(portal->get_navigation_type())
              {
                case LayoutItem_Portal::navigation_type::AUTOMATIC:
                  //We leave this blank to use the default.
                  break;
                case LayoutItem_Portal::navigation_type::NONE:
                  navigation_type_string = GLOM_ATTRIBUTE_PORTAL_NAVIGATION_TYPE_NONE;
                  break;
                case LayoutItem_Portal::navigation_type::SPECIFIC:
                  navigation_type_string = GLOM_ATTRIBUTE_PORTAL_NAVIGATION_TYPE_SPECIFIC;
                  break;
                default:
                  break;
              }

              //Empty means the default ("automatic")
              //In that case we don't even write the node, to keep the XML small:
              if(!navigation_type_string.empty())
              {
                auto child_navigation_relationship = child->add_child_element(GLOM_NODE_DATA_LAYOUT_PORTAL_NAVIGATIONRELATIONSHIP);

                save_before_layout_item_usesrelationship(child_navigation_relationship, relationship_navigation_specific);
                XmlUtils::set_node_attribute_value(child_navigation_relationship,
                  GLOM_ATTRIBUTE_PORTAL_NAVIGATION_TYPE, navigation_type_string);
              }

              if(!calendar_portal)
              {
                gulong rows_count_min = 0;
                gulong rows_count_max = 0;
                portal->get_rows_count(rows_count_min, rows_count_max);
                XmlUtils::set_node_attribute_value_as_decimal_double(child, 
                  GLOM_ATTRIBUTE_PORTAL_ROWS_COUNT_MIN, rows_count_min);
                XmlUtils::set_node_attribute_value_as_decimal_double(child, 
                  GLOM_ATTRIBUTE_PORTAL_ROWS_COUNT_MAX, rows_count_max);

                //Print Layout specific stuff:
                XmlUtils::set_node_attribute_value_as_decimal(child,
                  GLOM_ATTRIBUTE_PORTAL_PRINT_LAYOUT_ROW_HEIGHT, 
                  portal->get_print_layout_row_height());
                
                XmlUtils::set_node_attribute_value_as_decimal(child,
                  GLOM_ATTRIBUTE_PORTAL_PRINT_LAYOUT_ROW_LINE_WIDTH, 
                  portal->get_print_layout_row_line_width());
                XmlUtils::set_node_attribute_value(child,
                  GLOM_ATTRIBUTE_PORTAL_PRINT_LAYOUT_ROW_LINE_COLOR, 
                  portal->get_print_layout_line_color());
                XmlUtils::set_node_attribute_value(child,
                  GLOM_ATTRIBUTE_PORTAL_PRINT_LAYOUT_LINE_COLOR, 
                  portal->get_print_layout_line_color());
              }
            }
            else
            {
              std::shared_ptr<const LayoutItem_Notebook> notebook = std::dynamic_pointer_cast<const LayoutItem_Notebook>(group);
              if(notebook) //If it is a notebook.
              {
                child = node->add_child_element(GLOM_NODE_DATA_LAYOUT_NOTEBOOK);
              }
              else if(group)
              {
                child = node->add_child_element(GLOM_NODE_DATA_LAYOUT_GROUP);
              }
            }
          }
        }
      }
    }
  }

  if(!child)
    return;

  XmlUtils::set_node_attribute_value(child, GLOM_ATTRIBUTE_NAME, group->get_name());
  XmlUtils::set_node_attribute_value_as_decimal(child, GLOM_ATTRIBUTE_COLUMNS_COUNT, group->get_columns_count(), 1); //Default to 1 because 0 is meaningless.

  XmlUtils::set_node_attribute_value_as_decimal_double(child, GLOM_ATTRIBUTE_BORDER_WIDTH, group->get_border_width());

  //Translations:
  save_before_translations(child, group);

  //Print layout position:
  if(with_print_layout_positions)
    save_before_print_layout_position(child, group);

  //Add the child items:
  LayoutGroup::type_list_const_items items = group->get_items();
  for(const auto& item : items)
  {
    //g_warning("save_before_layout_group: child part type=%s", item->get_part_type_name().c_str());

    std::shared_ptr<const LayoutGroup> child_group = std::dynamic_pointer_cast<const LayoutGroup>(item);
    if(child_group) //If it is a group, portal, summary, or groupby.
    {
      //recurse:
      save_before_layout_group(child, child_group, with_print_layout_positions);
    }
    else
    {
      xmlpp::Element* nodeItem = nullptr;

      std::shared_ptr<const LayoutItem_FieldSummary> fieldsummary = std::dynamic_pointer_cast<const LayoutItem_FieldSummary>(item);
      if(fieldsummary) //If it is a summaryfield
      {
        nodeItem = child->add_child_element(GLOM_NODE_DATA_LAYOUT_ITEM_FIELDSUMMARY);
        save_before_layout_item_field(nodeItem, fieldsummary);
        XmlUtils::set_node_attribute_value(nodeItem, GLOM_ATTRIBUTE_LAYOUT_ITEM_FIELDSUMMARY_SUMMARYTYPE, fieldsummary->get_summary_type_sql()); //The SQL name is as good as anything as an identifier for the summary function.
      }
      else
      {
        std::shared_ptr<const LayoutItem_Field> field = std::dynamic_pointer_cast<const LayoutItem_Field>(item);
        if(field) //If it is a field
        {
          nodeItem = child->add_child_element(GLOM_NODE_DATA_LAYOUT_ITEM_FIELD);
          save_before_layout_item_field(nodeItem, field);
        }
        else
        {
          std::shared_ptr<const LayoutItem_Button> button = std::dynamic_pointer_cast<const LayoutItem_Button>(item);
          if(button) //If it is a button
          {
            nodeItem = child->add_child_element(GLOM_NODE_DATA_LAYOUT_BUTTON);
            XmlUtils::set_child_text_node(nodeItem, GLOM_NODE_BUTTON_SCRIPT, button->get_script());
            save_before_translations(nodeItem, button);
          }
          else
          {
            std::shared_ptr<const LayoutItem_Text> textobject = std::dynamic_pointer_cast<const LayoutItem_Text>(item);
            if(textobject) //If it is a text object.
            {
              nodeItem = child->add_child_element(GLOM_NODE_DATA_LAYOUT_TEXTOBJECT);
              save_before_translations(nodeItem, textobject);

              //The text is translatable too, so we use a node for it:
              auto element_text = nodeItem->add_child_element(GLOM_NODE_DATA_LAYOUT_TEXTOBJECT_TEXT);
              save_before_translations(element_text, textobject->m_text);
            }
            else
            {
              std::shared_ptr<const LayoutItem_Image> imageobject = std::dynamic_pointer_cast<const LayoutItem_Image>(item);
              if(imageobject) //If it is an image object.
              {
                nodeItem = child->add_child_element(GLOM_NODE_DATA_LAYOUT_IMAGEOBJECT);
                save_before_translations(nodeItem, imageobject);

                auto nodeValue = nodeItem->add_child_element(GLOM_NODE_VALUE);
                XmlUtils::set_node_text_child_as_value(nodeValue, imageobject->get_image(), Field::glom_field_type::IMAGE);
              }
              else
              {
                std::shared_ptr<const LayoutItem_Line> line = std::dynamic_pointer_cast<const LayoutItem_Line>(item);
                if(line) //If it is a line
                {
                  nodeItem = child->add_child_element(GLOM_NODE_DATA_LAYOUT_LINE);
                  //This has no translations: save_before_translations(nodeItem, line);

                  double start_x = 0;
                  double start_y = 0;
                  double end_x = 0;
                  double end_y = 0;
                  line->get_coordinates(start_x, start_y, end_x, end_y);

                  XmlUtils::set_node_attribute_value_as_decimal_double(nodeItem, GLOM_ATTRIBUTE_DATA_LAYOUT_LINE_START_X, start_x);
                  XmlUtils::set_node_attribute_value_as_decimal_double(nodeItem, GLOM_ATTRIBUTE_DATA_LAYOUT_LINE_START_Y, start_y);
                  XmlUtils::set_node_attribute_value_as_decimal_double(nodeItem, GLOM_ATTRIBUTE_DATA_LAYOUT_LINE_END_X, end_x);
                  XmlUtils::set_node_attribute_value_as_decimal_double(nodeItem, GLOM_ATTRIBUTE_DATA_LAYOUT_LINE_END_Y, end_y);
                  
                  XmlUtils::set_node_attribute_value_as_decimal_double(nodeItem, 
                    GLOM_ATTRIBUTE_DATA_LAYOUT_LINE_WIDTH, line->get_line_width());
                  XmlUtils::set_node_attribute_value(nodeItem, 
                    GLOM_ATTRIBUTE_DATA_LAYOUT_LINE_COLOR, line->get_line_color());
                }
              }
            }
          }
        }

        if(nodeItem)
        {
          //Save formatting for any layout items that use it:
        std::shared_ptr<const LayoutItem_WithFormatting> withformatting = std::dynamic_pointer_cast<const LayoutItem_WithFormatting>(item);
          if(withformatting)
          {
            auto elementFormat = nodeItem->add_child_element(GLOM_NODE_FORMAT);
              save_before_layout_item_formatting(elementFormat, withformatting);
          }
        }
      }

      if(nodeItem)
      {
        //Attributes that any layout item could have:
        const auto column_width = item->get_display_width();
        XmlUtils::set_node_attribute_value_as_decimal(nodeItem, GLOM_ATTRIBUTE_LAYOUT_ITEM_COLUMN_WIDTH, column_width);

        if(with_print_layout_positions)
          save_before_print_layout_position(nodeItem, item);
      }
    }

    //g_warning("save_before_layout_group: after child part type=%s", item->get_part_type_name().c_str());
  }
}

void Document::save_before_translations(xmlpp::Element* element, const std::shared_ptr<const TranslatableItem>& item)
{
  if(!element)
    return;

  const std::shared_ptr<const ChoiceValue> choicevalue = std::dynamic_pointer_cast<const ChoiceValue>(item);
  if(!choicevalue) //This item does not use the title, but uses the title translations to translate its value, if it is of type text.
  {
    XmlUtils::set_node_attribute_value(element, GLOM_ATTRIBUTE_TITLE, item->get_title_original());
  }

  if(!item->get_has_translations())
    return;

  auto child = element->add_child_element(GLOM_NODE_TRANSLATIONS_SET);

  const auto map_translations = item->_get_translations_map();
  for(const auto& translation_pair : map_translations)
  {
    auto childItem = child->add_child_element(GLOM_NODE_TRANSLATION);
    XmlUtils::set_node_attribute_value(childItem, GLOM_ATTRIBUTE_TRANSLATION_LOCALE, translation_pair.first);
    XmlUtils::set_node_attribute_value(childItem, GLOM_ATTRIBUTE_TRANSLATION_VALUE, translation_pair.second);
  }

  //If it has a singular title, then save that too:
  const std::shared_ptr<const HasTitleSingular> has_title_singular =
    std::dynamic_pointer_cast<const HasTitleSingular>(item);
  if(has_title_singular && has_title_singular->m_title_singular
    && !(has_title_singular->m_title_singular->get_title_original().empty()))
  {
    auto nodeTitleSingular = element->add_child_element(GLOM_NODE_TABLE_TITLE_SINGULAR);
    save_before_translations(nodeTitleSingular, has_title_singular->m_title_singular);
  }
}

void Document::save_before_print_layout_position(xmlpp::Element* nodeItem, const std::shared_ptr<const LayoutItem>& item)
{
  auto child = nodeItem->add_child_element(GLOM_NODE_POSITION);

  double x = 0;
  double y = 0;
  double width = 0;
  double height = 0;
  item->get_print_layout_position(x, y, width, height);

  XmlUtils::set_node_attribute_value_as_decimal_double(child, GLOM_ATTRIBUTE_POSITION_X, x);
  XmlUtils::set_node_attribute_value_as_decimal_double(child, GLOM_ATTRIBUTE_POSITION_Y, y);
  XmlUtils::set_node_attribute_value_as_decimal_double(child, GLOM_ATTRIBUTE_POSITION_WIDTH, width);
  XmlUtils::set_node_attribute_value_as_decimal_double(child, GLOM_ATTRIBUTE_POSITION_HEIGHT, height);

  //Avoid having an empty (or useless) XML element:
  if(child->get_attributes().empty())
    nodeItem->remove_node(child);
}

void Document::save_before_choicevalue(xmlpp::Element* nodeItem, const std::shared_ptr<const ChoiceValue>& item, Field::glom_field_type field_type)
{
  if(!item)
    return;

  XmlUtils::set_node_attribute_value_as_value(nodeItem, GLOM_ATTRIBUTE_VALUE, item->get_value(), field_type);
  save_before_translations(nodeItem, item);
}

bool Document::save_before()
{
  //TODO: Use some callback UI to show a busy cursor?
  /*
  //Use a std::shared_ptr<> to avoid even unncessarily instantiating a BusyCursor,
  //which would require GTK+ to be initialized:
  std::shared_ptr<BusyCursor> auto_cursor;
  if(m_parent_window)
    auto_cursor = std::shared_ptr<BusyCursor>( new BusyCursor(m_parent_window) );
  */

  //TODO: Add xmlpp::Document::remove_root_node() to libxml++
  auto nodeRoot = get_node_document();

  if(nodeRoot)
  {
    // Remove existing child nodes:
    for(const auto& item : nodeRoot->get_children())
      nodeRoot->remove_node(item);

    //Always save as the latest format,
    //possibly making it impossible to open this document in older versions of Glom:
    m_document_format_version = get_latest_known_document_format_version();
    XmlUtils::set_node_attribute_value_as_decimal(nodeRoot, GLOM_ATTRIBUTE_FORMAT_VERSION, m_document_format_version);

    XmlUtils::set_node_attribute_value_as_bool(nodeRoot, GLOM_ATTRIBUTE_IS_EXAMPLE, m_is_example);
    XmlUtils::set_node_attribute_value_as_bool(nodeRoot, GLOM_ATTRIBUTE_IS_BACKUP, m_is_backup);
    XmlUtils::set_node_attribute_value(nodeRoot, GLOM_ATTRIBUTE_TRANSLATION_ORIGINAL_LOCALE, m_translation_original_locale);

    save_before_translations(nodeRoot, m_database_title);

    XmlUtils::set_child_text_node(nodeRoot, GLOM_NODE_STARTUP_SCRIPT, m_startup_script);

    auto nodeConnection = XmlUtils::get_node_child_named_with_add(nodeRoot, GLOM_NODE_CONNECTION);

    switch(m_hosting_mode)
    {
    case HostingMode::POSTGRES_CENTRAL:
      XmlUtils::set_node_attribute_value(nodeConnection, 
        GLOM_ATTRIBUTE_CONNECTION_HOSTING_MODE, GLOM_ATTRIBUTE_CONNECTION_HOSTING_POSTGRES_CENTRAL);
      break;
    case HostingMode::POSTGRES_SELF:
      XmlUtils::set_node_attribute_value(nodeConnection,
        GLOM_ATTRIBUTE_CONNECTION_HOSTING_MODE, GLOM_ATTRIBUTE_CONNECTION_HOSTING_POSTGRES_SELF);
      break;
    case HostingMode::SQLITE:
      XmlUtils::set_node_attribute_value(nodeConnection,
        GLOM_ATTRIBUTE_CONNECTION_HOSTING_MODE, GLOM_ATTRIBUTE_CONNECTION_HOSTING_SQLITE);
      break;
    case HostingMode::MYSQL_CENTRAL:
      XmlUtils::set_node_attribute_value(nodeConnection, 
        GLOM_ATTRIBUTE_CONNECTION_HOSTING_MODE, GLOM_ATTRIBUTE_CONNECTION_HOSTING_MYSQL_CENTRAL);
      break;
    case HostingMode::MYSQL_SELF:
      XmlUtils::set_node_attribute_value(nodeConnection,
        GLOM_ATTRIBUTE_CONNECTION_HOSTING_MODE, GLOM_ATTRIBUTE_CONNECTION_HOSTING_MYSQL_SELF);
      break;
    default:
      g_assert_not_reached();
      break;
    }

    XmlUtils::set_node_attribute_value_as_bool(nodeConnection, GLOM_ATTRIBUTE_CONNECTION_NETWORK_SHARED, m_network_shared);

    XmlUtils::set_node_attribute_value(nodeConnection, GLOM_ATTRIBUTE_CONNECTION_SERVER, m_connection_server);
    XmlUtils::set_node_attribute_value_as_decimal(nodeConnection, GLOM_ATTRIBUTE_CONNECTION_PORT, m_connection_port);
    XmlUtils::set_node_attribute_value_as_bool(nodeConnection, GLOM_ATTRIBUTE_CONNECTION_TRY_OTHER_PORTS, m_connection_try_other_ports, true /* default */);
    XmlUtils::set_node_attribute_value(nodeConnection, GLOM_ATTRIBUTE_CONNECTION_DATABASE, m_connection_database);

    //Remove existing tables:
    xmlpp::Node::NodeList list_nodes_tables = nodeRoot->get_children(GLOM_NODE_TABLE);
    for(const auto& item : list_nodes_tables)
      nodeRoot->remove_node(item);

    //Add tables:
  for(const auto& table_pair : m_tables)
    {
      const std::shared_ptr<const DocumentTableInfo> doctableinfo = table_pair.second;
      if(!doctableinfo || !doctableinfo->m_info)
        continue;

      const auto table_name = doctableinfo->m_info->get_name();
      if(table_name.empty())
        std::cerr << G_STRFUNC << ": table name is empty." << std::endl;

      if(!table_name.empty())
      {
        auto nodeTable = nodeRoot->add_child_element(GLOM_NODE_TABLE);
        XmlUtils::set_node_attribute_value(nodeTable, GLOM_ATTRIBUTE_NAME, table_name);
        XmlUtils::set_node_attribute_value_as_bool(nodeTable, GLOM_ATTRIBUTE_HIDDEN, doctableinfo->m_info->get_hidden());
        XmlUtils::set_node_attribute_value_as_bool(nodeTable, GLOM_ATTRIBUTE_DEFAULT, doctableinfo->m_info->get_default());

        XmlUtils::set_node_attribute_value_as_float(nodeTable, GLOM_ATTRIBUTE_OVERVIEW_X, doctableinfo->m_overviewx);
        XmlUtils::set_node_attribute_value_as_float(nodeTable, GLOM_ATTRIBUTE_OVERVIEW_Y, doctableinfo->m_overviewy);

        if(m_is_example) //The example data is useless to non-example files (and is big):
        {
          auto nodeExampleRows = nodeTable->add_child_element(GLOM_NODE_EXAMPLE_ROWS);

          for(const auto& row_data : doctableinfo->m_example_rows)
          {
            auto nodeExampleRow = nodeExampleRows->add_child_element(GLOM_NODE_EXAMPLE_ROW);
            if(!row_data.empty())
            {
              const auto row_data_size = row_data.size();
              for(unsigned int i = 0; i < row_data_size; ++i)
              {
                std::shared_ptr<const Field> field = doctableinfo->m_fields[i];
                if(!field)
                  break;

                auto nodeField = nodeExampleRow->add_child_element(GLOM_NODE_VALUE);
                XmlUtils::set_node_attribute_value(nodeField, GLOM_ATTRIBUTE_COLUMN, field->get_name());
                XmlUtils::set_node_text_child_as_value(nodeField, row_data[i], field->get_glom_type());
              } // for each value
            } // !row_data.empty
          } // for each row
        } // m_is_example


        //Translations:
        save_before_translations(nodeTable, doctableinfo->m_info);

        //Fields:
        auto elemFields = nodeTable->add_child_element(GLOM_NODE_FIELDS);

        const auto type_names = Field::get_type_names();

        for(const auto& field : doctableinfo->m_fields)
        {
          auto elemField = elemFields->add_child_element(GLOM_NODE_FIELD);
          XmlUtils::set_node_attribute_value(elemField, GLOM_ATTRIBUTE_NAME, field->get_name());

          XmlUtils::set_node_attribute_value_as_bool(elemField, GLOM_ATTRIBUTE_PRIMARY_KEY, field->get_primary_key());
          XmlUtils::set_node_attribute_value_as_bool(elemField, GLOM_ATTRIBUTE_UNIQUE, field->get_unique_key());
          XmlUtils::set_node_attribute_value_as_bool(elemField, GLOM_ATTRIBUTE_AUTOINCREMENT, field->get_auto_increment());
          XmlUtils::set_node_attribute_value_as_value(elemField, GLOM_ATTRIBUTE_DEFAULT_VALUE, field->get_default_value(), field->get_glom_type());

          XmlUtils::set_child_text_node(elemField, GLOM_NODE_CALCULATION, field->get_calculation());

          Glib::ustring field_type;
          Field::type_map_type_names::const_iterator iterTypes = type_names.find( field->get_glom_type() );
          if(iterTypes != type_names.end())
            field_type = iterTypes->second;

          XmlUtils::set_node_attribute_value(elemField, GLOM_ATTRIBUTE_TYPE, field_type);

          //Add Lookup sub-node:
          if(field->get_is_lookup())
          {
            auto elemFieldLookup = elemField->add_child_element(GLOM_NODE_FIELD_LOOKUP);

            std::shared_ptr<Relationship> lookup_relationship = field->get_lookup_relationship();
            XmlUtils::set_node_attribute_value(elemFieldLookup, GLOM_ATTRIBUTE_RELATIONSHIP_NAME, glom_get_sharedptr_name(lookup_relationship));

            XmlUtils::set_node_attribute_value(elemFieldLookup, GLOM_ATTRIBUTE_FIELD, field->get_lookup_field());
          }

          //Default Formatting:
          auto elementFormat = elemField->add_child_element(GLOM_NODE_FORMAT);
          save_before_layout_item_formatting(elementFormat, field->m_default_formatting, field->get_glom_type());

          //Translations:
          save_before_translations(elemField, field);
        } /* fields */

        //Relationships:
        //Add new <relationships> node:
        auto elemRelationships = nodeTable->add_child_element(GLOM_NODE_RELATIONSHIPS);

        //Add each <relationship> node:
        for(const auto& relationship : doctableinfo->m_relationships)
        {
          if(relationship)
          {
            auto elemRelationship = elemRelationships->add_child_element(GLOM_NODE_RELATIONSHIP);
            XmlUtils::set_node_attribute_value(elemRelationship, GLOM_ATTRIBUTE_NAME, relationship->get_name());
            XmlUtils::set_node_attribute_value(elemRelationship, GLOM_ATTRIBUTE_KEY, relationship->get_from_field());
            XmlUtils::set_node_attribute_value(elemRelationship, GLOM_ATTRIBUTE_OTHER_TABLE, relationship->get_to_table());
            XmlUtils::set_node_attribute_value(elemRelationship, GLOM_ATTRIBUTE_OTHER_KEY, relationship->get_to_field());
            XmlUtils::set_node_attribute_value_as_bool(elemRelationship, GLOM_ATTRIBUTE_AUTO_CREATE, relationship->get_auto_create());
            XmlUtils::set_node_attribute_value_as_bool(elemRelationship, GLOM_ATTRIBUTE_ALLOW_EDIT, relationship->get_allow_edit());

            //Translations:
            save_before_translations(elemRelationship, relationship);
          }
        }

        //Layouts:
        auto nodeDataLayouts = nodeTable->add_child_element(GLOM_NODE_DATA_LAYOUTS);

        //Add the groups:
        //Make sure that we always get these _after_ the relationships.
        for(const auto& layout : doctableinfo->m_layouts)
        {
          auto nodeLayout = nodeDataLayouts->add_child_element(GLOM_NODE_DATA_LAYOUT);
          XmlUtils::set_node_attribute_value(nodeLayout, GLOM_ATTRIBUTE_NAME, layout.m_layout_name);
          XmlUtils::set_node_attribute_value(nodeLayout, GLOM_ATTRIBUTE_LAYOUT_PLATFORM, layout.m_layout_platform);

          auto nodeGroups = nodeLayout->add_child_element(GLOM_NODE_DATA_LAYOUT_GROUPS);

          for(const auto& group : layout.m_layout_groups)
          {
            save_before_layout_group(nodeGroups, group);
          }
        }

        //Reports:
        auto nodeReports = nodeTable->add_child_element(GLOM_NODE_REPORTS);

        //Add the groups:
        for(const auto& report_pair : doctableinfo->m_reports)
        {
          auto nodeReport = nodeReports->add_child_element(GLOM_NODE_REPORT);

          std::shared_ptr<const Report> report = report_pair.second;
          XmlUtils::set_node_attribute_value(nodeReport, GLOM_ATTRIBUTE_NAME, report->get_name());
          XmlUtils::set_node_attribute_value_as_bool(nodeReport, GLOM_ATTRIBUTE_REPORT_SHOW_TABLE_TITLE, report->get_show_table_title());

          auto nodeGroups = nodeReport->add_child_element(GLOM_NODE_DATA_LAYOUT_GROUPS);
          save_before_layout_group(nodeGroups, report->get_layout_group());

          //Translations:
          save_before_translations(nodeReport, report);
        }

        //Print Layouts:
        auto nodePrintLayouts = nodeTable->add_child_element(GLOM_NODE_PRINT_LAYOUTS);

        //Add the print :
        for(const auto& print_layout_pair : doctableinfo->m_print_layouts)
        {
          auto nodePrintLayout = nodePrintLayouts->add_child_element(GLOM_NODE_PRINT_LAYOUT);

          const auto& print_layout = print_layout_pair.second;
          XmlUtils::set_node_attribute_value(nodePrintLayout, GLOM_ATTRIBUTE_NAME, print_layout->get_name());
          XmlUtils::set_node_attribute_value_as_bool(nodePrintLayout, GLOM_ATTRIBUTE_REPORT_SHOW_TABLE_TITLE, print_layout->get_show_table_title());

          XmlUtils::set_node_attribute_value_as_bool(nodePrintLayout, GLOM_ATTRIBUTE_PRINT_LAYOUT_SHOW_GRID, print_layout->get_show_grid());
          XmlUtils::set_node_attribute_value_as_bool(nodePrintLayout, GLOM_ATTRIBUTE_PRINT_LAYOUT_SHOW_RULES, print_layout->get_show_rules());
          XmlUtils::set_node_attribute_value_as_bool(nodePrintLayout, GLOM_ATTRIBUTE_PRINT_LAYOUT_SHOW_OUTLINES, print_layout->get_show_outlines());

          //Save the rule lines:
          for(const auto& value : print_layout->get_horizontal_rules())
          {
            auto child = nodePrintLayout->add_child_element(GLOM_NODE_HORIZONTAL_RULE);
            XmlUtils::set_node_attribute_value_as_decimal(child, GLOM_ATTRIBUTE_RULE_POSITION, value);
          }

          for(const auto& value : print_layout->get_vertical_rules())
          {
            auto child = nodePrintLayout->add_child_element(GLOM_NODE_VERTICAL_RULE);
            XmlUtils::set_node_attribute_value_as_decimal(child, GLOM_ATTRIBUTE_RULE_POSITION, value);
          }

          //Page Setup:
          const auto page_setup = print_layout->get_page_setup();
          if(!page_setup.empty())
          {
            auto child = nodePrintLayout->add_child_element(GLOM_NODE_PAGE_SETUP);
            child->add_child_text( Utils::string_clean_for_xml(page_setup) );
          }
          
          XmlUtils::set_node_attribute_value_as_decimal(nodePrintLayout, GLOM_ATTRIBUTE_PRINT_LAYOUT_PAGE_COUNT, 
            print_layout->get_page_count(), 1);

          auto nodeGroups = nodePrintLayout->add_child_element(GLOM_NODE_DATA_LAYOUT_GROUPS);
          save_before_layout_group(nodeGroups, print_layout->get_layout_group(), true /* x,y positions too. */);

          //Translations:
          save_before_translations(nodePrintLayout, print_layout);
        }
      }

    } //for m_tables


    //Remove existing groups:
    for(const auto& item : nodeRoot->get_children(GLOM_NODE_GROUPS))
      nodeRoot->remove_node(item);

    //Add groups:
    auto nodeGroups = nodeRoot->add_child_element(GLOM_NODE_GROUPS);

    nodeGroups->add_child_comment("These are only used when recreating a database from an example file. The actual access-control is on the server, of course.");

    for(const auto& group_pair : m_groups)
    {
      const auto& group_info = group_pair.second;

      const auto group_name = group_info.get_name();
      if(group_name.empty())
      {
        //I saw this in at least one .glom file. murrayc.
        std::cerr << G_STRFUNC << ": The group name is empty." << std::endl;
        continue;
      }

      auto nodeGroup = nodeGroups->add_child_element(GLOM_NODE_GROUP);
      XmlUtils::set_node_attribute_value(nodeGroup, GLOM_ATTRIBUTE_NAME, group_name);
      XmlUtils::set_node_attribute_value_as_bool(nodeGroup, GLOM_ATTRIBUTE_DEVELOPER, group_info.m_developer);

      //The privileges for each table, for this group:
      for(const auto& priv_pair : group_info.m_map_privileges)
      {
        auto nodeTablePrivs = nodeGroup->add_child_element(GLOM_NODE_TABLE_PRIVS);

        XmlUtils::set_node_attribute_value(nodeTablePrivs, GLOM_ATTRIBUTE_TABLE_NAME, priv_pair.first);

        const Privileges& privs = priv_pair.second;
        XmlUtils::set_node_attribute_value_as_bool(nodeTablePrivs, GLOM_ATTRIBUTE_PRIV_VIEW, privs.m_view);
        XmlUtils::set_node_attribute_value_as_bool(nodeTablePrivs, GLOM_ATTRIBUTE_PRIV_EDIT, privs.m_edit);
        XmlUtils::set_node_attribute_value_as_bool(nodeTablePrivs, GLOM_ATTRIBUTE_PRIV_CREATE, privs.m_create);
        XmlUtils::set_node_attribute_value_as_bool(nodeTablePrivs, GLOM_ATTRIBUTE_PRIV_DELETE, privs.m_delete);
      }
    }

    //Remove existing library modules::
    auto list_nodes_lib_modules = nodeRoot->get_children(GLOM_NODE_LIBRARY_MODULES);
    for(const auto& item : list_nodes_lib_modules)
      nodeRoot->remove_node(item);

    //Add groups:
    auto nodeModules = nodeRoot->add_child_element(GLOM_NODE_LIBRARY_MODULES);

    for(const auto& script_pair : m_map_library_scripts)
    {
      const auto& name = script_pair.first;
      const auto& script = script_pair.second;

      auto nodeModule = nodeModules->add_child_element(GLOM_NODE_LIBRARY_MODULE);

      //The name is in an attribute:
      XmlUtils::set_node_attribute_value(nodeModule, GLOM_ATTRIBUTE_LIBRARY_MODULE_NAME, name);

      //The script is in a child text node:
      auto text_child = nodeModule->get_first_child_text();
      if(!text_child)
        nodeModule->add_child_text( Utils::string_clean_for_xml(script) );
      else
       text_child->set_content( Utils::string_clean_for_xml(script) );
    }
  }

  //We don't use set_write_formatted() because it doesn't handle text nodes well.
  add_indenting_white_space_to_node();

  return GlomBakery::Document_XML::save_before();
}

Glib::ustring Document::get_database_title_original() const
{
  return m_database_title->get_title_original();
}

Glib::ustring Document::get_database_title(const Glib::ustring& locale) const
{
  return m_database_title->get_title(locale);
}

void Document::set_database_title_original(const Glib::ustring& title)
{
  if(get_database_title_original() != title)
  {
    m_database_title->set_title_original(title);
    set_modified();
  }
}

Glib::ustring Document::get_name() const
{
  //Show the database title in the window title bar:
  const auto title = get_database_title_original();
  if(title.empty())
    return GlomBakery::Document_XML::get_name();
  else
    return title;
}

Document::type_list_groups Document::get_groups() const
{
  type_list_groups result;
  for(const auto& group_pair : m_groups)
  {
    result.push_back(group_pair.second);
  }

  return result;
}

///This adds the group if necessary.
void Document::set_group(GroupInfo& group)
{
  const auto name = group.get_name();
  auto iter = m_groups.find(name);
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

void Document::remove_group(const Glib::ustring& group_name)
{
  auto iter = m_groups.find(group_name);
  if(iter != m_groups.end())
  {
    m_groups.erase(iter);
    set_modified();
  }
}

std::vector<Glib::ustring> Document::get_report_names(const Glib::ustring& table_name) const
{
  const std::shared_ptr<const DocumentTableInfo> info = get_table_info(table_name);
  if(info)
  {
    std::vector<Glib::ustring> result;
    for(const auto& report_pair : info->m_reports)
    {
      result.push_back(report_pair.second->get_name());
    }

    return result;
  }
  else
    return std::vector<Glib::ustring>();
}

void Document::set_report(const Glib::ustring& table_name, const std::shared_ptr<Report>& report)
{
  const auto info = get_table_info(table_name);
  if(info)
  {
    info->m_reports[report->get_name()] = report;
    set_modified();
  }
}

std::shared_ptr<Report> Document::get_report(const Glib::ustring& table_name, const Glib::ustring& report_name) const
{
  const std::shared_ptr<const DocumentTableInfo> info = get_table_info(table_name);
  if(info)
  {
    DocumentTableInfo::type_reports::const_iterator iterFindReport = info->m_reports.find(report_name);
    if(iterFindReport != info->m_reports.end())
    {
      return iterFindReport->second;
    }
  }

  return std::shared_ptr<Report>();
}

void Document::remove_report(const Glib::ustring& table_name, const Glib::ustring& report_name)
{
  const auto info = get_table_info(table_name);
  if(info)
  {
    auto iterFindReport = info->m_reports.find(report_name);
    if(iterFindReport != info->m_reports.end())
    {
      info->m_reports.erase(iterFindReport);

      set_modified();
    }
  }
}


std::vector<Glib::ustring> Document::get_print_layout_names(const Glib::ustring& table_name) const
{
  const std::shared_ptr<const DocumentTableInfo> info = get_table_info(table_name);
  if(info)
  {
    std::vector<Glib::ustring> result;
    for(const auto& print_layout_pair : info->m_print_layouts)
    {
      result.push_back(print_layout_pair.second->get_name());
    }

    return result;
  }
  else
    return std::vector<Glib::ustring>();
}


void Document::set_print_layout(const Glib::ustring& table_name, const std::shared_ptr<PrintLayout>& print_layout)
{
  const auto info = get_table_info(table_name);
  if(info)
  {
    info->m_print_layouts[print_layout->get_name()] = print_layout;
    set_modified();
  }
}

std::shared_ptr<PrintLayout> Document::get_print_layout(const Glib::ustring& table_name, const Glib::ustring& print_layout_name) const
{
  const std::shared_ptr<const DocumentTableInfo> info = get_table_info(table_name);
  if(info)
  {
    DocumentTableInfo::type_print_layouts::const_iterator iterFindPrintLayout = info->m_print_layouts.find(print_layout_name);
    if(iterFindPrintLayout != info->m_print_layouts.end())
    {
      return iterFindPrintLayout->second;
    }
  }

  return std::shared_ptr<PrintLayout>();
}

void Document::remove_print_layout(const Glib::ustring& table_name, const Glib::ustring& print_layout_name)
{
  const auto info = get_table_info(table_name);
  if(info)
  {
    auto iterFindPrintLayout = info->m_print_layouts.find(print_layout_name);
    if(iterFindPrintLayout != info->m_print_layouts.end())
    {
      info->m_print_layouts.erase(iterFindPrintLayout);

      set_modified();
    }
  }
}



bool Document::get_relationship_is_to_one(const Glib::ustring& table_name, const Glib::ustring& relationship_name) const
{
  std::shared_ptr<const Relationship> relationship = get_relationship(table_name, relationship_name);
  if(relationship)
  {
    std::shared_ptr<const Field> field_to = get_field(relationship->get_to_table(), relationship->get_to_field());
    if(field_to)
      return (field_to->get_primary_key() || field_to->get_unique_key());
  }

  return false;
}

std::shared_ptr<const Relationship> Document::get_field_used_in_relationship_to_one(const Glib::ustring& table_name, const std::shared_ptr<const LayoutItem_Field>& layout_field) const
{
  std::shared_ptr<const Relationship> result;

  if(!layout_field)
  {
    std::cerr << G_STRFUNC << ": layout_field was null" << std::endl;
    return result;
  }

  const auto table_used = layout_field->get_table_used(table_name);
  const std::shared_ptr<const DocumentTableInfo> table_info = get_table_info(table_used);
  if(!table_info)
  {
    //This table is special. We would not create a relationship to it using a field:
    if(table_used == GLOM_STANDARD_TABLE_PREFS_TABLE_NAME)
      return result;
     
    std::cerr << G_STRFUNC << ": table not found:" << table_used << std::endl;
    return result;
  }

  //Look at each relationship:
  const auto field_name = layout_field->get_name();
  for(const auto& relationship : table_info->m_relationships)
  {
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

  return result;
}

void Document::forget_layout_record_viewed(const Glib::ustring& table_name)
{
  const auto info = get_table_info(table_name);
  if(info)
    info->m_map_current_record.clear();
}

void Document::set_layout_record_viewed(const Glib::ustring& table_name, const Glib::ustring& layout_name, const Gnome::Gda::Value& primary_key_value)
{
  const auto info = get_table_info(table_name);
  if(info)
    info->m_map_current_record[layout_name] = primary_key_value;
}

Gnome::Gda::Value Document::get_layout_record_viewed(const Glib::ustring& table_name, const Glib::ustring& layout_name) const
{
  const std::shared_ptr<const DocumentTableInfo> info = get_table_info(table_name);
  if(info)
  {
    DocumentTableInfo::type_map_layout_primarykeys::const_iterator iterLayoutKeys = info->m_map_current_record.find(layout_name);
    if(iterLayoutKeys != info->m_map_current_record.end())
      return iterLayoutKeys->second;
  }

  return Gnome::Gda::Value(); //not found.
}

void Document::set_layout_current(const Glib::ustring& table_name, const Glib::ustring& layout_name)
{
  const auto info = get_table_info(table_name);
  if(info)
    info->m_layout_current = layout_name;
}

void Document::set_criteria_current(const Glib::ustring& table_name, const FoundSet& found_set)
{
  const auto info = get_table_info(table_name);
  if(info)
    info->m_foundset_current = found_set;
}

Glib::ustring Document::get_layout_current(const Glib::ustring& table_name) const
{
  const std::shared_ptr<const DocumentTableInfo> info = get_table_info(table_name);
  if(info)
    return info->m_layout_current;

  return Glib::ustring(); //not found.
}

FoundSet Document::get_criteria_current(const Glib::ustring& table_name) const
{
  const std::shared_ptr<const DocumentTableInfo> info = get_table_info(table_name);
  if(info)
    return info->m_foundset_current;

  return FoundSet();
}


bool Document::get_is_example_file() const
{
  return m_is_example;
}

void Document::set_is_example_file(bool value)
{
  if(m_is_example != value)
  {
    m_is_example = value;
    set_modified();
  }
}


bool Document::get_is_backup_file() const
{
  return m_is_backup;
}

void Document::set_is_backup_file(bool value)
{
  if(m_is_backup != value)
  {
    m_is_backup = value;
    set_modified();
  }
}


void Document::set_translation_original_locale(const Glib::ustring& locale)
{
  if(m_translation_original_locale == locale)
    return;

  m_translation_original_locale = locale;
  set_modified();
}

Glib::ustring Document::get_translation_original_locale() const
{
  return m_translation_original_locale;
}

std::vector<Glib::ustring> Document::get_translation_available_locales() const
{
  return m_translation_available_locales;
}

namespace {

/**
 * Find the element in the container which is a pair which contains the same information.
 * This assumes that the element is a shared_ptr<>.
 */
template
<typename T_Container>
auto find_if_item_and_hint_equal(T_Container& container, const Document::pair_translatable_item_and_hint& item_and_hint) -> decltype(container.begin())
{
  return std::find_if(container.begin(), container.end(),
    [&item_and_hint](const auto& element)
    {
      if(!element.first && item_and_hint.first)
        return true;

      if(element.first && !item_and_hint.first)
        return true;

      if(element.first && item_and_hint.first)
      {
        if(element.first->get_title_original() != item_and_hint.first->get_title_original())
          return false;
      }

      if(get_po_context_for_item(element.first, element.second) != 
        get_po_context_for_item(item_and_hint.first, item_and_hint.second))
      {
        return false;
      }

      return true;
    }
  );
}

} //anonymous namespace

static void add_to_translatable_list(Document::type_list_translatables& list, const std::shared_ptr<TranslatableItem>& item, const Glib::ustring& hint)
{
  // Only add the item/hint combination if it is not there already:
  const Document::pair_translatable_item_and_hint item_and_hint(item, hint);
  if(find_if_item_and_hint_equal(list, item_and_hint) == list.end())
  {
    list.push_back( item_and_hint );
  }
}

static void add_to_translatable_list(Document::type_list_translatables& list, const Document::type_list_translatables& sublist)
{
  // Only add the item/hint combination if it is not there already:
  for(const auto& item_and_hint : sublist)
  {
    if(find_if_item_and_hint_equal(list, item_and_hint) == list.end())
    {
      list.push_back( item_and_hint );
    }
  }
}

template <typename T_List>
static void translatable_items_append_with_hint(Document::type_list_translatables& result, T_List& list_items, const Glib::ustring& hint)
{
  for(const auto& item : list_items)
  {
    add_to_translatable_list(result, item, hint);
  }
}

Document::type_list_translatables Document::get_translatable_items()
{
  type_list_translatables result;
  add_to_translatable_list(result, m_database_title, "");

  //Add tables:
  for(const auto& tableinfo : get_tables())
  {
    if(!tableinfo)
      continue;

    add_to_translatable_list(result, tableinfo, "");

    const auto table_name = tableinfo->get_name();

    //The table's field titles:
    const Glib::ustring hint = "Parent table: " + table_name;
    for(const auto& field : get_table_fields(table_name))
    {
      if(!field)
        continue;

      add_to_translatable_list(result, field, hint);

      //Custom Choices, if any:
      if(field->get_glom_type() == Field::glom_field_type::TEXT) //Choices for other field types could not be translated.
      {
        const auto this_hint = hint + ", Parent Field: " + field->get_name();   
        type_list_translatables list_choice_items;
        Document::fill_translatable_custom_choices(field->m_default_formatting, list_choice_items, this_hint);
        add_to_translatable_list(result, list_choice_items);
      }
    }

    //The table's relationships:
    type_vec_relationships relationships = get_relationships(table_name);
    translatable_items_append_with_hint(result, relationships, hint);

    //The table's report titles:
    for(const auto& report_name : get_report_names(table_name))
    {
      std::shared_ptr<Report> report = get_report(table_name, report_name);
      if(!report)
        continue;

      add_to_translatable_list(result, report, hint);
      
      //Translatable report items:
      const auto this_hint = hint + ", Parent Report: " + report->get_name();
      type_list_translatables list_layout_items = get_translatable_report_items(table_name, report_name, this_hint);
      add_to_translatable_list(result, list_layout_items);
    }

    //The table's print layout titles:
    for(const auto& print_layout_name : get_print_layout_names(table_name))
    {
      std::shared_ptr<PrintLayout> print_layout = get_print_layout(table_name, print_layout_name);
      if(!print_layout)
        continue;

      add_to_translatable_list(result, print_layout, hint);
      
      //Translatable print layout items:
      const auto this_hint = hint + ", Print Layout: " + print_layout->get_name();
      type_list_translatables list_layout_items = get_translatable_print_layout_items(table_name, print_layout_name, this_hint);
      add_to_translatable_list(result, list_layout_items);
    }

    //The table's translatable layout items:
    type_list_translatables list_layout_items = get_translatable_layout_items(table_name, hint);
    add_to_translatable_list(result, list_layout_items);
  } //for

  return result;
}

Document::type_list_translatables Document::get_translatable_layout_items(const Glib::ustring& table_name, const Glib::ustring& hint)
{
  type_list_translatables result;

  const auto info = get_table_info(table_name);
  if(!info)
    return result;

  //Look at each layout:
  for(const auto& layout : info->m_layouts)
  {
    //Look at each group:
    for(const auto& group : layout.m_layout_groups)
    {
      if(group)
      {
        fill_translatable_layout_items(group, result, hint);
      }
    }
  }

  return result;
}


Document::type_list_translatables Document::get_translatable_report_items(const Glib::ustring& table_name, const Glib::ustring& report_name, const Glib::ustring& hint)
{
  Document::type_list_translatables the_list;

  std::shared_ptr<Report> report = get_report(table_name, report_name);
  if(report)
    fill_translatable_layout_items(report->get_layout_group(), the_list, hint);

  return the_list;
}

Document::type_list_translatables Document::get_translatable_print_layout_items(const Glib::ustring& table_name, const Glib::ustring& print_layout_name, const Glib::ustring& hint)
{
  Document::type_list_translatables the_list;

  std::shared_ptr<PrintLayout> print_layout = get_print_layout(table_name, print_layout_name);
  if(print_layout)
    fill_translatable_layout_items(print_layout->get_layout_group(), the_list, hint);

  return the_list;
}

void Document::fill_translatable_custom_choices(Formatting& formatting, type_list_translatables& the_list, const Glib::ustring& hint)
{
  if(!formatting.get_has_custom_choices())
    return;

  for(const auto& value : formatting.get_choices_custom())
  {
    the_list.push_back( pair_translatable_item_and_hint(value, hint) );
  }
}

void Document::fill_translatable_layout_items(const std::shared_ptr<LayoutItem_Field>& layout_field, type_list_translatables& the_list, const Glib::ustring& hint)
{
  //LayoutItem_Field items do not have their own titles.
  //They use either the field's title or a custom title:
  std::shared_ptr<CustomTitle> custom_title = layout_field->get_title_custom();
  if(custom_title)
  {
    the_list.push_back( pair_translatable_item_and_hint(custom_title, hint) ); 
  }

  //The field will be added separately.
  
  //Custom Choices, if any:
  //Only text fields can have translated choice values:
  if(layout_field->get_glom_type() == Field::glom_field_type::TEXT)
  {
    const auto choice_hint = hint + ", Parent Field: " + layout_field->get_name();
    fill_translatable_custom_choices(layout_field->m_formatting, the_list, hint);
  }
}


void Document::fill_translatable_layout_items(const std::shared_ptr<LayoutGroup>& group, type_list_translatables& the_list, const Glib::ustring& hint)
{
  //Portals don't have their own titles - they use the relationship title (though we might want to allow custom titles)
  std::shared_ptr<LayoutItem_Portal> portal = std::dynamic_pointer_cast<LayoutItem_Portal>(group);
  if(!portal)
  {
    the_list.push_back( pair_translatable_item_and_hint(group, hint) );
  }
  
  const auto group_name = group->get_name();
  Glib::ustring this_hint = hint;
  if(!group_name.empty())
    this_hint += ", Parent Group: " + group_name;

  //Look at each item:
  for(const auto& item : group->get_items())
  {
    std::shared_ptr<LayoutGroup> child_group = std::dynamic_pointer_cast<LayoutGroup>(item);
    if(child_group) //If it is a group, portal, summary, or groupby.
    {
      std::shared_ptr<LayoutItem_GroupBy> group_by = std::dynamic_pointer_cast<LayoutItem_GroupBy>(child_group);
      if(group_by)
      {
        std::shared_ptr<LayoutItem_Field> field = group_by->get_field_group_by();
        fill_translatable_layout_items(field, the_list, hint);

        fill_translatable_layout_items(group_by->get_secondary_fields(), the_list, this_hint);
      }

      //recurse:
      fill_translatable_layout_items(child_group, the_list, this_hint);
    }
    else
    {
      //Buttons too:
      std::shared_ptr<LayoutItem_Button> button = std::dynamic_pointer_cast<LayoutItem_Button>(item);
      if(button)
        the_list.push_back( pair_translatable_item_and_hint(button, this_hint) );
      else
      {
        std::shared_ptr<LayoutItem_Text> text = std::dynamic_pointer_cast<LayoutItem_Text>(item);
        if(text)
        {
          the_list.push_back( pair_translatable_item_and_hint(text, this_hint) );
          if(text->m_text)
            the_list.push_back( pair_translatable_item_and_hint(text->m_text, this_hint) );
        }
        else
        {
          //Images have titles:
          std::shared_ptr<LayoutItem_Image> image = std::dynamic_pointer_cast<LayoutItem_Image>(item);
          if(image)
            the_list.push_back( pair_translatable_item_and_hint(image, this_hint) );
          else
          {
            std::shared_ptr<LayoutItem_Field> layout_field = std::dynamic_pointer_cast<LayoutItem_Field>(item);
            if(layout_field)
            {
              fill_translatable_layout_items(layout_field, the_list, hint);
            }
          }
        }
      }
    }
  }
}

void Document::set_file_uri(const Glib::ustring& file_uri, bool bEnforceFileExtension /* = false */)
{
  Glib::ustring file_uri_used = file_uri;

  //Enforce file extension:
  if(bEnforceFileExtension)
    file_uri_used = get_file_uri_with_extension(file_uri);

  //const bool changed = (file_uri_used != m_file_uri);
  //We override this because set_modified() triggers a save (to the old filename) in this derived class.

  //I'm not sure why this is in the base class's method anyway. murrayc:
  //if(file_uri != m_file_uri)
  //  set_modified(); //Ready to save() for a Save As.

  m_file_uri = file_uri_used;

  //Put this here instead. In the base class it's at the start:
  //Actually, we do not call this, because it would trigger a save, when using autosave,
  //and that could cause an empty document to be saved to a URL before calling load() to load the real document that was there.
  //if(changed)
  //  set_modified(); //Ready to save() for a Save As.
}

guint Document::get_document_format_version()
{
  return m_document_format_version;
}

guint Document::get_latest_known_document_format_version()
{
  // History:
  // Version 0: The first document format. (And the default version number when no version number was saved in the .XML)
  // Version 1: Saved scripts and other multiline text in text nodes instead of attributes. Can open Version 1 documents.
  // Version 2: hosting_mode="postgres-central|postgres-self|sqlite" instead of self_hosted="true|false". Can open Version 1 documents, by falling back to the self_hosted attribute if hosting_mode is not set.
  // Version 3: (Glom 1.10). Support for the old one-big-string example_rows format was removed, and we now use (unquoted) non-postgres libgda escaping.
  // Version 4: (Glom 1.12). Portal navigation options were simplified, with a "none" option. network_sharing was added, defaulting to off.
  // Version 5: (Glom 1.14). Extra layout item formatting options were added, plus a startup script.
  // Version 6: (Glom 1.16). Extra show_all option for choices that show related records. Extra related choice fields are now a layout group instead of just a second field name.
  // Version 7: (Glom 1.20). New print layout details. Related records: Number of rows can be specified. All colors can now be in CSS3 string format (via GdkRGBA)
  // Version 8: (Glom 1.22). The database_title attribute is replaced by the title attribute.
  // Version 9: (Glom 1.24). <value> tags now have a format="base64" attribute by default. Having no format attribute is deprecated. data_layout_image nodes now have child <value> nodes instead of using the "text" attribute to store image data.

  return 9;
}

std::vector<Glib::ustring> Document::get_library_module_names() const
{
  std::vector<Glib::ustring> result;
  for(const auto& script_pair : m_map_library_scripts)
  {
    result.push_back(script_pair.first);
  }

  return result;
}

void Document::set_library_module(const Glib::ustring& name, const Glib::ustring& script)
{
  if(name.empty())
    return;

  auto iter = m_map_library_scripts.find(name);
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

Glib::ustring Document::get_library_module(const Glib::ustring& name) const
{
  type_map_library_scripts::const_iterator iter = m_map_library_scripts.find(name);
  if(iter != m_map_library_scripts.end())
  {
    return iter->second;
  }

  return Glib::ustring();
}

void Document::remove_library_module(const Glib::ustring& name)
{
  auto iter = m_map_library_scripts.find(name);
  if(iter != m_map_library_scripts.end())
  {
     m_map_library_scripts.erase(iter);
     set_modified();
  }
}

Glib::ustring Document::get_startup_script() const
{
  return m_startup_script;
}

void Document::set_startup_script(const Glib::ustring& script)
{
  if(m_startup_script == script)
    return;

  m_startup_script = script;
  set_modified();
}

Glib::ustring Document::build_and_get_contents() const
{
  //save_before() probably should be const because it doesn't change much of the external behaviour:
  auto unconst = const_cast<Document*>(this);

  unconst->save_before(); //This is the part of the Document_XML overrides that sets the contents string from the XML tree.
  return get_contents();
}

void Document::set_opened_from_browse(bool val)
{
  m_opened_from_browse = val;

  //This should stop developer mode from being possible,
  //because we don't have access to the document:
  if(!val)
    m_app_state.set_userlevel(AppState::userlevels::OPERATOR);
}

bool Document::get_opened_from_browse() const
{
  return m_opened_from_browse;
}

bool Document::load(int& failure_code)
{
  return GlomBakery::Document_XML::load(failure_code);
}

namespace { //anonymous namespace

static void handle_archive_error(archive* a)
{
  std::cerr << "  " << archive_error_string(a) << std::endl;
}

// TODO: Use shared_ptr or unique_ptr?
// We use this to make sure that the C object is always released.
template <typename T_Object>
class ScopedArchivePtr
{
public:
  typedef int (*T_ReleaseFunc)(T_Object*);

  ScopedArchivePtr(T_Object* ptr, T_ReleaseFunc release_func)
  : ptr_(ptr),
    release_func_(release_func)
  {}

  ~ScopedArchivePtr() 
  {
    if(!release_func_)
      return;

    const auto r = (*release_func_)(ptr_);
    if(r != ARCHIVE_OK)
    {
      std::cerr << G_STRFUNC << ": The release_func failed." << std::endl;
      handle_archive_error(ptr_);
    }
  }

private:
  T_Object* ptr_;
  T_ReleaseFunc release_func_;

  ScopedArchivePtr(const ScopedArchivePtr<T_Object>&);
  ScopedArchivePtr<T_Object>& operator=(const ScopedArchivePtr<T_Object>&);
};

//The same as ScopedArchivePtr but with a different release function signature.
template <typename T_Object>
class ScopedArchiveEntryPtr
{
public:
  typedef void (*T_ReleaseFunc)(T_Object*);

  ScopedArchiveEntryPtr(T_Object* ptr, T_ReleaseFunc release_func)
  : ptr_(ptr),
    release_func_(release_func)
  {}

  ~ScopedArchiveEntryPtr() 
  {
    if(release_func_)
      (*release_func_)(ptr_);
  }

private:
  T_Object* ptr_;
  T_ReleaseFunc release_func_;

  explicit ScopedArchiveEntryPtr(const ScopedArchivePtr<T_Object>&);
  ScopedArchiveEntryPtr<T_Object>& operator=(const ScopedArchivePtr<T_Object>&);

};

/** A utility to add an archive entry to an archive.
 * @param a The libarchive archive, ready for writing.
 * @param parent_dir_path The parent (could be several levels up) that the file's path (in the archive) should be relative to.
 * @param filepath The path of the existing file on the file system.
 */
bool add_file_to_archive(archive* a, const std::string& parent_dir_path, const std::string& filepath)
{
//TODO: Use read_async() when this calling method is async.
  Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(filepath);
  Glib::RefPtr<Gio::FileInputStream> stream;
  try
  {
    stream = file->read();

    // Query size of the file, so that we can show progress:
    //TODO: stream->query_info_async(sigc::mem_fun(*this, &DialogImageLoadProgress::on_query_info), G_FILE_ATTRIBUTE_STANDARD_SIZE);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << ": Gio::File::read() failed: " << ex.what() << std::endl;
    std::cerr << "  : with path: " << filepath << std::endl;
    return false;
  }

  struct stat st;
  if(stat(filepath.c_str(), &st) < 0)
  {
    std::cerr << G_STRFUNC << ": stat() failed." << std::endl;
    std::cerr << "  : with path: " << filepath << std::endl;
    return false;
  }

  struct archive_entry* entry = archive_entry_new();
  ScopedArchiveEntryPtr<archive_entry> scoped_entry(entry, &archive_entry_free); //Make sure it is always released.

  archive_entry_copy_stat(entry, &st); //This has no return value.

  Glib::RefPtr<Gio::File> file_parent = Gio::File::create_for_path(parent_dir_path);
  const auto path = file_parent->get_relative_path(file);
  archive_entry_set_pathname(entry, path.c_str()); //This has no return value.

  if(archive_write_header(a, entry) != ARCHIVE_OK)
  {
    std::cerr << G_STRFUNC << ": Could not write archive header." << std::endl;
    handle_archive_error(a);
    return false;
  }

  //TODO: Use read_async() when this calling method is async.
  try
  {
    // Query size of the file, so that we can show progress:
    //TODO: stream->query_info_async(sigc::mem_fun(*this, &DialogImageLoadProgress::on_query_info), G_FILE_ATTRIBUTE_STANDARD_SIZE);
  
    const guint BYTES_TO_PROCESS = 256;
    guint buffer[BYTES_TO_PROCESS] = {0, }; // For each chunk.
    bool bContinue = true;
    while(bContinue)
    {
      const auto bytes_read = stream->read(buffer, BYTES_TO_PROCESS);
      if(bytes_read == 0)
        bContinue = false; //stop because we reached the end.
      else
      {
        // Add the data to the archive:
        ssize_t check = archive_write_data(a, buffer, bytes_read);
        if(check != bytes_read)
        {
          std::cerr << G_STRFUNC << ": archive_write_data() wrote an unexpected number of bytes. " << std::endl;
          handle_archive_error(a);
          return false;
        }
      }
    }
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << ": stream read() failed: " << ex.what() << std::endl;
    std::cerr << "  : with path: " << filepath << std::endl;
    return false;
  }

  //TODO? archive_write_finish_entry(entry);

  return true;
}

} ////anonymous namespace

//TODO: Make this async, using File::read_async() and IOStream::read_async().
Glib::ustring Document::save_backup_file(const Glib::ustring& uri, const SlotProgress& slot_progress)
{
  //Save a copy of the .glom document,
  //with the same name as the directory:
  //For instance <path>/chosendirectory/chosendirectory.glom
  const auto path_dir = Glib::filename_from_uri(uri);
  const auto basename_dir = Glib::path_get_basename(path_dir);
  const auto filepath_document = Glib::build_filename(path_dir, basename_dir + ".glom");
  const auto uri_document = Glib::filename_to_uri(filepath_document);

  const auto fileuri_old = get_file_uri();
  set_allow_autosave(false); //Prevent saving while we modify the document:
  set_file_uri(uri_document, true); //true = enforce file extension;
  set_is_backup_file(true);
  const auto saved = save();

  set_file_uri(fileuri_old);
  set_is_backup_file(false);
  set_allow_autosave(true);

  if(!saved)
  {
    std::cerr << G_STRFUNC << ": Saving of the backup .glom file failed with URI:" << uri_document << std::endl;
    return Glib::ustring();
  }


  //Save the data:
  auto connection_pool = ConnectionPool::get_instance();
  const bool data_saved = 
    connection_pool->save_backup(slot_progress, path_dir);
  if(!data_saved)
  {
    std::cerr << G_STRFUNC << ": Saving of the backup data failed with path_dir=" << path_dir << std::endl;
    return Glib::ustring();
  }

  //Compress the backup in a .tar.gz, so it is slightly more safe from changes:
  const std::string tarball_path = path_dir + ".tar.gz";

  struct archive* a = archive_write_new();
  ScopedArchivePtr<archive> scoped(a, &archive_write_free); //Make sure it is always released.

  if(archive_write_add_filter_gzip(a) != ARCHIVE_OK)
  {
    std::cerr << G_STRFUNC << ": libarchive does not support tar." << std::endl;
    handle_archive_error(a);
    return Glib::ustring();
  }

  if(archive_write_set_format_pax_restricted(a) != ARCHIVE_OK)
  {
    std::cerr << G_STRFUNC << ": libarchive does not support pax_restricted." << std::endl;
    handle_archive_error(a);
    return Glib::ustring();
  }

  if(archive_write_set_bytes_per_block(a, 4096) != ARCHIVE_OK)
  {
    std::cerr << G_STRFUNC << ": libarchive: cannot set bytes per block." << std::endl;
    handle_archive_error(a);
    return Glib::ustring();
  }

  if(archive_write_open_filename(a, tarball_path.c_str()) != ARCHIVE_OK)
  {
    std::cerr << G_STRFUNC << ": Could not open a new archive file for writing." << std::endl;
    handle_archive_error(a);
    return Glib::ustring();
  }

  if(!add_file_to_archive(a, path_dir, filepath_document))
    return Glib::ustring();

  //TODO: Avoid copy/pasting of this sub-path:
  const auto backup_data_path = Glib::build_filename(path_dir, "glom_postgres_data", "backup");
  if(!add_file_to_archive(a, path_dir, backup_data_path))
    return Glib::ustring();

  if(archive_write_close(a))
  {
    std::cerr << G_STRFUNC << ": Could not close archive." << std::endl;
    handle_archive_error(a);
    return Glib::ustring();
  }

  return Glib::filename_to_uri(tarball_path);
}

namespace { //anonymous namespace

void read_archive_entry_file_contents(archive* a, archive_entry* entry, std::string& file_contents)
{
  file_contents.clear();

  const auto size = archive_entry_size(entry);
  const Glib::ScopedPtr<char> buf ((char*) g_malloc(size + 1));

  const auto r = archive_read_data(a, buf.get(), size);
    
  if((r == ARCHIVE_FATAL) || (r == ARCHIVE_WARN) ||
    (r == ARCHIVE_RETRY)) //0 or a number of bytes read are the signs of success.
  {
      std::cerr << G_STRFUNC << ": Error while reading data from archive entry. r=" << r << std::endl;
      handle_archive_error(a);
      return;
  }

  try
  {
    //For std::string, size is number of characters. For ustring it would be number of characters.
    file_contents += std::string(buf.get(), r);
  }
  catch(const std::exception& ex)
  {
    std::cerr << G_STRFUNC << ": std::exception error while concatenating archive data: "
      << ex.what() << std::endl;
    return;
  }
}

} //anonymous namespaces

Glib::ustring Document::extract_backup_file(const Glib::ustring& backup_uri, std::string& backup_path, const SlotProgress& slot_progress)
{
  backup_path.clear();

  // We cannot use an uri here, because we cannot untar remote files.
  const auto filename_tarball = Glib::filename_from_uri(backup_uri);

  struct archive* a = archive_read_new();
  ScopedArchivePtr<archive> scoped(a, &archive_read_free); //Make sure it is always released.

  if(archive_read_support_filter_gzip(a) != ARCHIVE_OK)
  {
    std::cerr << G_STRFUNC << ": libarchive apparently does not support gzip." << std::endl;
    handle_archive_error(a);
    return Glib::ustring();
  }

  if(archive_read_support_format_all(a) != ARCHIVE_OK)
  {
    std::cerr << G_STRFUNC << ": libarchive apparently does not support standard formats." << std::endl;
    handle_archive_error(a);
    return Glib::ustring();
  }
  //archive_read_support_compression_all(a);

  if(archive_read_open_filename(a, filename_tarball.c_str(), 10240) != ARCHIVE_OK) //TODO
  {
    std::cerr << G_STRFUNC << ": could not read filename from archive." << std::endl;
    handle_archive_error(a);
    return Glib::ustring();
  }

  if(slot_progress)   
    slot_progress();


  //const char *name = archive_entry_pathname(entry);
  //std::cout << "debug: name=" << name << std::endl;

  if(slot_progress)  
    slot_progress();

  Glib::ustring contents;

  //Read the whole file in one go,
  //We'd have to keep it all in memory anyway as we concatentated it,
  //if we did it in chunks.
  if(slot_progress)
    slot_progress();


  std::string contents_glom_file;

  struct archive_entry* entry = nullptr;
  while(archive_read_next_header(a, &entry) == ARCHIVE_OK)
  {
    const auto pathname = archive_entry_pathname(entry);
    if(!pathname)
      continue;

    const auto basename = Glib::path_get_basename(pathname);
    //std::cout << G_STRFUNC << ": debug: basename=" << basename << std::endl;

    bool is_glom_file = false;
    const auto without_suffix = Glom::Utils::string_remove_suffix(basename, ".glom");
    if(without_suffix != basename)
      is_glom_file = true;

    if(is_glom_file)
    {
      read_archive_entry_file_contents(a, entry, contents_glom_file);
    }
    else if(basename == "backup")
    {
      std::string contents_backup_file;
      read_archive_entry_file_contents(a, entry, contents_backup_file);

      backup_path = Utils::get_temp_file_path("glom_backup");
      Glib::file_set_contents(backup_path, contents_backup_file);
      //std::cout << "debug: backup data path: " << backup_path << std::endl;
    }
  }

  return contents_glom_file;
}


Document::type_list_lookups Document::get_lookup_fields(const Glib::ustring& table_name, const Glib::ustring& field_name) const
{
  type_list_lookups result;

  //Examine all fields for this table:
  for(const auto& field : get_table_fields(table_name)) //TODO_Performance: Cache this?
  {
    //Examine each field that looks up its data from a relationship:
    if(field && field->get_is_lookup())
    {
      //Don't let a field trigger its own value.
      //(for instance, if a field uses a lookup whose relationship uses that
      //field itself as the from field.)
      if(field->get_name() == field_name)
      {
        continue;
      }

      //Get the relationship information:
      auto relationship = field->get_lookup_relationship();
      if(relationship)
      {
        //If the relationship is triggered by the specified field:
        if(relationship->get_from_field() == field_name)
        {
          //Add it:
          auto item = std::make_shared<LayoutItem_Field>();
          item->set_full_field_details(field);
          result.push_back( type_pairFieldTrigger(item, relationship) );
        }
      }
    }
  }

  return result;
}

} //namespace Glom
