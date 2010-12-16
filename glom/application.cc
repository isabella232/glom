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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "config.h" //For VERSION, GLOM_ENABLE_CLIENT_ONLY, GLOM_ENABLE_SQLITE

#include <glom/application.h>
#include <glom/dialog_existing_or_new.h>

#include <glom/dialog_progress_creating.h>

#ifndef GLOM_ENABLE_CLIENT_ONLY
#include <glom/mode_design/translation/dialog_change_language.h>
#include <glom/mode_design/translation/window_translations.h>
#include <glom/utility_widgets/filechooserdialog_saveextras.h>
#include <glom/glade_utils.h>
#endif // !GLOM_ENABLE_CLIENT_ONLY

#include <glom/utils_ui.h>
#include <glom/glade_utils.h>
#include <libglom/db_utils.h>
#include <libglom/privs.h>
#include <glom/python_embed/python_ui_callbacks.h>
#include <glom/python_embed/glom_python.h>
#include <libglom/spawn_with_feedback.h>

#include <cstdio>
#include <memory> //For std::auto_ptr<>
#include <giomm.h>
#include <sstream> //For stringstream.

#ifdef GLOM_ENABLE_MAEMO
#include <hildon/hildon.h>
#include <hildonmm/program.h>
#include <hildon-fmmm.h>
#endif // GLOM_ENABLE_MAEMO

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

//static const int GLOM_RESPONSE_BROWSE_NETWORK = 1;

// Global application variable
Application* global_application = 0;

const char* Application::glade_id("window_main");
const bool Application::glade_developer(false);

Application::Application(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: type_base(cobject, "Glom"),
  m_pBoxTop(0),
  m_pFrame(0),
#ifndef GLOM_ENABLE_CLIENT_ONLY
  m_window_translations(0),
#endif // !GLOM_ENABLE_CLIENT_ONLY
  m_menu_tables_ui_merge_id(0),
  m_menu_reports_ui_merge_id(0),
  m_menu_print_layouts_ui_merge_id(0),
#ifndef GLOM_ENABLE_CLIENT_ONLY
  m_ui_save_extra_showextras(false),
  m_ui_save_extra_newdb_hosting_mode(Document::HOSTING_MODE_DEFAULT),
  m_avahi_progress_dialog(0),
  m_dialog_progress_creating(0),
  m_dialog_progess_save_backup(0),
  m_dialog_progess_convert_backup(0),
#endif // !GLOM_ENABLE_CLIENT_ONLY
  m_show_sql_debug(false)
{
  Gtk::Window::set_default_icon_name("glom");

  //Load widgets from glade file:
  builder->get_widget("bakery_vbox", m_pBoxTop);
  builder->get_widget("sidebar_vbox", m_pBoxSidebar);
  builder->get_widget_derived("vbox_frame", m_pFrame); //This one is derived. There's a lot happening here.

  add_mime_type("application/x-glom"); //TODO: make this actually work - we need to register it properly.

#ifndef GLOM_ENABLE_CLIENT_ONLY
#ifndef G_OS_WIN32
  //Install UI hooks for this:
  ConnectionPool* connection_pool = ConnectionPool::get_instance();
  if(!connection_pool)
    connection_pool->set_avahi_publish_callbacks(
      sigc::mem_fun(*this, &Application::on_connection_avahi_begin),
      sigc::mem_fun(*this, &Application::on_connection_avahi_progress),
      sigc::mem_fun(*this, &Application::on_connection_avahi_done) );
#endif
#endif // !GLOM_ENABLE_CLIENT_ONLY

  global_application = this;
}

Application::~Application()
{
  #ifndef GLOM_ENABLE_CLIENT_ONLY
  if(m_window_translations)
  {
    m_pFrame->remove_view(m_window_translations);
    delete m_window_translations;
  }

  delete m_avahi_progress_dialog;
  m_avahi_progress_dialog = 0;

  delete m_dialog_progress_creating;
  m_dialog_progress_creating = 0;

  delete m_dialog_progess_save_backup;

  delete m_dialog_progess_convert_backup;
  m_dialog_progess_convert_backup = 0;
  #endif // !GLOM_ENABLE_CLIENT_ONLY

  #ifdef GLOM_ENABLE_MAEMO
  m_pFrame->remove_view(&m_appmenu_button_table);
  #endif

  //This was set in the constructor:
  global_application = 0;
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void Application::on_connection_avahi_begin()
{
  //Create the dialog:
  delete m_avahi_progress_dialog;
  m_avahi_progress_dialog = 0;

  m_avahi_progress_dialog = new Gtk::MessageDialog(Utils::bold_message(_("Glom: Generating Encryption Certificates")), true, Gtk::MESSAGE_INFO);
  m_avahi_progress_dialog->set_secondary_text(_("Please wait while Glom prepares your system for publishing over the network."));
  m_avahi_progress_dialog->set_transient_for(*this);
  m_avahi_progress_dialog->show();
}

void Application::on_connection_avahi_progress()
{
  //Allow GTK+ to process events, so that the UI is responsive:
  while(Gtk::Main::events_pending())
   Gtk::Main::iteration();
}

void Application::on_connection_avahi_done()
{
  //Delete the dialog:
  delete m_avahi_progress_dialog;
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

bool Application::init(const Glib::ustring& document_uri)
{
  return init(document_uri, false);
}

bool Application::init(const Glib::ustring& document_uri, bool restore)
{
  type_vec_strings vecAuthors;
  vecAuthors.push_back("Murray Cumming <murrayc@murrayc.com>");
  set_about_information(PACKAGE_VERSION, vecAuthors, _("© 2000-2010 Murray Cumming"), _("A Database GUI"));

  type_base::init(); //calls init_menus() and init_toolbars()

  //m_pFrame->set_shadow_type(Gtk::SHADOW_IN);

  if(document_uri.empty())
  {
    Document* pDocument = static_cast<Document*>(get_document());
    if(pDocument && pDocument->get_connection_database().empty()) //If it is a new (default) document.
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
      const bool test = open_document(document_uri);
      if(!test)
        return offer_new_or_existing();
    }
  }

  return true;
  //show_all();
}

bool Application::get_show_sql_debug() const
{
  return m_show_sql_debug;
}

void Application::set_show_sql_debug(bool val)
{
  m_show_sql_debug = val;
}

void Application::set_stop_auto_server_shutdown(bool val)
{
  ConnectionPool* connection_pool = ConnectionPool::get_instance();
  if(connection_pool)
    connection_pool->set_auto_server_shutdown(!val);
}

void Application::init_layout()
{
  //We override this method so that we can put everything in the vbox from the glade file, instead of the vbox from App_Gtk.

  //Add menu bar at the top:
  //These were defined in init_uimanager().
#ifndef GLOM_ENABLE_MAEMO //Maemo uses Hildon::AppMenu instead.
  Gtk::MenuBar* pMenuBar = static_cast<Gtk::MenuBar*>(m_refUIManager->get_widget("/Bakery_MainMenu"));
  m_pBoxTop->pack_start(*pMenuBar, Gtk::PACK_SHRINK);
#endif

  add_accel_group(m_refUIManager->get_accel_group());

  //Add placeholder, to be used by add():
  //m_pBoxTop->pack_start(m_VBox_PlaceHolder);
  //m_VBox_PlaceHolder.show();
}

void Application::init_toolbars()
{
  //We override this because
  //a) We don't want a toolbar, and
  //b) The default toolbar layout has actions that we don't have.

/*
  //Build part of the menu structure, to be merged in by using the "PH" placeholders:
  static const Glib::ustring ui_description =
    "<ui>"
    "  <toolbar name='Bakery_ToolBar'>"
    "    <placeholder name='Bakery_ToolBarItemsPH'>"
    "      <toolitem action='BakeryAction_File_New' />"
    "      <toolitem action='BakeryAction_File_Open' />"
    "      <toolitem action='BakeryAction_File_Save' />"
    "    </placeholder>"
    "  </toolbar>"
    "</ui>";

  add_ui_from_string(ui_description);
*/
}

#ifndef GLOM_ENABLE_MAEMO
void Application::init_menus_file()
{
  //Overridden to remove the Save and Save-As menu items,
  //because all changes are saved immediately and automatically.

  // File menu

  //Build actions:
  m_refFileActionGroup = Gtk::ActionGroup::create("BakeryFileActions");

  m_refFileActionGroup->add(Gtk::Action::create("BakeryAction_Menu_File", _("_File")));
  m_refFileActionGroup->add(Gtk::Action::create("BakeryAction_Menu_File_RecentFiles", _("_Recent Files")));

  //File actions
  m_refFileActionGroup->add(Gtk::Action::create("BakeryAction_File_New", Gtk::Stock::NEW),
                        sigc::mem_fun((App&)*this, &App::on_menu_file_new));
  m_refFileActionGroup->add(Gtk::Action::create("BakeryAction_File_Open", Gtk::Stock::OPEN),
                        sigc::mem_fun((App_WithDoc&)*this, &App_WithDoc::on_menu_file_open));

  Glib::RefPtr<Gtk::Action> action = Gtk::Action::create("BakeryAction_File_SaveAsExample", _("_Save as Example"));
  m_listDeveloperActions.push_back(action);

#ifndef GLOM_ENABLE_CLIENT_ONLY
  m_refFileActionGroup->add(action,
                        sigc::mem_fun((Application&)*this, &Application::on_menu_file_save_as_example));

  action = Gtk::Action::create("BakeryAction_Menu_File_Export", _("_Export"));
  m_refFileActionGroup->add(action, sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_file_export));
  m_listTableSensitiveActions.push_back(action);

  action = Gtk::Action::create("BakeryAction_Menu_File_Import", _("I_mport"));
  m_refFileActionGroup->add(action, sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_file_import));
  m_listTableSensitiveActions.push_back(action);
#endif // !GLOM_ENABLE_CLIENT_ONLY

  m_toggleaction_network_shared = Gtk::ToggleAction::create("BakeryAction_Menu_File_Share", _("S_hared on Network"));
  m_refFileActionGroup->add(m_toggleaction_network_shared);
  m_listTableSensitiveActions.push_back(m_toggleaction_network_shared);

#ifndef GLOM_ENABLE_CLIENT_ONLY
  m_connection_toggleaction_network_shared =
    m_toggleaction_network_shared->signal_toggled().connect(
      sigc::mem_fun(*this, &Application::on_menu_file_toggle_share) );
  m_listDeveloperActions.push_back(m_toggleaction_network_shared);
#endif //!GLOM_ENABLE_CLIENT_ONLY

  action = Gtk::Action::create("GlomAction_Menu_File_Print", Gtk::Stock::PRINT);
  m_refFileActionGroup->add(action);
  m_listTableSensitiveActions.push_back(action);
  m_refFileActionGroup->add(Gtk::Action::create("GlomAction_File_Print", _("_Standard")),
                        sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_file_print) );

#ifndef GLOM_ENABLE_CLIENT_ONLY
  Glib::RefPtr<Gtk::Action> action_print_edit = Gtk::Action::create("GlomAction_File_PrintEdit", _("_Edit Print Layouts"));
  m_refFileActionGroup->add(action_print_edit, sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_file_print_edit_layouts));
  m_listDeveloperActions.push_back(action_print_edit);
#endif // !GLOM_ENABLE_CLIENT_ONLY

  m_refFileActionGroup->add(Gtk::Action::create("BakeryAction_File_Close", Gtk::Stock::CLOSE),
                        sigc::mem_fun((App_WithDoc&)*this, &App_WithDoc::on_menu_file_close));

  m_refUIManager->insert_action_group(m_refFileActionGroup);

  //Build part of the menu structure, to be merged in by using the "PH" placeholders:
  static const Glib::ustring ui_description =
    "<ui>"
    "  <menubar name='Bakery_MainMenu'>"
    "    <placeholder name='Bakery_MenuPH_File'>"
    "      <menu action='BakeryAction_Menu_File'>"
    "        <menuitem action='BakeryAction_File_New' />"
    "        <menuitem action='BakeryAction_File_Open' />"
    "        <menu action='BakeryAction_Menu_File_RecentFiles'>"
    "        </menu>"
#ifndef GLOM_ENABLE_CLIENT_ONLY
    "        <menuitem action='BakeryAction_File_SaveAsExample' />"
    "        <separator/>"
    "        <menuitem action='BakeryAction_Menu_File_Export' />"
    "        <menuitem action='BakeryAction_Menu_File_Import' />"
    "        <menuitem action='BakeryAction_Menu_File_Share' />"
#endif // !GLOM_ENABLE_CLIENT_ONLY
    "        <separator/>"
    "        <menu action='GlomAction_Menu_File_Print'>"
    "          <menuitem action='GlomAction_File_Print' />"
    "          <placeholder name='Menu_PrintLayouts_Dynamic' />"
#ifndef GLOM_ENABLE_CLIENT_ONLY
    "          <menuitem action='GlomAction_File_PrintEdit' />"
#endif //GLOM_ENABLE_CLIENT_ONLY
    "        </menu>"
    "        <separator/>"
    "        <menuitem action='BakeryAction_File_Close' />"
    "      </menu>"
    "    </placeholder>"
    "  </menubar>"
    "</ui>";

  //Add menu:
  add_ui_from_string(ui_description);

  //Add recent-files submenu:
  init_menus_file_recentfiles("/Bakery_MainMenu/Bakery_MenuPH_File/BakeryAction_Menu_File/BakeryAction_Menu_File_RecentFiles");
}
#endif //GLOM_ENABLE_MAEMO

#ifdef GLOM_ENABLE_MAEMO
void Application::on_appmenu_button_table_value_changed()
{
  const Glib::ustring table_name = m_appmenu_button_table.get_table_name();
  if(m_pFrame)
    m_pFrame->on_box_tables_selected(table_name);
}

#endif //GLOM_ENABLE_MAEMO

#ifdef GLOM_ENABLE_MAEMO
static void add_button_to_appmenu(Hildon::AppMenu& appmenu, const Glib::ustring& title, const Glib::ustring& secondary, const sigc::slot<void>& clicked_handler)
{
  Hildon::Button* button =
    Gtk::manage(new Hildon::Button(
      Gtk::Hildon::SIZE_AUTO,
      Hildon::BUTTON_ARRANGEMENT_VERTICAL,
      title, secondary));
  button->show();
  button->signal_clicked().connect(clicked_handler);
  appmenu.append(*button);
}

void Application::init_menus()
{
  //There is no real menu on Maemo. We use HildonAppMenu instead.

  m_pFrame->add_view(&m_appmenu_button_table);
  m_appmenu_button_table.show();
  m_appmenu_button_table.signal_value_changed().connect(
    sigc::mem_fun(*this, &Application::on_appmenu_button_table_value_changed) );
  m_maemo_appmenu.append(m_appmenu_button_table);

  add_button_to_appmenu(m_maemo_appmenu,
    _("Find"), _("Search for records in the table"),
    sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_Mode_Toggle) );

  add_button_to_appmenu(m_maemo_appmenu,
    _("Add Record"), _("Create a new record in the table"),
    sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_add_record) );

  //set_app_menu(*appmenu); //TODO: Use this instead?
  Hildon::Program::get_instance()->set_common_app_menu(m_maemo_appmenu);
}
#else
void Application::init_menus()
{
  init_menus_file();
  init_menus_edit();

  //Build actions:
  m_refActionGroup_Others = Gtk::ActionGroup::create("GlomOthersActions");

  //"Tables" menu:
  m_refActionGroup_Others->add( Gtk::Action::create("Glom_Menu_Tables", _("_Tables")) );

//  Glib::RefPtr<Gtk::Action> action = Gtk::Action::create("GlomAction_Menu_Navigate_Database", _("_Database"));
//  m_listDeveloperActions.push_back(action);
//  m_refActionGroup_Others->add(action,
//                        sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_Navigate_Database) );

  Glib::RefPtr<Gtk::Action> action;

#ifndef GLOM_ENABLE_CLIENT_ONLY
  action = Gtk::Action::create("GlomAction_Menu_EditTables", _("_Edit Tables"));
  m_refActionGroup_Others->add(action,
                        sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_Tables_EditTables) );
  m_listDeveloperActions.push_back(action);

  action = Gtk::Action::create("GlomAction_Menu_AddRelatedTable", _("Add _Related Table"));
  m_refActionGroup_Others->add(action,
                        sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_Tables_AddRelatedTable) );
  m_listDeveloperActions.push_back(action);
#endif // !GLOM_ENABLE_CLIENT_ONLY

  //"Reports" menu:
  m_refActionGroup_Others->add( Gtk::Action::create("Glom_Menu_Reports", _("_Reports")) );

#ifndef GLOM_ENABLE_CLIENT_ONLY
  action = Gtk::Action::create("GlomAction_Menu_EditReports", _("_Edit Reports"));
  m_refActionGroup_Others->add(action,
                        sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_Reports_EditReports) );
  m_listDeveloperActions.push_back(action);
  m_listTableSensitiveActions.push_back(action);
#endif

#ifndef GLOM_ENABLE_CLIENT_ONLY
  //"UserLevel" menu:
  m_refActionGroup_Others->add(Gtk::Action::create("Glom_Menu_userlevel", _("_User Level")));
  Gtk::RadioAction::Group group_userlevel;

  m_action_menu_userlevel_developer = Gtk::RadioAction::create(group_userlevel, "GlomAction_Menu_userlevel_Developer", C_("User-level menu item", "_Developer"));
  m_refActionGroup_Others->add(m_action_menu_userlevel_developer,
                        sigc::mem_fun(*this, &Application::on_menu_userlevel_developer) );

  m_action_menu_userlevel_operator =  Gtk::RadioAction::create(group_userlevel, "GlomAction_Menu_userlevel_Operator", C_("User-level menu item", "_Operator"));
  m_refActionGroup_Others->add(m_action_menu_userlevel_operator,
                          sigc::mem_fun(*this, &Application::on_menu_userlevel_operator) );
#endif // !GLOM_ENABLE_CLIENT_ONLY

  //"Mode" menu:
  action =  Gtk::Action::create("Glom_Menu_Mode", _("_Mode"));
  m_refActionGroup_Others->add(action);

  //We remember this action, so that it can be explicitly activated later.
  m_action_mode_find = Gtk::ToggleAction::create("GlomAction_Menu_Mode_Toggle", _("_Find"), "", false);
  m_refActionGroup_Others->add(m_action_mode_find,  Gtk::AccelKey("<control>F"),
                        sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_Mode_Toggle) );
  m_listTableSensitiveActions.push_back(m_action_mode_find);

#ifndef GLOM_ENABLE_CLIENT_ONLY
  action = Gtk::Action::create("Glom_Menu_Developer", C_("Developer menu title", "_Developer"));
  m_listDeveloperActions.push_back(action);
  m_refActionGroup_Others->add(action);

  action = Gtk::Action::create("GlomAction_Menu_Developer_Database_Preferences", _("_Database Preferences"));
  m_listDeveloperActions.push_back(action);
  m_refActionGroup_Others->add(action, sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_developer_database_preferences) );


  action = Gtk::Action::create("GlomAction_Menu_Developer_Fields", _("_Fields"));
  m_listDeveloperActions.push_back(action);
  m_listTableSensitiveActions.push_back(action);
  m_refActionGroup_Others->add(action, sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_developer_fields) );

  action = Gtk::Action::create("GlomAction_Menu_Developer_RelationshipsOverview", _("Relationships _Overview"));
  m_listDeveloperActions.push_back(action);
  m_listTableSensitiveActions.push_back(action);
  m_refActionGroup_Others->add(action, sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_developer_relationships_overview) );

  action = Gtk::Action::create("GlomAction_Menu_Developer_Relationships", _("_Relationships for this Table"));
  m_listDeveloperActions.push_back(action);
  m_listTableSensitiveActions.push_back(action);
  m_refActionGroup_Others->add(action, sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_developer_relationships) );

  m_action_developer_users = Gtk::Action::create("GlomAction_Menu_Developer_Users", _("_Users"));
  m_listDeveloperActions.push_back(m_action_developer_users);
  m_refActionGroup_Others->add(m_action_developer_users, sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_developer_users));

  action = Gtk::Action::create("GlomAction_Menu_Developer_PrintLayouts", _("_Print Layouts")); //TODO: Rename? This looks like an action rather than a noun. It won't actually start printing.
  m_listDeveloperActions.push_back(action);
  m_listTableSensitiveActions.push_back(action);
  m_refActionGroup_Others->add(action, sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_developer_print_layouts));

  action = Gtk::Action::create("GlomAction_Menu_Developer_Reports", _("R_eports"));
  m_listDeveloperActions.push_back(action);
  m_listTableSensitiveActions.push_back(action);
  m_refActionGroup_Others->add(action, sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_developer_reports));

  action = Gtk::Action::create("GlomAction_Menu_Developer_Script_Library", _("Script _Library"));
  m_listDeveloperActions.push_back(action);
  m_refActionGroup_Others->add(action, sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_developer_script_library));


  action = Gtk::Action::create("GlomAction_Menu_Developer_Layout", _("_Layout"));
  m_listDeveloperActions.push_back(action);
  m_listTableSensitiveActions.push_back(action);
  m_refActionGroup_Others->add(action, sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_developer_layout));

  action = Gtk::Action::create("GlomAction_Menu_Developer_ChangeLanguage", _("Test Tra_nslation"));
  m_listDeveloperActions.push_back(action);
  m_refActionGroup_Others->add(action, sigc::mem_fun(*this, &Application::on_menu_developer_changelanguage));

  action = Gtk::Action::create("GlomAction_Menu_Developer_Translations", _("_Translations"));
  m_listDeveloperActions.push_back(action);
  m_refActionGroup_Others->add(action, sigc::mem_fun(*this, &Application::on_menu_developer_translations));


  //"Active Platform" menu:
  action =  Gtk::Action::create("Glom_Menu_Developer_ActivePlatform", _("_Active Platform"));
  m_refActionGroup_Others->add(action);
  Gtk::RadioAction::Group group_active_platform;

  action = Gtk::RadioAction::create(group_active_platform, "GlomAction_Menu_Developer_ActivePlatform_Normal",
    _("_Normal"), _("The layout to use for normal desktop environments."));
  m_listDeveloperActions.push_back(action);
  m_refActionGroup_Others->add(action, sigc::mem_fun(*this, &Application::on_menu_developer_active_platform_normal));

  action = Gtk::RadioAction::create(group_active_platform, "GlomAction_Menu_Developer_ActivePlatform_Maemo",
    _("_Maemo"), _("The layout to use for Maemo devices."));
  m_listDeveloperActions.push_back(action);
  m_refActionGroup_Others->add(action, sigc::mem_fun(*this, &Application::on_menu_developer_active_platform_maemo));


  action = Gtk::Action::create("GlomAction_Menu_Developer_ExportBackup", _("_Export Backup"));
  m_listDeveloperActions.push_back(action);
  m_refActionGroup_Others->add(action, sigc::mem_fun(*this, &Application::on_menu_developer_export_backup));

  action = Gtk::Action::create("GlomAction_Menu_Developer_RestoreBackup", _("_Restore Backup"));
  m_listDeveloperActions.push_back(action);
  m_refActionGroup_Others->add(action, sigc::mem_fun(*this, &Application::on_menu_developer_restore_backup));

  m_action_show_layout_toolbar = Gtk::ToggleAction::create("GlomAction_Menu_Developer_ShowLayoutToolbar", _("_Show Layout Toolbar"));
  m_listDeveloperActions.push_back(m_action_show_layout_toolbar);
  m_refActionGroup_Others->add(m_action_show_layout_toolbar, sigc::mem_fun(*this, &Application::on_menu_developer_show_layout_toolbar));

#endif // !GLOM_ENABLE_CLIENT_ONLY

  m_refUIManager->insert_action_group(m_refActionGroup_Others);

  //Build part of the menu structure, to be merged in by using the "Bakery_MenuPH_Others" placeholder:
  static const Glib::ustring ui_description =
    "<ui>"
    "  <menubar name='Bakery_MainMenu'>"
    "    <placeholder name='Bakery_MenuPH_Edit'>"
    "      <menu action='BakeryAction_Menu_Edit'>"
    "        <menuitem action='BakeryAction_Edit_Cut' />"
    "        <menuitem action='BakeryAction_Edit_Copy' />"
    "        <menuitem action='BakeryAction_Edit_Paste' />"
    "        <menuitem action='BakeryAction_Edit_Clear' />"
    "        <separator />"
    "        <menuitem action='GlomAction_Menu_Mode_Toggle' />"
    "      </menu>"
    "    </placeholder>"
    "    <placeholder name='Bakery_MenuPH_Others'>"
    "      <menu action='Glom_Menu_Tables'>"
    "        <placeholder name='Menu_Tables_Dynamic' />"
    "        <separator />"
#ifndef GLOM_ENABLE_CLIENT_ONLY
    "        <menuitem action='GlomAction_Menu_EditTables' />"
    "        <menuitem action='GlomAction_Menu_AddRelatedTable' />"
#endif // !GLOM_ENABLE_CLIENT_ONLY
    "     </menu>"
    "     <menu action='Glom_Menu_Reports'>"
    "        <placeholder name='Menu_Reports_Dynamic' />"
#ifndef GLOM_ENABLE_CLIENT_ONLY
    "        <separator />"
    "        <menuitem action='GlomAction_Menu_EditReports' />"
#endif // !GLOM_ENABLE_CLIENT_ONLY
    "     </menu>"
#ifndef GLOM_ENABLE_CLIENT_ONLY
    "      <menu action='Glom_Menu_userlevel'>"
    "        <menuitem action='GlomAction_Menu_userlevel_Developer' />"
    "        <menuitem action='GlomAction_Menu_userlevel_Operator' />"
    "      </menu>"
    "      <menu action='Glom_Menu_Developer'>"
    "        <menuitem action='GlomAction_Menu_Developer_Fields' />"
    "        <menuitem action='GlomAction_Menu_Developer_Relationships' />"
    "        <menuitem action='GlomAction_Menu_Developer_RelationshipsOverview' />"
    "        <menuitem action='GlomAction_Menu_Developer_Layout' />"
    "        <menuitem action='GlomAction_Menu_Developer_PrintLayouts' />"
    "        <menuitem action='GlomAction_Menu_Developer_Reports' />"
    "        <separator />"
    "        <menuitem action='GlomAction_Menu_Developer_Database_Preferences' />"
    "        <menuitem action='GlomAction_Menu_Developer_Users' />"
    "        <menuitem action='GlomAction_Menu_Developer_Script_Library' />"
    "        <separator />"
    "        <menuitem action='GlomAction_Menu_Developer_Translations' />"
    "        <menuitem action='GlomAction_Menu_Developer_ChangeLanguage' />"
    "        <separator />"
    "        <menu action='Glom_Menu_Developer_ActivePlatform'>"
    "          <menuitem action='GlomAction_Menu_Developer_ActivePlatform_Normal' />"
    "          <menuitem action='GlomAction_Menu_Developer_ActivePlatform_Maemo' />"
    "        </menu>"
    "        <menuitem action='GlomAction_Menu_Developer_ShowLayoutToolbar' />"
    "        <separator />"
    "        <menuitem action='GlomAction_Menu_Developer_ExportBackup' />"
    "        <menuitem action='GlomAction_Menu_Developer_RestoreBackup' />"
    "      </menu>"
#endif // !GLOM_ENABLE_CLIENT_ONLY
    "    </placeholder>"
    "  </menubar>"
    "</ui>";

/*  "        <menuitem action='GlomAction_Menu_Developer_RelationshipsOverview' />" */

  //Add menu:
  add_ui_from_string(ui_description);

  init_menus_help();

  update_table_sensitive_ui();

  fill_menu_tables();
}
#endif //GLOM_ENABLE_MAEMO

#ifndef GLOM_ENABLE_CLIENT_ONLY
void Application::on_menu_file_toggle_share()
{
  if(!m_pFrame)
    return;

  m_pFrame->on_menu_file_toggle_share(m_toggleaction_network_shared);
}

void Application::on_menu_userlevel_developer()
{
  if(!m_pFrame)
    return;

  m_pFrame->on_menu_userlevel_Developer(m_action_menu_userlevel_developer, m_action_menu_userlevel_operator);
  m_pFrame->show_layout_toolbar(m_action_show_layout_toolbar->get_active());
}

void Application::on_menu_userlevel_operator()
{
  if(m_pFrame)
  {
    m_pFrame->on_menu_userlevel_Operator(m_action_menu_userlevel_operator);
    m_pFrame->show_layout_toolbar(false);
  }
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

void Application::ui_warning_load_failed(int failure_code)
{
  if(failure_code == Document::LOAD_FAILURE_CODE_NOT_FOUND)
  {
    //TODO: Put this in the generic bakery code.
    ui_warning(_("Open Failed"),
      _("The document could not be found."));

    //TODO: Glom::Bakery::App_WithDoc removes the file from the recent history,
    //but the initial/welcome dialog doesn't yet update its list when the
    //recent history changes.
  }
  else if(failure_code == Document::LOAD_FAILURE_CODE_FILE_VERSION_TOO_NEW)
  {
    ui_warning(_("Open Failed"),
      _("The document could not be opened because it was created or modified by a newer version of Glom."));
  }
  else
    GlomBakery::App_WithDoc_Gtk::ui_warning_load_failed();
}


#ifndef G_OS_WIN32
void Application::open_browsed_document(const EpcServiceInfo* server, const Glib::ustring& service_name)
{
  gsize length = 0;
  gchar *document_contents = 0;

  bool keep_trying = true;
  while(keep_trying)
  {
    //Request a password to attempt retrieval of the document over the network:
    Dialog_Connection* dialog_connection = 0;
    //Load the Glade file and instantiate its widgets to get the dialog stuff:
    Utils::get_glade_widget_derived_with_warning(dialog_connection);
    dialog_connection->set_transient_for(*this);
    dialog_connection->set_connect_to_browsed();
    dialog_connection->set_database_name(service_name);
    const int response = Glom::Utils::dialog_run_with_help(dialog_connection);
    dialog_connection->hide();
    if(response != Gtk::RESPONSE_OK)
      keep_trying = false;
    else
    {
      //Open the document supplied by the other glom instance on the network:
      EpcConsumer* consumer = epc_consumer_new(server);

      Glib::ustring username, password;
      dialog_connection->get_username_and_password(username, password);
      epc_consumer_set_username(consumer, username.c_str());
      epc_consumer_set_password(consumer, password.c_str());

      GError *error = 0;
      document_contents = (gchar*)epc_consumer_lookup(consumer, "document", &length, &error);
      if(error)
      {
        std::cout << "debug: " << G_STRFUNC << ": " << std::endl << "  " << error->message << std::endl;
        const int error_code = error->code;
        g_clear_error(&error);

        if(error_code == SOUP_STATUS_FORBIDDEN ||
           error_code == SOUP_STATUS_UNAUTHORIZED)
        {
          //std::cout << "   SOUP_STATUS_FORBIDDEN or SOUP_STATUS_UNAUTHORIZED" << std::endl;

          Utils::show_ok_dialog(_("Connection Failed"), _("Glom could not connect to the database server. Maybe you entered an incorrect user name or password, or maybe the postgres database server is not running."), *this, Gtk::MESSAGE_ERROR); //TODO: Add help button.
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
    dialog_connection = 0;

  }


  if(document_contents && length)
  {
    //Create a temporary Document instance, so we can manipulate the data:
    Document document_temp;
    int failure_code = 0;
    const bool loaded = document_temp.load_from_data((const guchar*)document_contents, length, failure_code);
    if(loaded)
    {
      // Connection is always remote-hosted in client only mode:
#ifndef GLOM_ENABLE_CLIENT_ONLY
#ifdef GLOM_ENABLE_POSTGRESQL
      //Stop the document from being self-hosted (it's already hosted by the other networked Glom instance):
      if(document_temp.get_hosting_mode() == Document::HOSTING_MODE_POSTGRES_SELF)
        document_temp.set_hosting_mode(Document::HOSTING_MODE_POSTGRES_CENTRAL);
#endif //GLOM_ENABLE_POSTGRESQL
#endif // !GLOM_ENABLE_CLIENT_ONLY
      // TODO: Error out in case this is a sqlite database, since we probably
      // can't access it from this host?

      //If the publisher thinks that it's using a postgres database on localhost,
      //then we need to use a host name that means the same thing from the client's PC:
      const Glib::ustring host = document_temp.get_connection_server();
      if(hostname_is_localhost(host))
        document_temp.set_connection_server( epc_service_info_get_host(server) );

      //Make sure that we only use the specified port instead of connecting to some other postgres instance
      //on the same server:
      document_temp.set_connection_try_other_ports(false);
    }
    else
    {
      std::cerr << "Could not parse the document that was retrieved over the network: failure_code=" << failure_code << std::endl;
    }

    g_free(document_contents);
    document_contents = 0;

    //TODO_Performance: Horribly inefficient, but happens rarely:
    const Glib::ustring temp_document_contents = document_temp.build_and_get_contents();

    //This loads the document and connects to the database (using m_temp_username and m_temp_password):
    open_document_from_data((const guchar*)temp_document_contents.c_str(), temp_document_contents.bytes());

    //Mark the document as opened-from-browse
    //so we don't think that opening has failed because it has no URI,
    //and to stop us from allowing developer mode
    //(that would require changes to the original document).
    Document* document = dynamic_cast<Document*>(get_document());
    if(document)
    {
      document->set_opened_from_browse();
      document->set_userlevel(AppState::USERLEVEL_OPERATOR); //TODO: This should happen automatically.

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
//Copied from bakery:
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

Glib::ustring Application::get_file_uri_without_extension(const Glib::ustring& uri)
{
  if(uri.empty())
    return uri;

  Glib::RefPtr<Gio::File> file = Gio::File::create_for_uri(uri);
  if(!file)
    return uri; //Actually an error.

  const Glib::ustring filename_part = file->get_basename();

  const Glib::ustring::size_type pos_dot = filename_part.rfind(".");
  if(pos_dot == Glib::ustring::npos)
    return uri; //There was no extension, so just return the existing URI.
  else
  {
    const Glib::ustring filename_part_without_ext = filename_part.substr(0, pos_dot);

    //Use the Gio::File API to manipulate the URI:
    Glib::RefPtr<Gio::File> parent = file->get_parent();
    Glib::RefPtr<Gio::File> file_without_extension = parent->get_child(filename_part_without_ext);

    return file_without_extension->get_uri();
  }
}

void Application::new_instance(const Glib::ustring& uri) //Override
{
  Glib::ustring command = "glom";
  if(!uri.empty())
    command += ' ' + uri;

  GError* gerror = 0;
  gdk_spawn_command_line_on_screen(Glib::unwrap(get_screen()),
    command.c_str(),
    &gerror);
  if(gerror)
  {
    std::cerr << G_STRFUNC << ": " << gerror->message << std::endl;
  }
}

void Application::init_create_document()
{
  if(!m_pDocument)
  {
    Document* document_glom = new Document();
    m_pDocument = document_glom;
    //document_glom->set_parent_window(this); //So that it can show a BusyCursor when loading and saving.

    //Tell document about view:
    m_pDocument->set_view(m_pFrame);

    //Tell view about document:
    //(This calls set_document() in the child views too.)
    m_pFrame->set_document(static_cast<Document*>(m_pDocument));
  }

  type_base::init_create_document(); //Sets window title. Doesn't recreate doc.
}

bool Application::check_document_hosting_mode_is_supported(Document* document)
{
  //If it's an example then the document's hosting mode doesn't matter,
  //because the user will be asked to choose one when saving anyway.
  if(document->get_is_example_file())
    return true;

  //Check that the file's hosting mode is supported by this build:
  Glib::ustring error_message;
  switch(document->get_hosting_mode())
  {
    case Document::HOSTING_MODE_POSTGRES_SELF:
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
    case Document::HOSTING_MODE_POSTGRES_CENTRAL:
    {
      #ifndef GLOM_ENABLE_POSTGRESQL
      error_message = _("The file cannot be opened because this version of Glom does not support PostgreSQL databases.");
      #endif //GLOM_ENABLE_POSTGRESQL

      break;
    }
    case Document::HOSTING_MODE_SQLITE:
    {
      #ifndef GLOM_ENABLE_SQLITE
      error_message = _("The file cannot be opened because this version of Glom does not support SQLite databases.");
      #endif //GLOM_ENABLE_SQLITE

      break;
    }
    default:
    {
      //on_document_load() should have checked for this already, informing the user.
      std::cerr << G_STRFUNC << ": Unhandled hosting mode: " << document->get_hosting_mode() << std::endl;
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

bool Application::on_document_load()
{
  //Link to the database described in the document.
  //Need to ask user for user/password:
  //m_pFrame->load_from_document();
  Document* pDocument = static_cast<Document*>(get_document());
  if(!pDocument)
    return false;

  if(!pDocument->get_is_new() && !check_document_hosting_mode_is_supported(pDocument))
    return false;

  #ifdef GLOM_ENABLE_MAEMO
  //On Maemo, make regular (not specifically maemo) layouts single-column,
  //so they are more appropriate by default:
  pDocument->maemo_restrict_layouts_to_single_column();
  #endif //GLOM_ENABLE_MAEMO

#ifndef GLOM_ENABLE_CLIENT_ONLY
  //Connect signals:
  pDocument->signal_userlevel_changed().connect( sigc::mem_fun(*this, &Application::on_userlevel_changed) );

  //Disable/Enable actions, depending on userlevel:
  pDocument->emit_userlevel_changed();
#endif // !GLOM_ENABLE_CLIENT_ONLY

  if(pDocument->get_connection_database().empty()) //If it is a new (default) document.
  {
    //offer_new_or_existing();
  }
  else
  {
#ifndef GLOM_ENABLE_CLIENT_ONLY
    //Prevent saving until we are sure that everything worked.
    //This also stops us from losing the example data as soon as we say the new file (created from the example) is not an example.
    pDocument->set_allow_autosave(false);
#endif // !GLOM_ENABLE_CLIENT_ONLY

    // Example files and backup files are not supported in client only mode because they
    // would need to be saved, but saving support is disabled.
#ifndef GLOM_ENABLE_CLIENT_ONLY
    const bool is_example = pDocument->get_is_example_file();
    const bool is_backup = pDocument->get_is_backup_file();
#endif // !GLOM_ENABLE_CLIENT_ONLY
    const std::string original_uri = pDocument->get_file_uri();

    if(is_example || is_backup)
    {
#ifndef GLOM_ENABLE_CLIENT_ONLY
      // Remember the URI to the example file to be able to prevent
      // adding the URI to the recently used files in document_history_add.
      // We want to add the document that is created from the example
      // instead of the example itself.
      // TODO: This is a weird hack. Find a nicer way. murrayc.
      m_example_uri = original_uri;

      pDocument->set_file_uri(Glib::ustring()); //Prevent it from defaulting to the read-only examples directory when offering saveas.
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
      m_ui_save_extra_newdb_hosting_mode = Document::HOSTING_MODE_DEFAULT;


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
        pDocument->set_database_title(m_ui_save_extra_newdb_title);
        pDocument->set_hosting_mode(m_ui_save_extra_newdb_hosting_mode);
        m_ui_save_extra_newdb_hosting_mode = Document::HOSTING_MODE_DEFAULT;
        pDocument->set_is_example_file(false);
        pDocument->set_is_backup_file(false);

        // For self-hosting, we will choose a port later. For central
        // hosting, try several default ports. Don't use the values that
        // are set in the example file.
        pDocument->set_connection_port(0);
        pDocument->set_connection_try_other_ports(true);

        // We have a valid uri, so we can set it to !new and modified here
      }

      m_ui_save_extra_newdb_title.clear();
      m_ui_save_extra_showextras = false;

      if(get_operation_cancelled())
      {
        pDocument->set_modified(false);
        pDocument->set_is_new(true);
        pDocument->set_allow_autosave(true); //Turn this back on.
        std::cout << "debug: user cancelled creating database" << std::endl;
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
    Document::userLevelReason reason = Document::USER_LEVEL_REASON_UNKNOWN;
    const AppState::userlevels userlevel = pDocument->get_userlevel(reason);
    if( (userlevel == AppState::USERLEVEL_OPERATOR) && (reason == Document::USER_LEVEL_REASON_FILE_READ_ONLY) )
    {
      Gtk::MessageDialog dialog(Utils::bold_message(_("Opening Read-Only File.")), true,  Gtk::MESSAGE_INFO, Gtk::BUTTONS_NONE);
      dialog.set_secondary_text(_("This file is read only, so you will not be able to enter Developer mode to make design changes."));
      dialog.set_transient_for(*this);
      dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
      dialog.add_button(_("Continue without Developer Mode"), Gtk::RESPONSE_OK); //arbitrary response code.

      const int response = dialog.run();
      dialog.hide();
      if((response == Gtk::RESPONSE_CANCEL)  || (response == Gtk::RESPONSE_DELETE_EVENT))
        return false;
    }
#endif // !GLOM_ENABLE_CLIENT_ONLY

    //Read the connection information from the document:
    ConnectionPool* connection_pool = ConnectionPool::get_instance();
    if(!connection_pool)
      return false; //Impossible anyway.
    else
    {
#ifndef GLOM_ENABLE_CLIENT_ONLY
      connection_pool->set_get_document_func( sigc::mem_fun(*this, &Application::on_connection_pool_get_document) );
#endif

      connection_pool->set_ready_to_connect(true); //connect_to_server() will now attempt the connection-> Shared instances of m_Connection will also be usable.

      //Attempt to connect to the specified database:
      bool test = false;

#ifndef GLOM_ENABLE_CLIENT_ONLY
      if(is_example || is_backup)
      {
        //The user has already had the chance to specify a new filename and database name.
        test = m_pFrame->connection_request_password_and_choose_new_database_name();
      }
      else
#endif // !GLOM_ENABLE_CLIENT_ONLY
      {
        //Ask for the username/password and connect:
        //Note that m_temp_username and m_temp_password are set if
        //we already asked for them when getting the document over the network:

        //Use the default username/password if opening as non network-shared:
        if(!(pDocument->get_network_shared()))
        {
          // If the document is centrally hosted, don't pretend to know the
          // username or password, because we don't. The user will enter
          // the login credentials in a dialog.
          if(pDocument->get_hosting_mode() != Document::HOSTING_MODE_POSTGRES_CENTRAL)
            m_temp_username = Privs::get_default_developer_user_name(m_temp_password);
        }

        bool database_not_found = false;
        test = m_pFrame->connection_request_password_and_attempt(database_not_found, m_temp_username, m_temp_password);
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
            std::cerr << G_STRFUNC << ": unexpected database_not_found error when opening example." << std::endl;
        }
        else if(!test)
        {
          //std::cerr might show some hints, but we don't want to confront the user with them:
          //TODO: Actually complain about specific stuff such as missing data, because the user might really play with the file system.
          Frame_Glom::show_ok_dialog(_("Problem Loading Document"), _("Glom could not load the document."), *this, Gtk::MESSAGE_ERROR);
          std::cerr << G_STRFUNC << ": unexpected error." << std::endl;
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
        bool test = false;
        if(is_example)
          test = recreate_database_from_example(user_cancelled);
        else
          test = recreate_database_from_backup(original_uri, user_cancelled);

        if(!test)
        {
          // TODO: Do we need to call connection_pool->cleanup() here, for
          // stopping self-hosted databases? armin.
          connection_pool->cleanup( sigc::mem_fun(*this, &Application::on_connection_close_progress) );
          //If the database was not successfully recreated:
          return false;
        }
        else
        {
          //Make sure that the changes (mark as non example, and save the new database name) are really saved:
          //Change the user level temporarily so that save_changes() actually saves:
          const AppState::userlevels user_level = pDocument->get_userlevel();
          pDocument->set_userlevel(AppState::USERLEVEL_DEVELOPER);
          pDocument->set_modified(true);
          pDocument->set_allow_autosave(true); //Turn this back on.
          pDocument->set_userlevel(user_level); //Change it back.
        }
      }
#endif // !GLOM_ENABLE_CLIENT_ONLY

      //Switch to operator mode when opening new documents:
      pDocument->set_userlevel(AppState::USERLEVEL_OPERATOR);

      //Make sure that it's saved in history, even if it was saved from an example file:
      document_history_add(pDocument->get_file_uri());

      //Open default table, or show list of tables instead:
      m_pFrame->do_menu_Navigate_Table(true /* open the default if there is one */);
    }
  }

  //List the non-hidden tables in the menu:
  fill_menu_tables();

  update_network_shared_ui();

  //Run any startup script:
  const Glib::ustring script = pDocument->get_startup_script();
  if(!script.empty())
  {
    Glib::ustring error_message; //TODO: Check this and tell the user.
    ConnectionPool* connection_pool = ConnectionPool::get_instance();
    sharedptr<SharedConnection> sharedconnection = connection_pool->connect();
    AppPythonUICallbacks callbacks;
    glom_execute_python_function_implementation(script,
      type_map_fields(), //only used when there is a current table and record.
      pDocument,
      Glib::ustring() /* table_name */,
      sharedptr<Field>(), Gnome::Gda::Value(), // primary key - only used when there is a current table and record.
      sharedconnection->get_gda_connection(),
      callbacks,
      error_message);
  }

#ifndef GLOM_ENABLE_CLIENT_ONLY
  pDocument->set_allow_autosave(true);
#endif // !GLOM_ENABLE_CLIENT_ONLY

  return true; //Loading of the document into the application succeeded.
}

void Application::on_connection_close_progress()
{
  //TODO_murrayc
}

void Application::on_connection_save_backup_progress()
{
  if(!m_dialog_progess_save_backup)
    m_dialog_progess_save_backup = Utils::get_and_show_pulse_dialog(_("Exporting Backup"), this);

  m_dialog_progess_save_backup->pulse();
}

void Application::on_connection_convert_backup_progress()
{
  if(!m_dialog_progess_convert_backup)
    m_dialog_progess_convert_backup = Utils::get_and_show_pulse_dialog(_("Restoring Backup"), this);

  m_dialog_progess_convert_backup->pulse();
}

void Application::on_document_close()
{
#ifndef GLOM_ENABLE_CLIENT_ONLY
  //TODO: It would be better to do this in a Application::on_document_closed() virtual method,
  //but that would need an ABI break in Bakery:
  ConnectionPool* connection_pool = ConnectionPool::get_instance();
  if(!connection_pool)
    return;

  connection_pool->cleanup( sigc::mem_fun(*this, &Application::on_connection_close_progress) );
#endif // !GLOM_ENABLE_CLIENT_ONLY
}

/*
void Application::statusbar_set_text(const Glib::ustring& strText)
{
  m_pStatus->set_text(strText);
}

void Application::statusbar_clear()
{
  statusbar_set_text("");
}
*/


void Application::update_network_shared_ui()
{
  Document* document = dynamic_cast<Document*>(get_document());
  if(!document)
    return;

  //This is not used (yet) on Maemo:
  if(!m_connection_toggleaction_network_shared)
    return;

  //Show the status in the UI:
  //(get_network_shared() already enforces constraints).
  const bool shared = document->get_network_shared();
  //TODO: Our use of block() does not seem to work. The signal actually seems to be emitted some time later instead.
  m_connection_toggleaction_network_shared.block(); //Prevent signal handling.
  m_toggleaction_network_shared->set_active(shared);

  //Do not allow impossible changes:
  const Document::HostingMode hosting_mode = document->get_hosting_mode();
  if( (hosting_mode == Document::HOSTING_MODE_POSTGRES_CENTRAL) //Central hosting means that it must be shared on the network.
    || (hosting_mode == Document::HOSTING_MODE_SQLITE) ) //sqlite does not allow network sharing.
  {
    m_toggleaction_network_shared->set_sensitive(false);
  }

  m_connection_toggleaction_network_shared.unblock();
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void Application::on_userlevel_changed(AppState::userlevels /* userlevel */)
{
  update_userlevel_ui();
}

void Application::update_table_sensitive_ui()
{
  bool has_table = false;

  if(m_pFrame)
    has_table = !m_pFrame->get_shown_table_name().empty();

  for(type_listActions::iterator iter = m_listTableSensitiveActions.begin(); iter != m_listTableSensitiveActions.end(); ++iter)
  {
    Glib::RefPtr<Gtk::Action> action = *iter;
    action->set_sensitive(has_table);
  }
}

void Application::update_userlevel_ui()
{
  AppState::userlevels userlevel = get_userlevel();

  //Disable/Enable developer actions:
  for(type_listActions::iterator iter = m_listDeveloperActions.begin(); iter != m_listDeveloperActions.end(); ++iter)
  {
    Glib::RefPtr<Gtk::Action> action = *iter;
     action->set_sensitive( userlevel == AppState::USERLEVEL_DEVELOPER );
  }

  //Ensure table sensitive menus stay disabled if necessary.
  update_table_sensitive_ui();

  // Hide users entry from developer menu for connections that don't
  // support users
  if(userlevel == AppState::USERLEVEL_DEVELOPER)
  {
    sharedptr<SharedConnection> connection = ConnectionPool::get_and_connect();
    if(connection && !connection->get_gda_connection()->supports_feature(Gnome::Gda::CONNECTION_FEATURE_USERS))
      m_action_developer_users->set_sensitive(false);
  }

  //Make sure that the correct radio menu item is activated (the userlevel might have been set programmatically):
  //We only need to set/unset one, because the others are in the same radio group.
  if(userlevel ==  AppState::USERLEVEL_DEVELOPER)
  {
    if(!m_action_menu_userlevel_developer->get_active())
      m_action_menu_userlevel_developer->set_active();
  }
  else if(userlevel ==  AppState::USERLEVEL_OPERATOR)
  {
    if(!m_action_menu_userlevel_operator->get_active())
      m_action_menu_userlevel_operator->set_active();
    // Remove the drag layout toolbar
  }
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

#ifndef GLOM_ENABLE_MAEMO
Glib::RefPtr<Gtk::UIManager> Application::get_ui_manager()
{
  return m_refUIManager;
}
#else
Hildon::AppMenu* Application::get_maemo_appmenu()
{
  return &m_maemo_appmenu;
}
#endif //GLOM_ENABLE_MAEMO


bool Application::offer_new_or_existing()
{
  //Offer to load an existing document, or start a new one.
  const Glib::ustring glade_path = Utils::get_glade_file_path("glom.glade");

  Dialog_ExistingOrNew* dialog_raw = 0;
  Utils::get_glade_widget_derived_with_warning(dialog_raw);
  std::auto_ptr<Dialog_ExistingOrNew> dialog(dialog_raw);
  dialog->set_transient_for(*this);
/*
  dialog->signal_new().connect(sigc::mem_fun(*this, &Application::on_existing_or_new_new));
  dialog->signal_open_from_uri().connect(sigc::mem_fun(*this, &Application::on_existing_or_new_open_from_uri));
  dialog->signal_open_from_remote().connect(sigc::mem_fun(*this, &Application::on_existing_or_new_open_from_remote));
*/
  bool ask_again = true;
  while(ask_again)
  {
    const int response_id = Utils::dialog_run_with_help(dialog_raw);
    dialog->hide();

    if(response_id == Gtk::RESPONSE_ACCEPT)
    {
      switch(dialog->get_action())
      {
#ifndef GLOM_ENABLE_CLIENT_ONLY
      case Dialog_ExistingOrNew::NEW_EMPTY:
        existing_or_new_new();
        break;
      case Dialog_ExistingOrNew::NEW_FROM_TEMPLATE:
#endif // !GLOM_ENABLE_CLIENT_ONLY
      case Dialog_ExistingOrNew::OPEN_URI:
        open_document(dialog->get_uri());
        break;
#ifndef G_OS_WIN32
      case Dialog_ExistingOrNew::OPEN_REMOTE:
        open_browsed_document(dialog->get_service_info(), dialog->get_service_name());
        break;
#endif
      case Dialog_ExistingOrNew::NONE:
      default:
	std::cerr << G_STRFUNC << ": Unhandled action: " << dialog->get_action() << std::endl;
        g_assert_not_reached();
        break;
      }

      //Check that a document was opened:
      Document* document = dynamic_cast<Document*>(get_document());
      if(!document)
      {
        std::cerr << G_STRFUNC << ": document was NULL." << std::endl;
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
void Application::existing_or_new_new()
{
  // New empty document

  //Each document must have a location, so ask the user for one.
  //This will use an extended save dialog that also asks for the database title and some hosting details:
  Glib::ustring db_title;

  m_ui_save_extra_showextras = true; //Offer self-hosting or central hosting, and offer the database title.
  m_ui_save_extra_newdb_hosting_mode = Document::HOSTING_MODE_DEFAULT; /* Default to self-hosting */
  m_ui_save_extra_newdb_title.clear();
  offer_saveas();

  //Check that the document was given a location:
  Document* document = dynamic_cast<Document*>(get_document());
  if(!document->get_file_uri().empty())
  {
    //Get details from the extended save dialog:
    const Glib::ustring db_title = m_ui_save_extra_newdb_title;
    Document::HostingMode hosting_mode = m_ui_save_extra_newdb_hosting_mode;
    m_ui_save_extra_newdb_title.clear();
    m_ui_save_extra_newdb_hosting_mode = Document::HOSTING_MODE_DEFAULT;

    //Make sure that the user can do something with his new document:
    document->set_userlevel(AppState::USERLEVEL_DEVELOPER);
    // Try various ports if connecting to an existing database server instead
    // of self-hosting one:
    document->set_connection_try_other_ports(m_ui_save_extra_newdb_hosting_mode == Document::HOSTING_MODE_DEFAULT);

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
   ConnectionPool* connection_pool = ConnectionPool::get_instance();
   if(connection_pool)
     connection_pool->set_get_document_func( sigc::mem_fun(*this, &Application::on_connection_pool_get_document) );

    const bool connected = m_pFrame->connection_request_password_and_choose_new_database_name();
    if(!connected)
    {
      // Unset URI so that the offer_new_or_existing does not disappear
      // so the user can make a different choice about what document to open.
      // TODO: Show some error message?
      document->set_file_uri("");
    }
    else
    {
      const bool db_created = m_pFrame->create_database(document->get_connection_database(), db_title);
      if(db_created)
      {
        const Glib::ustring database_name_used = document->get_connection_database();
        ConnectionPool::get_instance()->set_database(database_name_used);
        document->set_database_title(db_title);
        m_pFrame->set_databases_selected(database_name_used);

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

void Application::set_mode_data()
{
  if (!m_pFrame)
    return;

  if (m_pFrame->m_Mode == Frame_Glom::MODE_Find)
    m_action_mode_find->activate();
}

void Application::set_mode_find()
{
  if (!m_pFrame)
    return;

  if (m_pFrame->m_Mode == Frame_Glom::MODE_Data)
    m_action_mode_find->activate();
}

#ifndef GLOM_ENABLE_MAEMO
void Application::init_menus_help()
{
  //Call base class:
  App_WithDoc_Gtk::init_menus_help();
  m_refHelpActionGroup->add( Gtk::Action::create("BakeryAction_Help_Contents",
                        _("_Contents"), _("Help with the application")),
                        sigc::mem_fun(*this, &Application::on_menu_help_contents) );

  //Build part of the menu structure, to be merged in by using the "PH" plaeholders:
  static const Glib::ustring ui_description =
    "<ui>"
    "  <menubar name='Bakery_MainMenu'>"
    "    <placeholder name='Bakery_MenuPH_Help'>"
    "      <menu action='BakeryAction_Menu_Help'>"
    "        <menuitem action='BakeryAction_Help_Contents' />"
    "      </menu>"
    "    </placeholder>"
    "  </menubar>"
    "</ui>";

  //Add menu:
  add_ui_from_string(ui_description);
}

void Application::on_menu_help_contents()
{
  Glom::Utils::show_help();
}
#endif //GLOM_ENABLE_MAEMO

#ifndef GLOM_ENABLE_CLIENT_ONLY

void Application::on_recreate_database_progress()
{
  //Show the user that something is happening, because the INSERTS might take time.
  //TOOD: This doesn't actually show up until near the end, even with Gtk::Main::instance()->iteration().
  if(!m_dialog_progress_creating)
  {
    Utils::get_glade_widget_derived_with_warning(m_dialog_progress_creating);

    m_dialog_progress_creating->set_message(_("Creating Glom Database"), _("Creating Glom database from example file."));

    m_dialog_progress_creating->set_transient_for(*this);
    m_dialog_progress_creating->show();

  }

  //Ensure that the dialog is shown, instead of waiting for the application to be idle:
  while(Gtk::Main::instance()->events_pending())
    Gtk::Main::instance()->iteration();

  m_dialog_progress_creating->pulse();
}

bool Application::recreate_database_from_example(bool& user_cancelled)
{
  //Create a database, based on the information in the current document:
  Document* pDocument = static_cast<Document*>(get_document());
  if(!pDocument)
    return false;

  ConnectionPool* connection_pool = ConnectionPool::get_instance();
  if(!connection_pool)
    return false; //Impossible anyway.

  //Check whether the database exists already.
  const Glib::ustring db_name = pDocument->get_connection_database();
  if(db_name.empty())
    return false;

  connection_pool->set_database(db_name);
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
#else
  std::auto_ptr<std::exception> error;
#endif // GLIBMM_EXCEPTIONS_ENABLED
  {
    connection_pool->set_ready_to_connect(); //This has succeeded already.
#ifdef GLIBMM_EXCEPTIONS_ENABLED
    sharedptr<SharedConnection> sharedconnection = connection_pool->connect();
#else
    sharedptr<SharedConnection> sharedconnection = connection_pool->connect(error);
    if(!error.get())
    {
#endif // GLIBMM_EXCEPTIONS_ENABLED
      g_warning("Application::recreate_database_from_example(): Failed because database exists already.");

      return false; //Connection to the database succeeded, because no exception was thrown. so the database exists already.
#ifndef GLIBMM_EXCEPTIONS_ENABLED
    }
#endif // !GLIBMM_EXCEPTIONS_ENABLED
  }
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  catch(const ExceptionConnection& ex)
  {
#else
  if(error.get())
  {
    const ExceptionConnection* exptr = dynamic_cast<ExceptionConnection*>(error.get());
    if(exptr)
    {
      const ExceptionConnection& ex = *exptr;
#endif // GLIBMM_EXCEPTIONS_ENABLED
      if(ex.get_failure_type() == ExceptionConnection::FAILURE_NO_SERVER)
      {
        user_cancelled = true; //Eventually, the user will cancel after retrying.
        g_warning("Application::recreate_database_from_example(): Failed because connection to server failed, without specifying a database.");
        return false;
      }
#ifndef GLIBMM_EXCEPTIONS_ENABLED
    }
#endif // !GLIBMM_EXCEPTIONS_ENABLED

    //Otherwise continue, because we _expected_ connect() to fail if the db does not exist yet.
  }

  //Show the user that something is happening, because the INSERTS might take time.
  //TOOD: This doesn't actually show up until near the end, even with Gtk::Main::instance()->iteration().
  Dialog_ProgressCreating* dialog_progress_temp = 0;
  Utils::get_glade_widget_derived_with_warning(dialog_progress_temp);
  dialog_progress_temp->set_message(_("Creating Glom Database"), _("Creating Glom database from example file."));
  std::auto_ptr<Dialog_ProgressCreating> dialog_progress(dialog_progress_temp); //Put the dialog in an auto_ptr so that it will be deleted (and hidden) when the current function returns.

  dialog_progress->set_transient_for(*this);
  dialog_progress->show();

  //Ensure that the dialog is shown, instead of waiting for the application to be idle:
  while(Gtk::Main::instance()->events_pending())
    Gtk::Main::instance()->iteration();

  dialog_progress->pulse();

  //Create the database: (This will show a connection dialog)
  connection_pool->set_database( Glib::ustring() );
  const bool db_created = m_pFrame->create_database(db_name, pDocument->get_database_title());

  if(!db_created)
  {
    return false;
  }
  else
    connection_pool->set_database(db_name); //Specify the new database when connecting from now on.

  dialog_progress->pulse();
  BusyCursor busy_cursor(this);

  sharedptr<SharedConnection> sharedconnection;
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
#endif // GLIBMM_EXCEPTIONS_ENABLED
  {
#ifdef GLIBMM_EXCEPTIONS_ENABLED
    sharedconnection = connection_pool->connect();
#else
    sharedconnection = connection_pool->connect(error);
    if(!error.get())
#endif // GLIBMM_EXCEPTIONS_ENABLED
      connection_pool->set_database(db_name); //The database was successfully created, so specify it when connecting from now on.
  }
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  catch(const ExceptionConnection& ex)
  {
#else
  if(error.get())
  {
    const std::exception& ex = *error.get();
#endif // GLIBMM_EXCEPTIONS_ENABLED
    g_warning("Application::recreate_database_from_example(): Failed to connect to the newly-created database.");
    return false;
  }

  dialog_progress->pulse();

  //Create the developer group, and make this user a member of it:
  dialog_progress->pulse();
  bool test = DbUtils::add_standard_groups(pDocument);
  if(!test)
    return false;

  //Add any extra groups from the example file:
  dialog_progress->pulse();
  test = DbUtils::add_groups_from_document(pDocument);
  if(!test)
    return false;

  //Create each table:
  Document::type_listTableInfo tables = pDocument->get_tables();
  for(Document::type_listTableInfo::const_iterator iter = tables.begin(); iter != tables.end(); ++iter)
  {
    sharedptr<const TableInfo> table_info = *iter;

    //Create SQL to describe all fields in this table:
    Glib::ustring sql_fields;
    Document::type_vec_fields fields = pDocument->get_table_fields(table_info->get_name());

    dialog_progress->pulse();
    const bool table_creation_succeeded = DbUtils::create_table(table_info, fields);
    dialog_progress->pulse();
    if(!table_creation_succeeded)
    {
      g_warning("Application::recreate_database_from_example(): CREATE TABLE failed with the newly-created database.");
      return false;
    }
  }

  dialog_progress->pulse();
  DbUtils::add_standard_tables(pDocument); //Add internal, hidden, tables.

  //Set table priviliges, using the groups we just added:
  dialog_progress->pulse();
  test = DbUtils::set_table_privileges_groups_from_document(pDocument);
  if(!test)
    return false;

  for(Document::type_listTableInfo::const_iterator iter = tables.begin(); iter != tables.end(); ++iter)
  {
    sharedptr<const TableInfo> table_info = *iter;

    //Add any example data to the table:
    dialog_progress->pulse();

    //try
    //{
      const bool table_insert_succeeded = DbUtils::insert_example_data(pDocument, table_info->get_name());

      if(!table_insert_succeeded)
      {
        g_warning("Application::recreate_database_from_example(): INSERT of example data failed with the newly-created database.");
        return false;
      }
    //}
    //catch(const std::exception& ex)
    //{
    //  std::cerr << "Application::recreate_database_from_example(): exception: " << ex.what() << std::endl;
      //HandleError(ex);
    //}

  } //for(tables)

  return true; //All tables created successfully.
}

bool Application::recreate_database_from_backup(const Glib::ustring& backup_uri, bool& user_cancelled)
{
  //Create a database, based on the information in the current document:
  Document* pDocument = static_cast<Document*>(get_document());
  if(!pDocument)
    return false;

  ConnectionPool* connection_pool = ConnectionPool::get_instance();
  if(!connection_pool)
    return false; //Impossible anyway.

  //Check whether the database exists already.
  const Glib::ustring db_name = pDocument->get_connection_database();
  if(db_name.empty())
    return false;

  connection_pool->set_database(db_name);
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
#else
  std::auto_ptr<std::exception> error;
#endif // GLIBMM_EXCEPTIONS_ENABLED
  {
    connection_pool->set_ready_to_connect(); //This has succeeded already.
#ifdef GLIBMM_EXCEPTIONS_ENABLED
    sharedptr<SharedConnection> sharedconnection = connection_pool->connect();
#else
    sharedptr<SharedConnection> sharedconnection = connection_pool->connect(error);
    if(!error.get())
    {
#endif // GLIBMM_EXCEPTIONS_ENABLED
      g_warning("Application::recreate_database_from_example(): Failed because database exists already.");

      return false; //Connection to the database succeeded, because no exception was thrown. so the database exists already.
#ifndef GLIBMM_EXCEPTIONS_ENABLED
    }
#endif // !GLIBMM_EXCEPTIONS_ENABLED
  }
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  catch(const ExceptionConnection& ex)
  {
#else
  if(error.get())
  {
    const ExceptionConnection* exptr = dynamic_cast<ExceptionConnection*>(error.get());
    if(exptr)
    {
      const ExceptionConnection& ex = *exptr;
#endif // GLIBMM_EXCEPTIONS_ENABLED
      if(ex.get_failure_type() == ExceptionConnection::FAILURE_NO_SERVER)
      {
        user_cancelled = true; //Eventually, the user will cancel after retrying.
        g_warning("Application::recreate_database_from_example(): Failed because connection to server failed, without specifying a database.");
        return false;
      }
#ifndef GLIBMM_EXCEPTIONS_ENABLED
    }
#endif // !GLIBMM_EXCEPTIONS_ENABLED

    //Otherwise continue, because we _expected_ connect() to fail if the db does not exist yet.
  }

  //Show the user that something is happening, because the INSERTS might take time.
  //TOOD: This doesn't actually show up until near the end, even with Gtk::Main::instance()->iteration().
  Dialog_ProgressCreating* dialog_progress_temp = 0;
  Utils::get_glade_widget_derived_with_warning(dialog_progress_temp);
  dialog_progress_temp->set_message(_("Creating Glom Database"), _("Creating Glom database from backup file."));
  std::auto_ptr<Dialog_ProgressCreating> dialog_progress(dialog_progress_temp); //Put the dialog in an auto_ptr so that it will be deleted (and hidden) when the current function returns.

  dialog_progress->set_transient_for(*this);
  dialog_progress->show();

  //Ensure that the dialog is shown, instead of waiting for the application to be idle:
  while(Gtk::Main::instance()->events_pending())
    Gtk::Main::instance()->iteration();

  dialog_progress->pulse();

  //Create the database: (This will show a connection dialog)
  connection_pool->set_database( Glib::ustring() );
  try
  {
    ConnectionPool::get_instance()->create_database(db_name);
  }
  catch(const Glib::Exception& ex) // libgda does not set error domain
  {
    std::cerr << G_STRFUNC << ": Gnome::Gda::Connection::create_database(" << db_name << ") failed: " << ex.what() << std::endl;

    //Tell the user:
    Gtk::Dialog* dialog = 0;
    Utils::get_glade_widget_with_warning("glom_developer.glade", "dialog_error_create_database", dialog);
    dialog->set_transient_for(*this);
    Glom::Utils::dialog_run_with_help(dialog, "dialog_error_create_database");
    delete dialog;

    return false;
  }

  connection_pool->set_database(db_name); //Specify the new database when connecting from now on.

  //Create the developer group, and make this user a member of it:
  dialog_progress->pulse();
  bool test = DbUtils::add_standard_groups(pDocument);
  if(!test)
    return false;

  //Add any extra groups from the example file:
  dialog_progress->pulse();
  test = DbUtils::add_groups_from_document(pDocument);
  if(!test)
    return false;

  //dialog_progress->pulse();
  //m_pFrame->add_standard_tables(); //Add internal, hidden, tables.

  //Restore the backup into the database:
  std::string original_dir_path;

  Glib::RefPtr<Gio::File> gio_file = Gio::File::create_for_uri(backup_uri);
  if(gio_file)
  {
    Glib::RefPtr<Gio::File> parent = gio_file->get_parent();
    if(parent)
    {
      try
      {
        original_dir_path = Glib::filename_from_uri(parent->get_uri());
      }
      catch(const Glib::Error& ex)
      {
        std::cerr << G_STRFUNC << ": Glib::filename_from_uri() failed: " << ex.what() << std::endl;
      }
    }
  }

  if(original_dir_path.empty())
  {
    std::cerr << G_STRFUNC << ": original_dir_path is empty." << std::endl;
    return false;
  }

  //Restore the database from the backup:
  //std::cout << "DEBUG: original_dir_path=" << original_dir_path << std::endl;
  const bool restored = connection_pool->convert_backup(
    sigc::mem_fun(*this, &Application::on_connection_convert_backup_progress), original_dir_path);

  delete m_dialog_progess_convert_backup;
  m_dialog_progess_convert_backup = 0;

  if(!restored)
  {
    std::cerr << G_STRFUNC << ": Restore failed." << std::endl;
    return false;
  }

  return true; //Restore successfully.
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

AppState::userlevels Application::get_userlevel() const
{
  const Document* document = dynamic_cast<const Document*>(get_document());
  if(document)
  {
    return document->get_userlevel();
  }
  else
    g_assert_not_reached();
    //return AppState::USERLEVEL_DEVELOPER; //This should never happen.
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void Application::add_developer_action(const Glib::RefPtr<Gtk::Action>& refAction)
{
  //Prevent it from being added twice:
  remove_developer_action(refAction);

  m_listDeveloperActions.push_back(refAction);
}

void Application::remove_developer_action(const Glib::RefPtr<Gtk::Action>& refAction)
{
  for(type_listActions::iterator iter = m_listDeveloperActions.begin(); iter != m_listDeveloperActions.end(); ++iter)
  {
    if(*iter == refAction)
    {
      m_listDeveloperActions.erase(iter);
      break;
    }
  }
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

#ifdef GLOM_ENABLE_MAEMO
void Application::fill_menu_tables()
{
  m_appmenu_button_table.fill_from_database();
}
#else
void Application::fill_menu_tables()
{
  //TODO: There must be a better way than building a ui_string like this:

  m_listNavTableActions.clear();
  if(m_menu_tables_ui_merge_id)
    m_refUIManager->remove_ui(m_menu_tables_ui_merge_id);

  if(m_refNavTablesActionGroup)
  {
    m_refUIManager->remove_action_group(m_refNavTablesActionGroup);
    m_refNavTablesActionGroup.reset();
  }

  m_refNavTablesActionGroup = Gtk::ActionGroup::create("NavTablesActions");

  Glib::ustring ui_description =
    "<ui>"
#ifdef GLOM_ENABLE_MAEMO
    "  <popup name='Bakery_MainMenu'>"
#else
    "  <menubar name='Bakery_MainMenu'>"
#endif
    "    <placeholder name='Bakery_MenuPH_Others'>"
    "      <menu action='Glom_Menu_Tables'>"
    "        <placeholder name='Menu_Tables_Dynamic'>";

  Document* document = dynamic_cast<Document*>(get_document());
  const Document::type_listTableInfo tables = document->get_tables();
  for(Document::type_listTableInfo::const_iterator iter = tables.begin(); iter != tables.end(); ++iter)
  {
    sharedptr<const TableInfo> table_info = *iter;
    if(!table_info->m_hidden)
    {
      const Glib::ustring action_name = "NavTableAction_" + table_info->get_name();

      ui_description += "<menuitem action='" + action_name + "' />";

      Glib::RefPtr<Gtk::Action> refAction = Gtk::Action::create(action_name, Utils::string_escape_underscores(table_info->get_title_or_name()));
      m_refNavTablesActionGroup->add(refAction,
        sigc::bind( sigc::mem_fun(*m_pFrame, &Frame_Glom::on_box_tables_selected), table_info->get_name()) );

      m_listNavTableActions.push_back(refAction);

      //m_refUIManager->add_ui(merge_id, path, table_info->m_title, refAction, UI_MANAGER_MENUITEM);
    }
  }

  m_refUIManager->insert_action_group(m_refNavTablesActionGroup);


  ui_description +=
    "     </placeholder>"
    "    </menu>"
    "    </placeholder>"
#ifdef GLOM_ENABLE_MAEMO
    "  </popup>"
#else
    "  </menubar>"
#endif
    "</ui>";

  //Add menus:
  try
  {
    m_menu_tables_ui_merge_id = m_refUIManager->add_ui_from_string(ui_description);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << ": building menus failed: " <<  ex.what() << std::endl;
    std::cerr << "   The ui_description was: " <<  ui_description << std::endl;
  }
}
#endif //GLOM_ENABLE_MAEMO

#ifdef GLOM_ENABLE_MAEMO
void Application::fill_menu_reports(const Glib::ustring& /* table_name */)
{
  //TODO: Change the Hildon::AppMenu.
}
#else
void Application::fill_menu_reports(const Glib::ustring& table_name)
{
  //TODO: There must be a better way than building a ui_string like this:

  m_listNavReportActions.clear();
  if(m_menu_reports_ui_merge_id)
    m_refUIManager->remove_ui(m_menu_reports_ui_merge_id);

  if(m_refNavReportsActionGroup)
  {
    m_refUIManager->remove_action_group(m_refNavReportsActionGroup);
    m_refNavReportsActionGroup.reset();
  }

  m_refNavReportsActionGroup = Gtk::ActionGroup::create("NavReportsActions");

  Glib::ustring ui_description =
    "<ui>"
#ifdef GLOM_ENABLE_MAEMO
    "  <popup name='Bakery_MainMenu'>"
#else
    "  <menubar name='Bakery_MainMenu'>"
#endif
    "    <placeholder name='Bakery_MenuPH_Others'>"
    "     <menu action='Glom_Menu_Reports'>"
    "        <placeholder name='Menu_Reports_Dynamic'>";

  Document* document = dynamic_cast<Document*>(get_document());
  const Document::type_listReports tables = document->get_report_names(table_name);
  for(Document::type_listReports::const_iterator iter = tables.begin(); iter != tables.end(); ++iter)
  {
    sharedptr<Report> report = document->get_report(table_name, *iter);
    if(report)
    {
      const Glib::ustring report_name = report->get_name();
      if(!report_name.empty())
      {
        const Glib::ustring action_name = "NavReportAction_" + report_name;

        ui_description += "<menuitem action='" + action_name + "' />";

        Glib::RefPtr<Gtk::Action> refAction = Gtk::Action::create( action_name, Utils::string_escape_underscores(report->get_title_or_name()));
        m_refNavReportsActionGroup->add(refAction,
          sigc::bind( sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_report_selected), report->get_name()) );

        m_listNavReportActions.push_back(refAction);

        //m_refUIManager->add_ui(merge_id, path, table_info->m_title, refAction, UI_MANAGER_MENUITEM);
      }
    }
  }

  m_refUIManager->insert_action_group(m_refNavReportsActionGroup);


  ui_description +=
    "     </placeholder>"
    "    </menu>"
    "    </placeholder>"
#ifdef GLOM_ENABLE_MAEMO
    "  </popup>"
#else
    "  </menubar>"
#endif
    "</ui>";

  //Add menus:
  try
  {
    m_menu_reports_ui_merge_id = m_refUIManager->add_ui_from_string(ui_description);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << ": building menus failed: " <<  ex.what();
  }
}
#endif //GLOM_ENABLE_MAEMO

#ifdef GLOM_ENABLE_MAEMO
void Application::fill_menu_print_layouts(const Glib::ustring& /* table_name */)
{
  //TODO: Change the Hildon::AppMenu.
}
#else
void Application::fill_menu_print_layouts(const Glib::ustring& table_name)
{
  //TODO: This is copy/pasted from fill_menu_print_reports. Can we generalize it?

  //TODO: There must be a better way than building a ui_string like this:

  m_listNavPrintLayoutActions.clear();
  if(m_menu_print_layouts_ui_merge_id)
    m_refUIManager->remove_ui(m_menu_print_layouts_ui_merge_id);

  //Only fill menu if we are in details mode,
  //because this feature is not (yet) available for lists:
  if(!m_pFrame || !m_pFrame->get_viewing_details())
    return;

  if(m_refNavPrintLayoutsActionGroup)
  {
    m_refUIManager->remove_action_group(m_refNavPrintLayoutsActionGroup);
    m_refNavPrintLayoutsActionGroup.reset();
  }

  m_refNavPrintLayoutsActionGroup = Gtk::ActionGroup::create("NavPrintLayoutsActions");

  Glib::ustring ui_description =
    "<ui>"
#ifdef GLOM_ENABLE_MAEMO
    "  <popup name='Bakery_MainMenu'>"
#else
    "  <menubar name='Bakery_MainMenu'>"
#endif
    "    <placeholder name='Bakery_MenuPH_File'>"
    "      <menu action='BakeryAction_Menu_File'>"
    "        <menu action='GlomAction_Menu_File_Print'>"
    "          <placeholder name='Menu_PrintLayouts_Dynamic'>";

  Document* document = dynamic_cast<Document*>(get_document());
  const Document::type_listPrintLayouts tables = document->get_print_layout_names(table_name);

  // TODO_clientonly: Should this be available in client only mode? We need to
  // depend on goocanvas in client only mode then:
#ifndef GLOM_ENABLE_CLIENT_ONLY
  for(Document::type_listPrintLayouts::const_iterator iter = tables.begin(); iter != tables.end(); ++iter)
  {
    sharedptr<PrintLayout> print_layout = document->get_print_layout(table_name, *iter);
    if(print_layout)
    {
      const Glib::ustring name = print_layout->get_name();
      if(!name.empty())
      {
        const Glib::ustring action_name = "NavPrintLayoutAction_" + name;

        ui_description += "<menuitem action='" + action_name + "' />";

        Glib::RefPtr<Gtk::Action> refAction = Gtk::Action::create( action_name, Utils::string_escape_underscores(print_layout->get_title_or_name()));
        m_refNavPrintLayoutsActionGroup->add(refAction,
          sigc::bind( sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_print_layout_selected), print_layout->get_name()) );

        m_listNavPrintLayoutActions.push_back(refAction);

        //m_refUIManager->add_ui(merge_id, path, table_info->m_title, refAction, UI_MANAGER_MENUITEM);
      }
    }
  }
#endif

  m_refUIManager->insert_action_group(m_refNavPrintLayoutsActionGroup);


  ui_description +=
    "       </placeholder>"
    "      </menu>"
    "    </menu>"
    "    </placeholder>"
#ifdef GLOM_ENABLE_MAEMO
    "  </popup>"
#else
    "  </menubar>"
#endif
    "</ui>";

  //Add menus:
  try
  {
    m_menu_print_layouts_ui_merge_id = m_refUIManager->add_ui_from_string(ui_description);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << ": building menus failed: " <<  ex.what();
  }
}
#endif //GLOM_ENABLE_MAEMO

#ifndef GLOM_ENABLE_CLIENT_ONLY
void Application::on_menu_file_save_as_example()
{
  //Based on the implementation of GlomBakery::App_WithDoc::on_menu_file_saveas()

  //Display File Save dialog and respond to choice:

  //Bring document window to front, to make it clear which document is being saved:
  //This doesn't work: TODO.
  ui_bring_to_front();

  //Show the save dialog:
  Document* document = dynamic_cast<Document*>(get_document());
  const Glib::ustring& file_uriOld = document->get_file_uri();

  m_ui_save_extra_showextras = false;
  m_ui_save_extra_title.clear();
  m_ui_save_extra_message.clear();
  m_ui_save_extra_newdb_title.clear();
  m_ui_save_extra_newdb_hosting_mode = Document::HOSTING_MODE_DEFAULT;

  Glib::ustring file_uri = ui_file_select_save(file_uriOld); //Also asks for overwrite confirmation.
  if(!file_uri.empty())
  {
    //Enforce the file extension:
    file_uri = document->get_file_uri_with_extension(file_uri);

    bool bUseThisFileUri = true;
    //We previously checked whether the file existed,
    //but The FileChooser checks that already,
    //so Bakery doesn't bother checking anymore,
    //and our old test always set bUseThisFileUri to true anyway. murryac.
    //TODO: So remove this bool. murrayc.

    //Save to this filepath:
    if(bUseThisFileUri)
    {
      //Prevent saving while we modify the document:
      document->set_allow_autosave(false);

      document->set_file_uri(file_uri, true); //true = enforce file extension
      document->set_is_example_file();

      //Save all data from all tables into the document:
      Document::type_listTableInfo list_table_info = document->get_tables();
      for(Document::type_listTableInfo::const_iterator iter = list_table_info.begin(); iter != list_table_info.end(); ++iter)
      {
        const Glib::ustring table_name = (*iter)->get_name();

        //const type_vec_fields vec_fields = document->get_table_fields(table_name);

        //export_data_to_stream() needs a type_list_layout_groups;
        Document::type_list_layout_groups sequence = document->get_data_layout_groups_default("list", table_name, "" /* layout_platform */);

        //std::cout << "debug: table_name=" << table_name << std::endl;

        Document::type_example_rows example_rows;
        FoundSet found_set;
        found_set.m_table_name = table_name;
        m_pFrame->export_data_to_vector(example_rows, found_set, sequence);
        //std::cout << "  debug after row_text=" << row_text << std::endl;

        document->set_table_example_data(table_name, example_rows);
      }

      const bool bTest = document->save();
      document->set_is_example_file(false);
      document->set_is_backup_file(false);
      document->set_file_uri(file_uriOld);
      document->set_allow_autosave(true);

      if(!bTest)
      {
        ui_warning(_("Save failed."), _("There was an error while saving the example file."));
      }
      else
      {
        //Disable Save and SaveAs menu items:
        after_successful_save();
      }

      update_window_title();


      //Close if this save was a result of a File|Close or File|Exit:.
      //if(bTest && m_bCloseAfterSave) //Don't close if the save failed.
      //{
      //  on_menu_file_close(); //This could be the second time, but now there are no unsaved changes.
      //}
    }
    else
    {
      //Let the user choose a different file path,
      //because he decided not to overwrite the 1st one.
      on_menu_file_save_as_example(); //recursive.
    }
  }
  else
  {
    cancel_close_or_exit();
  }
}

Glib::ustring Application::ui_file_select_save(const Glib::ustring& old_file_uri) //override
{
  //Reimplement this whole function, just so we can use our custom FileChooserDialog class:
  App& app = *this;

  std::auto_ptr<Gtk::FileChooserDialog> fileChooser_Save;
  Glom::FileChooserDialog_SaveExtras* fileChooser_SaveExtras = 0;

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

  // TODO_maemo: This should probably use Hildon FileChooser API
  fileChooser_Save->set_do_overwrite_confirmation(); //Ask the user if the file already exists.

  Gtk::Window* pWindow = dynamic_cast<Gtk::Window*>(&app);
  if(pWindow)
    fileChooser_Save->set_transient_for(*pWindow);

  fileChooser_Save->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  fileChooser_Save->add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);

  fileChooser_Save->set_default_response(Gtk::RESPONSE_OK);

  //This is the reason that we override this method:
  if(!m_ui_save_extra_title.empty())
    fileChooser_Save->set_title(m_ui_save_extra_title);

  if(fileChooser_SaveExtras)
  {
    fileChooser_SaveExtras->set_extra_message(m_ui_save_extra_message);


    //Start with something suitable:
    Document* document = dynamic_cast<Document*>(get_document());
    g_assert(document);
    const Glib::ustring filename = document->get_name(); //Get the filename without the path and extension.

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
    Glib::RefPtr<Gio::File> gio_file = Gio::File::create_for_uri(old_file_uri);
    if(gio_file)
    {
      Glib::RefPtr<Gio::File> parent = gio_file->get_parent();
      if(parent)
      {
        const Glib::ustring uri_parent = parent->get_uri();
        fileChooser_Save->set_uri(uri_parent);
      }
    }
  }


  //bool tried_once_already = false;

  bool try_again = true;
  while(try_again)
  {
    try_again = false;

    const int response_id = fileChooser_Save->run();
    fileChooser_Save->hide();
    if((response_id != Gtk::RESPONSE_CANCEL) && (response_id != Gtk::RESPONSE_DELETE_EVENT))
    {
      const Glib::ustring uri_chosen = fileChooser_Save->get_uri();

      //Change the URI, to put the file (and its data folder) in a folder:
      const Glib::ustring uri = get_file_uri_without_extension(uri_chosen);

      //Check whether the file exists, and that we have rights to it:
      Glib::RefPtr<Gio::File> file = Gio::File::create_for_uri(uri);
      if(!file)
        return Glib::ustring(); //Failure.


      //If the file exists (the FileChooser offers a "replace?" dialog, so this is not possible.):
      if(App_WithDoc::file_exists(uri))
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
      if(m_ui_save_extra_newdb_hosting_mode == Document::HOSTING_MODE_POSTGRES_SELF)
        is_self_hosted = true;
#endif //GLOM_ENABLE_POSTGRESQL

#ifdef GLOM_ENABLE_SQLITE
      if(m_ui_save_extra_newdb_hosting_mode == Document::HOSTING_MODE_SQLITE)
        is_self_hosted = true;
#endif // GLOM_ENABLE_SQLITE

      // Create a directory for self-hosted databases (sqlite or self-hosted
      // postgresql).
      if(!try_again && fileChooser_SaveExtras && is_self_hosted)
      {
        //Check that the directory does not exist already.
        //The GtkFileChooser could not check for that because it could not know that we would create a directory based on the filename:
        //Note that uri has no extension at this point:
        Glib::RefPtr<Gio::File> dir = Gio::File::create_for_uri(uri);
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
        Glib::RefPtr<Gio::File> file_with_ext = Gio::File::create_for_uri(uri_chosen);
        const Glib::ustring filename_part = file_with_ext->get_basename();

        //Add the filename part to the newly-created directory:
        Glib::RefPtr<Gio::File> file_whole = dir->get_child(filename_part);
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

void Application::stop_self_hosting_of_document_database()
{
  Document* pDocument = static_cast<Document*>(get_document());
  if(pDocument)
  {
    ConnectionPool* connection_pool = ConnectionPool::get_instance();
    if(!connection_pool)
      return;

    connection_pool->cleanup( sigc::mem_fun(*this, &Application::on_connection_close_progress ));
  }
}

void Application::on_menu_developer_changelanguage()
{
  Dialog_ChangeLanguage* dialog = 0;
  Utils::get_glade_widget_derived_with_warning(dialog);
  if(!dialog) //Unlikely and it already warns on stderr.
    return;
    
  dialog->set_transient_for(*this);
  const int response = Glom::Utils::dialog_run_with_help(dialog);
  dialog->hide();

  if(response == Gtk::RESPONSE_OK)
  {
    TranslatableItem::set_current_locale(dialog->get_locale());

    //Get the translations from the document (in Operator mode, we only load the necessary translations.)
    //This also updates the UI, so we show all the translated titles:
    int failure_code = 0;
    get_document()->load(failure_code);

    m_pFrame->show_table_refresh(); //load() doesn't seem to refresh the view.
  }

  delete dialog;
}

void Application::on_menu_developer_translations()
{
  if(!m_window_translations)
  {
    Utils::get_glade_widget_derived_with_warning(m_window_translations);
    if(m_window_translations)
    {
      m_pFrame->add_view(m_window_translations);
      m_window_translations->set_transient_for(*this);
      m_window_translations->set_document(static_cast<Document*>(m_pDocument));
      m_window_translations->load_from_document();
      m_window_translations->show();

      m_window_translations->signal_hide().connect(sigc::mem_fun(*this, &Application::on_window_translations_hide));
    }
  }
  else
  {
    m_window_translations->show();
    m_window_translations->load_from_document();
  }
}

void Application::on_menu_developer_active_platform_normal()
{
  Document* document = dynamic_cast<Document*>(get_document());
  if(document)
   document->set_active_layout_platform("");

  m_pFrame->show_table_refresh();
}

void Application::on_menu_developer_active_platform_maemo()
{
  Document* document = dynamic_cast<Document*>(get_document());
  if(document)
   document->set_active_layout_platform("maemo");

  m_pFrame->show_table_refresh();
}

void Application::on_menu_developer_export_backup()
{
  Document* document = dynamic_cast<Document*>(get_document());
  if(!document)
    return;

  // Ask the user to choose a new directory name.
  // Start with a name based on the existing name.
  const Glib::ustring fileuri_old = document->get_file_uri();
  const Glib::RefPtr<const Gio::File> file_old =
    Gio::File::create_for_uri( get_file_uri_without_extension(fileuri_old) );
  const std::string old_basename = file_old->get_basename();
  Glib::TimeVal timeval;
  timeval.assign_current_time();
  std::string starting_name = old_basename + "-backup-" + timeval.as_iso8601();
  //Replace : because that confuses (makes it fail) tar (and file-roller) when opening the file,
  //and --force-local is not relevant to opening files.
  starting_name = Utils::string_replace(starting_name, ":", "-");

  // This actually creates the directory:
  Gtk::FileChooserDialog dialog(*this, _("Save Backup"), Gtk::FILE_CHOOSER_ACTION_CREATE_FOLDER);
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_ACCEPT);
  dialog.set_local_only(); //Because pg_dump, pg_restore and tar can't use URIs.
  dialog.set_current_name(starting_name);
  const int result = dialog.run();
  dialog.hide();
  if(result != Gtk::RESPONSE_ACCEPT)
    return;

  const std::string& path_dir = dialog.get_filename();
  if(path_dir.empty())
    return;

  //Save a copy of the document there:
  //Save the .glom document with the same name as the directory:
  const std::string basename = Glib::path_get_basename(path_dir);
  const std::string& filepath_document = Glib::build_filename(path_dir, basename + ".glom");
  document->set_allow_autosave(false); //Prevent saving while we modify the document:
  document->set_file_uri(Glib::filename_to_uri(filepath_document), true); //true = enforce file extension;
  document->set_is_backup_file(true);
  bool saved = document->save();

  document->set_file_uri(fileuri_old);
  document->set_is_backup_file(false);
  document->set_allow_autosave(true);

  if(saved)
  {
    ConnectionPool* connection_pool = ConnectionPool::get_instance();
    saved = connection_pool->save_backup(sigc::mem_fun(*this, &Application::on_connection_save_backup_progress), path_dir);

    delete m_dialog_progess_save_backup;
    m_dialog_progess_save_backup = 0;
  }

  //Compress the backup in a .tar.gz, so it is slightly more safe from changes:
  const std::string path_tar = Glib::find_program_in_path("tar");
  if(path_tar.empty())
  {
    std::cerr << G_STRFUNC << ": The tar executable could not be found." << std::endl;
    saved = false;
  }
  else
  {
    Glib::RefPtr<const Gio::File> gio_file = Gio::File::create_for_path(path_dir);
    const std::string basename = gio_file->get_basename();
    Glib::RefPtr<const Gio::File> gio_file_parent = gio_file->get_parent();
    const std::string parent_dir = gio_file_parent->get_path();
    if(parent_dir.empty() || basename.empty())
    {
      std::cerr << G_STRFUNC << "parent_dir or basename are empty." << std::endl;
      saved = false;
    }
    else
    {
      //TODO: Find some way to do this without using the command-line,
      //which feels fragile:
      const std::string command_tar = "\"" + path_tar + "\"" +
        " --force-local --no-wildcards" + //Avoid side-effects of special characters.
        " --remove-files" +
        " -czf"
        " \"" + path_dir + ".tar.gz\"" +
        " --directory \"" + parent_dir + "\"" + //This must be right before the mention of the file name:
        " \"" + basename + "\"";

      //std::cout << "DEBUG: command_tar=" << command_tar << std::endl;

      saved = Glom::Spawn::execute_command_line_and_wait(command_tar,
        sigc::mem_fun(*this, &Application::on_connection_save_backup_progress));

      if(saved)
      {
        std::cerr << G_STRFUNC << "tar failed with command:" << command_tar << std::endl;
      }

      delete m_dialog_progess_save_backup;
      m_dialog_progess_save_backup = 0;
    }
  }

  if(!saved)
    ui_warning(_("Export Backup failed."), _("There was an error while exporting the backup."));
}

void Application::on_menu_developer_restore_backup()
{
  Gtk::FileChooserDialog file_dlg(_("Choose a backup file"), Gtk::FILE_CHOOSER_ACTION_OPEN);
  file_dlg.set_transient_for(*this);
  file_dlg.set_local_only(); //Because we can't untar remote files.

  Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
  filter->set_name(_(".tar.gz Backup files"));
  filter->add_pattern("*.tar.gz");
  filter->add_pattern("*.tgz");
  file_dlg.add_filter(filter);

  file_dlg.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  file_dlg.add_button(_("Restore"), Gtk::RESPONSE_OK);

  const int result = file_dlg.run();
  file_dlg.hide();
  if(result != Gtk::RESPONSE_OK)
    return;

  const std::string uri_tarball = file_dlg.get_uri();
  if(uri_tarball.empty())
    return;

  do_restore_backup(uri_tarball);
}

bool Application::do_restore_backup(const Glib::ustring& backup_uri)
{
  // We cannot use an uri here, because we cannot untar remote files.
  const std::string filename_tarball = Glib::filename_from_uri(backup_uri);

  const std::string path_tar = Glib::find_program_in_path("tar");
  if(path_tar.empty())
  {
    std::cerr << G_STRFUNC << ": The tar executable could not be found." << std::endl;
    return false;
  }

  //Create a temporary directory into which we will untar the tarball:
  std::string path_tmp = Glib::build_filename(
    Glib::get_tmp_dir(), Glib::path_get_basename(filename_tarball));
  path_tmp += "_extracted";

  //Make sure that the directory does not exist already:
  const Glib::ustring uri_tmp = Glib::filename_to_uri(path_tmp);
  if(!Utils::delete_directory(uri_tmp))
  {
    std::cerr << G_STRFUNC << "Error from Utils::delete_directory() while trying to remove directory: " << uri_tmp << std::endl;
    return false;
  }

  //Create the tmp directory:
  const int mkdir_succeeded = g_mkdir_with_parents(path_tmp.c_str(), 0770);
  if(mkdir_succeeded == -1)
  {
    std::cerr << G_STRFUNC << "Error from g_mkdir_with_parents() while trying to create directory: " << path_tmp << std::endl;
    perror("Error from g_mkdir_with_parents");

    return false;
  }

  //Untar into the tmp directory:
  //TODO: Find some way to do this without using the command-line,
  //which feels fragile:
  const std::string command_tar = "\"" + path_tar + "\"" +
    " --force-local --no-wildcards" + //Avoid side-effects of special characters.
    " -xzf"
    " \"" + filename_tarball + "\"" +
    " --directory \"" + path_tmp + "\"";

  //std::cout << "DEBUG: command_tar=" << command_tar << std::endl;

  const bool untarred = Glom::Spawn::execute_command_line_and_wait(command_tar,
    sigc::mem_fun(*this, &Application::on_connection_convert_backup_progress));
  if(!untarred)
  {
    std::cerr << G_STRFUNC << ": tar failed with command:" << command_tar << std::endl;
  }

  delete m_dialog_progess_convert_backup;
  m_dialog_progess_convert_backup = 0;

  if(!untarred)
    ui_warning(_("Restore Backup failed."), _("There was an error while restoring the backup. The tar utility failed to extract the archive."));

  //Open the .glom file that is in the tmp directory:
  const Glib::ustring untarred_uri = Utils::get_directory_child_with_suffix(uri_tmp, ".glom", true /* recurse */);
  if(untarred_uri.empty())
  {
    ui_warning(_("Restore Backup failed."), _("There was an error while restoring the backup. The .glom file could not be found."));
    return false;
  }

  //std::cout << "DEBUG: untarred_uri=" << untarred_uri << std::endl;
  open_document(untarred_uri);

  //Delete the temporary untarred directory:
  //Actually, we just leave this here, where the system will clean it up anyway,
  //because open_document() starts a new process,
  //so we don't know when we can safely delete the files.
  //Utils::delete_directory(uri_tmp);

  return true;
}

void Application::on_menu_developer_show_layout_toolbar()
{
  m_pFrame->show_layout_toolbar(m_action_show_layout_toolbar->get_active());
}


void Application::on_window_translations_hide()
{
  if(m_window_translations)
  {
    m_pFrame->on_developer_dialog_hide();
  }
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

Application* Application::get_application()
{
  return global_application;
}

void Application::document_history_add(const Glib::ustring& file_uri)
{
  // We override this so we can prevent example files from being saved in the recently-used list:

  bool prevent = false;
  if(!file_uri.empty())
  {
    prevent = (file_uri == m_example_uri);
    if(prevent)
      return;
  }

  GlomBakery::App_WithDoc_Gtk::document_history_add(file_uri);
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void Application::do_menu_developer_fields(Gtk::Window& parent, const Glib::ustring table_name)
{
  m_pFrame->do_menu_developer_fields(parent, table_name);
}

void Application::do_menu_developer_relationships(Gtk::Window& parent, const Glib::ustring table_name)
{
  m_pFrame->do_menu_developer_relationships(parent, table_name);
}

Document* Application::on_connection_pool_get_document()
{
  return dynamic_cast<Document*>(get_document());
}
#endif //GLOM_ENABLE_CLIENT_ONLY

//overridden to show the current table name in the window's title:
void Application::update_window_title()
{
  //Set application's main window title:

  Document* document = dynamic_cast<Document*>(get_document());
  if(!document)
    return;

  if(!m_pFrame)
    return;

  //Show the table title:
  const Glib::ustring table_name = m_pFrame->get_shown_table_name();
  Glib::ustring table_label = document->get_table_title(table_name);
  if(!table_label.empty())
  {
    #ifndef GLOM_ENABLE_CLIENT_ONLY
    if(document->get_userlevel() == AppState::USERLEVEL_DEVELOPER)
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

  #ifdef GLOM_ENABLE_MAEMO
  //Update the picker button too:
  m_appmenu_button_table.set_table_name(table_name);
  #endif //GLOM_ENABLE_MAEMO
}

void Application::show_table_details(const Glib::ustring& table_name, const Gnome::Gda::Value& primary_key_value)
{
  if(!m_pFrame)
    return;

  m_pFrame->show_table(table_name, primary_key_value);
}

void Application::show_table_list(const Glib::ustring& table_name)
{
  if(!m_pFrame)
    return;

  m_pFrame->show_table(table_name);
}


void Application::print_report(const Glib::ustring& report_name)
{
  if(!m_pFrame)
    return;

  m_pFrame->on_menu_report_selected(report_name);
}

void Application::print_layout()
{
  if(!m_pFrame)
    return;

  m_pFrame->on_menu_file_print();
}

void Application::start_new_record()
{
  m_pFrame->on_menu_add_record();

}


} //namespace Glom
