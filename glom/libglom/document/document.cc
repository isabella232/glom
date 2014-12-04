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


/// Can be used with std::find_if() to find a layout with the same parent_table and layout_name.
template<class T_Element>
class predicate_Layout
{
public:
  predicate_Layout(const Glib::ustring& layout_name, const Glib::ustring& layout_platform)
  : m_layout_name(layout_name),
    m_layout_platform(layout_platform)
  {
  }

  bool operator() (const T_Element& element)
  {
    return (element.m_layout_name == m_layout_name) &&
           (element.m_layout_platform == m_layout_platform);
  }

private:
  Glib::ustring m_layout_name, m_layout_platform;
};


Document::Document()
: m_hosting_mode(HOSTING_MODE_DEFAULT),
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
  m_database_title = sharedptr<DatabaseTitle>::create();

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
  const HostingMode hosting_mode = get_hosting_mode();
  if( (hosting_mode == HOSTING_MODE_POSTGRES_CENTRAL) ||
    (hosting_mode == HOSTING_MODE_MYSQL_CENTRAL) )
  {
    shared = true; //Central hosting means that it must be shared on the network.
  }
  else if(hosting_mode == HOSTING_MODE_SQLITE)
    shared = false; //sqlite does not allow network sharing.

  return shared;
}

std::string Document::get_connection_self_hosted_directory_uri() const
{
  const std::string uri_file = get_file_uri();
  if(uri_file.empty())
  {
    std::cerr << G_STRFUNC << ": file_uri is empty." << std::endl;
    return std::string();
  }
  else
  {
    //Use Gio::File API to construct the URI:
    Glib::RefPtr<Gio::File> file = Gio::File::create_for_uri(uri_file);

    Glib::RefPtr<Gio::File> parent = file->get_parent();
    Glib::RefPtr<Gio::File> datadir;

    if(parent)
    {
      switch(m_hosting_mode)
      {
      case HOSTING_MODE_POSTGRES_SELF:
        datadir = parent->get_child("glom_postgres_data");
        break;
      case HOSTING_MODE_POSTGRES_CENTRAL:
        datadir = parent;
        break;
      case HOSTING_MODE_SQLITE:
        datadir = parent;
        break;
      case HOSTING_MODE_MYSQL_SELF:
        datadir = parent->get_child("glom_mysql_data");
        break;
      case HOSTING_MODE_MYSQL_CENTRAL:
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


void Document::set_relationship(const Glib::ustring& table_name, const sharedptr<Relationship>& relationship)
{
  //Find the existing relationship:
  const sharedptr<DocumentTableInfo> info = get_table_info(table_name);
  if(!info)
    return;
   
  //Look for the relationship with this name:
  bool existing = false;
  const Glib::ustring relationship_name = glom_get_sharedptr_name(relationship);

  type_vec_relationships relationships = info->m_relationships;
  for(type_vec_relationships::iterator iter = relationships.begin(); iter != relationships.end(); ++iter)
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
    info->m_relationships.push_back(relationship);
  }
}

sharedptr<Relationship> Document::create_relationship_system_preferences(const Glib::ustring& table_name)
{
  sharedptr<Relationship> relationship = sharedptr<Relationship>::create();
  relationship->set_name(GLOM_RELATIONSHIP_NAME_SYSTEM_PROPERTIES);
  relationship->set_title_original(_("System Preferences"));
  relationship->set_from_table(table_name);
  relationship->set_to_table(GLOM_STANDARD_TABLE_PREFS_TABLE_NAME);
  relationship->set_allow_edit(false);

  return relationship;
}

sharedptr<TableInfo> Document::create_table_system_preferences()
{
  type_vec_fields fields_ignored;
  return create_table_system_preferences(fields_ignored);
}

sharedptr<TableInfo> Document::create_table_system_preferences(type_vec_fields& fields)
{
  sharedptr<TableInfo> prefs_table_info = sharedptr<TableInfo>::create();
  prefs_table_info->set_name(GLOM_STANDARD_TABLE_PREFS_TABLE_NAME);
  prefs_table_info->set_title_original(_("System Preferences"));
  prefs_table_info->set_hidden(true);


  fields.clear();

  sharedptr<Field> primary_key(new Field()); //It's not used, because there's only one record, but we must have one.
  primary_key->set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_ID);
  primary_key->set_glom_type(Field::TYPE_NUMERIC);
  fields.push_back(primary_key);

  sharedptr<Field> field_name(new Field());
  field_name->set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_NAME);
  field_name->set_title_original(_("System Name"));
  field_name->set_glom_type(Field::TYPE_TEXT);
  fields.push_back(field_name);

  sharedptr<Field> field_org_name(new Field());
  field_org_name->set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_NAME);
  field_org_name->set_title_original(_("Organisation Name"));
  field_org_name->set_glom_type(Field::TYPE_TEXT);
  fields.push_back(field_org_name);

  sharedptr<Field> field_org_logo(new Field());
  field_org_logo->set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_LOGO);
  field_org_logo->set_title_original(_("Organisation Logo"));
  field_org_logo->set_glom_type(Field::TYPE_IMAGE);
  fields.push_back(field_org_logo);

  sharedptr<Field> field_org_address_street(new Field());
  field_org_address_street->set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_STREET);
  field_org_address_street->set_title_original(_("Street"));
  field_org_address_street->set_glom_type(Field::TYPE_TEXT);
  fields.push_back(field_org_address_street);

  sharedptr<Field> field_org_address_street2(new Field());
  field_org_address_street2->set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_STREET2);
  field_org_address_street2->set_title_original(_("Street (line 2)"));
  field_org_address_street2->set_glom_type(Field::TYPE_TEXT);
  fields.push_back(field_org_address_street2);

  sharedptr<Field> field_org_address_town(new Field());
  field_org_address_town->set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_TOWN);
  field_org_address_town->set_title_original(_("City"));
  field_org_address_town->set_glom_type(Field::TYPE_TEXT);
  fields.push_back(field_org_address_town);

  sharedptr<Field> field_org_address_county(new Field());
  field_org_address_county->set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_COUNTY);
  field_org_address_county->set_title_original(_("State"));
  field_org_address_county->set_glom_type(Field::TYPE_TEXT);
  fields.push_back(field_org_address_county);

  sharedptr<Field> field_org_address_country(new Field());
  field_org_address_country->set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_COUNTRY);
  field_org_address_country->set_title_original(_("Country"));
  field_org_address_country->set_glom_type(Field::TYPE_TEXT);
  fields.push_back(field_org_address_country);

  sharedptr<Field> field_org_address_postcode(new Field());
  field_org_address_postcode->set_name(GLOM_STANDARD_TABLE_PREFS_FIELD_ORG_ADDRESS_POSTCODE);
  field_org_address_postcode->set_title_original(_("Zip Code"));
  field_org_address_postcode->set_glom_type(Field::TYPE_TEXT);
  fields.push_back(field_org_address_postcode);

  return prefs_table_info;
}

bool Document::get_relationship_is_system_properties(const sharedptr<const Relationship>& relationship)
{
  return relationship && (relationship->get_name() == GLOM_RELATIONSHIP_NAME_SYSTEM_PROPERTIES);
}

sharedptr<Relationship> Document::get_relationship(const Glib::ustring& table_name, const Glib::ustring& relationship_name) const
{
  sharedptr<Relationship> result;

  if(relationship_name == GLOM_RELATIONSHIP_NAME_SYSTEM_PROPERTIES)
  {
    return create_relationship_system_preferences(table_name);
  }

  const sharedptr<const DocumentTableInfo> info = get_table_info(table_name);
  if(info)
  {
    //Look for the relationship with this name:
    const type_vec_relationships relationships = info->m_relationships;
    for(type_vec_relationships::const_iterator iter = relationships.begin(); iter != relationships.end(); ++iter)
    {
      if(*iter && ((*iter)->get_name() == relationship_name))
      {
        result = *iter;
      }
    }
  }

  return result;
}

Document::type_vec_relationships Document::get_relationships(const Glib::ustring& table_name, bool plus_system_prefs) const
{
  const sharedptr<const DocumentTableInfo> info = get_table_info(table_name);
  if(info)
  {
    type_vec_relationships result = info->m_relationships;

    //Add the system properties if necessary:
    if(plus_system_prefs)
    {
      if(std::find_if(result.begin(), result.end(), predicate_FieldHasName<Relationship>(GLOM_RELATIONSHIP_NAME_SYSTEM_PROPERTIES)) == result.end())
        result.push_back(create_relationship_system_preferences(table_name));
    }

    return result;
  }
  else
    return type_vec_relationships();
}

void Document::set_relationships(const Glib::ustring& table_name, const type_vec_relationships& vecRelationships) //TODO_shared_relationships
{
  if(!table_name.empty())
  {
    const sharedptr<DocumentTableInfo> info = get_table_info_with_add(table_name);
    if(info)
      info->m_relationships = vecRelationships;

    set_modified();
  }
}

void Document::remove_relationship(const sharedptr<const Relationship>& relationship)
{
  //Get the table that this relationship is part of:
  const sharedptr<DocumentTableInfo> info = get_table_info(relationship->get_from_table());
  if(!info)
    return;

  const Glib::ustring relationship_name = glom_get_sharedptr_name(relationship);

  //Find the relationship and remove it:
  type_vec_relationships& relationships = info->m_relationships;
  type_vec_relationships::iterator iterRel = std::find_if(relationships.begin(), relationships.end(), predicate_FieldHasName<Relationship>(relationship_name));
  if(iterRel != relationships.end())
  {
    relationships.erase(iterRel);

    set_modified();
  }

  //Remove relationship from any layouts:
  DocumentTableInfo::type_layouts layouts = info->m_layouts;
  DocumentTableInfo::type_layouts::iterator iterLayouts = layouts.begin();
  while(iterLayouts != layouts.end())
  {
    LayoutInfo& layout_info = *iterLayouts;

    type_list_layout_groups::iterator iterGroups = layout_info.m_layout_groups.begin();
    while(iterGroups != layout_info.m_layout_groups.end())
    {
      //Remove any layout parts that use this relationship:
      sharedptr<LayoutGroup> group = *iterGroups;
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
  DocumentTableInfo::type_reports reports = info->m_reports;
  for(DocumentTableInfo::type_reports::iterator iterReports = reports.begin(); iterReports != reports.end(); ++iterReports)
  {
    sharedptr<Report> report = iterReports->second;
    sharedptr<LayoutGroup> group = report->get_layout_group();

    //Remove the field wherever it is a related field:
    group->remove_relationship(relationship);
  }
}

void Document::remove_field(const Glib::ustring& table_name, const Glib::ustring& field_name)
{
  //Remove the field itself:
  const sharedptr<DocumentTableInfo> info = get_table_info(table_name);
  if(info)
  {
    type_vec_fields& vecFields = info->m_fields;
    type_vec_fields::iterator iterFind = std::find_if( vecFields.begin(), vecFields.end(), predicate_FieldHasName<Field>(field_name) );
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
    const sharedptr<DocumentTableInfo> info = iter->second;
    if(!info)
      continue;

    if(!(info->m_relationships.empty()))
    {
      type_vec_relationships::iterator iterRel = info->m_relationships.begin();
      bool something_changed = true;
      while(something_changed && !info->m_relationships.empty())
      {
        sharedptr<Relationship> relationship = *iterRel;

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
    DocumentTableInfo::type_layouts layouts = info->m_layouts;
    for(DocumentTableInfo::type_layouts::iterator iterLayouts = layouts.begin(); iterLayouts != layouts.end(); ++iterLayouts)
    {
      LayoutInfo& layout_info = *iterLayouts;
      for(type_list_layout_groups::iterator iter = layout_info.m_layout_groups.begin(); iter != layout_info.m_layout_groups.end(); ++iter)
      {
        sharedptr<LayoutGroup> group = *iter;
        if(!group)
          continue;


        //Remove regular field from the layout:
        if(info->m_info)
        {
          const Glib::ustring layout_table_name = info->m_info->get_name();
          group->remove_field(layout_table_name, table_name, field_name);
        }
      }
    }

    //Remove field from any reports:
    DocumentTableInfo::type_reports reports = info->m_reports;
    for(DocumentTableInfo::type_reports::iterator iterReports = reports.begin(); iterReports != reports.end(); ++iterReports)
    {
      sharedptr<Report> report = iterReports->second;
      sharedptr<LayoutGroup> group = report->get_layout_group();

      //Remove regular fields if the field is in this layout's table:
      if(info->m_info)
      {
        const Glib::ustring layout_table_name = info->m_info->get_name();
        group->remove_field(layout_table_name, table_name, field_name);
      }
    }
  }
}

void Document::remove_table(const Glib::ustring& table_name)
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
    const sharedptr<DocumentTableInfo> info = iter->second;
    if(!info)
      continue;

    if(!(info->m_relationships.empty()))
    {
      type_vec_relationships::iterator iterRel = info->m_relationships.begin();
      bool something_changed = true;
      while(something_changed && !info->m_relationships.empty())
      {
        sharedptr<Relationship> relationship = *iterRel;

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
    const sharedptr<const DocumentTableInfo> info = get_table_info(table_name);
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
  type_vec_fields::iterator iterFind = std::find_if(result.begin(), result.end(), predicate_FieldHasName<Field>(GLOM_STANDARD_FIELD_LOCK));
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

    const sharedptr<DocumentTableInfo> info = get_table_info_with_add(table_name);
    if(!info)
      return;

    const bool will_change = true; //This won't work because we didn't clone the fields before changing them: (info.m_fields != vecFields); //TODO: Does this do a deep comparison?
    info->m_fields = vecFields;

    set_modified(will_change);
  }
}

sharedptr<Field> Document::get_field(const Glib::ustring& table_name, const Glib::ustring& strFieldName) const
{
  type_vec_fields vecFields = get_table_fields(table_name);
  type_vec_fields::iterator iterFind = std::find_if( vecFields.begin(), vecFields.end(), predicate_FieldHasName<Field>(strFieldName) );
  if(iterFind != vecFields.end()) //If it was found:
  {
    return  *iterFind; //A reference, not a copy.
  }

  return sharedptr<Field>();
}

sharedptr<Field> Document::get_field_primary_key(const Glib::ustring& table_name) const
{
  type_vec_fields vecFields = get_table_fields(table_name);
  for(type_vec_fields::const_iterator iter = vecFields.begin(); iter != vecFields.end(); ++iter)
  {
    sharedptr<Field> field = *iter;
    if(field && field->get_primary_key())
      return field;
  }

  return sharedptr<Field>();
}

void Document::change_field_name(const Glib::ustring& table_name, const Glib::ustring& strFieldNameOld, const Glib::ustring& strFieldNameNew)
{
  const sharedptr<DocumentTableInfo> info = get_table_info(table_name);
  if(info)
  {
    //Fields:
    type_vec_fields& vecFields = info->m_fields;
    type_vec_fields::iterator iterFind = std::find_if( vecFields.begin(), vecFields.end(), predicate_FieldHasName<Field>(strFieldNameOld) );
    if(iterFind != vecFields.end()) //If it was found:
    {
      //Change it:
      (*iterFind)->set_name(strFieldNameNew);
    }

    //Find any relationships, layouts, or formatting that use this field
    //Look at each table:
    for(type_tables::iterator iter = m_tables.begin(); iter != m_tables.end(); ++iter)
    {
      const sharedptr<DocumentTableInfo> infoInner = iter->second;
      if(!infoInner)
        continue;

      //Fields:
      type_vec_fields& vecFields = infoInner->m_fields;
      for(type_vec_fields::iterator iterField = vecFields.begin(); iterField != vecFields.end(); ++iterField)
      {
        sharedptr<Field> field = *iterField;
        if(!field)
          continue;

        //Formatting:
        Formatting& formatting = field->m_default_formatting;
        formatting.change_field_item_name(table_name, strFieldNameOld, strFieldNameNew);
      }

      //Look at each relationship in the table:
      for(type_vec_relationships::iterator iterRels = infoInner->m_relationships.begin(); iterRels != infoInner->m_relationships.end(); ++iterRels)
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

      const bool is_parent_table = (infoInner->m_info->get_name() == table_name);

      //Look at each layout:
      for(DocumentTableInfo::type_layouts::iterator iterLayouts = infoInner->m_layouts.begin(); iterLayouts != infoInner->m_layouts.end(); ++iterLayouts)
      {
        //Look at each group:
        for(type_list_layout_groups::iterator iterGroup = iterLayouts->m_layout_groups.begin(); iterGroup != iterLayouts->m_layout_groups.end(); ++iterGroup)
        {
          sharedptr<LayoutGroup> group = *iterGroup;
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
      for(DocumentTableInfo::type_reports::iterator iterReports = infoInner->m_reports.begin(); iterReports != infoInner->m_reports.end(); ++iterReports)
      {
        //Change the field if it is in this group:
        sharedptr<Report> report = iterReports->second;
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
  type_tables::iterator iterFindTable = m_tables.find(table_name_old);
  if(iterFindTable != m_tables.end())
  {
    //Change it:
    //We can't just change the key of the iterator (I think),
    //so we copy the whole thing and put it back in the map under a different key:

    //iterFindTable->first = table_name_new;
    const sharedptr<DocumentTableInfo> doctableinfo = iterFindTable->second;
    m_tables.erase(iterFindTable);

    if(doctableinfo && doctableinfo->m_info)
      doctableinfo->m_info->set_name(table_name_new);

    m_tables[table_name_new] = doctableinfo;

    //Find any relationships or layouts that use this table
    //Look at each table:
    for(type_tables::iterator iter = m_tables.begin(); iter != m_tables.end(); ++iter)
    {
      //Look at each relationship in the table:
      const sharedptr<DocumentTableInfo> doctableinfo = iter->second;
      if(!doctableinfo)
        continue;

      type_vec_relationships relationships = doctableinfo->m_relationships;
      for(type_vec_relationships::iterator iterRels = relationships.begin(); iterRels != relationships.end(); ++iterRels)
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

void Document::change_relationship_name(const Glib::ustring& table_name, const Glib::ustring& name, const Glib::ustring& name_new)
{
  const sharedptr<DocumentTableInfo> doctableinfo = get_table_info(table_name);
  if(doctableinfo)
  {
    type_vec_relationships relationships = doctableinfo->m_relationships;
      
    //Change the relationship name:
    type_vec_relationships::iterator iterRelFind = std::find_if(relationships.begin(), relationships.end(), predicate_FieldHasName<Relationship>(name) );
    if(iterRelFind != relationships.end())
      (*iterRelFind)->set_name(name_new);


    //Any layouts, reports, etc that use this relationship will already have the new name via the sharedptr<Relationship>.


    //Look at each table:
    for(type_tables::iterator iter = m_tables.begin(); iter != m_tables.end(); ++iter)
    {
      /*
       //Look at all field formatting:
      for(type_vec_fields::iterator iterFields = iter->second->m_fields.begin(); iterFields != iter->second->m_fields.end(); ++iterFields)
      {
        (*iterFields)->m_default_formatting.change_relationship_name(table_name, name, name_new);
      }

      const bool is_parent_table = (iter->second->m_info->get_name() == table_name);

      //Look at each layout:
      for(DocumentTableInfo::type_layouts::iterator iterLayouts = iter->second->m_layouts.begin(); iterLayouts != iter->second->m_layouts.end(); ++iterLayouts)
      {
        //Look at each group:
        for(type_list_layout_groups::iterator iterGroup = iterLayouts->m_layout_groups.begin(); iterGroup != iterLayouts->m_layout_groups.end(); ++iterGroup)
        {
          //Change the field if it is in this group:
          if(is_parent_table)
            iterGroup->second.change_relationship_name(table_name, name, name_new);
          else
            iterGroup->second.change_related_relationship_name(table_name, name, name_new);
        }
      }


      //Look at each report:
      for(DocumentTableInfo::type_reports::iterator iterReports = iter->second->m_reports.begin(); iterReports != iter->second->m_reports.end(); ++iterReports)
      {
        //Change the field if it is in this group:
        if(is_parent_table)
          iterReports->second->get_layout_group()->change_relationship_name(table_name, name, name_new);
        else
          iterReports->second->get_layout_group()->change_related_relationship_name(table_name, name, name_new);
      }
      */

     //TODO_SharedRelationshipCheck lookups.

    }

    set_modified();
  }
 }


Document::type_listConstTableInfo Document::get_tables(bool plus_system_prefs) const
{
  //TODO: Avoid the almost-duplicate implementation in the const and non-const methods.

  type_listConstTableInfo result;

  for(type_tables::const_iterator iter = m_tables.begin(); iter != m_tables.end(); ++iter)
  {
    const sharedptr<const DocumentTableInfo> doctableinfo = iter->second;
    if(doctableinfo)
      result.push_back(doctableinfo->m_info);

    //std::cout << "debug: " << G_STRFUNC << ": title=" << iter->second->m_info->get_title() << std::endl;
  }

  //Add the system properties if necessary:
  if(plus_system_prefs)
  {
    if(std::find_if(result.begin(), result.end(), predicate_FieldHasName<TableInfo>(GLOM_STANDARD_TABLE_PREFS_TABLE_NAME)) == result.end())
      result.push_back(create_table_system_preferences());
  }

  return result;
}

Document::type_listTableInfo Document::get_tables(bool plus_system_prefs)
{
  type_listTableInfo result;

  for(type_tables::const_iterator iter = m_tables.begin(); iter != m_tables.end(); ++iter)
  {
    const sharedptr<DocumentTableInfo> doctableinfo = iter->second;
    if(doctableinfo)
      result.push_back(doctableinfo->m_info);

    //std::cout << "debug: " << G_STRFUNC << ": title=" << iter->second->m_info->get_title() << std::endl;
  }

  //Add the system properties if necessary:
  if(plus_system_prefs)
  {
    if(std::find_if(result.begin(), result.end(), predicate_FieldHasName<TableInfo>(GLOM_STANDARD_TABLE_PREFS_TABLE_NAME)) == result.end())
      result.push_back(create_table_system_preferences());
  }

  return result;
}

std::vector<Glib::ustring> Document::get_table_names(bool plus_system_prefs) const
{
  type_listConstTableInfo list_full = get_tables(plus_system_prefs);
  std::vector<Glib::ustring> result;
  for(type_listConstTableInfo::const_iterator iter = list_full.begin(); iter != list_full.end(); ++iter)
  {
    const sharedptr<const TableInfo> info = *iter;
    if(info)
      result.push_back(info->get_name());
  }

  return result;
}

sharedptr<TableInfo> Document::get_table(const Glib::ustring& table_name) const
{
  const sharedptr<const DocumentTableInfo> doctableinfo = get_table_info(table_name);
  if(doctableinfo)
    return doctableinfo->m_info;

  return sharedptr<TableInfo>();
}

void Document::add_table(const sharedptr<TableInfo>& table_info)
{
  if(!table_info)
    return;

  type_tables::const_iterator iterfind = m_tables.find(table_info->get_name());
  if(iterfind == m_tables.end())
  {
    const sharedptr<DocumentTableInfo> item = sharedptr<DocumentTableInfo>::create();
    item->m_info = table_info;
    m_tables[table_info->get_name()] = item;
    set_modified();
  }
}

bool Document::get_table_overview_position(const Glib::ustring& table_name, float& x, float& y) const
{
  const sharedptr<const DocumentTableInfo> doctableinfo = get_table_info(table_name);
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
  const sharedptr<DocumentTableInfo> doctableinfo = get_table_info(table_name);
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
  for(type_tables::iterator iter = m_tables.begin(); iter != m_tables.end(); ++iter)
  {
    const sharedptr<DocumentTableInfo> doctableinfo = iter->second;
    if(!doctableinfo)
      continue;

    sharedptr<TableInfo> info = doctableinfo->m_info;
    if(!info)
      continue;

    const Glib::ustring table_name = info->get_name();

    //If the table is also in the supplied list:
    type_listTableInfo::const_iterator iterfind = std::find_if(tables.begin(), tables.end(), predicate_FieldHasName<TableInfo>(table_name));
    if(iterfind != tables.end())
    {
      sharedptr<TableInfo> infoFound = *iterfind;
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
  for(Formatting::type_list_sort_fields::iterator iter = sort_fields.begin(); iter != sort_fields.end(); ++iter)
  {
    sharedptr<const LayoutItem_Field> sort_field = iter->first;
    if(!sort_field)
     continue;
 
    //TODO: Avoid this unconst?
    sharedptr<LayoutItem_Field> unconst_sort_field = sharedptr<LayoutItem_Field>::cast_const(sort_field);
    sharedptr<Field> field = get_field( sort_field->get_table_used(parent_table_name), sort_field->get_name() );
    unconst_sort_field->set_full_field_details(field);
  }
}

void Document::fill_layout_field_details(const Glib::ustring& parent_table_name, const sharedptr<LayoutGroup>& layout_group) const
{
  if(!layout_group)
    return;

  //Get the full field information for the LayoutItem_Fields in this group:

  for(LayoutGroup::type_list_items::iterator iter = layout_group->m_list_items.begin(); iter != layout_group->m_list_items.end(); ++iter)
  {
    sharedptr<LayoutItem> layout_item = *iter;

    //Check custom Field Formatting:
    sharedptr<LayoutItem_WithFormatting> layout_withformatting = 
      sharedptr<LayoutItem_WithFormatting>::cast_dynamic(layout_item);
    if(layout_withformatting)
    {
      sharedptr<const Relationship> choice_relationship;
      sharedptr<LayoutItem_Field> choice_layout_first;
      sharedptr<LayoutGroup> choice_extra_layouts;
      Formatting::type_list_sort_fields choice_sort_fields;
      bool choice_show_all = false;
      layout_withformatting->m_formatting.get_choices_related(choice_relationship, choice_layout_first, choice_extra_layouts, choice_sort_fields, choice_show_all);
      
      const Glib::ustring table_name = (choice_relationship ? choice_relationship->get_to_table() : Glib::ustring());
      if(choice_layout_first)
        choice_layout_first->set_full_field_details( get_field(table_name, choice_layout_first->get_name()) );
      fill_layout_field_details(parent_table_name, choice_extra_layouts); //recurse

      fill_sort_field_details(table_name, choice_sort_fields);
    }

    sharedptr<LayoutItem_Field> layout_field = sharedptr<LayoutItem_Field>::cast_dynamic(layout_item);
    if(layout_field)
    {
      const sharedptr<Field> field = get_field(layout_field->get_table_used(parent_table_name), layout_field->get_name());
      layout_field->set_full_field_details(field);
      if(field)
      {
        //Check default Field Formatting:
        sharedptr<const Relationship> choice_relationship;
        sharedptr<LayoutItem_Field> choice_layout_first;
        sharedptr<LayoutGroup> choice_extra_layouts;
        Formatting::type_list_sort_fields choice_sort_fields;
        bool choice_show_all = false;
        field->m_default_formatting.get_choices_related(choice_relationship, choice_layout_first, choice_extra_layouts, choice_sort_fields, choice_show_all);
        
        const Glib::ustring table_name = (choice_relationship ? choice_relationship->get_to_table() : Glib::ustring());
        if(choice_layout_first)
          choice_layout_first->set_full_field_details( get_field(table_name, choice_layout_first->get_name()) );
        fill_layout_field_details(parent_table_name, choice_extra_layouts); //recurse

        fill_sort_field_details(table_name, choice_sort_fields);
      }
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

void Document::fill_layout_field_details(const Glib::ustring& parent_table_name, type_list_layout_groups& groups) const
{
  for(type_list_layout_groups::iterator iterGroups = groups.begin(); iterGroups != groups.end(); ++iterGroups)
  {
    sharedptr<LayoutGroup> group = *iterGroups;
    if(group)
      fill_layout_field_details(parent_table_name, group);
  }
}

Document::type_list_layout_groups Document::get_data_layout_groups_default(const Glib::ustring& layout_name, const Glib::ustring& parent_table_name, const Glib::ustring& /* layout_platform */) const
{
  type_list_layout_groups result;

  //Add one if necessary:
  sharedptr<LayoutGroup> overview;
  sharedptr<LayoutGroup> details;

  if(layout_name == "details") //The Details default layout is a bit more complicated.
  {
    overview = sharedptr<LayoutGroup>::create();;
    overview->set_name("overview");
    overview->set_title_original(_("Overview"));
    overview->set_columns_count(2);
    result.push_back(overview);

    details = sharedptr<LayoutGroup>::create();
    details->set_name("details");
    details->set_title_original(_("Details"));
    details->set_columns_count(2);
    result.push_back(details);
  }
  
  //If, for some reason, we didn't create the-subgroups, add everything to a top level group:
  if(!overview && !details)
  {
    overview = sharedptr<LayoutGroup>::create();
    overview->set_name("main");
    overview->set_columns_count(1);
    result.push_back(overview);
      
    details = overview; //Adding anything to details adds it to the overview, which is the only group.
  }


  //Discover new fields, and add them:
  type_vec_fields all_fields = get_table_fields(parent_table_name);
  for(type_vec_fields::const_iterator iter = all_fields.begin(); iter != all_fields.end(); ++iter)
  {
    const Glib::ustring field_name = (*iter)->get_name();
    if(!field_name.empty())
    {
      //See whether it's already in the result:
      //TODO_Performance: There is a lot of iterating and comparison here:
      bool found = false; //TODO: This is horrible.
      for(type_list_layout_groups::const_iterator iterFind = result.begin(); iterFind != result.end(); ++iterFind)
      {
        sharedptr<const LayoutGroup> group = *iterFind;
        if(group && group->has_field(parent_table_name, parent_table_name, field_name))
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
  type_list_layout_groups result = get_data_layout_groups(layout_name, parent_table_name, layout_platform);

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
      for(type_list_layout_groups::iterator iter = result.begin(); iter != result.end(); ++iter)
      {
        sharedptr<LayoutGroup> layout_group = *iter;
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
    Document* nonconst_this = const_cast<Document*>(this); //TODO: This is not ideal.
    nonconst_this->set_data_layout_groups(layout_name, parent_table_name, layout_platform, result);
    nonconst_this->set_modified(false); //This might have happened in operator mode, but in that case we don't really need to save it, or mark the document as unsaved.
  }

  return result;
}

Document::type_list_layout_groups Document::get_data_layout_groups(const Glib::ustring& layout_name, const Glib::ustring& parent_table_name, const Glib::ustring& layout_platform) const
{
  //std::cout << "debug: " << G_STRFUNC << ": layout_name=" << layout_name << ", parent_table_name=" << parent_table_name << ", layout_platform=" << layout_platform << std::endl;

  const sharedptr<const DocumentTableInfo> info = get_table_info(parent_table_name);
  if(info)
  {
    const DocumentTableInfo::type_layouts layouts = info->m_layouts;

    //Look for the layout with this name:
    DocumentTableInfo::type_layouts::const_iterator iter = std::find_if(layouts.begin(), layouts.end(), predicate_Layout<LayoutInfo>(layout_name, layout_platform));
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
  type_list_layout_groups layout_groups = get_data_layout_groups(layout_name, parent_table_name, layout_platform);
  for(type_list_layout_groups::iterator iter = layout_groups.begin(); iter != layout_groups.end(); ++iter)
  {
    sharedptr<LayoutGroup> layout_group = *iter;
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
    const sharedptr<DocumentTableInfo> info = get_table_info_with_add(parent_table_name);
    if(!info)
      return;

    LayoutInfo layout_info;
    layout_info.m_layout_name = layout_name;
    layout_info.m_layout_groups = groups;

    DocumentTableInfo::type_layouts& layouts = info->m_layouts;
    DocumentTableInfo::type_layouts::iterator iter = std::find_if(layouts.begin(), layouts.end(), predicate_Layout<LayoutInfo>(layout_name, layout_platform));
    if(iter == layouts.end())
      layouts.push_back(layout_info);
    else
      *iter = layout_info;

    set_modified();
  }
}

sharedptr<Document::DocumentTableInfo> Document::get_table_info(const Glib::ustring& table_name)
{
  type_tables::iterator iterFind = m_tables.find(table_name);
  if(iterFind != m_tables.end())
    return iterFind->second;

  return sharedptr<DocumentTableInfo>();
}

sharedptr<const Document::DocumentTableInfo> Document::get_table_info(const Glib::ustring& table_name) const
{
  type_tables::const_iterator iterFind = m_tables.find(table_name);
  if(iterFind != m_tables.end())
    return iterFind->second;

  return sharedptr<const DocumentTableInfo>();
}

sharedptr<Document::DocumentTableInfo> Document::get_table_info_with_add(const Glib::ustring& table_name)
{
  sharedptr<DocumentTableInfo>  doctableinfo = get_table_info(table_name);
  if(doctableinfo)
  {
    return doctableinfo;
  }
  else
  {
    doctableinfo = sharedptr<DocumentTableInfo>::create();
    doctableinfo->m_info->set_name(table_name);
    m_tables[table_name] = doctableinfo;
    return doctableinfo;
  }
}

Glib::ustring Document::get_table_title(const Glib::ustring& table_name, const Glib::ustring& locale) const
{
  const sharedptr<const DocumentTableInfo> doctableinfo = get_table_info(table_name);
  if(doctableinfo && doctableinfo->m_info)
    return doctableinfo->m_info->get_title(locale);

  return Glib::ustring();
}

Glib::ustring Document::get_table_title_original(const Glib::ustring& table_name) const
{
  const sharedptr<const DocumentTableInfo> doctableinfo = get_table_info(table_name);
  if(doctableinfo && doctableinfo->m_info)
    return doctableinfo->m_info->get_title_original();

  return Glib::ustring();
}

Glib::ustring Document::get_table_title_singular(const Glib::ustring& table_name, const Glib::ustring& locale) const
{
  const sharedptr<const DocumentTableInfo> doctableinfo = get_table_info(table_name);
  if(doctableinfo && doctableinfo->m_info)
    return doctableinfo->m_info->get_title_singular_with_fallback(locale);

  return Glib::ustring();
}

Glib::ustring Document::get_table_title_singular_original(const Glib::ustring& table_name) const
{
  const sharedptr<const DocumentTableInfo> doctableinfo = get_table_info(table_name);
  if(doctableinfo && doctableinfo->m_info)
    return doctableinfo->m_info->get_title_singular_original();

  return Glib::ustring();
}

void Document::set_table_title(const Glib::ustring& table_name, const Glib::ustring& value, const Glib::ustring& locale)
{
  //std::cout << "debug: " << G_STRFUNC << ": table_name=" << table_name << ", value=" << value << std::endl;
  if(!table_name.empty())
  {
    const sharedptr<DocumentTableInfo> info = get_table_info(table_name);
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
    const sharedptr<DocumentTableInfo> info = get_table_info_with_add(table_name);
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
  const sharedptr<const DocumentTableInfo> doctableinfo = get_table_info(table_name);
  if(doctableinfo)
   return doctableinfo->m_example_rows;

  return type_example_rows();
}

bool Document::get_table_is_known(const Glib::ustring& table_name) const
{
  type_tables::const_iterator iterFind = m_tables.find(table_name);
  return (iterFind != m_tables.end());
}

bool Document::get_table_is_hidden(const Glib::ustring& table_name) const
{
  const sharedptr<const DocumentTableInfo> doctableinfo = get_table_info(table_name);
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
  reason = USER_LEVEL_REASON_UNKNOWN;

  if(get_read_only())
  {
    reason = USER_LEVEL_REASON_FILE_READ_ONLY;
    return AppState::USERLEVEL_OPERATOR; //A read-only document cannot be changed, so there's no point in being in developer mode. This is one way to control the user level on purpose.
  }
  else if(get_opened_from_browse())
  {
    reason = USER_LEVEL_REASON_OPENED_FROM_BROWSE;
    return AppState::USERLEVEL_OPERATOR; //Developer mode would require changes to the original document.
  }
  else if(m_file_uri.empty()) //If it has never been saved then this is a new default document, so the user created it, so the user can be a developer.
  {
    return AppState::USERLEVEL_DEVELOPER;
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
  if((userlevel == AppState::USERLEVEL_DEVELOPER) && get_read_only())
  {
    std::cout << "debug: " << G_STRFUNC << ": Developer mode denied because get_read_only() returned true." << std::endl;
    std::cout << "  DEBUG: get_read_only()=" << get_read_only() << std::endl;
    std::cout << "  DEBUG: get_file_uri()=" << get_file_uri() << std::endl;

    m_app_state.set_userlevel(AppState::USERLEVEL_OPERATOR);
    return false;
  }
  else if(get_opened_from_browse())
  {
    m_app_state.set_userlevel(AppState::USERLEVEL_OPERATOR);
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
  for(type_tables::const_iterator iter = m_tables.begin(); iter != m_tables.end(); ++iter)
  {
    const sharedptr<const DocumentTableInfo> doctableinfo = iter->second;
    if(!doctableinfo)
      continue;

    const sharedptr<const TableInfo> info = doctableinfo->m_info;
    if(info && info->get_default())
      return info->get_name();
  }

  //If there is only one table then pretend that is the default:
  if(m_tables.size() == 1)
  {
    type_tables::const_iterator iter = m_tables.begin();

    const sharedptr<const DocumentTableInfo> doctableinfo = iter->second;
    if(doctableinfo && doctableinfo->m_info)
      return doctableinfo->m_info->get_name();
  }

  return Glib::ustring();
}

Glib::ustring Document::get_first_table() const
{
  if(m_tables.empty())
    return Glib::ustring();

  type_tables::const_iterator iter = m_tables.begin();
  const sharedptr<const DocumentTableInfo> doctableinfo = iter->second;
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
  if(get_userlevel() == AppState::USERLEVEL_DEVELOPER)
  {
    //This rebuilds the whole XML DOM and saves the whole document,
    //so we need to be careful not to call set_modified() too often.

    bool test = save_before();
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
    //std::cout << "debug: " << G_STRFUNC << ": Not saving, because not AppState::USERLEVEL_DEVELOPER" << std::endl;
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

  if(get_userlevel() != AppState::USERLEVEL_DEVELOPER)
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

void Document::load_after_layout_item_formatting(const xmlpp::Element* element, const sharedptr<LayoutItem_WithFormatting>& layout_item, const Glib::ustring& table_name)
{
  if(!layout_item)
    return;

  Formatting& format = layout_item->m_formatting;

  sharedptr<LayoutItem_Field> field = sharedptr<LayoutItem_Field>::cast_dynamic(layout_item);

  Field::glom_field_type field_type = Field::TYPE_INVALID;
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
  if(!field_name.empty() && (field_type == Field::TYPE_NUMERIC))
  {
    format.m_numeric_format.m_use_thousands_separator = XmlUtils::get_node_attribute_value_as_bool(element, GLOM_ATTRIBUTE_FORMAT_THOUSANDS_SEPARATOR);
    format.m_numeric_format.m_decimal_places_restricted = XmlUtils::get_node_attribute_value_as_bool(element, GLOM_ATTRIBUTE_FORMAT_DECIMAL_PLACES_RESTRICTED);
    format.m_numeric_format.m_decimal_places = XmlUtils::get_node_attribute_value_as_decimal(element, GLOM_ATTRIBUTE_FORMAT_DECIMAL_PLACES);
    format.m_numeric_format.m_currency_symbol = XmlUtils::get_node_attribute_value(element, GLOM_ATTRIBUTE_FORMAT_CURRENCY_SYMBOL);
    format.m_numeric_format.m_alt_foreground_color_for_negatives =
      XmlUtils::get_node_attribute_value_as_bool(element, GLOM_ATTRIBUTE_FORMAT_USE_ALT_NEGATIVE_COLOR);
  }

  //Text formatting:
  if(field_type == Field::TYPE_TEXT)
  {
    format.set_text_format_multiline( XmlUtils::get_node_attribute_value_as_bool(element, GLOM_ATTRIBUTE_FORMAT_TEXT_MULTILINE) );
    format.set_text_format_multiline_height_lines( XmlUtils::get_node_attribute_value_as_decimal(element, 
      GLOM_ATTRIBUTE_FORMAT_TEXT_MULTILINE_HEIGHT_LINES, GLOM_ATTRIBUTE_FORMAT_TEXT_MULTILINE_HEIGHT_LINES_DEFAULT) );
  }

  format.set_text_format_font( XmlUtils::get_node_attribute_value (element, GLOM_ATTRIBUTE_FORMAT_TEXT_FONT) );
  format.set_text_format_color_foreground( XmlUtils::get_node_attribute_value (element, GLOM_ATTRIBUTE_FORMAT_TEXT_COLOR_FOREGROUND) );
  format.set_text_format_color_background( XmlUtils::get_node_attribute_value (element, GLOM_ATTRIBUTE_FORMAT_TEXT_COLOR_BACKGROUND) );

  //Alignment. Not-specified means auto.
  Formatting::HorizontalAlignment alignment = Formatting::HORIZONTAL_ALIGNMENT_AUTO;
  const Glib::ustring alignment_str = XmlUtils::get_node_attribute_value (element, GLOM_ATTRIBUTE_FORMAT_HORIZONTAL_ALIGNMENT);
  if(alignment_str == GLOM_ATTRIBUTE_FORMAT_HORIZONTAL_ALIGNMENT_LEFT)
    alignment = Formatting::HORIZONTAL_ALIGNMENT_LEFT;
  else if(alignment_str == GLOM_ATTRIBUTE_FORMAT_HORIZONTAL_ALIGNMENT_RIGHT)
    alignment = Formatting::HORIZONTAL_ALIGNMENT_RIGHT;

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
      const xmlpp::Element* nodeChoiceList = XmlUtils::get_node_child_named(element, GLOM_ATTRIBUTE_FORMAT_CHOICES_CUSTOM_LIST);
      if(nodeChoiceList)
      {
        Formatting::type_list_values list_values;

        xmlpp::Node::NodeList listNodesCustomChoices = nodeChoiceList->get_children(GLOM_NODE_FORMAT_CUSTOM_CHOICE);
        for(xmlpp::Node::NodeList::iterator iter = listNodesCustomChoices.begin(); iter != listNodesCustomChoices.end(); ++iter)
        {
          const xmlpp::Element* element = dynamic_cast<const xmlpp::Element*>(*iter);
          if(element)
          {
            if(field_type == Field::TYPE_INVALID)
            {
              //Discover the field type, so we can interpret the text as a value.
              //Not all calling functions know this, so they don't all supply the correct value.
              //TODO_Performance.
              const sharedptr<const Field> field_temp = get_field(table_name, field_name);
              if(field_temp)
                field_type = field_temp->get_glom_type();
            }

            sharedptr<ChoiceValue> value = sharedptr<ChoiceValue>::create();
            load_after_choicevalue(element, value, field_type);
            list_values.push_back(value);
          }
        }

        format.set_choices_custom(list_values);
      }
    }

    format.set_has_related_choices( XmlUtils::get_node_attribute_value_as_bool(element, GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED) );

    const Glib::ustring relationship_name = XmlUtils::get_node_attribute_value(element, GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_RELATIONSHIP);
    if(!relationship_name.empty())
    {
      sharedptr<const Relationship> relationship = get_relationship(table_name, relationship_name);

      bool show_all = XmlUtils::get_node_attribute_value_as_bool(element, GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_SHOW_ALL);
      if(get_document_format_version() < 6)
      {
        show_all = true; //This was the behaviour before this checkbox existed.
      }

      const Glib::ustring field_first = XmlUtils::get_node_attribute_value(element, GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_FIELD);
      sharedptr<LayoutItem_Field> layout_field_first = sharedptr<LayoutItem_Field>::create();
      layout_field_first->set_name(field_first);

      sharedptr<LayoutGroup> extra_layouts;

      //Previous versions just saved a single extra field name, instead of a whole set of layoutgroups:
      if(m_document_format_version < 6)
      {
        //The deprecated way:
        const Glib::ustring field_second = XmlUtils::get_node_attribute_value(element, GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_SECOND);
        if(!field_second.empty())
        {
          extra_layouts = sharedptr<LayoutGroup>::create();
          sharedptr<LayoutItem_Field> item = sharedptr<LayoutItem_Field>::create();
          item->set_name(field_second);
          extra_layouts->add_item(item);
        }
      }
      else
      {
        //Get the extra layout for related choices:
        xmlpp::Element* nodeExtraLayout = XmlUtils::get_node_child_named(element, GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_EXTRA_LAYOUT);
        if(nodeExtraLayout)
        {
          xmlpp::Element* nodeGroups = XmlUtils::get_node_child_named(nodeExtraLayout, GLOM_NODE_DATA_LAYOUT_GROUPS);
          if(nodeGroups)
          {

            sharedptr<LayoutGroup> layout_group = sharedptr<LayoutGroup>::create();
            load_after_layout_group(nodeGroups, relationship->get_to_table(), layout_group);
            if(layout_group && !(layout_group->m_list_items.empty()))
            {
              //We actually want the sub-group:
              extra_layouts = sharedptr<LayoutGroup>::cast_dynamic( layout_group->m_list_items[0] );
            }
          }
        }
      }

      //Sort fields:
      Formatting::type_list_sort_fields sort_fields;
      xmlpp::Element* elementSortBy = XmlUtils::get_node_child_named(element, GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_SORTBY);
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

void Document::load_after_layout_item_usesrelationship(const xmlpp::Element* element, const Glib::ustring& table_name, const sharedptr<UsesRelationship>& item)
{
  if(!element || !item)
    return;

  const Glib::ustring relationship_name = XmlUtils::get_node_attribute_value(element, GLOM_ATTRIBUTE_RELATIONSHIP_NAME);
  sharedptr<Relationship> relationship;
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

  const Glib::ustring related_relationship_name = XmlUtils::get_node_attribute_value(element, GLOM_ATTRIBUTE_RELATED_RELATIONSHIP_NAME);
  if(!related_relationship_name.empty() && relationship)
  {
    sharedptr<Relationship> related_relationship = get_relationship(relationship->get_to_table(), related_relationship_name);
    if(!related_relationship)
      std::cerr << G_STRFUNC << ": related relationship not found in table=" << relationship->get_to_table() << ",  name=" << related_relationship_name << std::endl;

    item->set_related_relationship(related_relationship);
  }
}

void Document::load_after_layout_item_field(const xmlpp::Element* element, const Glib::ustring& table_name, const sharedptr<LayoutItem_Field>& item)
{
  const Glib::ustring name = XmlUtils::get_node_attribute_value(element, GLOM_ATTRIBUTE_NAME);
  item->set_name(name);

  load_after_layout_item_usesrelationship(element, table_name, item);

  //Needed to decide what formatting to load/save:
  const sharedptr<const Field> field = get_field(item->get_table_used(table_name), name);
  
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


  const xmlpp::Element* nodeCustomTitle = XmlUtils::get_node_child_named(element, GLOM_NODE_LAYOUT_ITEM_CUSTOM_TITLE);
  if(nodeCustomTitle)
  {
    sharedptr<CustomTitle> custom_title = sharedptr<CustomTitle>::create();
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

  xmlpp::Node::NodeList listNodes = node->get_children(GLOM_NODE_DATA_LAYOUT_ITEM_FIELD);
  for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
  {
    const xmlpp::Element* element = dynamic_cast<const xmlpp::Element*>(*iter);
    if(element)
    {
      sharedptr<LayoutItem_Field> item = sharedptr<LayoutItem_Field>::create();
      //item.set_full_field_details_empty();
      load_after_layout_item_field(element, table_name, item);
      item->set_full_field_details( get_field(item->get_table_used(table_name), item->get_name()) );

      const bool ascending = XmlUtils::get_node_attribute_value_as_bool(element, GLOM_ATTRIBUTE_SORT_ASCENDING);

      list_fields.push_back( LayoutItem_GroupBy::type_pair_sort_field(item, ascending) );
    }
  }
}

void Document::load_after_layout_group(const xmlpp::Element* node, const Glib::ustring& table_name, const sharedptr<LayoutGroup>& group, bool with_print_layout_positions)
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
  sharedptr<LayoutGroup> temp = group;
  load_after_translations(node, temp);

  //Get the child items:
  xmlpp::Node::NodeList listNodes = node->get_children();
  for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
  {
    sharedptr<LayoutItem> item_added;

    //Create the layout item:
    const xmlpp::Element* element = dynamic_cast<const xmlpp::Element*>(*iter);
    if(element)
    {
      if(element->get_name() == GLOM_NODE_DATA_LAYOUT_ITEM_FIELD)
      {
        sharedptr<LayoutItem_Field> item = sharedptr<LayoutItem_Field>::create();
        //item.set_full_field_details_empty();
        load_after_layout_item_field(element, table_name, item);

        item_added = item;
      }
      else if(element->get_name() == GLOM_NODE_DATA_LAYOUT_BUTTON)
      {
        sharedptr<LayoutItem_Button> item = sharedptr<LayoutItem_Button>::create();

        item->set_script( XmlUtils::get_child_text_node(element, GLOM_NODE_BUTTON_SCRIPT) );
        if(!(item->get_has_script())) //Try the deprecated attribute instead
           item->set_script( XmlUtils::get_node_attribute_value(element, GLOM_DEPRECATED_ATTRIBUTE_BUTTON_SCRIPT) );

        load_after_translations(element, item);

        item_added = item;
      }
      else if(element->get_name() == GLOM_NODE_DATA_LAYOUT_TEXTOBJECT)
      {
        sharedptr<LayoutItem_Text> item = sharedptr<LayoutItem_Text>::create();
        load_after_translations(element, item);

        //The text can be translated too, so it has its own node:
        const xmlpp::Element* element_text = XmlUtils::get_node_child_named(element, GLOM_NODE_DATA_LAYOUT_TEXTOBJECT_TEXT);
        if(element_text)
        {
          sharedptr<StaticText> translatable_text = sharedptr<StaticText>::create();
          load_after_translations(element_text, translatable_text);
          item->m_text = translatable_text;
          //std::cout << "  DEBUG: text: " << item->m_text->get_title_or_name() << std::endl;
        }

        item_added = item;
      }
      else if(element->get_name() == GLOM_NODE_DATA_LAYOUT_IMAGEOBJECT)
      {
        sharedptr<LayoutItem_Image> item = sharedptr<LayoutItem_Image>::create();
        load_after_translations(element, item);

        Gnome::Gda::Value value_image;
        const xmlpp::Element* nodeValue = XmlUtils::get_node_child_named(element, GLOM_NODE_VALUE);
        if(nodeValue)
        {
          value_image = XmlUtils::get_node_text_child_as_value(nodeValue, Field::TYPE_IMAGE);
        }

        if(value_image.is_null())
        {
          //Try the deprecated way:
          value_image = XmlUtils::get_node_attribute_value_as_value(element, GLOM_ATTRIBUTE_DATA_LAYOUT_IMAGEOBJECT_IMAGE, Field::TYPE_IMAGE);
        }

        item->set_image(value_image);

        item_added = item;
      }
      else if(element->get_name() == GLOM_NODE_DATA_LAYOUT_LINE)
      {
        sharedptr<LayoutItem_Line> item = sharedptr<LayoutItem_Line>::create();
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
        sharedptr<LayoutItem_FieldSummary> item = sharedptr<LayoutItem_FieldSummary>::create();
        //item.set_full_field_details_empty();
        load_after_layout_item_field(element, table_name, item);
        item->set_full_field_details( get_field(item->get_table_used(table_name), item->get_name()) );
        item->set_summary_type_from_sql( XmlUtils::get_node_attribute_value(element, GLOM_ATTRIBUTE_LAYOUT_ITEM_FIELDSUMMARY_SUMMARYTYPE) );

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
      else if( (element->get_name() == GLOM_NODE_DATA_LAYOUT_PORTAL) || (element->get_name() == GLOM_NODE_DATA_LAYOUT_CALENDAR_PORTAL) )
      {
        sharedptr<LayoutItem_Portal> portal;
        sharedptr<LayoutItem_CalendarPortal> calendar_portal;

        if(element->get_name() == GLOM_NODE_DATA_LAYOUT_PORTAL)
          portal = sharedptr<LayoutItem_Portal>::create();
        else if(element->get_name() == GLOM_NODE_DATA_LAYOUT_CALENDAR_PORTAL)
        {
          calendar_portal = sharedptr<LayoutItem_CalendarPortal>::create();
          portal = calendar_portal;
        }

        load_after_layout_item_usesrelationship(element, table_name, portal);

        xmlpp::Element* elementNavigationRelationshipSpecific = XmlUtils::get_node_child_named(element, GLOM_NODE_DATA_LAYOUT_PORTAL_NAVIGATIONRELATIONSHIP);
        if(elementNavigationRelationshipSpecific)
        {
          const Glib::ustring navigation_type_as_string =
            XmlUtils::get_node_attribute_value(elementNavigationRelationshipSpecific,
            GLOM_ATTRIBUTE_PORTAL_NAVIGATION_TYPE);
          if(navigation_type_as_string.empty() ||
             navigation_type_as_string == GLOM_ATTRIBUTE_PORTAL_NAVIGATION_TYPE_AUTOMATIC)
          {
            portal->set_navigation_type(LayoutItem_Portal::NAVIGATION_AUTOMATIC);
          }
          else if(navigation_type_as_string == GLOM_ATTRIBUTE_PORTAL_NAVIGATION_TYPE_NONE)
          {
            portal->set_navigation_type(LayoutItem_Portal::NAVIGATION_NONE);
          }
          else if(navigation_type_as_string == GLOM_ATTRIBUTE_PORTAL_NAVIGATION_TYPE_SPECIFIC)
          {
            //Read the specified relationship name:
            sharedptr<UsesRelationship> relationship_navigation_specific = sharedptr<UsesRelationship>::create();
            load_after_layout_item_usesrelationship(elementNavigationRelationshipSpecific, portal->get_table_used(table_name), relationship_navigation_specific);
            portal->set_navigation_relationship_specific(relationship_navigation_specific);
          }
        }

        load_after_layout_group(element, portal->get_table_used(table_name), portal, with_print_layout_positions);

        //Get the calendar portal's date field:
        if(calendar_portal)
        {
          const Glib::ustring date_field_name = XmlUtils::get_node_attribute_value(element, GLOM_ATTRIBUTE_PORTAL_CALENDAR_DATE_FIELD);
          sharedptr<Field> date_field = get_field(calendar_portal->get_table_used(table_name), date_field_name);
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
        sharedptr<LayoutItem_GroupBy> child_group = sharedptr<LayoutItem_GroupBy>::create();
        //Recurse:
        load_after_layout_group(element, table_name, child_group, with_print_layout_positions);

        //Group-By field:
        sharedptr<LayoutItem_Field> field_groupby = sharedptr<LayoutItem_Field>::create();
        xmlpp::Element* elementGroupBy = XmlUtils::get_node_child_named(element, GLOM_NODE_REPORT_ITEM_GROUPBY_GROUPBY);
        if(elementGroupBy)
        {
          load_after_layout_item_field(elementGroupBy, table_name, field_groupby);
          field_groupby->set_full_field_details( get_field(field_groupby->get_table_used(table_name), field_groupby->get_name()) );
        }
        child_group->set_field_group_by(field_groupby);


        //field_groupby.set_full_field_details_empty();

        //Sort fields:
        xmlpp::Element* elementSortBy = XmlUtils::get_node_child_named(element, GLOM_NODE_REPORT_ITEM_GROUPBY_SORTBY);
        if(elementSortBy)
        {
          LayoutItem_GroupBy::type_list_sort_fields sort_fields;
          load_after_sort_by(elementSortBy, table_name, sort_fields);
          child_group->set_fields_sort_by(sort_fields);
        }

        //Secondary fields:
        xmlpp::Element* elementSecondary = XmlUtils::get_node_child_named(element, GLOM_NODE_DATA_LAYOUT_GROUP_SECONDARYFIELDS);
        if(elementSecondary)
        {
          xmlpp::Element* elementGroup = XmlUtils::get_node_child_named(elementSecondary, GLOM_NODE_DATA_LAYOUT_GROUP);
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

    //Load formatting for any layout type that uses it:
    sharedptr<LayoutItem_WithFormatting> withformatting = sharedptr<LayoutItem_WithFormatting>::cast_dynamic(item_added);
    if(withformatting)
    {
       const xmlpp::Element* elementFormatting = XmlUtils::get_node_child_named(element, GLOM_NODE_FORMAT);
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

void Document::load_after_translations(const xmlpp::Element* element, const sharedptr<TranslatableItem>& item)
{
  if(!element)
    return;

  const sharedptr<ChoiceValue> choicevalue = sharedptr<ChoiceValue>::cast_dynamic(item);
  if(!choicevalue) //This item does not use the title, but uses the title translations to translate its value, if it is of type text.
  {
    item->set_title_original( XmlUtils::get_node_attribute_value(element, GLOM_ATTRIBUTE_TITLE));
  }

  const xmlpp::Element* nodeTranslations = XmlUtils::get_node_child_named(element, GLOM_NODE_TRANSLATIONS_SET);
  if(nodeTranslations)
  {
    xmlpp::Node::NodeList listNodesTranslations = nodeTranslations->get_children(GLOM_NODE_TRANSLATION);
    for(xmlpp::Node::NodeList::iterator iter = listNodesTranslations.begin(); iter != listNodesTranslations.end(); ++iter)
    {
      const xmlpp::Element* element = dynamic_cast<const xmlpp::Element*>(*iter);
      if(element)
      {
        const Glib::ustring locale = XmlUtils::get_node_attribute_value(element, GLOM_ATTRIBUTE_TRANSLATION_LOCALE);
        const Glib::ustring translation = XmlUtils::get_node_attribute_value(element, GLOM_ATTRIBUTE_TRANSLATION_VALUE);
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
  const sharedptr<HasTitleSingular> has_title_singular =
     sharedptr<HasTitleSingular>::cast_dynamic(item);
  if(has_title_singular)
  {
    const xmlpp::Element* nodeTitleSingular = XmlUtils::get_node_child_named(element, GLOM_NODE_TABLE_TITLE_SINGULAR);

    if(!has_title_singular->m_title_singular)
     has_title_singular->m_title_singular = sharedptr<TranslatableItem>::create();

    load_after_translations(nodeTitleSingular, has_title_singular->m_title_singular);
  }
}

void Document::load_after_print_layout_position(const xmlpp::Element* nodeItem, const sharedptr<LayoutItem>& item)
{
  if(!nodeItem)
    return;

  const xmlpp::Element* child = XmlUtils::get_node_child_named(nodeItem, GLOM_NODE_POSITION);
  if(child)
  {
    const double x = XmlUtils::get_node_attribute_value_as_decimal_double(child, GLOM_ATTRIBUTE_POSITION_X);
    const double y = XmlUtils::get_node_attribute_value_as_decimal_double(child, GLOM_ATTRIBUTE_POSITION_Y);
    const double width = XmlUtils::get_node_attribute_value_as_decimal_double(child, GLOM_ATTRIBUTE_POSITION_WIDTH);
    const double height = XmlUtils::get_node_attribute_value_as_decimal_double(child, GLOM_ATTRIBUTE_POSITION_HEIGHT);
    item->set_print_layout_position(x, y, width, height);
  }
}

void Document::load_after_choicevalue(const xmlpp::Element* element, const sharedptr<ChoiceValue>& item, Field::glom_field_type field_type)
{
  const Gnome::Gda::Value value = XmlUtils::get_node_attribute_value_as_value(element, GLOM_ATTRIBUTE_VALUE, field_type);
  item->set_value(value);

  sharedptr<ChoiceValue> nonconst_item = item; //TODO: Avoid this.
  load_after_translations(element, nonconst_item);
}

bool Document::load_after(int& failure_code)
{
  //Initialize the output variable:
  failure_code = 0;

  //TODO: Use some callback UI to show a busy cursor?
  /*
  //Use a std::auto_ptr<> to avoid even unncessarily instantiating a BusyCursor,
  //which would require GTK+ to be initialized:
  std::auto_ptr<BusyCursor> auto_cursor;
  if(m_parent_window)
    auto_cursor = std::auto_ptr<BusyCursor>( new BusyCursor(m_parent_window) );
  */

  m_block_modified_set = true; //Prevent the set_ functions from triggering a save.

  bool result = GlomBakery::Document_XML::load_after(failure_code);

  m_block_cache_update = true; //Don't waste time repeatedly updating this until we have finished.

  if(result)
  {
    m_translation_available_locales.clear();

    const xmlpp::Element* nodeRoot = get_node_document();
    if(nodeRoot)
    {
      m_document_format_version = XmlUtils::get_node_attribute_value_as_decimal(nodeRoot, GLOM_ATTRIBUTE_FORMAT_VERSION);

      if(m_document_format_version > get_latest_known_document_format_version())
      {
        std::cerr << G_STRFUNC << ": Loading failed because format_version=" << m_document_format_version << ", but latest known format version is " << get_latest_known_document_format_version() << std::endl;
        failure_code = LOAD_FAILURE_CODE_FILE_VERSION_TOO_NEW;
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

      const xmlpp::Element* nodeConnection = XmlUtils::get_node_child_named(nodeRoot, GLOM_NODE_CONNECTION);
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

        const Glib::ustring attr_mode = XmlUtils::get_node_attribute_value(nodeConnection, GLOM_ATTRIBUTE_CONNECTION_HOSTING_MODE);

        HostingMode mode = HOSTING_MODE_DEFAULT;

        if(attr_mode.empty())
        {
          // If no hosting mode is set, then try the self_hosted flag which
          // was used before sqlite support was implemented.
          const bool self_hosted = XmlUtils::get_node_attribute_value_as_bool(nodeConnection, GLOM_ATTRIBUTE_CONNECTION_SELF_HOSTED);
          mode = self_hosted ? HOSTING_MODE_POSTGRES_SELF : HOSTING_MODE_POSTGRES_CENTRAL;
        }
        else
        {
          if(attr_mode == GLOM_ATTRIBUTE_CONNECTION_HOSTING_POSTGRES_CENTRAL)
            mode = HOSTING_MODE_POSTGRES_CENTRAL;
          else if(attr_mode == GLOM_ATTRIBUTE_CONNECTION_HOSTING_POSTGRES_SELF)
            mode = HOSTING_MODE_POSTGRES_SELF;
          else if(attr_mode == GLOM_ATTRIBUTE_CONNECTION_HOSTING_SQLITE)
            mode = HOSTING_MODE_SQLITE;
          else if(attr_mode == GLOM_ATTRIBUTE_CONNECTION_HOSTING_MYSQL_CENTRAL)
            mode = HOSTING_MODE_MYSQL_CENTRAL;
          else if(attr_mode == GLOM_ATTRIBUTE_CONNECTION_HOSTING_MYSQL_SELF)
            mode = HOSTING_MODE_MYSQL_SELF;
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
      xmlpp::Node::NodeList listNodes = nodeRoot->get_children(GLOM_NODE_TABLE);
      for(xmlpp::Node::NodeList::const_iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
      {
        xmlpp::Element* nodeTable = dynamic_cast<xmlpp::Element*>(*iter);
        if(nodeTable)
        {
          const Glib::ustring table_name = XmlUtils::get_node_attribute_value(nodeTable, GLOM_ATTRIBUTE_NAME);

          const sharedptr<DocumentTableInfo> doctableinfo = sharedptr<DocumentTableInfo>::create();
          m_tables[table_name] = doctableinfo;

          sharedptr<TableInfo> table_info(new TableInfo());
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
          const xmlpp::Element* nodeRelationships = XmlUtils::get_node_child_named(nodeTable, GLOM_NODE_RELATIONSHIPS);
          if(nodeRelationships)
          {
            const xmlpp::Node::NodeList listNodes = nodeRelationships->get_children(GLOM_NODE_RELATIONSHIP);
            for(xmlpp::Node::NodeList::const_iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
            {
              const xmlpp::Element* nodeChild = dynamic_cast<xmlpp::Element*>(*iter);
              if(nodeChild)
              {
                sharedptr<Relationship> relationship = sharedptr<Relationship>::create();
                const Glib::ustring relationship_name = XmlUtils::get_node_attribute_value(nodeChild, GLOM_ATTRIBUTE_NAME);

                relationship->set_from_table(table_name);
                relationship->set_name(relationship_name);;

                relationship->set_from_field( XmlUtils::get_node_attribute_value(nodeChild, GLOM_ATTRIBUTE_KEY) );
                relationship->set_to_table( XmlUtils::get_node_attribute_value(nodeChild, GLOM_ATTRIBUTE_OTHER_TABLE) );
                relationship->set_to_field( XmlUtils::get_node_attribute_value(nodeChild, GLOM_ATTRIBUTE_OTHER_KEY) );
                relationship->set_auto_create( XmlUtils::get_node_attribute_value_as_bool(nodeChild, GLOM_ATTRIBUTE_AUTO_CREATE) );
                relationship->set_allow_edit( XmlUtils::get_node_attribute_value_as_bool(nodeChild, GLOM_ATTRIBUTE_ALLOW_EDIT) );

                //Translations:
                load_after_translations(nodeChild, relationship);

                doctableinfo->m_relationships.push_back(relationship);
              }
            }
          }

          //Fields:
          const xmlpp::Element* nodeFields = XmlUtils::get_node_child_named(nodeTable, GLOM_NODE_FIELDS);
          if(nodeFields)
          {
            const Field::type_map_type_names type_names = Field::get_type_names();

            //Loop through Field child nodes:
            xmlpp::Node::NodeList listNodes = nodeFields->get_children(GLOM_NODE_FIELD);
            for(xmlpp::Node::NodeList::const_iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
            {
              const xmlpp::Element* nodeChild = dynamic_cast<xmlpp::Element*>(*iter);
              if(nodeChild)
              {
                sharedptr<Field> field(new Field());

                const Glib::ustring strName = XmlUtils::get_node_attribute_value(nodeChild, GLOM_ATTRIBUTE_NAME);
                field->set_name( strName );

                field->set_primary_key( XmlUtils::get_node_attribute_value_as_bool(nodeChild, GLOM_ATTRIBUTE_PRIMARY_KEY) );
                field->set_unique_key( XmlUtils::get_node_attribute_value_as_bool(nodeChild, GLOM_ATTRIBUTE_UNIQUE) );
                field->set_auto_increment( XmlUtils::get_node_attribute_value_as_bool(nodeChild, GLOM_ATTRIBUTE_AUTOINCREMENT) );

                //Get lookup information, if present.
                xmlpp::Element* nodeLookup = XmlUtils::get_node_child_named(nodeChild, GLOM_NODE_FIELD_LOOKUP);
                if(nodeLookup)
                {
                  const Glib::ustring lookup_relationship_name = XmlUtils::get_node_attribute_value(nodeLookup, GLOM_ATTRIBUTE_RELATIONSHIP_NAME);
                  sharedptr<Relationship> lookup_relationship = get_relationship(table_name, lookup_relationship_name);
                  field->set_lookup_relationship(lookup_relationship);

                  field->set_lookup_field( XmlUtils::get_node_attribute_value(nodeLookup, GLOM_ATTRIBUTE_FIELD) );
                }

                field->set_calculation( XmlUtils::get_child_text_node(nodeChild, GLOM_NODE_CALCULATION) );
                if(!(field->get_has_calculation())) //Try the deprecated attribute instead
                  field->set_calculation( XmlUtils::get_node_attribute_value(nodeChild, GLOM_DEPRECATED_ATTRIBUTE_CALCULATION) );

                //Field Type:
                const Glib::ustring field_type = XmlUtils::get_node_attribute_value(nodeChild, GLOM_ATTRIBUTE_TYPE);

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


                //We set this after set_field_info(), because that gets a glom type from the (not-specified) gdatype. Yes, that's strange, and should probably be more explicit.
                field->set_glom_type(field_type_enum);

                field->set_default_value( XmlUtils::get_node_attribute_value_as_value(nodeChild, GLOM_ATTRIBUTE_DEFAULT_VALUE, field_type_enum) );

                //Default Formatting:
                const xmlpp::Element* elementFormatting = XmlUtils::get_node_child_named(nodeChild, GLOM_NODE_FORMAT);
                if(elementFormatting)
                  load_after_layout_item_formatting(elementFormatting, field->m_default_formatting, field_type_enum, table_name, strName);

                //Translations:
                load_after_translations(nodeChild, field);

                doctableinfo->m_fields.push_back(field);
              }
            }
          } //Fields

          // Load Example Rows after fields have been loaded, because they
          // need the fields to be able to associate a value to a named field.
          // TODO: Allow this to be loaded progressively from disk later,
          // instead of storing in it all in memory?
          const xmlpp::Element* nodeExampleRows = XmlUtils::get_node_child_named(nodeTable, GLOM_NODE_EXAMPLE_ROWS);
          if(nodeExampleRows)
          {
            //Loop through example_row child nodes:
            xmlpp::Node::NodeList listExampleRows = nodeExampleRows->get_children(GLOM_NODE_EXAMPLE_ROW);
            for(xmlpp::Node::NodeList::const_iterator iter = listExampleRows.begin(); iter != listExampleRows.end(); ++ iter)
            {
              const xmlpp::Element* nodeChild = dynamic_cast<xmlpp::Element*>(*iter);
              if(nodeChild)
              {
                type_row_data field_values(doctableinfo->m_fields.size());
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
                      //std::cout << "DEBUG: column_name = " << column_name->get_value() << " fields size=" << doctableinfo->m_fields.size() << std::endl;

                      // TODO_Performance: If it's too many rows we could
                      // consider a map to find the column more quickly.
                      for(unsigned int i = 0; i < doctableinfo->m_fields.size(); ++i)
                      {
                        sharedptr<const Field> field = doctableinfo->m_fields[i];
                        //std::cout << "  DEBUG: searching: field i=" << i << " =" << field->get_name() << std::endl;
                        if(field && (field->get_name() == column_name->get_value()))
                        {
                          field_values[i] = XmlUtils::get_node_text_child_as_value(nodeChild, field->get_glom_type());
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
      for(xmlpp::Node::NodeList::const_iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
      {
        xmlpp::Element* nodeTable = dynamic_cast<xmlpp::Element*>(*iter);
        if(nodeTable)
        {
          const Glib::ustring table_name = XmlUtils::get_node_attribute_value(nodeTable, GLOM_ATTRIBUTE_NAME);
          const sharedptr<DocumentTableInfo> doctableinfo = m_tables[table_name];

          //Layouts:
          const xmlpp::Element* nodeDataLayouts = XmlUtils::get_node_child_named(nodeTable, GLOM_NODE_DATA_LAYOUTS);
          if(nodeDataLayouts)
          {
            xmlpp::Node::NodeList listNodes = nodeDataLayouts->get_children(GLOM_NODE_DATA_LAYOUT);
            for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
            {
              xmlpp::Element* node = dynamic_cast<xmlpp::Element*>(*iter);
              if(node)
              {
                const Glib::ustring layout_name = XmlUtils::get_node_attribute_value(node, GLOM_ATTRIBUTE_NAME);
                const Glib::ustring layout_platform = XmlUtils::get_node_attribute_value(node, GLOM_ATTRIBUTE_LAYOUT_PLATFORM);

                type_list_layout_groups layout_groups;

                const xmlpp::Element* nodeGroups = XmlUtils::get_node_child_named(node, GLOM_NODE_DATA_LAYOUT_GROUPS);
                if(nodeGroups)
                {
                  //Look at all its children:
                  xmlpp::Node::NodeList listNodes = nodeGroups->get_children(GLOM_NODE_DATA_LAYOUT_GROUP);
                  for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
                  {
                    const xmlpp::Element* node = dynamic_cast<const xmlpp::Element*>(*iter);
                    if(node)
                    {
                      const Glib::ustring group_name = XmlUtils::get_node_attribute_value(node, GLOM_ATTRIBUTE_NAME);
                      if(!group_name.empty())
                      {
                        sharedptr<LayoutGroup> group(new LayoutGroup());
                        load_after_layout_group(node, table_name, group);

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
          const xmlpp::Element* nodeReports = XmlUtils::get_node_child_named(nodeTable, GLOM_NODE_REPORTS);
          if(nodeReports)
          {
            xmlpp::Node::NodeList listNodes = nodeReports->get_children(GLOM_NODE_REPORT);
            for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
            {
              xmlpp::Element* node = dynamic_cast<xmlpp::Element*>(*iter);
              if(node)
              {
                const Glib::ustring report_name = XmlUtils::get_node_attribute_value(node, GLOM_ATTRIBUTE_NAME);
                const bool show_table_title = XmlUtils::get_node_attribute_value_as_bool(node, GLOM_ATTRIBUTE_REPORT_SHOW_TABLE_TITLE);

                //type_list_layout_groups layout_groups;

                sharedptr<Report> report(new Report());
                report->set_name(report_name);
                report->set_show_table_title(show_table_title);

                const xmlpp::Element* nodeGroups = XmlUtils::get_node_child_named(node, GLOM_NODE_DATA_LAYOUT_GROUPS);
                if(nodeGroups)
                {
                  //Look at all its children:
                  xmlpp::Node::NodeList listNodes = nodeGroups->get_children(GLOM_NODE_DATA_LAYOUT_GROUP);
                  for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
                  {
                    const xmlpp::Element* node = dynamic_cast<const xmlpp::Element*>(*iter);
                    if(node)
                    {
                      sharedptr<LayoutGroup> group = report->get_layout_group();
                      group->remove_all_items();
                      load_after_layout_group(node, table_name, group);

                      fill_layout_field_details(table_name, group); //Get full field details from the field names.
                    }
                  }
                }

                //Translations:
                load_after_translations(node, report);

                doctableinfo->m_reports[report->get_name()] = report;
              }
            }
          } //if(nodeReports)


          //Print Layouts:
          const xmlpp::Element* nodePrintLayouts = XmlUtils::get_node_child_named(nodeTable, GLOM_NODE_PRINT_LAYOUTS);
          if(nodePrintLayouts)
          {
            xmlpp::Node::NodeList listNodes = nodePrintLayouts->get_children(GLOM_NODE_PRINT_LAYOUT);
            for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
            {
              xmlpp::Element* node = dynamic_cast<xmlpp::Element*>(*iter);
              if(node)
              {
                const Glib::ustring name = XmlUtils::get_node_attribute_value(node, GLOM_ATTRIBUTE_NAME);
                const bool show_table_title = XmlUtils::get_node_attribute_value_as_bool(node, GLOM_ATTRIBUTE_REPORT_SHOW_TABLE_TITLE);

                sharedptr<PrintLayout> print_layout(new PrintLayout());
                print_layout->set_name(name);
                print_layout->set_show_table_title(show_table_title);

                print_layout->set_show_grid(
                  XmlUtils::get_node_attribute_value_as_bool(node, GLOM_ATTRIBUTE_PRINT_LAYOUT_SHOW_GRID) );
                print_layout->set_show_rules(
                  XmlUtils::get_node_attribute_value_as_bool(node, GLOM_ATTRIBUTE_PRINT_LAYOUT_SHOW_RULES) );
                print_layout->set_show_outlines(
                  XmlUtils::get_node_attribute_value_as_bool(node, GLOM_ATTRIBUTE_PRINT_LAYOUT_SHOW_OUTLINES) );

                //Get the horizontal and vertical rules:
                PrintLayout::type_vec_doubles vec_rules_h;
                xmlpp::Node::NodeList listRules = node->get_children(GLOM_NODE_HORIZONTAL_RULE);
                for(xmlpp::Node::NodeList::iterator iter = listRules.begin(); iter != listRules.end(); ++iter)
                {
                  const xmlpp::Element* node = dynamic_cast<const xmlpp::Element*>(*iter);
                  if(!node)
                    continue;

                  const double pos = XmlUtils::get_node_attribute_value_as_decimal(node, GLOM_ATTRIBUTE_RULE_POSITION);
                  vec_rules_h.push_back(pos);
                }
		print_layout->set_horizontal_rules(vec_rules_h);

		PrintLayout::type_vec_doubles vec_rules_v;
                listRules = node->get_children(GLOM_NODE_VERTICAL_RULE);
                for(xmlpp::Node::NodeList::iterator iter = listRules.begin(); iter != listRules.end(); ++iter)
                {
                  const xmlpp::Element* node = dynamic_cast<const xmlpp::Element*>(*iter);
                  if(!node)
                    continue;

                  const double pos = XmlUtils::get_node_attribute_value_as_decimal(node, GLOM_ATTRIBUTE_RULE_POSITION);
                  vec_rules_v.push_back(pos);
                }
		print_layout->set_vertical_rules(vec_rules_v);


                //Page Setup:
                const Glib::ustring key_file_text = XmlUtils::get_child_text_node(node, GLOM_NODE_PAGE_SETUP);
                print_layout->set_page_setup(key_file_text);
                
                print_layout->set_page_count(
                  XmlUtils::get_node_attribute_value_as_decimal(node, GLOM_ATTRIBUTE_PRINT_LAYOUT_PAGE_COUNT, 1));
                 

                //Layout Groups:
                const xmlpp::Element* nodeGroups = XmlUtils::get_node_child_named(node, GLOM_NODE_DATA_LAYOUT_GROUPS);
                if(nodeGroups)
                {
                  //Look at all its children:
                  xmlpp::Node::NodeList listNodes = nodeGroups->get_children(GLOM_NODE_DATA_LAYOUT_GROUP);
                  for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
                  {
                    const xmlpp::Element* node = dynamic_cast<const xmlpp::Element*>(*iter);
                    if(node)
                    {
                      sharedptr<LayoutGroup> group = print_layout->get_layout_group();
                      group->remove_all_items();
                      load_after_layout_group(node, table_name, group, true /* load positions too. */);

                      fill_layout_field_details(table_name, group); //Get full field details from the field names.
                    }
                  }
                }

                //Translations:
                load_after_translations(node, print_layout);

                doctableinfo->m_print_layouts[print_layout->get_name()] = print_layout;
              }
            }
          } //if(nodePrintLayouts)


          //Groups:
          //These are only used when recreating the database, for instance from an example file.
          m_groups.clear();

          const xmlpp::Element* nodeGroups = XmlUtils::get_node_child_named(nodeRoot, GLOM_NODE_GROUPS);
          if(nodeGroups)
          {
            xmlpp::Node::NodeList listNodes = nodeGroups->get_children(GLOM_NODE_GROUP);
            for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
            {
              xmlpp::Element* node = dynamic_cast<xmlpp::Element*>(*iter);
              if(node)
              {
                GroupInfo group_info;

                group_info.set_name( XmlUtils::get_node_attribute_value(node, GLOM_ATTRIBUTE_NAME) );
                group_info.m_developer = XmlUtils::get_node_attribute_value_as_bool(node, GLOM_ATTRIBUTE_DEVELOPER);

                xmlpp::Node::NodeList listTablePrivs = node->get_children(GLOM_NODE_TABLE_PRIVS);
                for(xmlpp::Node::NodeList::iterator iter = listTablePrivs.begin(); iter != listTablePrivs.end(); ++iter)
                {
                  xmlpp::Element* node = dynamic_cast<xmlpp::Element*>(*iter);
                  if(node)
                  {
                    const Glib::ustring table_name = XmlUtils::get_node_attribute_value(node, GLOM_ATTRIBUTE_TABLE_NAME);

                    Privileges privs;
                    privs.m_view = XmlUtils::get_node_attribute_value_as_bool(node, GLOM_ATTRIBUTE_PRIV_VIEW);
                    privs.m_edit = XmlUtils::get_node_attribute_value_as_bool(node, GLOM_ATTRIBUTE_PRIV_EDIT);
                    privs.m_create = XmlUtils::get_node_attribute_value_as_bool(node, GLOM_ATTRIBUTE_PRIV_CREATE);
                    privs.m_delete = XmlUtils::get_node_attribute_value_as_bool(node, GLOM_ATTRIBUTE_PRIV_DELETE);

                    group_info.m_map_privileges[table_name] = privs;
                  }
                }

                m_groups[group_info.get_name()] = group_info;
              }
            }
          }


          //Library Modules:
          m_map_library_scripts.clear();

          const xmlpp::Element* nodeModules = XmlUtils::get_node_child_named(nodeRoot, GLOM_NODE_LIBRARY_MODULES);
          if(nodeModules)
          {
            xmlpp::Node::NodeList listNodes = nodeModules->get_children(GLOM_NODE_LIBRARY_MODULE);
            for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
            {
              xmlpp::Element* node = dynamic_cast<xmlpp::Element*>(*iter);
              if(node)
              {
                //The name is in an attribute:
                const Glib::ustring module_name = XmlUtils::get_node_attribute_value(node, GLOM_ATTRIBUTE_LIBRARY_MODULE_NAME);

                //The string is in a child text node:
                Glib::ustring script;

                const xmlpp::TextNode* text_child = node->get_child_text();
                if(text_child)
                  script = text_child->get_content();

                //Fall back to the deprecated attribute:
                if(script.empty())
                  script = XmlUtils::get_node_attribute_value(node, GLOM_ATTRIBUTE_LIBRARY_MODULE_SCRIPT);

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

void Document::save_before_layout_item_formatting(xmlpp::Element* nodeItem, const sharedptr<const LayoutItem_WithFormatting>& layout_item)
{
  if(!layout_item)
    return;

  const Formatting& format = layout_item->m_formatting;

  sharedptr<const LayoutItem_Field> field = sharedptr<const LayoutItem_Field>::cast_dynamic(layout_item);

  Field::glom_field_type field_type = Field::TYPE_INVALID;
  if(field)
    field_type = field->get_glom_type();

  save_before_layout_item_formatting(nodeItem, format, field_type);
}

void Document::save_before_layout_item_formatting(xmlpp::Element* nodeItem, const Formatting& format, Field::glom_field_type field_type)
{
  //Numeric format:
  if(field_type != Field::TYPE_INVALID)  //These options are only for fields:
  {
    if(field_type == Field::TYPE_NUMERIC)
    {
      XmlUtils::set_node_attribute_value_as_bool(nodeItem, GLOM_ATTRIBUTE_FORMAT_THOUSANDS_SEPARATOR,  format.m_numeric_format.m_use_thousands_separator);
      XmlUtils::set_node_attribute_value_as_bool(nodeItem, GLOM_ATTRIBUTE_FORMAT_DECIMAL_PLACES_RESTRICTED, format.m_numeric_format.m_decimal_places_restricted);
      XmlUtils::set_node_attribute_value_as_decimal(nodeItem, GLOM_ATTRIBUTE_FORMAT_DECIMAL_PLACES, format.m_numeric_format.m_decimal_places);
      XmlUtils::set_node_attribute_value(nodeItem, GLOM_ATTRIBUTE_FORMAT_CURRENCY_SYMBOL, format.m_numeric_format.m_currency_symbol);
      XmlUtils::set_node_attribute_value_as_bool(nodeItem, GLOM_ATTRIBUTE_FORMAT_USE_ALT_NEGATIVE_COLOR,
        format.m_numeric_format.m_alt_foreground_color_for_negatives);
    }

    bool as_radio_buttons = false;
    const bool choices_restricted = format.get_choices_restricted(as_radio_buttons);
    XmlUtils::set_node_attribute_value_as_bool(nodeItem, GLOM_ATTRIBUTE_FORMAT_CHOICES_RESTRICTED, choices_restricted);
    XmlUtils::set_node_attribute_value_as_bool(nodeItem, GLOM_ATTRIBUTE_FORMAT_CHOICES_RESTRICTED_AS_RADIO_BUTTONS, as_radio_buttons);
    XmlUtils::set_node_attribute_value_as_bool(nodeItem, GLOM_ATTRIBUTE_FORMAT_CHOICES_CUSTOM, format.get_has_custom_choices());
  }

  //Text formatting:
  if(field_type == Field::TYPE_TEXT)
  {
    XmlUtils::set_node_attribute_value_as_bool(nodeItem, GLOM_ATTRIBUTE_FORMAT_TEXT_MULTILINE, format.get_text_format_multiline());
    XmlUtils::set_node_attribute_value_as_decimal(nodeItem, GLOM_ATTRIBUTE_FORMAT_TEXT_MULTILINE_HEIGHT_LINES, 
      format.get_text_format_multiline_height_lines(), GLOM_ATTRIBUTE_FORMAT_TEXT_MULTILINE_HEIGHT_LINES_DEFAULT);
  }

  XmlUtils::set_node_attribute_value(nodeItem, GLOM_ATTRIBUTE_FORMAT_TEXT_FONT, format.get_text_format_font());
  XmlUtils::set_node_attribute_value(nodeItem, GLOM_ATTRIBUTE_FORMAT_TEXT_COLOR_FOREGROUND, format.get_text_format_color_foreground());
  XmlUtils::set_node_attribute_value(nodeItem, GLOM_ATTRIBUTE_FORMAT_TEXT_COLOR_BACKGROUND, format.get_text_format_color_background());

  //Alignment:
  const Formatting::HorizontalAlignment alignment = format.get_horizontal_alignment();
  if(alignment != Formatting::HORIZONTAL_ALIGNMENT_AUTO) //Save file-size by not even writing this.
  {
    const Glib::ustring alignment_str =
      (alignment == Formatting::HORIZONTAL_ALIGNMENT_LEFT  ? GLOM_ATTRIBUTE_FORMAT_HORIZONTAL_ALIGNMENT_LEFT : GLOM_ATTRIBUTE_FORMAT_HORIZONTAL_ALIGNMENT_RIGHT);
    XmlUtils::set_node_attribute_value(nodeItem, GLOM_ATTRIBUTE_FORMAT_HORIZONTAL_ALIGNMENT, alignment_str);
  }

  //Choices:
  if(field_type != Field::TYPE_INVALID)
  {
    if(format.get_has_custom_choices())
    {
      xmlpp::Element* child = nodeItem->add_child(GLOM_ATTRIBUTE_FORMAT_CHOICES_CUSTOM_LIST);

      const Formatting::type_list_values list_values = format.get_choices_custom();
      for(Formatting::type_list_values::const_iterator iter = list_values.begin(); iter != list_values.end(); ++iter)
      {
        const sharedptr<const ChoiceValue> value = *iter; 
        xmlpp::Element* childChoice = child->add_child(GLOM_NODE_FORMAT_CUSTOM_CHOICE);
        save_before_choicevalue(childChoice, value, field_type);
      }
    }

    XmlUtils::set_node_attribute_value_as_bool(nodeItem, GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED, format.get_has_related_choices() );

    sharedptr<const Relationship> choice_relationship;
    sharedptr<const LayoutItem_Field> choice_layout_first;
    sharedptr<const LayoutGroup> choice_extra_layouts;
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
        xmlpp::Element* nodeExtraLayout = nodeItem->add_child(GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_EXTRA_LAYOUT);
        xmlpp::Element* nodeGroups = nodeExtraLayout->add_child(GLOM_NODE_DATA_LAYOUT_GROUPS);
        save_before_layout_group(nodeGroups, choice_extra_layouts);
      }

      if(!choice_sort_fields.empty())
      {
        xmlpp::Element* nodeSortBy = nodeItem->add_child(GLOM_ATTRIBUTE_FORMAT_CHOICES_RELATED_SORTBY);
        save_before_sort_by(nodeSortBy, choice_sort_fields);
      }
    }
  }
}

void Document::save_before_layout_item_usesrelationship(xmlpp::Element* nodeItem, const sharedptr<const UsesRelationship>& item)
{
  if(!item)
    return;

  XmlUtils::set_node_attribute_value(nodeItem, GLOM_ATTRIBUTE_RELATIONSHIP_NAME, item->get_relationship_name());
  XmlUtils::set_node_attribute_value(nodeItem, GLOM_ATTRIBUTE_RELATED_RELATIONSHIP_NAME, item->get_related_relationship_name());
}

void Document::save_before_layout_item_field(xmlpp::Element* nodeItem, const sharedptr<const LayoutItem_Field>& field)
{
  if(!field)
    return;

  XmlUtils::set_node_attribute_value(nodeItem, GLOM_ATTRIBUTE_NAME, field->get_name());
  save_before_layout_item_usesrelationship(nodeItem, field);
  XmlUtils::set_node_attribute_value_as_bool(nodeItem, GLOM_ATTRIBUTE_EDITABLE, field->get_editable());

  XmlUtils::set_node_attribute_value_as_bool(nodeItem, GLOM_ATTRIBUTE_DATA_LAYOUT_ITEM_FIELD_USE_DEFAULT_FORMATTING, field->get_formatting_use_default());

  sharedptr<const CustomTitle> custom_title = field->get_title_custom();
  if(custom_title)
  {
    xmlpp::Element* elementCustomTitle = nodeItem->add_child(GLOM_NODE_LAYOUT_ITEM_CUSTOM_TITLE);
    XmlUtils::set_node_attribute_value_as_bool(elementCustomTitle, GLOM_ATTRIBUTE_LAYOUT_ITEM_CUSTOM_TITLE_USE, custom_title->get_use_custom_title());

    save_before_translations(elementCustomTitle, custom_title);
  }
}

void Document::save_before_sort_by(xmlpp::Element* node, const LayoutItem_GroupBy::type_list_sort_fields& list_fields)
{
  if(!node)
    return;

  for(LayoutItem_GroupBy::type_list_sort_fields::const_iterator iter = list_fields.begin(); iter != list_fields.end(); ++iter)
  {
    sharedptr<const LayoutItem_Field> field = iter->first;

    xmlpp::Element* nodeChild = node->add_child(GLOM_NODE_DATA_LAYOUT_ITEM_FIELD);
    save_before_layout_item_field(nodeChild, field);

    XmlUtils::set_node_attribute_value_as_bool(nodeChild, GLOM_ATTRIBUTE_SORT_ASCENDING, iter->second);
  }
}

void Document::save_before_layout_group(xmlpp::Element* node, const sharedptr<const LayoutGroup>& group, bool with_print_layout_positions)
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
    if(!group_by->get_secondary_fields()->m_list_items.empty())
    {
      xmlpp::Element* secondary_fields = child->add_child(GLOM_NODE_DATA_LAYOUT_GROUP_SECONDARYFIELDS);
      save_before_layout_group(secondary_fields, group_by->get_secondary_fields(), with_print_layout_positions);
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
              sharedptr<const LayoutItem_CalendarPortal> calendar_portal = sharedptr<const LayoutItem_CalendarPortal>::cast_dynamic(portal);
              if(calendar_portal)
              {
                child = node->add_child(GLOM_NODE_DATA_LAYOUT_CALENDAR_PORTAL);
                sharedptr<const Field> date_field = calendar_portal->get_date_field();
                if(date_field)
                  XmlUtils::set_node_attribute_value(child, GLOM_ATTRIBUTE_PORTAL_CALENDAR_DATE_FIELD, date_field->get_name());
              }
              else
                child = node->add_child(GLOM_NODE_DATA_LAYOUT_PORTAL);

              save_before_layout_item_usesrelationship(child, portal);

              //Portal navigation details:
              Glib::ustring navigation_type_string;
              sharedptr<const UsesRelationship> relationship_navigation_specific;

              switch(portal->get_navigation_type())
              {
                case LayoutItem_Portal::NAVIGATION_AUTOMATIC:
                  //We leave this blank to use the default.
                  break;
                case LayoutItem_Portal::NAVIGATION_NONE:
                  navigation_type_string = GLOM_ATTRIBUTE_PORTAL_NAVIGATION_TYPE_NONE;
                  break;
                case LayoutItem_Portal::NAVIGATION_SPECIFIC:
                  navigation_type_string = GLOM_ATTRIBUTE_PORTAL_NAVIGATION_TYPE_SPECIFIC;
                  break;
                default:
                  break;
              }

              //Empty means the default ("automatic")
              //In that case we don't even write the node, to keep the XML small:
              if(!navigation_type_string.empty())
              {
                xmlpp::Element* child_navigation_relationship = child->add_child(GLOM_NODE_DATA_LAYOUT_PORTAL_NAVIGATIONRELATIONSHIP);

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
              sharedptr<const LayoutItem_Notebook> notebook = sharedptr<const LayoutItem_Notebook>::cast_dynamic(group);
              if(notebook) //If it is a notebook.
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
  for(LayoutGroup::type_list_const_items::const_iterator iterItems = items.begin(); iterItems != items.end(); ++iterItems)
  {
    sharedptr<const LayoutItem> item = *iterItems;
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
        XmlUtils::set_node_attribute_value(nodeItem, GLOM_ATTRIBUTE_LAYOUT_ITEM_FIELDSUMMARY_SUMMARYTYPE, fieldsummary->get_summary_type_sql()); //The SQL name is as good as anything as an identifier for the summary function.
      }
      else
      {
        sharedptr<const LayoutItem_Field> field = sharedptr<const LayoutItem_Field>::cast_dynamic(item);
        if(field) //If it is a field
        {
          nodeItem = child->add_child(GLOM_NODE_DATA_LAYOUT_ITEM_FIELD);
          save_before_layout_item_field(nodeItem, field);
        }
        else
        {
          sharedptr<const LayoutItem_Button> button = sharedptr<const LayoutItem_Button>::cast_dynamic(item);
          if(button) //If it is a button
          {
            nodeItem = child->add_child(GLOM_NODE_DATA_LAYOUT_BUTTON);
            XmlUtils::set_child_text_node(nodeItem, GLOM_NODE_BUTTON_SCRIPT, button->get_script());
            save_before_translations(nodeItem, button);
          }
          else
          {
            sharedptr<const LayoutItem_Text> textobject = sharedptr<const LayoutItem_Text>::cast_dynamic(item);
            if(textobject) //If it is a text object.
            {
              nodeItem = child->add_child(GLOM_NODE_DATA_LAYOUT_TEXTOBJECT);
              save_before_translations(nodeItem, textobject);

              //The text is translatable too, so we use a node for it:
              xmlpp::Element* element_text = nodeItem->add_child(GLOM_NODE_DATA_LAYOUT_TEXTOBJECT_TEXT);
              save_before_translations(element_text, textobject->m_text);
            }
            else
            {
              sharedptr<const LayoutItem_Image> imageobject = sharedptr<const LayoutItem_Image>::cast_dynamic(item);
              if(imageobject) //If it is an image object.
              {
                nodeItem = child->add_child(GLOM_NODE_DATA_LAYOUT_IMAGEOBJECT);
                save_before_translations(nodeItem, imageobject);

                xmlpp::Element* nodeValue = nodeItem->add_child(GLOM_NODE_VALUE);
                XmlUtils::set_node_text_child_as_value(nodeValue, imageobject->get_image(), Field::TYPE_IMAGE);
              }
              else
              {
                sharedptr<const LayoutItem_Line> line = sharedptr<const LayoutItem_Line>::cast_dynamic(item);
                if(line) //If it is a line
                {
                  nodeItem = child->add_child(GLOM_NODE_DATA_LAYOUT_LINE);
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
          sharedptr<const LayoutItem_WithFormatting> withformatting = sharedptr<const LayoutItem_WithFormatting>::cast_dynamic(item);
          if(withformatting)
          {
            xmlpp::Element* elementFormat = nodeItem->add_child(GLOM_NODE_FORMAT);
              save_before_layout_item_formatting(elementFormat, withformatting);
          }
        }
      }

      if(nodeItem)
      {
        //Attributes that any layout item could have:
        const guint column_width = item->get_display_width();
        XmlUtils::set_node_attribute_value_as_decimal(nodeItem, GLOM_ATTRIBUTE_LAYOUT_ITEM_COLUMN_WIDTH, column_width);

        if(with_print_layout_positions)
          save_before_print_layout_position(nodeItem, item);
      }
    }

    //g_warning("save_before_layout_group: after child part type=%s", item->get_part_type_name().c_str());
  }
}

void Document::save_before_translations(xmlpp::Element* element, const sharedptr<const TranslatableItem>& item)
{
  if(!element)
    return;

  const sharedptr<const ChoiceValue> choicevalue = sharedptr<const ChoiceValue>::cast_dynamic(item);
  if(!choicevalue) //This item does not use the title, but uses the title translations to translate its value, if it is of type text.
  {
    XmlUtils::set_node_attribute_value(element, GLOM_ATTRIBUTE_TITLE, item->get_title_original());
  }

  if(!item->get_has_translations())
    return;

  xmlpp::Element* child = element->add_child(GLOM_NODE_TRANSLATIONS_SET);

  const TranslatableItem::type_map_locale_to_translations& map_translations = item->_get_translations_map();
  for(TranslatableItem::type_map_locale_to_translations::const_iterator iter = map_translations.begin(); iter != map_translations.end(); ++iter)
  {
    xmlpp::Element* childItem = child->add_child(GLOM_NODE_TRANSLATION);
    XmlUtils::set_node_attribute_value(childItem, GLOM_ATTRIBUTE_TRANSLATION_LOCALE, iter->first);
    XmlUtils::set_node_attribute_value(childItem, GLOM_ATTRIBUTE_TRANSLATION_VALUE, iter->second);
  }

  //If it has a singular title, then save that too:
  const sharedptr<const HasTitleSingular> has_title_singular =
    sharedptr<const HasTitleSingular>::cast_dynamic(item);
  if(has_title_singular && has_title_singular->m_title_singular
    && !(has_title_singular->m_title_singular->get_title_original().empty()))
  {
    xmlpp::Element* nodeTitleSingular = element->add_child(GLOM_NODE_TABLE_TITLE_SINGULAR);
    save_before_translations(nodeTitleSingular, has_title_singular->m_title_singular);
  }
}

void Document::save_before_print_layout_position(xmlpp::Element* nodeItem, const sharedptr<const LayoutItem>& item)
{
  xmlpp::Element* child = nodeItem->add_child(GLOM_NODE_POSITION);

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
    nodeItem->remove_child(child);
}

void Document::save_before_choicevalue(xmlpp::Element* nodeItem, const sharedptr<const ChoiceValue>& item, Field::glom_field_type field_type)
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
  //Use a std::auto_ptr<> to avoid even unncessarily instantiating a BusyCursor,
  //which would require GTK+ to be initialized:
  std::auto_ptr<BusyCursor> auto_cursor;
  if(m_parent_window)
    auto_cursor = std::auto_ptr<BusyCursor>( new BusyCursor(m_parent_window) );
  */

  //TODO: Add xmlpp::Document::remove_root_node() to libxml++
  xmlpp::Element* nodeRoot = get_node_document();

  if(nodeRoot)
  {
    // Remove existing child nodes:
    xmlpp::Node::NodeList listNodesToRemove = nodeRoot->get_children();
    for(xmlpp::Node::NodeList::iterator iter = listNodesToRemove.begin(); iter != listNodesToRemove.end(); ++iter)
      nodeRoot->remove_child(*iter);

    //Always save as the latest format,
    //possibly making it impossible to open this document in older versions of Glom:
    m_document_format_version = get_latest_known_document_format_version();
    XmlUtils::set_node_attribute_value_as_decimal(nodeRoot, GLOM_ATTRIBUTE_FORMAT_VERSION, m_document_format_version);

    XmlUtils::set_node_attribute_value_as_bool(nodeRoot, GLOM_ATTRIBUTE_IS_EXAMPLE, m_is_example);
    XmlUtils::set_node_attribute_value_as_bool(nodeRoot, GLOM_ATTRIBUTE_IS_BACKUP, m_is_backup);
    XmlUtils::set_node_attribute_value(nodeRoot, GLOM_ATTRIBUTE_TRANSLATION_ORIGINAL_LOCALE, m_translation_original_locale);

    save_before_translations(nodeRoot, m_database_title);

    XmlUtils::set_child_text_node(nodeRoot, GLOM_NODE_STARTUP_SCRIPT, m_startup_script);

    xmlpp::Element* nodeConnection = XmlUtils::get_node_child_named_with_add(nodeRoot, GLOM_NODE_CONNECTION);

    switch(m_hosting_mode)
    {
    case HOSTING_MODE_POSTGRES_CENTRAL:
      XmlUtils::set_node_attribute_value(nodeConnection, 
        GLOM_ATTRIBUTE_CONNECTION_HOSTING_MODE, GLOM_ATTRIBUTE_CONNECTION_HOSTING_POSTGRES_CENTRAL);
      break;
    case HOSTING_MODE_POSTGRES_SELF:
      XmlUtils::set_node_attribute_value(nodeConnection,
        GLOM_ATTRIBUTE_CONNECTION_HOSTING_MODE, GLOM_ATTRIBUTE_CONNECTION_HOSTING_POSTGRES_SELF);
      break;
    case HOSTING_MODE_SQLITE:
      XmlUtils::set_node_attribute_value(nodeConnection,
        GLOM_ATTRIBUTE_CONNECTION_HOSTING_MODE, GLOM_ATTRIBUTE_CONNECTION_HOSTING_SQLITE);
      break;
    case HOSTING_MODE_MYSQL_CENTRAL:
      XmlUtils::set_node_attribute_value(nodeConnection, 
        GLOM_ATTRIBUTE_CONNECTION_HOSTING_MODE, GLOM_ATTRIBUTE_CONNECTION_HOSTING_MYSQL_CENTRAL);
      break;
    case HOSTING_MODE_MYSQL_SELF:
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
    xmlpp::Node::NodeList listNodes = nodeRoot->get_children(GLOM_NODE_TABLE);
    for(xmlpp::Node::NodeList::iterator iter = listNodes.begin(); iter != listNodes.end(); ++iter)
      nodeRoot->remove_child(*iter);

    //Add tables:
    for(type_tables::const_iterator iter = m_tables.begin(); iter != m_tables.end(); ++iter)
    {
      const sharedptr<const DocumentTableInfo> doctableinfo = iter->second;
      if(!doctableinfo || !doctableinfo->m_info)
        continue;

      const Glib::ustring table_name = doctableinfo->m_info->get_name();
      if(table_name.empty())
        std::cerr << G_STRFUNC << ": table name is empty." << std::endl;

      if(!table_name.empty())
      {
        xmlpp::Element* nodeTable = nodeRoot->add_child(GLOM_NODE_TABLE);
        XmlUtils::set_node_attribute_value(nodeTable, GLOM_ATTRIBUTE_NAME, table_name);
        XmlUtils::set_node_attribute_value_as_bool(nodeTable, GLOM_ATTRIBUTE_HIDDEN, doctableinfo->m_info->get_hidden());
        XmlUtils::set_node_attribute_value_as_bool(nodeTable, GLOM_ATTRIBUTE_DEFAULT, doctableinfo->m_info->get_default());

        XmlUtils::set_node_attribute_value_as_float(nodeTable, GLOM_ATTRIBUTE_OVERVIEW_X, doctableinfo->m_overviewx);
        XmlUtils::set_node_attribute_value_as_float(nodeTable, GLOM_ATTRIBUTE_OVERVIEW_Y, doctableinfo->m_overviewy);

        if(m_is_example) //The example data is useless to non-example files (and is big):
        {
          xmlpp::Element* nodeExampleRows = nodeTable->add_child(GLOM_NODE_EXAMPLE_ROWS);

          for(type_example_rows::const_iterator iter = doctableinfo->m_example_rows.begin(); iter != doctableinfo->m_example_rows.end(); ++iter)
          {
            xmlpp::Element* nodeExampleRow = nodeExampleRows->add_child(GLOM_NODE_EXAMPLE_ROW);
            const type_row_data& row_data = *iter;
            if(!row_data.empty())
            {
              const unsigned int row_data_size = row_data.size();
              for(unsigned int i = 0; i < row_data_size; ++i)
              {
                sharedptr<const Field> field = doctableinfo->m_fields[i];
                if(!field)
                  break;

                xmlpp::Element* nodeField = nodeExampleRow->add_child(GLOM_NODE_VALUE);
                XmlUtils::set_node_attribute_value(nodeField, GLOM_ATTRIBUTE_COLUMN, field->get_name());
                XmlUtils::set_node_text_child_as_value(nodeField, row_data[i], field->get_glom_type());
              } // for each value
            } // !row_data.empty
          } // for each row
        } // m_is_example


        //Translations:
        save_before_translations(nodeTable, doctableinfo->m_info);

        //Fields:
        xmlpp::Element* elemFields = nodeTable->add_child(GLOM_NODE_FIELDS);

        const Field::type_map_type_names type_names = Field::get_type_names();

        for(type_vec_fields::const_iterator iter = doctableinfo->m_fields.begin(); iter != doctableinfo->m_fields.end(); ++iter)
        {
          sharedptr<const Field> field = *iter;

          xmlpp::Element* elemField = elemFields->add_child(GLOM_NODE_FIELD);
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
            xmlpp::Element* elemFieldLookup = elemField->add_child(GLOM_NODE_FIELD_LOOKUP);

            sharedptr<Relationship> lookup_relationship = field->get_lookup_relationship();
            XmlUtils::set_node_attribute_value(elemFieldLookup, GLOM_ATTRIBUTE_RELATIONSHIP_NAME, glom_get_sharedptr_name(lookup_relationship));

            XmlUtils::set_node_attribute_value(elemFieldLookup, GLOM_ATTRIBUTE_FIELD, field->get_lookup_field());
          }

          //Default Formatting:
          xmlpp::Element* elementFormat = elemField->add_child(GLOM_NODE_FORMAT);
          save_before_layout_item_formatting(elementFormat, field->m_default_formatting, field->get_glom_type());

          //Translations:
          save_before_translations(elemField, field);
        } /* fields */

        //Relationships:
        //Add new <relationships> node:
        xmlpp::Element* elemRelationships = nodeTable->add_child(GLOM_NODE_RELATIONSHIPS);

        //Add each <relationship> node:
        const type_vec_relationships relationships = doctableinfo->m_relationships;
        for(type_vec_relationships::const_iterator iter = relationships.begin(); iter != relationships.end(); ++iter)
        {
          sharedptr<const Relationship> relationship = *iter;
          if(relationship)
          {
            xmlpp::Element* elemRelationship = elemRelationships->add_child(GLOM_NODE_RELATIONSHIP);
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
        xmlpp::Element* nodeDataLayouts = nodeTable->add_child(GLOM_NODE_DATA_LAYOUTS);

        //Add the groups:
        //Make sure that we always get these _after_ the relationships.
        for(DocumentTableInfo::type_layouts::const_iterator iter = doctableinfo->m_layouts.begin(); iter != doctableinfo->m_layouts.end(); ++iter)
        {
          xmlpp::Element* nodeLayout = nodeDataLayouts->add_child(GLOM_NODE_DATA_LAYOUT);
          XmlUtils::set_node_attribute_value(nodeLayout, GLOM_ATTRIBUTE_NAME, iter->m_layout_name);
          XmlUtils::set_node_attribute_value(nodeLayout, GLOM_ATTRIBUTE_LAYOUT_PLATFORM, iter->m_layout_platform);

          xmlpp::Element* nodeGroups = nodeLayout->add_child(GLOM_NODE_DATA_LAYOUT_GROUPS);

          const type_list_layout_groups& groups = iter->m_layout_groups;
          for(type_list_layout_groups::const_iterator iterGroups = groups.begin(); iterGroups != groups.end(); ++iterGroups)
          {
            save_before_layout_group(nodeGroups, *iterGroups);
          }
        }

        //Reports:
        xmlpp::Element* nodeReports = nodeTable->add_child(GLOM_NODE_REPORTS);

        //Add the groups:
        for(DocumentTableInfo::type_reports::const_iterator iter = doctableinfo->m_reports.begin(); iter != doctableinfo->m_reports.end(); ++iter)
        {
          xmlpp::Element* nodeReport = nodeReports->add_child(GLOM_NODE_REPORT);

          sharedptr<const Report> report = iter->second;
          XmlUtils::set_node_attribute_value(nodeReport, GLOM_ATTRIBUTE_NAME, report->get_name());
          XmlUtils::set_node_attribute_value_as_bool(nodeReport, GLOM_ATTRIBUTE_REPORT_SHOW_TABLE_TITLE, report->get_show_table_title());

          xmlpp::Element* nodeGroups = nodeReport->add_child(GLOM_NODE_DATA_LAYOUT_GROUPS);
          save_before_layout_group(nodeGroups, report->get_layout_group());

          //Translations:
          save_before_translations(nodeReport, report);
        }

        //Print Layouts:
        xmlpp::Element* nodePrintLayouts = nodeTable->add_child(GLOM_NODE_PRINT_LAYOUTS);

        //Add the print :
        for(DocumentTableInfo::type_print_layouts::const_iterator iter = doctableinfo->m_print_layouts.begin(); iter != doctableinfo->m_print_layouts.end(); ++iter)
        {
          xmlpp::Element* nodePrintLayout = nodePrintLayouts->add_child(GLOM_NODE_PRINT_LAYOUT);

          sharedptr<const PrintLayout> print_layout = iter->second;
          XmlUtils::set_node_attribute_value(nodePrintLayout, GLOM_ATTRIBUTE_NAME, print_layout->get_name());
          XmlUtils::set_node_attribute_value_as_bool(nodePrintLayout, GLOM_ATTRIBUTE_REPORT_SHOW_TABLE_TITLE, print_layout->get_show_table_title());

          XmlUtils::set_node_attribute_value_as_bool(nodePrintLayout, GLOM_ATTRIBUTE_PRINT_LAYOUT_SHOW_GRID, print_layout->get_show_grid());
          XmlUtils::set_node_attribute_value_as_bool(nodePrintLayout, GLOM_ATTRIBUTE_PRINT_LAYOUT_SHOW_RULES, print_layout->get_show_rules());
          XmlUtils::set_node_attribute_value_as_bool(nodePrintLayout, GLOM_ATTRIBUTE_PRINT_LAYOUT_SHOW_OUTLINES, print_layout->get_show_outlines());

          //Save the rule lines:
          const PrintLayout::type_vec_doubles h_rules = print_layout->get_horizontal_rules();
          for(PrintLayout::type_vec_doubles::const_iterator iter = h_rules.begin();
            iter != h_rules.end(); ++iter)
          {
            xmlpp::Element* child = nodePrintLayout->add_child(GLOM_NODE_HORIZONTAL_RULE);
            XmlUtils::set_node_attribute_value_as_decimal(child, GLOM_ATTRIBUTE_RULE_POSITION, *iter);
          }

          const PrintLayout::type_vec_doubles v_rules = print_layout->get_vertical_rules();
          for(PrintLayout::type_vec_doubles::const_iterator iter = v_rules.begin();
            iter != v_rules.end(); ++iter)
          {
            xmlpp::Element* child = nodePrintLayout->add_child(GLOM_NODE_VERTICAL_RULE);
            XmlUtils::set_node_attribute_value_as_decimal(child, GLOM_ATTRIBUTE_RULE_POSITION, *iter);
          }

          //Page Setup:
          const std::string page_setup = print_layout->get_page_setup();
          if(!page_setup.empty())
          {
            xmlpp::Element* child = nodePrintLayout->add_child(GLOM_NODE_PAGE_SETUP);
            child->add_child_text( Utils::string_clean_for_xml(page_setup) );
          }
          
          XmlUtils::set_node_attribute_value_as_decimal(nodePrintLayout, GLOM_ATTRIBUTE_PRINT_LAYOUT_PAGE_COUNT, 
            print_layout->get_page_count(), 1);

          xmlpp::Element* nodeGroups = nodePrintLayout->add_child(GLOM_NODE_DATA_LAYOUT_GROUPS);
          save_before_layout_group(nodeGroups, print_layout->get_layout_group(), true /* x,y positions too. */);

          //Translations:
          save_before_translations(nodePrintLayout, print_layout);
        }
      }

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

      const Glib::ustring group_name = group_info.get_name();
      if(group_name.empty())
      {
        //I saw this in at least one .glom file. murrayc.
        std::cerr << G_STRFUNC << ": The group name is empty." << std::endl;
        continue;
      }

      xmlpp::Element* nodeGroup = nodeGroups->add_child(GLOM_NODE_GROUP);
      XmlUtils::set_node_attribute_value(nodeGroup, GLOM_ATTRIBUTE_NAME, group_name);
      XmlUtils::set_node_attribute_value_as_bool(nodeGroup, GLOM_ATTRIBUTE_DEVELOPER, group_info.m_developer);

      //The privileges for each table, for this group:
      for(GroupInfo::type_map_table_privileges::const_iterator iter = group_info.m_map_privileges.begin(); iter != group_info.m_map_privileges.end(); ++iter)
      {
        xmlpp::Element* nodeTablePrivs = nodeGroup->add_child(GLOM_NODE_TABLE_PRIVS);

        XmlUtils::set_node_attribute_value(nodeTablePrivs, GLOM_ATTRIBUTE_TABLE_NAME, iter->first);

        const Privileges& privs = iter->second;
        XmlUtils::set_node_attribute_value_as_bool(nodeTablePrivs, GLOM_ATTRIBUTE_PRIV_VIEW, privs.m_view);
        XmlUtils::set_node_attribute_value_as_bool(nodeTablePrivs, GLOM_ATTRIBUTE_PRIV_EDIT, privs.m_edit);
        XmlUtils::set_node_attribute_value_as_bool(nodeTablePrivs, GLOM_ATTRIBUTE_PRIV_CREATE, privs.m_create);
        XmlUtils::set_node_attribute_value_as_bool(nodeTablePrivs, GLOM_ATTRIBUTE_PRIV_DELETE, privs.m_delete);
      }
    }

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

      //The name is in an attribute:
      XmlUtils::set_node_attribute_value(nodeModule, GLOM_ATTRIBUTE_LIBRARY_MODULE_NAME, name);

      //The script is in a child text node:
      xmlpp::TextNode* text_child = nodeModule->get_child_text();
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
  const Glib::ustring title = get_database_title_original();
  if(title.empty())
    return GlomBakery::Document_XML::get_name();
  else
    return title;
}

Document::type_list_groups Document::get_groups() const
{
  type_list_groups result;
  for(type_map_groups::const_iterator iter = m_groups.begin(); iter != m_groups.end(); ++iter)
  {
    result.push_back(iter->second);
  }

  return result;
}

///This adds the group if necessary.
void Document::set_group(GroupInfo& group)
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

void Document::remove_group(const Glib::ustring& group_name)
{
  type_map_groups::iterator iter = m_groups.find(group_name);
  if(iter != m_groups.end())
  {
    m_groups.erase(iter);
    set_modified();
  }
}

std::vector<Glib::ustring> Document::get_report_names(const Glib::ustring& table_name) const
{
  const sharedptr<const DocumentTableInfo> info = get_table_info(table_name);
  if(info)
  {
    std::vector<Glib::ustring> result;
    for(DocumentTableInfo::type_reports::const_iterator iter = info->m_reports.begin(); iter != info->m_reports.end(); ++iter)
    {
      result.push_back(iter->second->get_name());
    }

    return result;
  }
  else
    return std::vector<Glib::ustring>();
}

void Document::set_report(const Glib::ustring& table_name, const sharedptr<Report>& report)
{
  const sharedptr<DocumentTableInfo> info = get_table_info(table_name);
  if(info)
  {
    info->m_reports[report->get_name()] = report;
    set_modified();
  }
}

sharedptr<Report> Document::get_report(const Glib::ustring& table_name, const Glib::ustring& report_name) const
{
  const sharedptr<const DocumentTableInfo> info = get_table_info(table_name);
  if(info)
  {
    DocumentTableInfo::type_reports::const_iterator iterFindReport = info->m_reports.find(report_name);
    if(iterFindReport != info->m_reports.end())
    {
      return iterFindReport->second;
    }
  }

  return sharedptr<Report>();
}

void Document::remove_report(const Glib::ustring& table_name, const Glib::ustring& report_name)
{
  const sharedptr<DocumentTableInfo> info = get_table_info(table_name);
  if(info)
  {
    DocumentTableInfo::type_reports::iterator iterFindReport = info->m_reports.find(report_name);
    if(iterFindReport != info->m_reports.end())
    {
      info->m_reports.erase(iterFindReport);

      set_modified();
    }
  }
}


std::vector<Glib::ustring> Document::get_print_layout_names(const Glib::ustring& table_name) const
{
  const sharedptr<const DocumentTableInfo> info = get_table_info(table_name);
  if(info)
  {
    std::vector<Glib::ustring> result;
    for(DocumentTableInfo::type_print_layouts::const_iterator iter = info->m_print_layouts.begin(); iter != info->m_print_layouts.end(); ++iter)
    {
      result.push_back(iter->second->get_name());
    }

    return result;
  }
  else
    return std::vector<Glib::ustring>();
}


void Document::set_print_layout(const Glib::ustring& table_name, const sharedptr<PrintLayout>& print_layout)
{
  const sharedptr<DocumentTableInfo> info = get_table_info(table_name);
  if(info)
  {
    info->m_print_layouts[print_layout->get_name()] = print_layout;
    set_modified();
  }
}

sharedptr<PrintLayout> Document::get_print_layout(const Glib::ustring& table_name, const Glib::ustring& print_layout_name) const
{
  const sharedptr<const DocumentTableInfo> info = get_table_info(table_name);
  if(info)
  {
    DocumentTableInfo::type_print_layouts::const_iterator iterFindPrintLayout = info->m_print_layouts.find(print_layout_name);
    if(iterFindPrintLayout != info->m_print_layouts.end())
    {
      return iterFindPrintLayout->second;
    }
  }

  return sharedptr<PrintLayout>();
}

void Document::remove_print_layout(const Glib::ustring& table_name, const Glib::ustring& print_layout_name)
{
  const sharedptr<DocumentTableInfo> info = get_table_info(table_name);
  if(info)
  {
    DocumentTableInfo::type_print_layouts::iterator iterFindPrintLayout = info->m_print_layouts.find(print_layout_name);
    if(iterFindPrintLayout != info->m_print_layouts.end())
    {
      info->m_print_layouts.erase(iterFindPrintLayout);

      set_modified();
    }
  }
}



bool Document::get_relationship_is_to_one(const Glib::ustring& table_name, const Glib::ustring& relationship_name) const
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

sharedptr<const Relationship> Document::get_field_used_in_relationship_to_one(const Glib::ustring& table_name, const sharedptr<const LayoutItem_Field>& layout_field) const
{
  sharedptr<const Relationship> result;

  if(!layout_field)
  {
    std::cerr << G_STRFUNC << ": layout_field was null" << std::endl;
    return result;
  }

  const Glib::ustring table_used = layout_field->get_table_used(table_name);
  const sharedptr<const DocumentTableInfo> table_info = get_table_info(table_used);
  if(!table_info)
  {
    //This table is special. We would not create a relationship to it using a field:
    if(table_used == GLOM_STANDARD_TABLE_PREFS_TABLE_NAME)
      return result;
     
    std::cerr << G_STRFUNC << ": table not found:" << table_used << std::endl;
    return result;
  }

  //Look at each relationship:
  const Glib::ustring field_name = layout_field->get_name();
  for(type_vec_relationships::const_iterator iterRel = table_info->m_relationships.begin(); iterRel != table_info->m_relationships.end(); ++iterRel)
  {
    sharedptr<const Relationship> relationship = *iterRel;
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
  const sharedptr<DocumentTableInfo> info = get_table_info(table_name);
  if(info)
    info->m_map_current_record.clear();
}

void Document::set_layout_record_viewed(const Glib::ustring& table_name, const Glib::ustring& layout_name, const Gnome::Gda::Value& primary_key_value)
{
  const sharedptr<DocumentTableInfo> info = get_table_info(table_name);
  if(info)
    info->m_map_current_record[layout_name] = primary_key_value;
}

Gnome::Gda::Value Document::get_layout_record_viewed(const Glib::ustring& table_name, const Glib::ustring& layout_name) const
{
  const sharedptr<const DocumentTableInfo> info = get_table_info(table_name);
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
  const sharedptr<DocumentTableInfo> info = get_table_info(table_name);
  if(info)
    info->m_layout_current = layout_name;
}

void Document::set_criteria_current(const Glib::ustring& table_name, const FoundSet& found_set)
{
  const sharedptr<DocumentTableInfo> info = get_table_info(table_name);
  if(info)
    info->m_foundset_current = found_set;
}

Glib::ustring Document::get_layout_current(const Glib::ustring& table_name) const
{
  const sharedptr<const DocumentTableInfo> info = get_table_info(table_name);
  if(info)
    return info->m_layout_current;

  return Glib::ustring(); //not found.
}

FoundSet Document::get_criteria_current(const Glib::ustring& table_name) const
{
  const sharedptr<const DocumentTableInfo> info = get_table_info(table_name);
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

/** A predicate for use with std::find_if() to find a pair which contains the same information.
 */
class predicate_ItemAndHintEqual
{
public:
  predicate_ItemAndHintEqual(const Document::pair_translatable_item_and_hint& item_and_hint)
  {
    m_item_and_hint = item_and_hint;
  }

  virtual ~predicate_ItemAndHintEqual()
  {
  }

  bool operator() (const Document::pair_translatable_item_and_hint& item_and_hint)
  {
    if(!item_and_hint.first && m_item_and_hint.first)
      return true;

    if(item_and_hint.first && !m_item_and_hint.first)
      return true;

    if(item_and_hint.first && m_item_and_hint.first)
    {
      if(item_and_hint.first->get_title_original() != m_item_and_hint.first->get_title_original())
        return false;
    }

    if(get_po_context_for_item(item_and_hint.first, item_and_hint.second) != 
      get_po_context_for_item(m_item_and_hint.first, m_item_and_hint.second))
    {
      return false;
    }

    return true;
  }

private:
  Document::pair_translatable_item_and_hint m_item_and_hint;
};

} //anonymous namespace

static void add_to_translatable_list(Document::type_list_translatables& list, const sharedptr<TranslatableItem>& item, const Glib::ustring& hint)
{
  // Only add the item/hint combination if it is not there already:
  const Document::pair_translatable_item_and_hint item_and_hint(item, hint);
  if(std::find_if(list.begin(), list.end(),
    predicate_ItemAndHintEqual(item_and_hint)) == list.end())
  {
    list.push_back( item_and_hint );
  }
}

static void add_to_translatable_list(Document::type_list_translatables& list, const Document::type_list_translatables& sublist)
{
  // Only add the item/hint combination if it is not there already:
  for(Document::type_list_translatables::const_iterator iter = sublist.begin(); iter != sublist.end(); ++iter)
  {
    const Document::pair_translatable_item_and_hint& item_and_hint = *iter;
    if(std::find_if(list.begin(), list.end(),
      predicate_ItemAndHintEqual(item_and_hint)) == list.end())
    {
      list.push_back( item_and_hint );
    }
  }
}

template <typename T_List>
static void translatable_items_append_with_hint(Document::type_list_translatables& result, T_List& list_items, const Glib::ustring& hint)
{
  for(typename T_List::iterator iter = list_items.begin(); iter != list_items.end(); ++iter)
  {
    add_to_translatable_list(result, *iter, hint);
  }
}

Document::type_list_translatables Document::get_translatable_items()
{
  type_list_translatables result;
  add_to_translatable_list(result, m_database_title, "");

  //Add tables:
  type_listTableInfo tables = get_tables();
  for(type_listTableInfo::const_iterator iter = tables.begin(); iter != tables.end(); ++iter)
  {
    sharedptr<TableInfo> tableinfo = *iter;
    if(!tableinfo)
      continue;

    add_to_translatable_list(result, tableinfo, "");

    const Glib::ustring table_name = tableinfo->get_name();

    //The table's field titles:
    const Glib::ustring hint = "Parent table: " + table_name;
    type_vec_fields fields = get_table_fields(table_name);
    for(type_vec_fields::iterator iter = fields.begin(); iter != fields.end(); ++iter)
    {
      sharedptr<Field> field = *iter;
      if(!field)
        continue;

      add_to_translatable_list(result, field, hint);

      //Custom Choices, if any:
      if(field->get_glom_type() == Field::TYPE_TEXT) //Choices for other field types could not be translated.
      {
        const Glib::ustring this_hint = hint + ", Parent Field: " + field->get_name();   
        type_list_translatables list_choice_items;
        Document::fill_translatable_custom_choices(field->m_default_formatting, list_choice_items, this_hint);
        add_to_translatable_list(result, list_choice_items);
      }
    }

    //The table's relationships:
    type_vec_relationships relationships = get_relationships(table_name);
    translatable_items_append_with_hint(result, relationships, hint);

    //The table's report titles:
    std::vector<Glib::ustring> listReports = get_report_names(table_name);
    for(std::vector<Glib::ustring>::iterator iter = listReports.begin(); iter != listReports.end(); ++iter)
    {
      const Glib::ustring report_name = *iter;
      sharedptr<Report> report = get_report(table_name, report_name);
      if(!report)
        continue;

      add_to_translatable_list(result, report, hint);
      
      //Translatable report items:
      const Glib::ustring this_hint = hint + ", Parent Report: " + report->get_name();
      type_list_translatables list_layout_items = get_translatable_report_items(table_name, report_name, this_hint);
      add_to_translatable_list(result, list_layout_items);
    }

    //The table's print layout titles:
    std::vector<Glib::ustring> listPrintLayouts = get_print_layout_names(table_name);
    for(std::vector<Glib::ustring>::iterator iter = listPrintLayouts.begin(); iter != listPrintLayouts.end(); ++iter)
    {
      const Glib::ustring print_layout_name = *iter;
      sharedptr<PrintLayout> print_layout = get_print_layout(table_name, print_layout_name);
      if(!print_layout)
        continue;

      add_to_translatable_list(result, print_layout, hint);
      
      //Translatable print layout items:
      const Glib::ustring this_hint = hint + ", Print Layout: " + print_layout->get_name();
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

  const sharedptr<DocumentTableInfo> info = get_table_info(table_name);
  if(!info)
    return result;

  //Look at each layout:
  DocumentTableInfo::type_layouts layouts = info->m_layouts;
  for(DocumentTableInfo::type_layouts::iterator iterLayouts = info->m_layouts.begin(); iterLayouts != info->m_layouts.end(); ++iterLayouts)
  {
    //Look at each group:
    for(type_list_layout_groups::iterator iterGroup = iterLayouts->m_layout_groups.begin(); iterGroup != iterLayouts->m_layout_groups.end(); ++iterGroup)
    {
      sharedptr<LayoutGroup> group = *iterGroup;
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

  sharedptr<Report> report = get_report(table_name, report_name);
  if(report)
    fill_translatable_layout_items(report->get_layout_group(), the_list, hint);

  return the_list;
}

Document::type_list_translatables Document::get_translatable_print_layout_items(const Glib::ustring& table_name, const Glib::ustring& print_layout_name, const Glib::ustring& hint)
{
  Document::type_list_translatables the_list;

  sharedptr<PrintLayout> print_layout = get_print_layout(table_name, print_layout_name);
  if(print_layout)
    fill_translatable_layout_items(print_layout->get_layout_group(), the_list, hint);

  return the_list;
}

void Document::fill_translatable_custom_choices(Formatting& formatting, type_list_translatables& the_list, const Glib::ustring& hint)
{
  if(!formatting.get_has_custom_choices())
    return;

  Formatting::type_list_values values = formatting.get_choices_custom();
  for(Formatting::type_list_values::iterator iter = values.begin(); iter != values.end(); ++iter)
  {
    sharedptr<ChoiceValue> value = *iter;

    the_list.push_back( pair_translatable_item_and_hint(value, hint) );
  }
}

void Document::fill_translatable_layout_items(const sharedptr<LayoutItem_Field>& layout_field, type_list_translatables& the_list, const Glib::ustring& hint)
{
  //LayoutItem_Field items do not have their own titles.
  //They use either the field's title or a custom title:
  sharedptr<CustomTitle> custom_title = layout_field->get_title_custom();
  if(custom_title)
  {
    the_list.push_back( pair_translatable_item_and_hint(custom_title, hint) ); 
  }

  //The field will be added separately.
  
  //Custom Choices, if any:
  //Only text fields can have translated choice values:
  if(layout_field->get_glom_type() == Field::TYPE_TEXT)
  {
    const Glib::ustring choice_hint = hint + ", Parent Field: " + layout_field->get_name();
    fill_translatable_custom_choices(layout_field->m_formatting, the_list, hint);
  }
}


void Document::fill_translatable_layout_items(const sharedptr<LayoutGroup>& group, type_list_translatables& the_list, const Glib::ustring& hint)
{
  //Portals don't have their own titles - they use the relationship title (though we might want to allow custom titles)
  sharedptr<LayoutItem_Portal> portal = sharedptr<LayoutItem_Portal>::cast_dynamic(group);
  if(!portal)
  {
    the_list.push_back( pair_translatable_item_and_hint(group, hint) );
  }
  
  const Glib::ustring group_name = group->get_name();
  Glib::ustring this_hint = hint;
  if(!group_name.empty())
    this_hint += ", Parent Group: " + group_name;

  //Look at each item:
  LayoutGroup::type_list_items items = group->get_items();
  for(LayoutGroup::type_list_items::const_iterator iterItems = items.begin(); iterItems != items.end(); ++iterItems)
  {
    sharedptr<LayoutItem> item = *iterItems;

    sharedptr<LayoutGroup> child_group = sharedptr<LayoutGroup>::cast_dynamic(item);
    if(child_group) //If it is a group, portal, summary, or groupby.
    {
      sharedptr<LayoutItem_GroupBy> group_by = sharedptr<LayoutItem_GroupBy>::cast_dynamic(child_group);
      if(group_by)
      {
        sharedptr<LayoutItem_Field> field = group_by->get_field_group_by();
        fill_translatable_layout_items(field, the_list, hint);

        fill_translatable_layout_items(group_by->get_secondary_fields(), the_list, this_hint);
      }

      //recurse:
      fill_translatable_layout_items(child_group, the_list, this_hint);
    }
    else
    {
      //Buttons too:
      sharedptr<LayoutItem_Button> button = sharedptr<LayoutItem_Button>::cast_dynamic(item);
      if(button)
        the_list.push_back( pair_translatable_item_and_hint(button, this_hint) );
      else
      {
        sharedptr<LayoutItem_Text> text = sharedptr<LayoutItem_Text>::cast_dynamic(item);
        if(text)
        {
          the_list.push_back( pair_translatable_item_and_hint(text, this_hint) );
          if(text->m_text)
            the_list.push_back( pair_translatable_item_and_hint(text->m_text, this_hint) );
        }
        else
        {
          //Images have titles:
          sharedptr<LayoutItem_Image> image = sharedptr<LayoutItem_Image>::cast_dynamic(item);
          if(image)
            the_list.push_back( pair_translatable_item_and_hint(image, this_hint) );
          else
          {
            sharedptr<LayoutItem_Field> layout_field = sharedptr<LayoutItem_Field>::cast_dynamic(item);
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
  for(type_map_library_scripts::const_iterator iter = m_map_library_scripts.begin(); iter != m_map_library_scripts.end(); ++iter)
  {
    result.push_back(iter->first);
  }

  return result;
}

void Document::set_library_module(const Glib::ustring& name, const Glib::ustring& script)
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
  type_map_library_scripts::iterator iter = m_map_library_scripts.find(name);
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
  Document* unconst = const_cast<Document*>(this);

  unconst->save_before(); //This is the part of the Document_XML overrides that sets the contents string from the XML tree.
  return get_contents();
}

void Document::set_opened_from_browse(bool val)
{
  m_opened_from_browse = val;

  //This should stop developer mode from being possible,
  //because we don't have access to the document:
  if(!val)
    m_app_state.set_userlevel(AppState::USERLEVEL_OPERATOR);
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

    const int r = (*release_func_)(ptr_);
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

  ScopedArchiveEntryPtr(const ScopedArchivePtr<T_Object>&);
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
  const std::string path = file_parent->get_relative_path(file);
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
      const gssize bytes_read = stream->read(buffer, BYTES_TO_PROCESS);
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
  const std::string path_dir = Glib::filename_from_uri(uri);
  const std::string basename_dir = Glib::path_get_basename(path_dir);
  const std::string& filepath_document = Glib::build_filename(path_dir, basename_dir + ".glom");
  const Glib::ustring uri_document = Glib::filename_to_uri(filepath_document);

  const Glib::ustring fileuri_old = get_file_uri();
  set_allow_autosave(false); //Prevent saving while we modify the document:
  set_file_uri(uri_document, true); //true = enforce file extension;
  set_is_backup_file(true);
  const bool saved = save();

  set_file_uri(fileuri_old);
  set_is_backup_file(false);
  set_allow_autosave(true);

  if(!saved)
  {
    std::cerr << G_STRFUNC << ": Saving of the backup .glom file failed with URI:" << uri_document << std::endl;
    return Glib::ustring();
  }


  //Save the data:
  ConnectionPool* connection_pool = ConnectionPool::get_instance();
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
  const std::string backup_data_path = Glib::build_filename(path_dir, "glom_postgres_data", "backup");
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

  const size_t size = archive_entry_size(entry);
  const Glib::ScopedPtr<char> buf ((char*) g_malloc(size + 1));

  const ssize_t r = archive_read_data(a, buf.get(), size);
    
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
  const std::string filename_tarball = Glib::filename_from_uri(backup_uri);

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

  slot_progress();


  //const char *name = archive_entry_pathname(entry);
  //std::cout << "debug: name=" << name << std::endl;

  slot_progress();

  Glib::ustring contents;

  //Read the whole file in one go,
  //We'd have to keep it all in memory anyway as we concatentated it,
  //if we did it in chunks.
  slot_progress();



  std::string contents_glom_file;

  struct archive_entry* entry = 0;
  while(archive_read_next_header(a, &entry) == ARCHIVE_OK)
  {
    const char* pathname = archive_entry_pathname(entry);
    if(!pathname)
      continue;

    const std::string basename = Glib::path_get_basename(pathname);
    //std::cout << G_STRFUNC << ": debug: basename=" << basename << std::endl;

    bool is_glom_file = false;
    const std::string without_suffix = Glom::Utils::string_remove_suffix(basename, ".glom");
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
  const type_vec_fields fields = get_table_fields(table_name); //TODO_Performance: Cache this?
  for(type_vec_fields::const_iterator iter = fields.begin(); iter != fields.end();  ++iter)
  {
    sharedptr<Field> field = *iter;

    //Examine each field that looks up its data from a relationship:
    if(field && field->get_is_lookup())
    {
      //Get the relationship information:
      sharedptr<Relationship> relationship = field->get_lookup_relationship();
      if(relationship)
      {
        //If the relationship is triggererd by the specified field:
        if(relationship->get_from_field() == field_name)
        {
          //Add it:
          sharedptr<LayoutItem_Field> item = sharedptr<LayoutItem_Field>::create();
          item->set_full_field_details(field);
          result.push_back( type_pairFieldTrigger(item, relationship) );
        }
      }
    }
  }

  return result;
}

} //namespace Glom
