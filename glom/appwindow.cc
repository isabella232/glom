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

#include "config.h" //For VERSION, GLOM_ENABLE_CLIENT_ONLY, GLOM_ENABLE_SQLITE

#include <glom/appwindow.h>
#include <glom/dialog_existing_or_new.h>
#include <glom/bakery/dialog_offersave.h>
#include <libglom/string_utils.h>
#include <libglom/file_utils.h>

#ifndef GLOM_ENABLE_CLIENT_ONLY
#include <glom/mode_design/translation/dialog_change_language.h>
#include <glom/mode_design/translation/window_translations.h>
#include <glom/utility_widgets/filechooserdialog_saveextras.h>
#include <glom/glade_utils.h>
#endif // !GLOM_ENABLE_CLIENT_ONLY

#include <glom/utils_ui.h>
#include <glom/glade_utils.h>
#include <libglom/algorithms_utils.h>
#include <libglom/db_utils.h>
#include <libglom/db_utils_export.h>
#include <libglom/privs.h>
#include <libglom/utils.h>
#include <glom/python_embed/python_ui_callbacks.h>
#include <glom/python_embed/glom_python.h>
#include <libglom/spawn_with_feedback.h>
#include <giomm/menu.h>

#include <gtkmm/main.h>

#include <giomm/file.h>
#include <glibmm/spawn.h>
#include <glibmm/convert.h>
#include <sstream> //For stringstream.

#ifndef G_OS_WIN32
#include <libepc/consumer.h>
#include <libsoup/soup-status.h>
#endif // !G_OS_WIN32

#ifndef G_OS_WIN32
# include <netdb.h> //For gethostbyname().
#endif

#include <glibmm/i18n.h>

namespace Glom
{

static const Glib::ustring ACTION_GROUP_NAME_REPORTS = "reports-list";
static const Glib::ustring ACTION_GROUP_NAME_TABLES = "tables-list";
static const Glib::ustring ACTION_GROUP_NAME_PRINT_LAYOUTS = "print-layouts-list";


//static const int GLOM_RESPONSE_BROWSE_NETWORK = 1;

// Global application variable
AppWindow* global_appwindow = nullptr;

Glib::ustring AppWindow::m_current_locale;
Glib::ustring AppWindow::m_original_locale;

const char* AppWindow::glade_id("window_main");
const bool AppWindow::glade_developer(false);

AppWindow::AppWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: GlomBakery::AppWindow_WithDoc("Glom"),
  Gtk::ApplicationWindow(cobject),
  m_builder(builder),
  m_vbox(nullptr),
  m_vbox_placeHolder(Gtk::ORIENTATION_VERTICAL),
  m_box_top(nullptr),
  m_frame(nullptr),
  m_about_shown(false),
  m_about(nullptr),
#ifndef GLOM_ENABLE_CLIENT_ONLY
  m_window_translations(nullptr),
#endif // !GLOM_ENABLE_CLIENT_ONLY
#ifndef GLOM_ENABLE_CLIENT_ONLY
  m_ui_save_extra_showextras(false),
  m_ui_save_extra_newdb_hosting_mode(Document::HostingMode::DEFAULT),
  m_avahi_progress_dialog(nullptr),
#endif // !GLOM_ENABLE_CLIENT_ONLY
  m_show_sql_debug(false)
{
  init_app_name("Glom");

  Gtk::Window::set_default_icon_name("glom");

  //Load widgets from glade file:
  builder->get_widget("bakery_vbox", m_box_top);
  builder->get_widget_derived("vbox_frame", m_frame); //This one is derived. There's a lot happening here.
  builder->get_widget_derived("infobar_progress", m_infobar_progress);

  //Add menu bar at the top:
  auto object =
    builder->get_object("mainmenu");
  auto gmenu =
    Glib::RefPtr<Gio::Menu>::cast_dynamic(object);
  if(!gmenu)
    g_warning("GMenu not found");

  m_menubar = std::make_unique<Gtk::MenuBar>(gmenu);
  m_menubar->show();
  m_box_top->pack_start(*m_menubar, Gtk::PACK_SHRINK);

  //TODO: Remove our use of add_accelerator() in application.cc,
  //if there is ever an easier way to make the 'accel's from the .glade file just work.
  //See https://bugzilla.gnome.org/show_bug.cgi?id=708905

  add_mime_type("application/x-glom"); //TODO: make this actually work - we need to register it properly.

#ifndef GLOM_ENABLE_CLIENT_ONLY
#ifndef G_OS_WIN32
  //Install UI hooks for this:
  auto connection_pool = ConnectionPool::get_instance();
  if(connection_pool)
  {
    connection_pool->set_avahi_publish_callbacks(
      std::bind(&AppWindow::on_connection_avahi_begin, this),
      std::bind(&AppWindow::on_connection_avahi_progress, this),
      std::bind(&AppWindow::on_connection_avahi_done, this) );
  }
#endif
#endif // !GLOM_ENABLE_CLIENT_ONLY

  global_appwindow = this;
}

AppWindow::~AppWindow()
{
  #ifndef GLOM_ENABLE_CLIENT_ONLY
  if(m_window_translations)
  {
    m_frame->remove_view(m_window_translations);
    delete m_window_translations;
  }

  delete m_avahi_progress_dialog;
  m_avahi_progress_dialog = nullptr;

  #endif // !GLOM_ENABLE_CLIENT_ONLY
  
  delete m_about;
  m_about = nullptr;

  //This was set in the constructor:
  global_appwindow = nullptr;
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void AppWindow::on_connection_avahi_begin()
{
  //Create the dialog:
  delete m_avahi_progress_dialog;
  m_avahi_progress_dialog = nullptr;

  m_avahi_progress_dialog = new Gtk::MessageDialog(UiUtils::bold_message(_("Glom: Generating Encryption Certificates")), true, Gtk::MESSAGE_INFO);
  m_avahi_progress_dialog->set_secondary_text(_("Please wait while Glom prepares your system for publishing over the network."));
  m_avahi_progress_dialog->set_transient_for(*this);
  m_avahi_progress_dialog->show();
}

void AppWindow::on_connection_avahi_progress()
{
  //Allow GTK+ to process events, so that the UI is responsive:
  while(Gtk::Main::events_pending())
   Gtk::Main::iteration();
}

void AppWindow::on_connection_avahi_done()
{
  //Delete the dialog:
  delete m_avahi_progress_dialog;
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

void AppWindow::init()
{  
  GlomBakery::AppWindow_WithDoc::init(); //Create document and ask to show it in the UI.
  init_layout();
  show();
}

bool AppWindow::init_with_document(const Glib::ustring& document_uri, bool restore)
{
  init(); //calls init_menus()

  //m_frame->set_shadow_type(Gtk::SHADOW_IN);

  if(document_uri.empty())
  {
    auto document = std::static_pointer_cast<Document>(get_document());
    if(document && document->get_connection_database().empty()) //If it is a new (default) document.
    {
        return offer_new_or_existing();
    }
  }
  else
  {
    if(restore)
      return do_restore_backup(document_uri);
    else
    {
      const auto test = open_document(document_uri);
      if(!test)
        return offer_new_or_existing();
    }
  }

  return true;
  //show_all();
}

bool AppWindow::get_show_sql_debug() const
{
  return m_show_sql_debug;
}

void AppWindow::set_show_sql_debug(bool val)
{
  m_show_sql_debug = val;

  ConnectionPool::get_instance()->set_show_debug_output(val);
}

void AppWindow::set_stop_auto_server_shutdown(bool val)
{
  auto connection_pool = ConnectionPool::get_instance();
  if(connection_pool)
    connection_pool->set_auto_server_shutdown(!val);
}

void AppWindow::init_layout()
{
}

void AppWindow::init_menus_file()
{
  // File menu

  //Build actions:
  m_action_group_file = Gio::SimpleActionGroup::create();

  //File actions
  m_action_group_file->add_action("new",
    sigc::mem_fun((AppWindow&)*this, &AppWindow::on_menu_file_new));
  m_action_group_file->add_action("open",
    sigc::mem_fun((GlomBakery::AppWindow_WithDoc&)*this, &GlomBakery::AppWindow_WithDoc::on_menu_file_open));

  Glib::RefPtr<Gio::SimpleAction> action;
#ifndef GLOM_ENABLE_CLIENT_ONLY
  action = m_action_group_file->add_action("save-as-example",
    sigc::mem_fun((AppWindow&)*this, &AppWindow::on_menu_file_save_as_example));
  add_developer_action(action);

  action = m_action_group_file->add_action("export",
    sigc::mem_fun(*m_frame, &Frame_Glom::on_menu_file_export));
  m_listTableSensitiveActions.emplace_back(action);

  action = m_action_group_file->add_action("import",
    sigc::mem_fun(*m_frame, &Frame_Glom::on_menu_file_import));
  m_listTableSensitiveActions.emplace_back(action);

  m_toggleaction_network_shared = m_action_group_file->add_action_bool("share",
    sigc::mem_fun(*this, &AppWindow::on_menu_file_toggle_share) );
  m_listTableSensitiveActions.emplace_back(m_toggleaction_network_shared);
#endif //!GLOM_ENABLE_CLIENT_ONLY

  action = m_action_group_file->add_action("print",
    sigc::mem_fun(*m_frame, &Frame_Glom::on_menu_file_print) );
  m_listTableSensitiveActions.emplace_back(action);


#ifndef GLOM_ENABLE_CLIENT_ONLY
  auto action_print_edit =
    m_action_group_file->add_action("edit-print-layouts",
      sigc::mem_fun(*m_frame, &Frame_Glom::on_menu_file_print_edit_layouts));
  m_listDeveloperActions.emplace_back(action_print_edit);
#endif // !GLOM_ENABLE_CLIENT_ONLY

  //Add to the window's regular "win" ActionMap:
  add_action("close",
    sigc::mem_fun((GlomBakery::AppWindow_WithDoc&)*this, &GlomBakery::AppWindow_WithDoc::on_menu_file_close));

  insert_action_group("file", m_action_group_file);
}

void AppWindow::init_menus()
{
  init_menus_file();
  init_menus_edit();


  //Build actions:
  m_action_group_tables = Gio::SimpleActionGroup::create();

  Glib::RefPtr<Gio::SimpleAction> action;

#ifndef GLOM_ENABLE_CLIENT_ONLY
  action = m_action_group_tables->add_action("edit-tables",
    sigc::mem_fun(*m_frame, &Frame_Glom::on_menu_Tables_EditTables) );
  m_listDeveloperActions.emplace_back(action);

/* Commented out because it is useful but confusing to new users:
  action = m_action_group_tables->add_action("GlomAction_Menu_AddRelatedTable", //_("Add _Related Table"));
    sigc::mem_fun(*m_frame, &Frame_Glom::on_menu_Tables_AddRelatedTable) );
  m_listDeveloperActions.emplace_back(action);
*/
#endif // !GLOM_ENABLE_CLIENT_ONLY

  insert_action_group("tables", m_action_group_tables);


  //"Reports" menu:
  m_action_group_reports = Gio::SimpleActionGroup::create();

#ifndef GLOM_ENABLE_CLIENT_ONLY
  m_action_group_reports->add_action("edit-reports",
    sigc::mem_fun(*m_frame, &Frame_Glom::on_menu_Reports_EditReports) );
  m_listDeveloperActions.emplace_back(action);
  m_listTableSensitiveActions.emplace_back(action);
#endif

  insert_action_group("reports", m_action_group_developer);

  //Developer menu:
  m_action_group_developer = Gio::SimpleActionGroup::create();

#ifndef GLOM_ENABLE_CLIENT_ONLY
  m_action_menu_developer_usermode =
    m_action_group_developer->add_action_radio_integer("usermode",
      sigc::mem_fun(*this, &AppWindow::on_menu_developer_usermode),
      Utils::to_utype(AppState::userlevels::OPERATOR) );

  action = m_action_group_developer->add_action("database-preferences",
    sigc::mem_fun(*m_frame, &Frame_Glom::on_menu_developer_database_preferences) );
  m_listDeveloperActions.emplace_back(action);

  action = m_action_group_developer->add_action("fields",
   sigc::mem_fun(*m_frame, &Frame_Glom::on_menu_developer_fields) );
  m_listDeveloperActions.emplace_back(action);
  m_listTableSensitiveActions.emplace_back(action);

  action = m_action_group_developer->add_action("relationships-overview",
    sigc::mem_fun(*m_frame, &Frame_Glom::on_menu_developer_relationships_overview) );
  m_listDeveloperActions.emplace_back(action);
  m_listTableSensitiveActions.emplace_back(action);

  action = m_action_group_developer->add_action("relationships",
    sigc::mem_fun(*m_frame, &Frame_Glom::on_menu_developer_relationships) );
  m_listDeveloperActions.emplace_back(action);
  m_listTableSensitiveActions.emplace_back(action);

  m_action_developer_users = m_action_group_developer->add_action("users",
    sigc::mem_fun(*m_frame, &Frame_Glom::on_menu_developer_users));
  m_listDeveloperActions.emplace_back(m_action_developer_users);

  action = m_action_group_developer->add_action("print-layouts",
    sigc::mem_fun(*m_frame, &Frame_Glom::on_menu_developer_print_layouts));
  m_listDeveloperActions.emplace_back(action);
  m_listTableSensitiveActions.emplace_back(action);

  action = m_action_group_developer->add_action("reports",
    sigc::mem_fun(*m_frame, &Frame_Glom::on_menu_developer_reports));
  m_listDeveloperActions.emplace_back(action);
  m_listTableSensitiveActions.emplace_back(action);

  action = m_action_group_developer->add_action("script-library",
    sigc::mem_fun(*m_frame, &Frame_Glom::on_menu_developer_script_library));
  m_listDeveloperActions.emplace_back(action);

  action = m_action_group_developer->add_action("layout",
    sigc::mem_fun(*m_frame, &Frame_Glom::on_menu_developer_layout));
  m_listDeveloperActions.emplace_back(action);
  m_listTableSensitiveActions.emplace_back(action);

  action = m_action_group_developer->add_action("change-language",
    sigc::mem_fun(*this, &AppWindow::on_menu_developer_changelanguage));
  m_listDeveloperActions.emplace_back(action);

  action = m_action_group_developer->add_action("translations",
    sigc::mem_fun(*this, &AppWindow::on_menu_developer_translations));
  m_listDeveloperActions.emplace_back(action);

  //"Active Platform" menu:
  m_action_menu_developer_active_platform = m_action_group_developer->add_action_radio_string("active-platform",
    sigc::mem_fun(*this, &AppWindow::on_menu_developer_active_platform),
    "");
  m_listDeveloperActions.emplace_back(m_action_menu_developer_active_platform);

  action = m_action_group_developer->add_action("export-backup",
    sigc::mem_fun(*this, &AppWindow::on_menu_developer_export_backup));
  m_listDeveloperActions.emplace_back(action);

  action = m_action_group_developer->add_action("restore-backup",
    sigc::mem_fun(*this, &AppWindow::on_menu_developer_restore_backup));
  m_listDeveloperActions.emplace_back(action);

  //TODO: Think of a better name for this menu item,
  //though it mostly only exists because it is not quite ready to be on by default:
  //Note to translators: Drag and Drop is part of the name, not a verb or action:
  m_action_enable_layout_drag_and_drop =
    m_action_group_developer->add_action_bool("drag-and-drop-layout",
      sigc::mem_fun(*this, &AppWindow::on_menu_developer_enable_layout_drag_and_drop));
  m_listDeveloperActions.emplace_back(m_action_enable_layout_drag_and_drop);

#endif // !GLOM_ENABLE_CLIENT_ONLY

  insert_action_group("developer", m_action_group_developer);

  
  m_help_action_group = Gio::SimpleActionGroup::create();
 
  m_help_action_group->add_action("about",
    sigc::mem_fun(*this, &AppWindow::on_menu_help_about) );
  m_help_action_group->add_action("contents",
    sigc::mem_fun(*this, &AppWindow::on_menu_help_contents) );
  insert_action_group("help", m_help_action_group);

  update_table_sensitive_ui();

  fill_menu_tables();
}

#ifndef GLOM_ENABLE_CLIENT_ONLY

void AppWindow::on_menu_help_about()
{
  if(m_about && m_about_shown) // "About" box hasn't been closed, so just raise it
  {
    m_about->set_transient_for(*this);

    auto about_win = m_about->get_window();
    about_win->show();
    about_win->raise();
  }
  else
  {
    //Re-create About box:
    delete m_about;
    m_about = new Gtk::AboutDialog;

    m_about->set_program_name(m_strAppName);
    m_about->set_comments(_("A Database GUI"));
    m_about->set_version(PACKAGE_VERSION);
    m_about->set_copyright(_("© 2000-2011 Murray Cumming, Openismus GmbH"));
    const std::vector<Glib::ustring> vecAuthors({"Murray Cumming <murrayc@murrayc.com>"});
    m_about->set_authors(vecAuthors);
    
    //For some reason this use of the resource:// syntax does not work:
    const char* about_icon_name = "48x48/glom.png";
    //const Glib::ustring glom_icon_path = "resource://" + UiUtils::get_icon_path(about_icon_name);
    //Glib::RefPtr<Gdk::Pixbuf> logo = Gdk::Pixbuf::create_from_file(glom_icon_path);

    const auto glom_icon_path = UiUtils::get_icon_path(about_icon_name);

    //TODO: Use this, instead of the C API, when we can depend on gtkmm 3.12, with a try/catch:
    //Glib::RefPtr<Gdk::Pixbuf> logo = Gdk::Pixbuf::create_from_resource(glom_icon_path);
    GError* gerror = nullptr;
    auto logo =
      Glib::wrap(gdk_pixbuf_new_from_resource(glom_icon_path.c_str(), &gerror));
    if(gerror)
    {
      std::cerr << G_STRFUNC << ": Could not load icon from resource path=" << glom_icon_path << std::endl;
      g_clear_error(&gerror);
    }

    if(logo)
      m_about->set_logo(logo);
    else
      std::cout << G_STRFUNC << ": Could not load icon from resource path=" << glom_icon_path << std::endl;

    m_about->signal_hide().connect( sigc::mem_fun(*this, &AppWindow::on_about_close) );
    m_about_shown = true;
    static_cast<Gtk::Dialog*>(m_about)->run(); //show() would be better. see below:
    m_about->hide();
    //m_about->show(); //TODO: respond to the OK button.
  }
}

void AppWindow::on_about_close()
{
  m_about_shown = false;
}

void AppWindow::on_menu_file_toggle_share()
{
  if(!m_frame)
    return;

  //The state is not changed automatically:
  bool active = false;
  m_toggleaction_network_shared->get_state(active);

  const auto changed = m_frame->attempt_toggle_shared(!active);

  if(changed)
    m_toggleaction_network_shared->change_state(!active);
}

void AppWindow::on_menu_developer_usermode(int parameter)
{
  if(!m_frame)
    return;

  const bool developer = parameter == Utils::to_utype(AppState::userlevels::DEVELOPER);

  bool changed = false;
  if(developer)
    changed = m_frame->attempt_change_usermode_to_developer();
  else
    changed = m_frame->attempt_change_usermode_to_operator();

  //Change the menu's state:
  if(changed)
    m_action_menu_developer_usermode->change_state(parameter);

  m_frame->set_enable_layout_drag_and_drop(false);
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

static bool hostname_is_localhost(const Glib::ustring& hostname)
{
  if(hostname.empty())
    return false;

  //Quick short cut:
  if(hostname == "localhost")
    return true;
  else if(hostname == "localhost.localdomain")
    return true;
  else if(hostname == "127.0.0.1") //Standard IP address for localhost.
    return true;

  //TODO: Is there some way to compare hostents?
  /*
  hostent* a = gethostbyname("localhost");
  hostent* b = gethostbyname(hostname.c_str());
  if(!a || !b)
  {
    return a == b;
  }

  //TODO: return are_equal(a, b);
  */

  return true;
}

void AppWindow::ui_warning_load_failed(int failure_code)
{
  if(failure_code == Utils::to_utype(Document::LoadFailureCodes::NOT_FOUND))
  {
    //TODO: Put this in the generic bakery code.
    ui_warning(_("Open Failed"),
      _("The document could not be found."));

    //TODO: Glom::Bakery::GlomBakery::AppWindow_WithDoc removes the file from the recent history,
    //but the initial/welcome dialog doesn't yet update its list when the
    //recent history changes.
  }
  else if(failure_code == Utils::to_utype(Document::load_failure_codes::FILE_VERSION_TOO_NEW))
  {
    ui_warning(_("Open Failed"),
      _("The document could not be opened because it was created or modified by a newer version of Glom."));
  }
  else
    GlomBakery::AppWindow_WithDoc::ui_warning_load_failed();
}


#ifndef G_OS_WIN32
void AppWindow::open_browsed_document(const EpcServiceInfo* server, const Glib::ustring& service_name)
{
  gsize length = 0;
  gchar *document_contents = nullptr;

  bool keep_trying = true;
  while(keep_trying)
  {
    //Request a password to attempt retrieval of the document over the network:
    Dialog_Connection* dialog_connection = nullptr;
    //Load the Glade file and instantiate its widgets to get the dialog stuff:
    Utils::get_glade_widget_derived_with_warning(dialog_connection);
    if(!dialog_connection)
    {
      std::cerr << G_STRFUNC << ": dialog_connection is null.\n";
      return;
    }

    dialog_connection->set_transient_for(*this);
    dialog_connection->set_connect_to_browsed();
    dialog_connection->set_database_name(service_name);
    const auto response = Glom::UiUtils::dialog_run_with_help(dialog_connection);
    dialog_connection->hide();
    if(response != Gtk::RESPONSE_OK)
      keep_trying = false;
    else
    {
      //Open the document supplied by the other glom instance on the network:
      auto consumer = epc_consumer_new(server);

      Glib::ustring username, password;
      dialog_connection->get_username_and_password(username, password);
      epc_consumer_set_username(consumer, username.c_str());
      epc_consumer_set_password(consumer, password.c_str());

      GError *error = nullptr;
      document_contents = (gchar*)epc_consumer_lookup(consumer, "document", &length, &error);
      if(error)
      {
        std::cout << "debug: " << G_STRFUNC << ": \n" << "  " << error->message << std::endl;
        const int error_code = error->code;
        g_clear_error(&error);

        if(error_code == SOUP_STATUS_FORBIDDEN ||
           error_code == SOUP_STATUS_UNAUTHORIZED)
        {
          //std::cout << "   SOUP_STATUS_FORBIDDEN or SOUP_STATUS_UNAUTHORIZED\n";

          UiUtils::show_ok_dialog(_("Connection Failed"), _("Glom could not connect to the database server. Maybe you entered an incorrect user name or password, or maybe the postgres database server is not running."), *this, Gtk::MESSAGE_ERROR); //TODO: Add help button.
        }
      }
      else
      {
        //Store the username and password (now known to be correct) temporarily,
        //so we can use them when connecting directly to the database later:
        //(We can't just put them in the temp document instance, because these are not saved to disk.)
        m_temp_username = username;
        m_temp_password = password;

        keep_trying = false; //Finished.
      }
    }

    delete dialog_connection;
    dialog_connection = nullptr;

  }


  if(document_contents && length)
  {
    //Create a temporary Document instance, so we can manipulate the data:
    Document document_temp;
    int failure_code = 0;
    const auto loaded = document_temp.load_from_data((const guchar*)document_contents, length, failure_code);
    if(loaded)
    {
      // Connection is always remote-hosted in client only mode:
#ifndef GLOM_ENABLE_CLIENT_ONLY
#ifdef GLOM_ENABLE_POSTGRESQL
      //Stop the document from being self-hosted (it's already hosted by the other networked Glom instance):
      if(document_temp.get_hosting_mode() == Document::HostingMode::POSTGRES_SELF)
        document_temp.set_hosting_mode(Document::HostingMode::POSTGRES_CENTRAL);
#endif //GLOM_ENABLE_POSTGRESQL
#endif // !GLOM_ENABLE_CLIENT_ONLY
      // TODO: Error out in case this is a sqlite database, since we probably
      // can't access it from this host?

      //If the publisher thinks that it's using a postgres database on localhost,
      //then we need to use a host name that means the same thing from the client's PC:
      const auto host = document_temp.get_connection_server();
      if(hostname_is_localhost(host))
        document_temp.set_connection_server( epc_service_info_get_host(server) );

      //Make sure that we only use the specified port instead of connecting to some other postgres instance
      //on the same server:
      document_temp.set_connection_try_other_ports(false);
    }
    else
    {
      std::cerr << G_STRFUNC << ": Could not parse the document that was retrieved over the network: failure_code=" << failure_code << std::endl;
    }

    g_free(document_contents);
    document_contents = 0;

    //TODO_Performance: Horribly inefficient, but happens rarely:
    const auto temp_document_contents = document_temp.build_and_get_contents();

    //This loads the document and connects to the database (using m_temp_username and m_temp_password):
    open_document_from_data((const guchar*)temp_document_contents.c_str(), temp_document_contents.bytes());

    //Mark the document as opened-from-browse
    //so we don't think that opening has failed because it has no URI,
    //and to stop us from allowing developer mode
    //(that would require changes to the original document).
    auto document = std::dynamic_pointer_cast<Document>(get_document());
    if(document)
    {
      document->set_opened_from_browse();
      document->set_userlevel(AppState::userlevels::OPERATOR); //TODO: This should happen automatically.

      document->set_network_shared(true); //It is shared by the computer that we opened this from.
      update_network_shared_ui();

#ifndef GLOM_ENABLE_CLIENT_ONLY

      update_userlevel_ui();
#endif // !GLOM_ENABLE_CLIENT_ONLY
    }
  }
}
#endif // !G_OS_WIN32

#ifndef GLOM_ENABLE_CLIENT_ONLY
static bool uri_is_writable(const Glib::RefPtr<const Gio::File>& uri)
{
  if(!uri)
    return false;

  Glib::RefPtr<const Gio::FileInfo> file_info;

  try
  {
    file_info = uri->query_info(G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE);
  }
  catch(const Glib::Error& /* ex */)
  {
    return false;
  }

  if(file_info)
  {
    return file_info->get_attribute_boolean(G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE);
  }
  else
    return true; //Not every URI protocol supports access rights, so assume that it's writable and complain later.
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

//TODO: Use Gio::AppWindow? Is this even used?
void AppWindow::new_instance(const Glib::ustring& uri) //Override
{
  Glib::ustring command = "glom";
  if(!uri.empty())
    command += ' ' + uri;

  try
  {
    Glib::spawn_command_line_sync(command);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << ": " << ex.what() << std::endl;
  }
}

void AppWindow::init_create_document()
{
  if(!m_document)
  {
    auto document_glom = std::make_shared<Document>();

    //By default, we assume that the original is in the current locale.
    document_glom->set_translation_original_locale(AppWindow::get_current_locale());

    m_document = document_glom;
    //document_glom->set_parent_window(this); //So that it can show a BusyCursor when loading and saving.


    //Tell document about view:
    m_document->set_view(m_frame);

    //Tell view about document:
    //(This calls set_document() in the child views too.)
    m_frame->set_document(std::static_pointer_cast<Document>(m_document));
  }

  GlomBakery::AppWindow_WithDoc::init_create_document(); //Sets window title. Doesn't recreate doc.
}

bool AppWindow::check_document_hosting_mode_is_supported(const std::shared_ptr<Document>& document)
{
  //If it's an example then the document's hosting mode doesn't matter,
  //because the user will be asked to choose one when saving anyway.
  if(document->get_is_example_file())
    return true;

  //Check that the file's hosting mode is supported by this build:
  Glib::ustring error_message;
  switch(document->get_hosting_mode())
  {
    case Document::HostingMode::POSTGRES_SELF:
    {
      #ifdef GLOM_ENABLE_CLIENT_ONLY
      error_message = _("The file cannot be opened because this version of Glom does not support self-hosting of databases.");
      break;
      #endif //GLOM_ENABLE_CLIENT_ONLY

      #ifndef GLOM_ENABLE_POSTGRESQL
      error_message = _("The file cannot be opened because this version of Glom does not support PostgreSQL databases.");
      break;
      #endif //GLOM_ENABLE_POSTGRESQL

      break;
    }
    case Document::HostingMode::POSTGRES_CENTRAL:
    {
      #ifndef GLOM_ENABLE_POSTGRESQL
      error_message = _("The file cannot be opened because this version of Glom does not support PostgreSQL databases.");
      #endif //GLOM_ENABLE_POSTGRESQL

      break;
    }
    case Document::HostingMode::SQLITE:
    {
      #ifndef GLOM_ENABLE_SQLITE
      error_message = _("The file cannot be opened because this version of Glom does not support SQLite databases.");
      #endif //GLOM_ENABLE_SQLITE

      break;
    }
    default:
    {
      //on_document_load() should have checked for this already, informing the user.
      std::cerr << G_STRFUNC << ": Unhandled hosting mode: " << Utils::to_utype(document->get_hosting_mode()) << std::endl;
     g_assert_not_reached();
     break;
    }
  }

  if(error_message.empty())
    return true;

  //Warn the user.
  Frame_Glom::show_ok_dialog(_("File Uses Unsupported Database Backend"), error_message, *this, Gtk::MESSAGE_ERROR);
  return false;
}

bool AppWindow::on_document_load()
{
  //Link to the database described in the document.
  //Need to ask user for user/password:
  //m_frame->load_from_document();
  auto document = std::static_pointer_cast<Document>(get_document());
  if(!document)
    return false;

  //Set this so that AppWindow::get_current_locale() works as expected:
  AppWindow::set_original_locale(document->get_translation_original_locale());

  if(!document->get_is_new() && !check_document_hosting_mode_is_supported(document))
    return false;

#ifndef GLOM_ENABLE_CLIENT_ONLY
  //Connect signals:
  document->signal_userlevel_changed().connect( sigc::mem_fun(*this, &AppWindow::on_userlevel_changed) );

  //Disable/Enable actions, depending on userlevel:
  document->emit_userlevel_changed();
#endif // !GLOM_ENABLE_CLIENT_ONLY

  if(document->get_connection_database().empty()) //If it is a new (default) document.
  {
    //offer_new_or_existing();
  }
  else
  {
#ifndef GLOM_ENABLE_CLIENT_ONLY
    //Prevent saving until we are sure that everything worked.
    //This also stops us from losing the example data as soon as we say the new file (created from the example) is not an example.
    document->set_allow_autosave(false);
#endif // !GLOM_ENABLE_CLIENT_ONLY

    // Example files and backup files are not supported in client only mode because they
    // would need to be saved, but saving support is disabled.
#ifndef GLOM_ENABLE_CLIENT_ONLY
    const auto is_example = document->get_is_example_file();
    const auto is_backup = document->get_is_backup_file();
#endif // !GLOM_ENABLE_CLIENT_ONLY

    //Note that the URI will be empty if we are loading from data,
    //such as when loading a backup.
    const auto original_uri = document->get_file_uri();

    if(is_example || is_backup)
    {
#ifndef GLOM_ENABLE_CLIENT_ONLY
      // Remember the URI to the example file to be able to prevent
      // adding the URI to the recently used files in document_history_add.
      // We want to add the document that is created from the example
      // instead of the example itself.
      // TODO: This is a weird hack. Find a nicer way. murrayc.
      m_example_uri = original_uri;

      document->set_file_uri(Glib::ustring()); //Prevent it from defaulting to the read-only examples directory when offering saveas.
      //m_ui_save_extra_* are used by offer_saveas() if it's not empty:
      m_ui_save_extra_showextras = true;

      if(is_example)
      {
        m_ui_save_extra_title = _("Creating From Example File");
        m_ui_save_extra_message = _("To use this example file you must save an editable copy of the file. A new database will also be created on the server.");
      }
      else if(is_backup)
      {
        m_ui_save_extra_title = _("Creating From Backup File");
        m_ui_save_extra_message = _("To use this backup file you must save an editable copy of the file. A new database will also be created on the server.");
      }

      m_ui_save_extra_newdb_title = "TODO";
      m_ui_save_extra_newdb_hosting_mode = Document::HostingMode::DEFAULT;


      // Reinit cancelled state
      set_operation_cancelled(false);

      offer_saveas();
      // Note that bakery will try to add the example file itself to the
      // recently used documents, which is not what we want.
      m_ui_save_extra_message.clear();
      m_ui_save_extra_title.clear();

      if(!get_operation_cancelled())
      {
        //Get the results from the extended save dialog:
        document->set_database_title_original(m_ui_save_extra_newdb_title);
        document->set_hosting_mode(m_ui_save_extra_newdb_hosting_mode);
        m_ui_save_extra_newdb_hosting_mode = Document::HostingMode::DEFAULT;
        document->set_is_example_file(false);
        document->set_is_backup_file(false);

        // For self-hosting, we will choose a port later. For central
        // hosting, try several default ports. Don't use the values that
        // are set in the example file.
        document->set_connection_port(0);
        document->set_connection_try_other_ports(true);

        // We have a valid uri, so we can set it to !new and modified here
      }

      m_ui_save_extra_newdb_title.clear();
      m_ui_save_extra_showextras = false;

      if(get_operation_cancelled())
      {
        document->set_modified(false);
        document->set_is_new(true);
        document->set_allow_autosave(true); //Turn this back on.
        std::cout << "debug: user cancelled creating database\n";
        return false;
      }

#else // !GLOM_ENABLE_CLIENT_ONLY
      // TODO_clientonly: Tell the user that opening example files is
      // not supported. This could alternatively also be done in
      // Document_after::load_after, I am not sure which is better.
      ui_warning_load_failed(0);
      return false;
#endif // GLOM_ENABLE_CLIENT_ONLY
    }

#ifndef GLOM_ENABLE_CLIENT_ONLY
    //Warn about read-only files, because users will otherwise wonder why they can't use Developer mode:
    Document::userLevelReason reason = Document::userLevelReason::UNKNOWN;
    const auto userlevel = document->get_userlevel(reason);
    if( (userlevel == AppState::userlevels::OPERATOR) && (reason == Document::userLevelReason::FILE_READ_ONLY) )
    {
      Gtk::MessageDialog dialog(UiUtils::bold_message(_("Opening Read-Only File.")), true,  Gtk::MESSAGE_INFO, Gtk::BUTTONS_NONE);
      dialog.set_secondary_text(_("This file is read only, so you will not be able to enter Developer mode to make design changes."));
      dialog.set_transient_for(*this);
      dialog.add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
      dialog.add_button(_("Continue without Developer Mode"), Gtk::RESPONSE_OK); //arbitrary response code.

      const auto response = dialog.run();
      dialog.hide();
      if((response == Gtk::RESPONSE_CANCEL)  || (response == Gtk::RESPONSE_DELETE_EVENT))
        return false;
    }
#endif // !GLOM_ENABLE_CLIENT_ONLY

    //Read the connection information from the document:
    auto connection_pool = ConnectionPool::get_instance();
    if(!connection_pool)
      return false; //Impossible anyway.
    else
    {
#ifndef GLOM_ENABLE_CLIENT_ONLY
      connection_pool->set_get_document_func( std::bind(&AppWindow::on_connection_pool_get_document, this) );
#endif

      connection_pool->set_ready_to_connect(true); //connect_to_server() will now attempt the connection-> Shared instances of m_Connection will also be usable.

      //Attempt to connect to the specified database:
      bool test = false;

#ifndef GLOM_ENABLE_CLIENT_ONLY
      if(is_example || is_backup)
      {
        //The user has already had the chance to specify a new filename and database name.
        test = m_frame->connection_request_password_and_choose_new_database_name();
      }
      else
#endif // !GLOM_ENABLE_CLIENT_ONLY
      {
        //Ask for the username/password and connect:
        //Note that m_temp_username and m_temp_password are set if
        //we already asked for them when getting the document over the network:

        //Use the default username/password if opening as non network-shared:
        if(!(document->get_network_shared()))
        {
          // If the document is centrally hosted, don't pretend to know the
          // username or password, because we don't. The user will enter
          // the login credentials in a dialog.
          const auto hosting_mode = document->get_hosting_mode();
          if(hosting_mode != Document::HostingMode::POSTGRES_CENTRAL)
            m_temp_username = Privs::get_default_developer_user_name(m_temp_password, hosting_mode);
        }

        bool database_not_found = false;
        test = m_frame->connection_request_password_and_attempt(database_not_found, m_temp_username, m_temp_password);
        m_temp_username = Glib::ustring();
        m_temp_password = Glib::ustring();

        if(!test && database_not_found)
        {
          #ifndef GLOM_ENABLE_CLIENT_ONLY
          if(!is_example)
          {
            //The connection to the server is OK, but the database is not there yet.
            Frame_Glom::show_ok_dialog(_("Database Not Found On Server"), _("The database could not be found on the server. Please consult your system administrator."), *this, Gtk::MESSAGE_ERROR);
          }
          else
          #endif // !GLOM_ENABLE_CLIENT_ONLY
            std::cerr << G_STRFUNC << ": unexpected database_not_found error when opening example.\n";
        }
        else if(!test)
        {
          //std::cerr might show some hints, but we don't want to confront the user with them:
          //TODO: Actually complain about specific stuff such as missing data, because the user might really play with the file system.
          Frame_Glom::show_ok_dialog(_("Problem Loading Document"), _("Glom could not load the document."), *this, Gtk::MESSAGE_ERROR);
          std::cerr << G_STRFUNC << ": unexpected error.\n";
        }
      }

      if(!test)
        return false; //Failed. Close the document.

#ifndef GLOM_ENABLE_CLIENT_ONLY
      if(is_example || is_backup)
      {
        //Create the example database:
        //connection_request_password_and_choose_new_database_name() has already change the database name to a new unused one:

        bool user_cancelled = false;
        bool recreated = false;
        if(is_example)
          recreated = recreate_database_from_example(user_cancelled);
        else
        {
          recreated = recreate_database_from_backup(m_backup_data_filepath, user_cancelled);
          m_backup_data_filepath.clear();
        }

        if(!recreated)
        {
          // TODO: Do we need to call connection_pool->cleanup() here, for
          // stopping self-hosted databases? armin.
          connection_pool->cleanup( sigc::mem_fun(*this, &AppWindow::on_connection_close_progress) );
          //If the database was not successfully recreated:
          return false;
        }
        else
        {
          //Make sure that the changes (mark as non example, and save the new database name) are really saved:
          //Change the user level temporarily so that save_changes() actually saves:
          const auto user_level = document->get_userlevel();
          document->set_userlevel(AppState::userlevels::DEVELOPER);
          document->set_modified(true);
          document->set_allow_autosave(true); //Turn this back on.
          document->set_userlevel(user_level); //Change it back.
        }
      }
#endif // !GLOM_ENABLE_CLIENT_ONLY

      //Switch to operator mode when opening new documents:
      document->set_userlevel(AppState::userlevels::OPERATOR);

      //Make sure that it's saved in history, even if it was saved from an example file:
      document_history_add(document->get_file_uri());

      //Open default table, or show list of tables instead:
      m_frame->do_menu_Navigate_Table(true /* open the default if there is one */);
    }
  }

  //List the non-hidden tables in the menu:
  fill_menu_tables();

  update_network_shared_ui();

  //Run any startup script:
  const auto script = document->get_startup_script();
  if(!script.empty())
  {
    Glib::ustring error_message; //TODO: Check this and tell the user.
    auto connection_pool = ConnectionPool::get_instance();
    auto sharedconnection = connection_pool->connect();
    AppPythonUICallbacks callbacks;
    glom_execute_python_function_implementation(script,
      type_map_fields(), //only used when there is a current table and record.
      document,
      Glib::ustring() /* table_name */,
      std::shared_ptr<Field>(), Gnome::Gda::Value(), // primary key - only used when there is a current table and record.
      sharedconnection->get_gda_connection(),
      callbacks,
      error_message);

    if(!error_message.empty())
    {
      std::cerr << G_STRFUNC << ": Python Error: " << error_message << std::endl;
    }
  }

#ifndef GLOM_ENABLE_CLIENT_ONLY
  document->set_allow_autosave(true);
#endif // !GLOM_ENABLE_CLIENT_ONLY

  return true; //Loading of the document into the application succeeded.
}

void AppWindow::on_connection_create_database_progress()
{
  pulse_progress_message();
}

void AppWindow::on_connection_close_progress()
{
  //TODO_murrayc
}

void AppWindow::on_connection_save_backup_progress()
{
  pulse_progress_message();
}

void AppWindow::on_connection_convert_backup_progress()
{
  pulse_progress_message();
}

void AppWindow::on_document_close()
{
#ifndef GLOM_ENABLE_CLIENT_ONLY
  //TODO: It would be better to do this in a AppWindow::on_document_closed() virtual method,
  //but that would need an ABI break in Bakery:
  auto connection_pool = ConnectionPool::get_instance();
  if(!connection_pool)
    return;

  connection_pool->cleanup( sigc::mem_fun(*this, &AppWindow::on_connection_close_progress) );
#endif // !GLOM_ENABLE_CLIENT_ONLY
}

/*
void AppWindow::statusbar_set_text(const Glib::ustring& strText)
{
  m_pStatus->set_text(strText);
}

void AppWindow::statusbar_clear()
{
  statusbar_set_text("");
}
*/


void AppWindow::update_network_shared_ui()
{
  auto document = std::dynamic_pointer_cast<Document>(get_document());
  if(!document)
    return;

  if(!m_connection_toggleaction_network_shared)
    return;

  //Show the status in the UI:
  //(get_network_shared() already enforces constraints).
  const auto shared = document->get_network_shared();
  //TODO: Our use of block() does not seem to work. The signal actually seems to be emitted some time later instead.
  m_connection_toggleaction_network_shared.block(); //Prevent signal handling.
  m_toggleaction_network_shared->change_state(shared);

  //Do not allow impossible changes:
  const auto hosting_mode = document->get_hosting_mode();
  if( (hosting_mode == Document::HostingMode::POSTGRES_CENTRAL) //Central hosting means that it must be shared on the network.
    || (hosting_mode == Document::HostingMode::SQLITE) ) //sqlite does not allow network sharing.
  {
    m_toggleaction_network_shared->set_enabled(false);
  }

  m_connection_toggleaction_network_shared.unblock();
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void AppWindow::on_userlevel_changed(AppState::userlevels /* userlevel */)
{
  update_userlevel_ui();
}

void AppWindow::update_table_sensitive_ui()
{
  AppState::userlevels userlevel = get_userlevel();

  bool has_table = false;

  if(m_frame)
    has_table = !m_frame->get_shown_table_name().empty();

  for(const auto& action : m_listTableSensitiveActions)
  { 
    bool sensitive = has_table;

    const bool is_developer_item = 
      (Utils::find_exists(m_listDeveloperActions, action));
    if(is_developer_item)
      sensitive = sensitive && (userlevel == AppState::userlevels::DEVELOPER);

    action->set_enabled(sensitive);
  }
}

void AppWindow::update_userlevel_ui()
{
  AppState::userlevels userlevel = get_userlevel();

  //Disable/Enable developer actions:
  for(const auto& action : m_listDeveloperActions)
  {
     action->set_enabled( userlevel == AppState::userlevels::DEVELOPER );
  }

  //Ensure table sensitive menus stay disabled if necessary.
  update_table_sensitive_ui();

  // Hide users entry from developer menu for connections that don't
  // support users
  if(userlevel == AppState::userlevels::DEVELOPER)
  {
    if(ConnectionPool::get_instance_is_ready())
    {
      auto connection = ConnectionPool::get_and_connect();
      if(connection && !connection->get_gda_connection()->supports_feature(Gnome::Gda::CONNECTION_FEATURE_USERS))
        m_action_developer_users->set_enabled(false);
    }
  }

  //Make sure that the correct radio menu item is activated (the userlevel might have been set programmatically):
  //We only need to set/unset one, because the others are in the same radio group.
  //TODO:
  /*
  if(userlevel == AppState::userlevels::DEVELOPER)
  {
    if(!m_action_menu_developer_developer->get_active())
      m_action_menu_developer_developer->set_active();
  }
  else if(userlevel ==  AppState::userlevels::OPERATOR)
  {
    if(!m_action_menu_developer_operator->get_active())
      m_action_menu_developer_operator->set_active();
    // Remove the drag layout toolbar
  }
  */
  
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

bool AppWindow::offer_new_or_existing()
{
  //Offer to load an existing document, or start a new one.
  Dialog_ExistingOrNew* dialog_raw = nullptr;
  Utils::get_glade_widget_derived_with_warning(dialog_raw);
  std::shared_ptr<Dialog_ExistingOrNew> dialog(dialog_raw);
  dialog->set_transient_for(*this);
/*
  dialog->signal_new().connect(sigc::mem_fun(*this, &AppWindow::on_existing_or_new_new));
  dialog->signal_open_from_uri().connect(sigc::mem_fun(*this, &AppWindow::on_existing_or_new_open_from_uri));
  dialog->signal_open_from_remote().connect(sigc::mem_fun(*this, &AppWindow::on_existing_or_new_open_from_remote));
*/
  bool ask_again = true;
  while(ask_again)
  {
    const auto response_id = UiUtils::dialog_run_with_help(dialog_raw);
    dialog->hide();

    if(response_id == Gtk::RESPONSE_ACCEPT)
    {
      switch(dialog->get_action())
      {
#ifndef GLOM_ENABLE_CLIENT_ONLY
      case Dialog_ExistingOrNew::Action::NEW_EMPTY:
        existing_or_new_new();
        break;
      case Dialog_ExistingOrNew::Action::NEW_FROM_TEMPLATE:
#endif // !GLOM_ENABLE_CLIENT_ONLY
      case Dialog_ExistingOrNew::Action::OPEN_URI:
        open_document(dialog->get_uri());
        break;
#ifndef G_OS_WIN32
      case Dialog_ExistingOrNew::Action::OPEN_REMOTE:
        open_browsed_document(dialog->get_service_info(), dialog->get_service_name());
        break;
#endif
      case Dialog_ExistingOrNew::Action::NONE:
      default:
        std::cerr << G_STRFUNC << ": Unhandled action: " << Utils::to_utype(dialog->get_action()) << std::endl;
        g_assert_not_reached();
        break;
      }

      //Check that a document was opened:
      auto document = std::dynamic_pointer_cast<Document>(get_document());
      if(!document)
      {
        std::cerr << G_STRFUNC << ": document was NULL.\n";
        return false;
      }

      if(!document->get_file_uri().empty() || (document->get_opened_from_browse()))
        ask_again = false;
    }
    else if((response_id == Gtk::RESPONSE_CLOSE)  || (response_id == Gtk::RESPONSE_DELETE_EVENT))
    {
      return false; //close the window to close the application, because they need to choose a new or existing document.
    }
    else if((response_id == Gtk::RESPONSE_NONE)
     || (response_id == 0))
    {
       //For instance, the file-open dialog was cancelled after Dialog_ExistingOrNew opened it,
       //so just ask again.
       //TODO: Stop Dialog_ExistingOrNew from emitting a response in this case.
    }
    else
    {
      // This would mean that we got a unhandled response from the dialog
      g_return_val_if_reached(false);
    }
  }

  return true;
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void AppWindow::existing_or_new_new()
{
  // New empty document

  //Each document must have a location, so ask the user for one.
  //This will use an extended save dialog that also asks for the database title and some hosting details:
  m_ui_save_extra_showextras = true; //Offer self-hosting or central hosting, and offer the database title.
  m_ui_save_extra_newdb_hosting_mode = Document::HostingMode::DEFAULT; /* Default to self-hosting */
  m_ui_save_extra_newdb_title.clear();
  offer_saveas();

  //Check that the document was given a location:
  auto document = std::dynamic_pointer_cast<Document>(get_document());
  if(!document)
  {
    std::cerr << G_STRFUNC << ": document is null.\n";
    return;
  }

  if(!document->get_file_uri().empty())
  {
    //Get details from the extended save dialog:
    const Glib::ustring db_title = m_ui_save_extra_newdb_title;
    Document::HostingMode hosting_mode = m_ui_save_extra_newdb_hosting_mode;
    m_ui_save_extra_newdb_title.clear();
    m_ui_save_extra_newdb_hosting_mode = Document::HostingMode::DEFAULT;

    //Make sure that the user can do something with his new document:
    document->set_userlevel(AppState::userlevels::DEVELOPER);
    // Try various ports if connecting to an existing database server instead
    // of self-hosting one:
    document->set_connection_try_other_ports(m_ui_save_extra_newdb_hosting_mode == Document::HostingMode::DEFAULT);

    //Each new document must have an associated new database,
    //so choose a name

    //Create a database name based on the title.
    //The user will (almost) never see this anyway but it's nicer than using a random number:
    Glib::ustring db_name = Utils::create_name_from_title(db_title);

    //Prefix glom_ to the database name, so it's more obvious
    //for the system administrator.
    //This database name should never be user-visible again, either prefixed or not prefixed.
    db_name = "glom_" + db_name;

    //Connect to the server and choose a variation of this db_name that does not exist yet:
    document->set_connection_database(db_name);
    document->set_hosting_mode(hosting_mode);

   //Tell the connection pool about the document:
   auto connection_pool = ConnectionPool::get_instance();
   if(connection_pool)
     connection_pool->set_get_document_func( std::bind(&AppWindow::on_connection_pool_get_document, this) );

    const auto connected = m_frame->connection_request_password_and_choose_new_database_name();
    if(!connected)
    {
      // Unset URI so that the offer_new_or_existing does not disappear
      // so the user can make a different choice about what document to open.
      // TODO: Show some error message?
      document->set_file_uri("");
    }
    else
    {
      const auto db_created = m_frame->create_database(document->get_connection_database(), db_title);
      if(db_created)
      {
        const auto database_name_used = document->get_connection_database();
        ConnectionPool::get_instance()->set_database(database_name_used);
        document->set_database_title_original(db_title);
        m_frame->set_databases_selected(database_name_used);

        // Add the document to recent files
	    document_history_add(document->get_file_uri());
      }
      else
      {
        // Unset URI so that the offer_new_or_existing does not disappear
        // so the user can make a different choice about what document to open.
        // TODO: Show some error message?
        document->set_file_uri("");
      }
    }
  }
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

void AppWindow::set_mode_data()
{
  if(!m_frame)
    return;

  //Update the visual feedback in the menu.
  //This doesn't trigger the activate signal:
  m_action_mode_find->change_state(false);

  m_frame->set_mode_data();
}

void AppWindow::set_mode_find()
{
  if(!m_frame)
    return;

  //Update the visual feedback in the menu.
  //This doesn't trigger the activate signal:
  m_action_mode_find->change_state(true);

  m_frame->set_mode_find();
}

void AppWindow::on_menu_help_contents()
{
  Glom::UiUtils::show_help(this);
}

#ifndef GLOM_ENABLE_CLIENT_ONLY

void AppWindow::on_recreate_database_progress()
{
  //Show the user that something is happening, because the INSERTS might take time.
  pulse_progress_message();

  //Ensure that the infobar is shown, instead of waiting for the application to be idle.
  while(Gtk::Main::events_pending())
    Gtk::Main::iteration();
}

bool AppWindow::recreate_database_from_example(bool& user_cancelled)
{
  ShowProgressMessage progress_message(_("Creating Glom database from example file."));

  //Create a database, based on the information in the current document:
  const auto document = std::static_pointer_cast<Document>(get_document());
  if(!document)
    return false;

  auto connection_pool = ConnectionPool::get_instance();
  if(!connection_pool)
    return false; //Impossible anyway.

  //Check whether the database exists already.
  const auto db_name = document->get_connection_database();
  if(db_name.empty())
    return false;

  connection_pool->set_database(db_name);
  try
  {
    connection_pool->set_ready_to_connect(); //This has succeeded already.
    auto sharedconnection = connection_pool->connect();
    std::cerr << G_STRFUNC << ": Failed because database exists already.\n";

    return false; //Connection to the database succeeded, because no exception was thrown. so the database exists already.
  }
  catch(const ExceptionConnection& ex)
  {
    if(ex.get_failure_type() == ExceptionConnection::failure_type::NO_SERVER)
    {
      user_cancelled = true; //Eventually, the user will cancel after retrying.
      std::cerr << G_STRFUNC << ": Failed because connection to server failed, without specifying a database.\n";
      return false;
    }

    //Otherwise continue, because we _expected_ connect() to fail if the db does not exist yet.
  }

  //Show the user that something is happening, because the INSERTS might take time.
  pulse_progress_message();

  //Ensure that the infobar is shown, instead of waiting for the application to be idle.
  while(Gtk::Main::events_pending())
    Gtk::Main::iteration();

  //Create the database: (This will show a connection dialog)
  connection_pool->set_database( Glib::ustring() );
  const auto db_created = m_frame->create_database(db_name, document->get_database_title_original());

  if(!db_created)
  {
    return false;
  }
  else
    connection_pool->set_database(db_name); //Specify the new database when connecting from now on.

  pulse_progress_message();
  BusyCursor busy_cursor(this);

  std::shared_ptr<SharedConnection> sharedconnection;
  try
  {
    sharedconnection = connection_pool->connect();
    connection_pool->set_database(db_name); //The database was successfully created, so specify it when connecting from now on.
  }
  catch(const ExceptionConnection& ex)
  {
    std::cerr << G_STRFUNC << ": Failed to connect to the newly-created database.\n";
    return false;
  }

  //Create the developer group, and make this user a member of it:
  pulse_progress_message();
  bool test = DbUtils::add_standard_groups(document);
  if(!test)
    return false;

  //Add any extra groups from the example file:
  pulse_progress_message();
  test = DbUtils::add_groups_from_document(document);
  if(!test)
    return false;

  //Create each table:
  const auto tables = document->get_tables();
  for(const auto& table_info : tables)
  {
    //Create SQL to describe all fields in this table:
    Glib::ustring sql_fields;
    Document::type_vec_fields fields = document->get_table_fields(table_info->get_name());

    pulse_progress_message();
    const auto table_creation_succeeded = DbUtils::create_table(document->get_hosting_mode(), table_info, fields);
    pulse_progress_message();
    if(!table_creation_succeeded)
    {
      std::cerr << G_STRFUNC << ": CREATE TABLE failed with the newly-created database.\n";
      return false;
    }
  }

  pulse_progress_message();
  DbUtils::add_standard_tables(document); //Add internal, hidden, tables.

  //Set table priviliges, using the groups we just added:
  pulse_progress_message();
  test = DbUtils::set_table_privileges_groups_from_document(document);
  if(!test)
    return false;

  for(const auto& table_info : tables)
  {
    //Add any example data to the table:
    pulse_progress_message();

    //try
    //{
      const auto table_insert_succeeded = DbUtils::insert_example_data(document, table_info->get_name());

      if(!table_insert_succeeded)
      {
        std::cerr << G_STRFUNC << ": INSERT of example data failed with the newly-created database.\n";
        return false;
      }
    //}
    //catch(const std::exception& ex)
    //{
    //  std::cerr << G_STRFUNC << ": AppWindow::recreate_database_from_example(): exception: " << ex.what() << std::endl;
      //HandleError(ex);
    //}

  } //for(tables)

  return true; //All tables created successfully.
}

//TODO: Remove duplication with recreate_database_from_example().
bool AppWindow::recreate_database_from_backup(const std::string& backup_data_file_path, bool& user_cancelled)
{
  if(backup_data_file_path.empty())
  {
    std::cerr << G_STRFUNC << ": backup_data_file_path is empty.\n";
    return false;
  }

  ShowProgressMessage progress_message(_("Creating Glom database from backup file."));

  //Create a database, based on the information in the current document:
  auto document = std::static_pointer_cast<Document>(get_document());
  if(!document)
    return false;

  auto connection_pool = ConnectionPool::get_instance();
  if(!connection_pool)
    return false; //Impossible anyway.

  //Check whether the database exists already.
  const auto db_name = document->get_connection_database();
  if(db_name.empty())
    return false;

  connection_pool->set_database(db_name);
  try
  {
    connection_pool->set_ready_to_connect(); //This has succeeded already.
    auto sharedconnection = connection_pool->connect();
    std::cerr << G_STRFUNC << ": Failed because database exists already.\n";

    return false; //Connection to the database succeeded, because no exception was thrown. so the database exists already.
  }
  catch(const ExceptionConnection& ex)
  {
    if(ex.get_failure_type() == ExceptionConnection::failure_type::NO_SERVER)
    {
      user_cancelled = true; //Eventually, the user will cancel after retrying.
      std::cerr << G_STRFUNC << ": Failed because connection to server failed, without specifying a database.\n";
      return false;
    }

    //Otherwise continue, because we _expected_ connect() to fail if the db does not exist yet.
  }

  //Show the user that something is happening, because the INSERTS might take time.
  pulse_progress_message();

  //Ensure that the infobar is shown, instead of waiting for the application to be idle.
  while(Gtk::Main::events_pending())
    Gtk::Main::iteration();

  pulse_progress_message();

  //Create the database: (This will show a connection dialog)
  connection_pool->set_database( Glib::ustring() );
  try
  {
    ConnectionPool::get_instance()->create_database(
      sigc::mem_fun(*this, &AppWindow::on_connection_convert_backup_progress),
      db_name);
  }
  catch(const Glib::Exception& ex) // libgda does not set error domain
  {
    std::cerr << G_STRFUNC << ": Gnome::Gda::Connection::create_database(" << db_name << ") failed: " << ex.what() << std::endl;


    const auto message = _("Glom could not create the new database. Maybe you do not have the necessary access rights. Please contact your system administrator.");
    Gtk::MessageDialog dialog(UiUtils::bold_message(_("Database Creation Failed")), true, Gtk::MESSAGE_ERROR );
    dialog.set_secondary_text(message);
    dialog.set_transient_for(*this);

    dialog.run();

    return false;
  }

  connection_pool->set_database(db_name); //Specify the new database when connecting from now on.

  //Create the developer group, and make this user a member of it:
  pulse_progress_message();
  bool test = DbUtils::add_standard_groups(document);
  if(!test)
  {
    std::cerr << G_STRFUNC << ": DbUtils::add_standard_groups(): failed.\n";
    return false;
  }

  //m_frame->add_standard_tables(); //Add internal, hidden, tables.

  //Add any extra groups from the example file.
  //The backup file refers to these,
  //so the restore will fail if they are not present.
  pulse_progress_message();
  test = DbUtils::add_groups_from_document(document);
  if(!test)
    return false;

  //Restore the backup into the database:
  const auto restored = connection_pool->convert_backup(
    sigc::mem_fun(*this, &AppWindow::on_connection_convert_backup_progress), backup_data_file_path);

  if(!restored)
  {
    std::cerr << G_STRFUNC << ": Restore failed.\n";
    return false;
  }

  return true; //Restore successfully.
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

AppState::userlevels AppWindow::get_userlevel() const
{
  const auto document = std::dynamic_pointer_cast<const Document>(get_document());
  if(document)
  {
    return document->get_userlevel();
  }
  else
    g_assert_not_reached();
    //return AppState::userlevels::DEVELOPER; //This should never happen.
}

#ifndef GLOM_ENABLE_CLIENT_ONLY

void AppWindow::add_developer_action(const Glib::RefPtr<Gio::SimpleAction>& refAction)
{
  //Prevent it from being added twice:
  remove_developer_action(refAction);

  m_listDeveloperActions.emplace_back(refAction);
}

void AppWindow::remove_developer_action(const Glib::RefPtr<Gio::SimpleAction>& refAction)
{
  for(auto iter = m_listDeveloperActions.begin(); iter != m_listDeveloperActions.end(); ++iter)
  {
    if(*iter == refAction)
    {
      m_listDeveloperActions.erase(iter);
      break;
    }
  }
}

#endif // !GLOM_ENABLE_CLIENT_ONLY

namespace
{

static Glib::ustring escape_for_action_name(const Glib::ustring& str)
{
  //TODO: This is incredibly inefficient:
  //TODO: Check this with g_action_parse_detailed_name().
  //See https://developer.gnome.org/gio/stable/GAction.html#g-action-parse-detailed-name
  Glib::ustring result = str;
  result = Utils::string_replace(result, "\n", "newline");
  result = Utils::string_replace(result, " ", "space");
  result = Utils::string_replace(result, "\t", "space");
  result = Utils::string_replace(result, ":", "double-colon");
  result = Utils::string_replace(result, "(", "open-parentheses");
  result = Utils::string_replace(result, "(", "close-parentheses");

  return result;
}

} //anonymous namespace

void AppWindow::fill_menu_tables()
{
  m_nav_table_actions.clear();
  //TODO: Clear existing items

  if(m_nav_tables_action_group)
  {
    remove_action_group(ACTION_GROUP_NAME_TABLES);
    m_nav_tables_action_group.reset();
  }

  m_nav_tables_action_group = Gio::SimpleActionGroup::create();

  auto object =
    m_builder->get_object("tables-list");
  auto menu =
    Glib::RefPtr<Gio::Menu>::cast_dynamic(object);
  if(!menu)
  {
    std::cerr << G_STRFUNC << ": GMenu not found\n";
    return;
  }

  //TODO: Add API for this?
  while(menu->get_n_items())
  {
    menu->remove(0);
  }

  const auto document = std::dynamic_pointer_cast<Document>(get_document());
  if(!document)
  {
    std::cerr << G_STRFUNC << ": document is null.\n";
    return;
  }

  for(const auto& table_info : document->get_tables())
  {
    if(!table_info->get_hidden())
    {
      const auto title = Utils::string_escape_underscores(item_get_title_or_name(table_info));
      const auto action_name = escape_for_action_name(table_info->get_name());
  
      menu->append(title, ACTION_GROUP_NAME_TABLES + "." + action_name);

      auto action = m_nav_tables_action_group->add_action(action_name,
        sigc::bind( sigc::mem_fun(*m_frame, &Frame_Glom::on_box_tables_selected), table_info->get_name()) );
      m_nav_table_actions.emplace_back(action);
    }
  }

  insert_action_group(ACTION_GROUP_NAME_TABLES, m_nav_tables_action_group);
}

void AppWindow::fill_menu_reports(const Glib::ustring& table_name)
{
  m_nav_report_actions.clear();

  //Remove existing items.
  auto object =
    m_builder->get_object("reports-list");
  auto menu =
    Glib::RefPtr<Gio::Menu>::cast_dynamic(object);
  if(!menu)
  {
    std::cerr << G_STRFUNC << ": GMenu not found\n";
    return;
  }

  //TODO: Add API for this?
  while(menu->get_n_items())
  {
    menu->remove(0);
  }

  if(m_nav_reports_action_group)
  {
    remove_action_group(ACTION_GROUP_NAME_REPORTS);
    m_nav_reports_action_group.reset();
  }

  m_nav_reports_action_group = Gio::SimpleActionGroup::create();

  const auto document = std::dynamic_pointer_cast<Document>(get_document());
  if(!document)
  {
    std::cerr << G_STRFUNC << ": document is null.\n";
    return;
  }

  for(const auto& item : document->get_report_names(table_name))
  {
    const auto report = document->get_report(table_name, item);
    if(report)
    {
      const auto report_name = report->get_name();
      if(!report_name.empty())
      {
        const auto title = Utils::string_escape_underscores(item_get_title_or_name(report));
        const Glib::ustring action_name = report_name;
  
        menu->append(title, ACTION_GROUP_NAME_REPORTS + "." + report_name);

        auto action = m_nav_reports_action_group->add_action(action_name,
          sigc::bind( sigc::mem_fun(*m_frame, &Frame_Glom::on_menu_report_selected), report->get_name()) );
        m_nav_report_actions.emplace_back(action);
     }
    }
  }

  insert_action_group(ACTION_GROUP_NAME_REPORTS, m_nav_reports_action_group);
}

void AppWindow::enable_menu_print_layouts_details(bool enable)
{
  if(!m_nav_print_layouts_action_group)
    return;

  //TODO: See https://bugzilla.gnome.org/show_bug.cgi?id=708149 about having this API in GSimpleActionGroup:
  //m_nav_print_layouts_action_group->set_enabled(enable);

  //Enable/Disable each action in the group:
  //TODO: Suggest a simpler get_actions() method?
  for(const auto& name : m_nav_print_layouts_action_group->list_actions())
  {
    auto action = 
      Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(m_nav_print_layouts_action_group->lookup_action(name));
    if(action)
      action->set_enabled(enable);
  }
}

void AppWindow::fill_menu_print_layouts(const Glib::ustring& table_name)
{
  //TODO: This is copy/pasted from fill_menu_reports(). Can we generalize it?

  m_nav_print_layout_actions.clear();

  //Remove existing items.
  auto object =
    m_builder->get_object("print-layouts-list");
  auto menu =
    Glib::RefPtr<Gio::Menu>::cast_dynamic(object);
  if(!menu)
  {
    std::cerr << G_STRFUNC << ": GMenu not found\n";
    return;
  }

  //TODO: Add API for this?
  while(menu->get_n_items())
  {
    menu->remove(0);
  }

  if(m_nav_print_layouts_action_group)
  {
    remove_action_group(ACTION_GROUP_NAME_PRINT_LAYOUTS);
    m_nav_print_layouts_action_group.reset();
  }

  m_nav_print_layouts_action_group = Gio::SimpleActionGroup::create();

  auto document = std::dynamic_pointer_cast<Document>(get_document());
  if(!document)
  {
    std::cerr << G_STRFUNC << ": document is null.\n";
    return;
  }

  // TODO_clientonly: Should this be available in client only mode? We need to
  // depend on goocanvas in client only mode then:
#ifndef GLOM_ENABLE_CLIENT_ONLY
  for(const auto& item : document->get_print_layout_names(table_name))
  {
    const auto layout = document->get_print_layout(table_name, item);
    if(layout)
    {
      const auto name = layout->get_name();
      if(!name.empty())
      {
        const auto title = Utils::string_escape_underscores(item_get_title(layout));
        const Glib::ustring action_name = name;

        menu->append(title, ACTION_GROUP_NAME_PRINT_LAYOUTS + "." + action_name);

        auto action = m_nav_print_layouts_action_group->add_action(action_name,
          sigc::bind( sigc::mem_fun(*m_frame, &Frame_Glom::on_menu_print_layout_selected), name) );

        m_nav_print_layout_actions.emplace_back(action);
      }
    }
  }
#endif

  insert_action_group(ACTION_GROUP_NAME_PRINT_LAYOUTS, m_nav_print_layouts_action_group);
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void AppWindow::on_menu_file_save_as_example()
{
  //Based on the implementation of GlomBakery::GlomBakery::AppWindow_WithDoc::on_menu_file_saveas()

  //Display File Save dialog and respond to choice:

  //Bring document window to front, to make it clear which document is being saved:
  //This doesn't work: TODO.
  ui_bring_to_front();

  //Show the save dialog:
  bool bTest = false;
  auto document = std::dynamic_pointer_cast<Document>(get_document());
  if(!document) {
    std::cerr << G_STRFUNC << ": document was null.\n";
  } else {
    const auto file_uriOld = document->get_file_uri();

    m_ui_save_extra_showextras = false;
    m_ui_save_extra_title.clear();
    m_ui_save_extra_message.clear();
    m_ui_save_extra_newdb_title.clear();
    m_ui_save_extra_newdb_hosting_mode = Document::HostingMode::DEFAULT;

    Glib::ustring file_uri = ui_file_select_save(file_uriOld); //Also asks for overwrite confirmation.
    if(!file_uri.empty())
    {
      //Enforce the file extension:
      file_uri = document->get_file_uri_with_extension(file_uri);

      //Prevent saving while we modify the document:
      document->set_allow_autosave(false);

      document->set_file_uri(file_uri, true); //true = enforce file extension
      document->set_is_example_file();

      //Save all data from all tables into the document:
      for(const auto& item : document->get_tables())
      {
        const auto table_name = item->get_name();

        //const auto vec_fields = document->get_table_fields(table_name);

        //export_data_to_*() needs a type_list_layout_groups;
        Document::type_list_layout_groups sequence = document->get_data_layout_groups_default("list", table_name, "" /* layout_platform */);

        //std::cout << "debug: table_name=" << table_name << std::endl;

        Document::type_example_rows example_rows;
        FoundSet found_set;
        found_set.m_table_name = table_name;
        auto const_sequence = Utils::const_list(sequence);
        DbUtilsExport::export_data_to_vector(document, example_rows, found_set, const_sequence);
        //std::cout << "  debug after row_text=" << row_text << std::endl;

        document->set_table_example_data(table_name, example_rows);
      }

      bTest = document->save();
      document->set_is_example_file(false);
      document->set_is_backup_file(false);
      document->set_file_uri(file_uriOld);
      document->set_allow_autosave(true);

      if(bTest)
      {
        //Disable Save and SaveAs menu items:
        after_successful_save();
      }

      update_window_title();
    }
    else
    {
      cancel_close_or_exit();
      return;
    }
  }

  if(!bTest)
  {
    ui_warning(_("Save failed."), _("There was an error while saving the example file."));
  }
}

Glib::ustring AppWindow::ui_file_select_save(const Glib::ustring& old_file_uri) //override
{
  //Reimplement this whole function, just so we can use our custom FileChooserDialog class:
  AppWindow& app = *this;

  std::shared_ptr<Gtk::FileChooserDialog> fileChooser_Save;
  Glom::FileChooserDialog_SaveExtras* fileChooser_SaveExtras = nullptr;

  //Create the appropriate dialog, depending on how the caller set m_ui_save_extra_showextras:
  if(m_ui_save_extra_showextras)
  {
    fileChooser_SaveExtras = new Glom::FileChooserDialog_SaveExtras(_("Save Document"), Gtk::FILE_CHOOSER_ACTION_SAVE);
    fileChooser_Save.reset(fileChooser_SaveExtras);
  }
  else
  {
    fileChooser_Save.reset(new Gtk::FileChooserDialog(gettext("Save Document"), Gtk::FILE_CHOOSER_ACTION_SAVE));
  }

  fileChooser_Save->set_do_overwrite_confirmation(); //Ask the user if the file already exists.

  auto pWindow = dynamic_cast<Gtk::Window*>(&app);
  if(pWindow)
    fileChooser_Save->set_transient_for(*pWindow);

  fileChooser_Save->add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
  fileChooser_Save->add_button(_("_Save"), Gtk::RESPONSE_OK);

  fileChooser_Save->set_default_response(Gtk::RESPONSE_OK);

  //This is the reason that we override this method:
  if(!m_ui_save_extra_title.empty())
    fileChooser_Save->set_title(m_ui_save_extra_title);

  if(fileChooser_SaveExtras)
  {
    fileChooser_SaveExtras->set_extra_message(m_ui_save_extra_message);


    //Start with something suitable:
    auto document = std::dynamic_pointer_cast<Document>(get_document());
    g_assert(document);
    const auto filename = document->get_name(); //Get the filename without the path and extension.

    //Avoid ".". TODO: Find out why it happens:
    if(filename == ".")
      m_ui_save_extra_newdb_title = Glib::ustring();
    else
      m_ui_save_extra_newdb_title = Utils::title_from_string( filename ); //Start with something suitable.

    fileChooser_SaveExtras->set_extra_newdb_title(m_ui_save_extra_newdb_title);
    fileChooser_SaveExtras->set_extra_newdb_hosting_mode(m_ui_save_extra_newdb_hosting_mode);
  }


  //Make the save dialog show the existing filename, if any:
  if(!old_file_uri.empty())
  {
    //Just start with the parent folder,
    //instead of the whole name, to avoid overwriting:
    auto gio_file = Gio::File::create_for_uri(old_file_uri);
    if(gio_file)
    {
      auto parent = gio_file->get_parent();
      if(parent)
      {
        const auto uri_parent = parent->get_uri();
        fileChooser_Save->set_uri(uri_parent);
      }
    }
  }


  //bool tried_once_already = false;

  bool try_again = true;
  while(try_again)
  {
    try_again = false;

    const auto response_id = fileChooser_Save->run();
    fileChooser_Save->hide();
    if((response_id != Gtk::RESPONSE_CANCEL) && (response_id != Gtk::RESPONSE_DELETE_EVENT))
    {
      const auto uri_chosen = fileChooser_Save->get_uri();

      //Change the URI, to put the file (and its data folder) in a folder:
      const auto uri = FileUtils::get_file_uri_without_extension(uri_chosen);

      //Check whether the file exists, and that we have rights to it:
      auto file = Gio::File::create_for_uri(uri);
      if(!file)
        return Glib::ustring(); //Failure.


      //If the file exists (the FileChooser offers a "replace?" dialog, so this is not possible.):
      if(GlomBakery::AppWindow_WithDoc::file_exists(uri))
      {
        //Check whether we have rights to the file to change it:
        //Really, GtkFileChooser should do this for us.
        if(!uri_is_writable(file))
        {
           //Warn the user:
           ui_warning(gettext("Read-only File."), _("You may not overwrite the existing file, because you do not have sufficient access rights."));
           try_again = true; //Try again.
           continue;
        }
      }

      //Check whether we have rights to the directory, to create a new file in it:
      //Really, GtkFileChooser should do this for us.
      Glib::RefPtr<const Gio::File> parent = file->get_parent();
      if(parent)
      {
        if(!uri_is_writable(parent))
        {
          //Warn the user:
           ui_warning(gettext("Read-only Directory."), _("You may not create a file in this directory, because you do not have sufficient access rights."));
           try_again = true; //Try again.
           continue;
        }
      }

      if(!try_again && fileChooser_SaveExtras)
      {
        //Get the extra details from the extended save dialog:
        m_ui_save_extra_newdb_title = fileChooser_SaveExtras->get_extra_newdb_title();

#ifndef GLOM_ENABLE_CLIENT_ONLY
        m_ui_save_extra_newdb_hosting_mode = fileChooser_SaveExtras->get_extra_newdb_hosting_mode();
#endif // !GLOM_ENABLE_CLIENT_ONLY

        if(m_ui_save_extra_newdb_title.empty())
        {
          Frame_Glom::show_ok_dialog(_("Database Title missing"), _("You must specify a title for the new database."), *this, Gtk::MESSAGE_ERROR);

          try_again = true;
          continue;
        }
      }

      bool is_self_hosted = false;

#ifdef GLOM_ENABLE_POSTGRESQL
      if(m_ui_save_extra_newdb_hosting_mode == Document::HostingMode::POSTGRES_SELF)
        is_self_hosted = true;
#endif //GLOM_ENABLE_POSTGRESQL

#ifdef GLOM_ENABLE_MYSQL
      if(m_ui_save_extra_newdb_hosting_mode == Document::HostingMode::MYSQL_SELF)
        is_self_hosted = true;
#endif // GLOM_ENABLE_MYSQL

#ifdef GLOM_ENABLE_SQLITE
      if(m_ui_save_extra_newdb_hosting_mode == Document::HostingMode::SQLITE)
        is_self_hosted = true;
#endif // GLOM_ENABLE_SQLITE

      // Create a directory for self-hosted databases (sqlite or self-hosted
      // postgresql).
      if(!try_again && fileChooser_SaveExtras && is_self_hosted)
      {
        //Check that the directory does not exist already.
        //The GtkFileChooser could not check for that because it could not know that we would create a directory based on the filename:
        //Note that uri has no extension at this point:
        auto dir = Gio::File::create_for_uri(uri);
        if(dir->query_exists())
        {
          ui_warning(_("Directory Already Exists"), _("There is an existing directory with the same name as the directory that should be created for the new database files. You should specify a different filename to use a new directory instead."));
          try_again = true; //Try again.
          continue;
        }

        //Create the directory, so that file creation can succeed later:
        //0770 means "this user and his group can read and write this "executable" (can add child files) directory".
        //The 0 prefix means that this is octal.
        try
        {
          //TODO: ensure that we use 0770? murrayc.
          dir->make_directory();
        }
        catch(const Gio::Error& ex)
        {
          std::cerr << G_STRFUNC << ": " << ex.what() << std::endl;
        }

        //Add the filename (Note that the caller will add the extension if necessary, so we don't do it here.)
        auto file_with_ext = Gio::File::create_for_uri(uri_chosen);
        const auto filename_part = file_with_ext->get_basename();

        //Add the filename part to the newly-created directory:
        auto file_whole = dir->get_child(filename_part);
        return file_whole->get_uri();
      }

      if(!try_again)
      {
        return uri_chosen;
      }
    }
    else
      return Glib::ustring(); //The user cancelled.
  }

  return Glib::ustring();
}

/*
void AppWindow::stop_self_hosting_of_document_database()
{
  auto document = std::static_pointer_cast<Document>(get_document());
  if(document)
  {
    auto connection_pool = ConnectionPool::get_instance();
    if(!connection_pool)
      return;

    connection_pool->cleanup( sigc::mem_fun(*this, &AppWindow::on_connection_close_progress ));
  }
}
*/

void AppWindow::on_menu_developer_changelanguage()
{
  Dialog_ChangeLanguage* dialog = nullptr;
  Utils::get_glade_widget_derived_with_warning(dialog);
  if(!dialog) //Unlikely and it already warns on stderr.
    return;
    
  dialog->set_transient_for(*this);
  const auto response = Glom::UiUtils::dialog_run_with_help(dialog);
  dialog->hide();

  if(response == Gtk::RESPONSE_OK)
  {
    AppWindow::set_current_locale(dialog->get_locale());

    //Get the translations from the document (in Operator mode, we only load the necessary translations.)
    //This also updates the UI, so we show all the translated titles:
    int failure_code = 0;
    get_document()->load(failure_code);

    m_frame->show_table_refresh(); //load() doesn't seem to refresh the view.
  }

  delete dialog;
}

void AppWindow::on_menu_developer_translations()
{
  if(!m_window_translations)
  {
    Utils::get_glade_widget_derived_with_warning(m_window_translations);
    if(m_window_translations)
    {
      m_frame->add_view(m_window_translations);
      m_window_translations->set_transient_for(*this);
      m_window_translations->set_document(std::static_pointer_cast<Document>(m_document));
      m_window_translations->load_from_document();
      m_window_translations->show();

      m_window_translations->signal_hide().connect(sigc::mem_fun(*this, &AppWindow::on_window_translations_hide));
    }
  }
  else
  {
    m_window_translations->show();
    m_window_translations->load_from_document();
  }
}

void AppWindow::on_menu_developer_active_platform(const Glib::ustring& parameter)
{
  //The state is not changed automatically:
  m_action_menu_developer_active_platform->change_state(parameter);

  auto document = std::dynamic_pointer_cast<Document>(get_document());
  if(document)
   document->set_active_layout_platform(parameter);

  m_frame->show_table_refresh();
}

void AppWindow::on_menu_developer_export_backup()
{
  auto document = std::dynamic_pointer_cast<Document>(get_document());
  if(!document)
    return;

  // Ask the user to choose a new directory name.
  // Start with a name based on the existing name.
  const auto fileuri_old = document->get_file_uri();
  const Glib::RefPtr<const Gio::File> file_old =
    Gio::File::create_for_uri( FileUtils::get_file_uri_without_extension(fileuri_old) );
  const auto old_basename = file_old->get_basename();
  Glib::TimeVal timeval;
  timeval.assign_current_time();
  std::string starting_name = old_basename + "-backup-" + timeval.as_iso8601();
  //Replace : because that confuses (makes it fail) tar (and file-roller) when opening the file,
  //and --force-local is not relevant to opening files.
  starting_name = Utils::string_replace(starting_name, ":", "-");

  // This actually creates the directory:
  Gtk::FileChooserDialog dialog(*this, _("Save Backup"), Gtk::FILE_CHOOSER_ACTION_CREATE_FOLDER);
  dialog.add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
  dialog.add_button(_("_Save"), Gtk::RESPONSE_ACCEPT);
  dialog.set_local_only(); //Because pg_dump, pg_restore and tar can't use URIs.
  dialog.set_current_name(starting_name);
  const auto result = dialog.run();
  dialog.hide();
  if(result != Gtk::RESPONSE_ACCEPT)
    return;

  //Get the path to the directory in which the .glom and data files will be created.
  //The .tar.gz will then be created next to it:
  const auto path_dir = dialog.get_filename();
  if(path_dir.empty())
    return;

  ShowProgressMessage progress_message(_("Exporting backup"));
  const auto tarball_uri = document->save_backup_file(
    Glib::filename_to_uri(path_dir),
    sigc::mem_fun(*this, &AppWindow::on_connection_save_backup_progress));

  if(tarball_uri.empty())
    ui_warning(_("Export Backup failed."), _("There was an error while exporting the backup."));
  //TODO: Offer to show the tarball file in the file manager?
}

void AppWindow::on_menu_developer_restore_backup()
{
  Gtk::FileChooserDialog file_dlg(_("Choose a backup file"), Gtk::FILE_CHOOSER_ACTION_OPEN);
  file_dlg.set_transient_for(*this);
  file_dlg.set_local_only(); //Because we can't untar remote files.

  auto filter = Gtk::FileFilter::create();
  filter->set_name(_(".tar.gz Backup files"));
  filter->add_pattern("*.tar.gz");
  filter->add_pattern("*.tgz");
  file_dlg.add_filter(filter);

  file_dlg.add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
  file_dlg.add_button(_("Restore"), Gtk::RESPONSE_OK);

  const auto result = file_dlg.run();
  file_dlg.hide();
  if(result != Gtk::RESPONSE_OK)
    return;

  const auto uri_tarball = file_dlg.get_uri();
  if(uri_tarball.empty())
    return;

  do_restore_backup(uri_tarball);
}

void AppWindow::do_print_layout(const Glib::ustring& print_layout_name, bool preview, Gtk::Window* transient_for)
{
  m_frame->do_print_layout(print_layout_name, preview, transient_for);
}

bool AppWindow::do_restore_backup(const Glib::ustring& backup_uri)
{
  auto document = std::dynamic_pointer_cast<Document>(get_document());
  if(!document)
    return false;
    
  ShowProgressMessage progress_message(_("Restoring backup"));
  const auto backup_glom_file_contents = Glom::Document::extract_backup_file(
    backup_uri, m_backup_data_filepath,
    sigc::mem_fun(*this, &AppWindow::on_connection_convert_backup_progress));

  if(backup_glom_file_contents.empty() || m_backup_data_filepath.empty())
  {
    ui_warning(_("Restore Backup failed."), _("There was an error while extracting the backup."));
    return false;
  }

  return open_document_from_data((const guchar*)backup_glom_file_contents.c_str(), backup_glom_file_contents.bytes());
}

void AppWindow::on_menu_developer_enable_layout_drag_and_drop()
{
  bool state = false;
  m_action_enable_layout_drag_and_drop->get_state(state);

  m_frame->set_enable_layout_drag_and_drop(state); //TODO: Change the menu's state.
}


void AppWindow::on_window_translations_hide()
{
  if(m_window_translations)
  {
    m_frame->on_developer_dialog_hide();
  }
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

AppWindow* AppWindow::get_appwindow()
{
  return global_appwindow;
}

void AppWindow::document_history_add(const Glib::ustring& file_uri)
{
  // We override this so we can prevent example files from being saved in the recently-used list:

  if(file_uri.empty())
    return;

  if(!file_uri.empty())
  {
    //Prevent saving of example file templates just because we opened them:
    if(file_uri == m_example_uri)
      return;
  }

  //This can sometimes be called for a file that does not yet exist on disk.
  //Avoid warning in RecentManager if that is the case.
  //For instance, Glom does this when the user chooses a new filename, 
  //but before Glom has enough information to save a useful file.
  if(!file_exists(file_uri))
    return;

  Gtk::RecentManager::get_default()->add_item(file_uri);
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void AppWindow::do_menu_developer_fields(Gtk::Window& parent, const Glib::ustring table_name)
{
  m_frame->do_menu_developer_fields(parent, table_name);
}

void AppWindow::do_menu_developer_relationships(Gtk::Window& parent, const Glib::ustring table_name)
{
  m_frame->do_menu_developer_relationships(parent, table_name);
}

std::shared_ptr<Document> AppWindow::on_connection_pool_get_document()
{
  return std::dynamic_pointer_cast<Document>(get_document());
}
#endif //GLOM_ENABLE_CLIENT_ONLY

//Show the current table name in the window's title:
void AppWindow::update_window_title()
{
  //Set application's main window title:

  auto document = std::dynamic_pointer_cast<Document>(get_document());
  if(!document)
    return;

  if(!m_frame)
    return;

  //Show the table title:
  const auto table_name = m_frame->get_shown_table_name();
  Glib::ustring table_label = document->get_table_title(table_name, AppWindow::get_current_locale());
  if(!table_label.empty())
  {
    #ifndef GLOM_ENABLE_CLIENT_ONLY
    if(document->get_userlevel() == AppState::userlevels::DEVELOPER)
      table_label += " (" + table_name + ")"; //Show the table name as well, if in developer mode.
    #endif // GLOM_ENABLE_CLIENT_ONLY
  }
  else //Use the table name if there is no table title.
    table_label = table_name;

  Glib::ustring strTitle = document->get_name();
  if(!table_label.empty())
    strTitle += ": " + table_label;

  #ifndef GLOM_ENABLE_CLIENT_ONLY
  //Indicate unsaved changes:
  if(document->get_modified())
    strTitle += " *";

  //Indicate read-only files:
  if(document->get_read_only())
    strTitle += _(" (read-only)");
  #endif //GLOM_ENABLE_CLIENT_ONLY

  strTitle +=  " - " + m_strAppName;

  set_title(strTitle);
}

void AppWindow::show_table_details(const Glib::ustring& table_name, const Gnome::Gda::Value& primary_key_value)
{
  if(!m_frame)
    return;

  m_frame->show_table(table_name, primary_key_value);
}

void AppWindow::show_table_list(const Glib::ustring& table_name)
{
  if(!m_frame)
    return;

  m_frame->show_table(table_name);
}


void AppWindow::print_report(const Glib::ustring& report_name)
{
  if(!m_frame)
    return;

  m_frame->on_menu_report_selected(report_name);
}

void AppWindow::print_layout()
{
  if(!m_frame)
    return;

  m_frame->on_menu_file_print();
}

void AppWindow::start_new_record()
{
  m_frame->on_menu_add_record();
}

void AppWindow::set_progress_message(const Glib::ustring& message)
{
  const auto title = _("Processing");
  const std::string collate_key = (title + message).collate_key();

  if(collate_key != m_progress_collate_key)
  {
    // New progress message.
    m_progress_collate_key = collate_key;
    m_infobar_progress->set_message(title, message);
    m_infobar_progress->show();
  }

  // Pulse the progress bar regardless of whether the message is new or not.
  m_infobar_progress->pulse();
  
  //Block interaction with the rest of the UI.
  if(m_menubar)
    m_menubar->set_sensitive(false);

  m_frame->set_sensitive(false);
}

void AppWindow::pulse_progress_message()
{
  m_infobar_progress->pulse();
}

void AppWindow::clear_progress_message()
{
  m_progress_collate_key.clear();
  m_infobar_progress->hide();

  if(m_menubar)
    m_menubar->set_sensitive();

  m_frame->set_sensitive();
}

void AppWindow::set_current_locale(const Glib::ustring& locale)
{
  if(locale.empty())
    return;

  m_current_locale = locale;
}

void AppWindow::set_original_locale(const Glib::ustring& locale)
{
  if(locale.empty())
    return;

  m_original_locale = locale;
}

Glib::ustring AppWindow::get_original_locale()
{
  //Default to English:
  if(m_original_locale.empty())
    m_original_locale = "en";

  return m_original_locale; 
}

bool AppWindow::get_current_locale_not_original()
{
  if(m_original_locale.empty())
    get_original_locale();

  if(m_current_locale.empty())
    get_current_locale();

  return m_original_locale != m_current_locale;
}

Glib::ustring AppWindow::get_current_locale()
{
  //Return a previously-set current locale, if any:
  if(!m_current_locale.empty())
    return m_current_locale;

  //Get the user's current locale:
  const auto cLocale = setlocale(LC_ALL, 0); //Passing NULL means query, instead of set.
  if(cLocale)
  {
    //std::cout << "debug1: " << G_STRFUNC << ": locale=" << cLocale << std::endl;
    return Utils::locale_simplify(cLocale);
    //std::cout << "debug2: " << G_STRFUNC << ": m_current_locale=" << m_current_locale << std::endl;
  }
  else
    return "C";
}

Glib::ustring item_get_title(const std::shared_ptr<const TranslatableItem>& item)
{
  if(!item)
    return Glib::ustring();

  return item->get_title(AppWindow::get_current_locale());
}

Glib::ustring item_get_title_or_name(const std::shared_ptr<const TranslatableItem>& item)
{
  if(!item)
    return Glib::ustring();

  return item->get_title_or_name(AppWindow::get_current_locale());
}


void AppWindow::on_hide()
{
  ui_signal_hide().emit();
}

void AppWindow::ui_hide()
{
  hide();  
}

void AppWindow::ui_bring_to_front()
{
  get_window()->raise();
}

void AppWindow::init_menus_edit()
{
  //Edit menu
  
  //Build actions:
  m_action_group_edit = Gio::SimpleActionGroup::create();

  add_action("cut",
    sigc::mem_fun((AppWindow&)*this, &AppWindow::on_menu_edit_cut_activate));
  add_action("copy",
    sigc::mem_fun((AppWindow&)*this, &AppWindow::on_menu_edit_copy_activate));
  add_action("paste",
    sigc::mem_fun((AppWindow&)*this, &AppWindow::on_menu_edit_paste_activate));
  add_action("clear");
    //TODO? sigc::mem_fun((AppWindow&)*this, &AppWindow::on_menu_edit_clear_activate));

  //We remember this action, so that it can be explicitly activated later.
  m_action_mode_find = m_action_group_edit->add_action_bool("find",
    sigc::mem_fun((AppWindow&)*this, &AppWindow::on_menu_edit_find),
    false);
  m_listTableSensitiveActions.emplace_back(m_action_mode_find);

  insert_action_group("edit", m_action_group_edit);
}

void AppWindow::add(Gtk::Widget& child)
{
  m_vbox_placeHolder.pack_start(child);
}

bool AppWindow::on_delete_event(GdkEventAny* /* event */)
{
  //Clicking on the [x] in the title bar should be like choosing File|Close
  on_menu_file_close();

  return true; // true = don't hide, don't destroy
}

void AppWindow::ui_warning(const Glib::ustring& text, const Glib::ustring& secondary_text)
{
  Gtk::Window* pWindow = this;

  Gtk::MessageDialog dialog(AppWindow::util_bold_message(text), true /* use markup */, Gtk::MESSAGE_WARNING);
  dialog.set_secondary_text(secondary_text);

  dialog.set_title(""); //The HIG says that alert dialogs should not have titles. The default comes from the message type.

  if(pWindow)
    dialog.set_transient_for(*pWindow);

  dialog.run();
}


Glib::ustring AppWindow::util_bold_message(const Glib::ustring& message)
{
  return "<b>" + message + "</b>";
}

Glib::ustring AppWindow::ui_file_select_open(const Glib::ustring& starting_folder_uri)
{
  Gtk::Window* pWindow = this;

  Gtk::FileChooserDialog fileChooser_Open(_("Open Document"), Gtk::FILE_CHOOSER_ACTION_OPEN);
  fileChooser_Open.add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
  fileChooser_Open.add_button(_("_Open"), Gtk::RESPONSE_OK);
  fileChooser_Open.set_default_response(Gtk::RESPONSE_OK);

  if(pWindow)
    fileChooser_Open.set_transient_for(*pWindow);

  if(!starting_folder_uri.empty())
    fileChooser_Open.set_current_folder_uri(starting_folder_uri);

  const auto response_id = fileChooser_Open.run();
  fileChooser_Open.hide();
  if(response_id != Gtk::RESPONSE_CANCEL)
  {
    return fileChooser_Open.get_uri();
  }
  else
    return Glib::ustring();
}


void AppWindow::ui_show_modification_status()
{
  const auto modified = m_document->get_modified();

  //Enable Save and SaveAs menu items:
  if(m_action_save)
    m_action_save->set_enabled(modified);

  if(m_action_saveas)
    m_action_saveas->set_enabled(modified);
}

AppWindow::enumSaveChanges AppWindow::ui_offer_to_save_changes()
{
  GlomBakery::AppWindow_WithDoc::enumSaveChanges result = GlomBakery::AppWindow_WithDoc::enumSaveChanges::Cancel;

  if(!m_document)
    return result;

  GlomBakery::Dialog_OfferSave* pDialogQuestion 
    = new GlomBakery::Dialog_OfferSave( m_document->get_file_uri() );

  Gtk::Window* pWindow = this;
  if(pWindow)
    pDialogQuestion->set_transient_for(*pWindow);

  GlomBakery::Dialog_OfferSave::enumButtons buttonClicked = (GlomBakery::Dialog_OfferSave::enumButtons)pDialogQuestion->run();
  delete pDialogQuestion;
  pDialogQuestion = nullptr;

  if(buttonClicked == GlomBakery::Dialog_OfferSave::enumButtons::Save)
     result = GlomBakery::AppWindow_WithDoc::enumSaveChanges::Save;
  else if(buttonClicked == GlomBakery::Dialog_OfferSave::enumButtons::Discard)
     result = GlomBakery::AppWindow_WithDoc::enumSaveChanges::Discard;
  else
     result = GlomBakery::AppWindow_WithDoc::enumSaveChanges::Cancel;

  return result;
}

void AppWindow::document_history_remove(const Glib::ustring& file_uri)
{
  if(!file_uri.empty())
  {
    //Glib::ustring filename_e = Gnome::Vfs::escape_path_string(file_uri.c_str());
    const Glib::ustring uri = file_uri; //"file://" + filename_e;

    Gtk::RecentManager::get_default()->remove_item(uri);
  }
}

void AppWindow::on_menu_edit_copy_activate()
{
  auto widget = get_focus();
  auto editable = dynamic_cast<Gtk::Editable*>(widget);

  if(editable)
  {
    editable->copy_clipboard();
    return;
  }

  //GtkTextView does not implement GtkEditable.
  //See GTK+ bug: https://bugzilla.gnome.org/show_bug.cgi?id=667008
  auto textview = dynamic_cast<Gtk::TextView*>(widget);
  if(textview)
  {
    auto buffer = textview->get_buffer();
    if(buffer)
    {
      auto clipboard = 
        Gtk::Clipboard::get_for_display(get_display());
      buffer->copy_clipboard(clipboard);
    }
  }
}

void AppWindow::on_menu_edit_cut_activate()
{
  auto widget = get_focus();
  auto editable = dynamic_cast<Gtk::Editable*>(widget);

  if(editable)
  {
    editable->cut_clipboard();
    return;
  }

  //GtkTextView does not implement GtkEditable.
  //See GTK+ bug: https://bugzilla.gnome.org/show_bug.cgi?id=667008
  auto textview = dynamic_cast<Gtk::TextView*>(widget);
  if(textview)
  {
    auto buffer = textview->get_buffer();
    if(buffer)
    {
      auto clipboard = 
        Gtk::Clipboard::get_for_display(get_display());
      buffer->cut_clipboard(clipboard, textview->get_editable());
    }
  }
}

void AppWindow::on_menu_edit_paste_activate()
{
  auto widget = get_focus();
  auto editable = dynamic_cast<Gtk::Editable*>(widget);

  if(editable)
  {
    editable->paste_clipboard();
    return;
  }

  //GtkTextView does not implement GtkEditable.
  //See GTK+ bug: https://bugzilla.gnome.org/show_bug.cgi?id=667008
  auto textview = dynamic_cast<Gtk::TextView*>(widget);
  if(textview)
  {
    auto buffer = textview->get_buffer();
    if(buffer)
    {
      auto clipboard = 
        Gtk::Clipboard::get_for_display(get_display());
      buffer->paste_clipboard(clipboard);
    }
  }
}

void AppWindow::on_menu_edit_find()
{
  //The state is not changed automatically:
  bool active = false;
  m_action_mode_find->get_state(active);
  active = !active;
  m_action_mode_find->change_state(active);

  if(active)
    m_frame->set_mode_find();
  else
    m_frame->set_mode_data();
}

} //namespace Glom
