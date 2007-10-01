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

#include "application.h"

#include <glom/libglom/dialog_progress_creating.h>

#ifndef GLOM_ENABLE_CLIENT_ONLY
#include <glom/translation/dialog_change_language.h>
#include <glom/translation/window_translations.h>
#endif // !GLOM_ENABLE_CLIENT_ONLY

#include <glom/utility_widgets/filechooserdialog.h>
#include <glom/libglom/utils.h>

#include "config.h" //For VERSION.

#include <cstdio>
#include <memory> //For std::auto_ptr<>
#include <libgnomevfsmm.h>
#include <sstream> //For stringstream.
#include <glibmm/i18n.h>

#ifdef GLOM_ENABLE_MAEMO
#include <hildon/hildon-window.h>
#endif // GLOM_ENABLE_MAEMO

#ifdef GLOM_ENABLE_MAEMO
namespace {
	HildonWindow* turn_gtk_window_into_hildon_window(GtkWindow* cobject)
	{
		GtkWidget* child = cobject->bin.child;
		g_assert(child);

		g_object_ref(G_OBJECT(child));
		gtk_container_remove(GTK_CONTAINER(cobject), child);

		GtkWidget* window = hildon_window_new();
		gtk_container_add(GTK_CONTAINER(window), child);
		g_object_unref(G_OBJECT(child));

		gtk_widget_destroy(GTK_WIDGET(cobject));
		return HILDON_WINDOW(window);
	}
}
#endif // GLOM_ENABLE_MAEMO

namespace Glom
{

// Global application variable
App_Glom* global_application = NULL;

App_Glom::App_Glom(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
  // Note cobject is actually a GtkWindow, not a HildonWindow, because the
  // glade file specified the type as GtkWindow.
#ifdef GLOM_ENABLE_MAEMO
: ParentWindow(turn_gtk_window_into_hildon_window(GTK_WINDOW(cobject))), //It's a virtual base class, so we have to call the specific constructor to prevent the default constructor from being called.
#else
: ParentWindow(cobject), //It's a virtual base class, so we have to call the specific constructor to prevent the default constructor from being called.
#endif
  type_base(cobject, "Glom"),
  m_pBoxTop(0),
  m_pFrame(0),
#ifndef GLOM_ENABLE_CLIENT_ONLY
  m_window_translations(0),
#endif // !GLOM_ENABLE_CLIENT_ONLY
  m_menu_tables_ui_merge_id(0),
  m_menu_reports_ui_merge_id(0),
  m_ui_save_extra_showextras(false),
#ifndef GLOM_ENABLE_CLIENT_ONLY
  m_ui_save_extra_newdb_selfhosted(false),
#endif // !GLOM_ENABLE_CLIENT_ONLY
  m_show_sql_debug(false)
{
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
#else
  std::auto_ptr<Glib::Error> error;
#endif
  {
    //Show the icon in the window manager's window title bar and in the list of running applications:
#ifdef GLIBMM_EXCEPTIONS_ENABLED
    set_icon_from_file(GLOM_ICON_DIR "/glom.png");
#else
    set_icon_from_file(GLOM_ICON_DIR "/glom.png", error);
#endif
  }
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  catch(const Glib::Error& ex)
  {
#else
  if(error.get() != NULL)
  {
    const Glib::Error& ex = *error.get();
#endif
    std::cerr << "App_Glom::App_Glom(): Could not set icon: " << ex.what() << std::endl;
  } 

  //Load widgets from glade file:
  refGlade->get_widget("bakery_vbox", m_pBoxTop);
  refGlade->get_widget_derived("vbox_frame", m_pFrame); //This one is derived. There's a lot happening here.

  add_mime_type("application/x-glom"); //TODO: make this actually work - we need to register it properly.

  //Hide the toolbar because it doesn't contain anything useful for this app.
  m_HandleBox_Toolbar.hide();
  
  global_application = this;
}

App_Glom::~App_Glom()
{
#ifndef GLOM_ENABLE_CLIENT_ONLY
  if(m_window_translations)
  {
    m_pFrame->remove_view(m_window_translations);
    delete m_window_translations;
  }
#endif // !GLOM_ENABLE_CLIENT_ONLY
}

bool App_Glom::init(const Glib::ustring& document_uri)
{
  type_vecStrings vecAuthors;
  vecAuthors.push_back("Murray Cumming <murrayc@murrayc.com>");
  set_about_information(VERSION, vecAuthors, _("(C) 2000-2005 Murray Cumming"), _("A Database GUI"));

  type_base::init(); //calls init_menus() and init_toolbars()

  //m_pFrame->set_shadow_type(Gtk::SHADOW_IN);

  //Hide the toolbar because it doesn't contain anything useful for this app.
  //m_HandleBox_Toolbar.hide();

  if(document_uri.empty())
  {
    Document_Glom* pDocument = static_cast<Document_Glom*>(get_document());
    if(pDocument && pDocument->get_connection_database().empty()) //If it is a new (default) document.
    {
      return offer_new_or_existing();
    }
  }
  else
  {
    const bool test = open_document(document_uri);
    if(!test)
      return offer_new_or_existing();
  }

  return true;
  //show_all();
}

bool App_Glom::get_show_sql_debug() const
{
  return m_show_sql_debug;
}

void App_Glom::set_show_sql_debug(bool val)
{
  m_show_sql_debug = val;
}

void App_Glom::init_layout()
{
  //We override this method so that we can put everything in the vbox from the glade file, instead of the vbox from App_Gtk.

  //Add menu bar at the top:
  //These were defined in init_uimanager().
#ifdef GLOM_ENABLE_MAEMO
  Gtk::Menu* pMenu = static_cast<Gtk::Menu*>(m_refUIManager->get_widget("/Bakery_MainMenu"));
  set_menu(*pMenu);
#else
  Gtk::MenuBar* pMenuBar = static_cast<Gtk::MenuBar*>(m_refUIManager->get_widget("/Bakery_MainMenu"));
  m_pBoxTop->pack_start(*pMenuBar, Gtk::PACK_SHRINK);
#endif

  //Do not create the toolbar because it doesn't contain anything useful for this app.
  //Gtk::Toolbar* pToolBar = static_cast<Gtk::Toolbar*>(m_refUIManager->get_widget("/Bakery_ToolBar"));
  //m_HandleBox_Toolbar.add(*pToolBar);
  //m_HandleBox_Toolbar.show();

  add_accel_group(m_refUIManager->get_accel_group());

  //m_pBoxTop->pack_start(m_HandleBox_Toolbar, Gtk::PACK_SHRINK);

  //Add placeholder, to be used by add():
  //m_pBoxTop->pack_start(m_VBox_PlaceHolder);
  //m_VBox_PlaceHolder.show();
}

void App_Glom::init_toolbars()
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

void App_Glom::init_menus_file()
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

  Glib::RefPtr<Gtk::Action> action = Gtk::Action::create("BakeryAction_File_SaveAsExample", _("Save As Example"));
  m_listDeveloperActions.push_back(action);

#ifndef GLOM_ENABLE_CLIENT_ONLY
  m_refFileActionGroup->add(action,
                        sigc::mem_fun((App_Glom&)*this, &App_Glom::on_menu_file_save_as_example));
#endif // !GLOM_ENABLE_CLIENT_ONLY

  m_refFileActionGroup->add(Gtk::Action::create("BakeryAction_Menu_File_Export", _("_Export")),
                        sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_file_export));

  m_refFileActionGroup->add(Gtk::Action::create("GlomAction_File_Print", Gtk::Stock::PRINT),
                        sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_file_print) );

  m_refFileActionGroup->add(Gtk::Action::create("BakeryAction_File_Close", Gtk::Stock::CLOSE),
                        sigc::mem_fun((App_WithDoc&)*this, &App_WithDoc::on_menu_file_close));
  /*m_refFileActionGroup->add(Gtk::Action::create("BakeryAction_File_Exit", Gtk::Stock::QUIT),
                        sigc::mem_fun((App&)*this, &App::on_menu_file_exit));*/

  m_refUIManager->insert_action_group(m_refFileActionGroup);

  //Build part of the menu structure, to be merged in by using the "PH" placeholders:
  static const Glib::ustring ui_description =
    "<ui>"
#ifdef GLOM_ENABLE_MAEMO
    "  <popup name='Bakery_MainMenu'>"
#else
    "  <menubar name='Bakery_MainMenu'>"
#endif
    "    <placeholder name='Bakery_MenuPH_File'>"
    "      <menu action='BakeryAction_Menu_File'>"
    "        <menuitem action='BakeryAction_File_New' />"
    "        <menuitem action='BakeryAction_File_Open' />"
    "        <menu action='BakeryAction_Menu_File_RecentFiles'>"
    "        </menu>"
#ifndef GLOM_ENABLE_CLIENT_ONLY
    "        <menuitem action='BakeryAction_File_SaveAsExample' />"
#endif // !GLOM_ENABLE_CLIENT_ONLY
    "        <menuitem action='BakeryAction_Menu_File_Export' />"
    "        <separator/>"
    "        <menuitem action='GlomAction_File_Print' />"
    "        <separator/>"
    "        <menuitem action='BakeryAction_File_Close' />"
    "      </menu>"
    "    </placeholder>"
#ifdef GLOM_ENABLE_MAEMO
    "  </popup>"
#else
    "  </menubar>"
#endif
    "</ui>";

  //Add menu:
  add_ui_from_string(ui_description);

  //Add recent-files submenu:
  init_menus_file_recentfiles("/Bakery_MainMenu/Bakery_MenuPH_File/BakeryAction_Menu_File/BakeryAction_Menu_File_RecentFiles");
}

void App_Glom::init_menus()
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
                        sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_Tables_EditReports) );
  m_listDeveloperActions.push_back(action);
#endif

#ifndef GLOM_ENABLE_CLIENT_ONLY
  //"UserLevel" menu:
  m_refActionGroup_Others->add(Gtk::Action::create("Glom_Menu_userlevel", _("_User Level")));
  Gtk::RadioAction::Group group_userlevel;

  m_action_menu_userlevel_developer = Gtk::RadioAction::create(group_userlevel, "GlomAction_Menu_userlevel_Developer", _("_Developer"));
  m_refActionGroup_Others->add(m_action_menu_userlevel_developer,
                        sigc::mem_fun(*this, &App_Glom::on_menu_userlevel_developer) );

  m_action_menu_userlevel_operator =  Gtk::RadioAction::create(group_userlevel, "GlomAction_Menu_userlevel_Operator", _("_Operator"));
  m_refActionGroup_Others->add(m_action_menu_userlevel_operator,
                          sigc::mem_fun(*this, &App_Glom::on_menu_userlevel_operator) );
#endif // !GLOM_ENABLE_CLIENT_ONLY

  //"Mode" menu:
  action =  Gtk::Action::create("Glom_Menu_Mode", _("_Mode"));
  m_refActionGroup_Others->add(action);
  Gtk::RadioAction::Group group_mode;

  //We remember this action, so that it can be explicitly activated later.
  m_action_mode_data = Gtk::RadioAction::create(group_mode, "GlomAction_Menu_Mode_Data", _("D_ata"));
  m_refActionGroup_Others->add(m_action_mode_data,
                        sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_Mode_Data) );

  m_action_mode_find = Gtk::RadioAction::create(group_mode, "GlomAction_Menu_Mode_Find", _("_Find"));
  m_refActionGroup_Others->add(m_action_mode_find,  Gtk::AccelKey("<control>F"),
                        sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_Mode_Find) );

#ifndef GLOM_ENABLE_CLIENT_ONLY
  action = Gtk::Action::create("Glom_Menu_Developer", _("_Developer"));
  m_listDeveloperActions.push_back(action);
  m_refActionGroup_Others->add(action);

  action = Gtk::Action::create("GlomAction_Menu_Developer_Database_Preferences", _("_Database Preferences"));
  m_listDeveloperActions.push_back(action);  
  m_refActionGroup_Others->add(action, sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_developer_database_preferences) );


  action = Gtk::Action::create("GlomAction_Menu_Developer_Fields", _("_Fields"));
  m_listDeveloperActions.push_back(action);  
  m_refActionGroup_Others->add(action, sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_developer_fields) );

  action = Gtk::Action::create("GlomAction_Menu_Developer_RelationshipsOverview", _("_Relationships Overview"));
  m_listDeveloperActions.push_back(action);
  m_refActionGroup_Others->add(action, sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_developer_relationships_overview) );

  action = Gtk::Action::create("GlomAction_Menu_Developer_Relationships", _("_Relationships for this Table"));
  m_listDeveloperActions.push_back(action);
  m_refActionGroup_Others->add(action, sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_developer_relationships) );

  action = Gtk::Action::create("GlomAction_Menu_Developer_Users", _("_Users"));
  m_listDeveloperActions.push_back(action);
  m_refActionGroup_Others->add(action, sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_developer_users));

  action = Gtk::Action::create("GlomAction_Menu_Developer_Reports", _("R_eports"));
  m_listDeveloperActions.push_back(action);
  m_refActionGroup_Others->add(action, sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_developer_reports));

  action = Gtk::Action::create("GlomAction_Menu_Developer_Script_Library", _("Script _Library"));
  m_listDeveloperActions.push_back(action);
  m_refActionGroup_Others->add(action, sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_developer_script_library));


  action = Gtk::Action::create("GlomAction_Menu_Developer_Layout", _("_Layout"));
  m_listDeveloperActions.push_back(action);
  m_refActionGroup_Others->add(action, sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_developer_layout));

  action = Gtk::Action::create("GlomAction_Menu_Developer_ChangeLanguage", _("_Test Translation"));
  m_listDeveloperActions.push_back(action);
  m_refActionGroup_Others->add(action, sigc::mem_fun(*this, &App_Glom::on_menu_developer_changelanguage));

  action = Gtk::Action::create("GlomAction_Menu_Developer_Translations", _("_Translations"));
  m_listDeveloperActions.push_back(action);
  m_refActionGroup_Others->add(action, sigc::mem_fun(*this, &App_Glom::on_menu_developer_translations));
#endif // !GLOM_ENABLE_CLIENT_ONLY

  m_refUIManager->insert_action_group(m_refActionGroup_Others);

  //Build part of the menu structure, to be merged in by using the "Bakery_MenuPH_Others" placeholder:
  static const Glib::ustring ui_description =
    "<ui>"
#ifdef GLOM_ENABLE_MAEMO
    "  <popup name='Bakery_MainMenu'>"
#else
    "  <menubar name='Bakery_MainMenu'>"
#endif
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
    "      <menu action='Glom_Menu_Mode'>"
    "        <menuitem action='GlomAction_Menu_Mode_Data' />"
    "        <menuitem action='GlomAction_Menu_Mode_Find' />"
    "      </menu>"
#ifndef GLOM_ENABLE_CLIENT_ONLY
    "      <menu action='Glom_Menu_userlevel'>"
    "        <menuitem action='GlomAction_Menu_userlevel_Developer' />"
    "        <menuitem action='GlomAction_Menu_userlevel_Operator' />"
    "      </menu>"
    "      <menu action='Glom_Menu_Developer'>"
    "        <menuitem action='GlomAction_Menu_Developer_Database_Preferences' />"
    "        <menuitem action='GlomAction_Menu_Developer_Fields' />"
    "        <menuitem action='GlomAction_Menu_Developer_Relationships' />"
    "        <menuitem action='GlomAction_Menu_Developer_RelationshipsOverview' />"
    "        <menuitem action='GlomAction_Menu_Developer_Layout' />"
    "        <menuitem action='GlomAction_Menu_Developer_Users' />"
    "        <menuitem action='GlomAction_Menu_Developer_Reports' />"
    "        <menuitem action='GlomAction_Menu_Developer_Script_Library' />"
    "        <separator />"
    "        <menuitem action='GlomAction_Menu_Developer_Translations' />"
    "        <menuitem action='GlomAction_Menu_Developer_ChangeLanguage' />"
    "      </menu>"
#endif // !GLOM_ENABLE_CLIENT_ONLY
    "    </placeholder>"
#ifdef GLOM_ENABLE_MAEMO
    "  </popup>"
#else
    "  </menubar>"
#endif
    "</ui>";

/*  "        <menuitem action='GlomAction_Menu_Developer_RelationshipsOverview' />" */

  //Add menu:
  add_ui_from_string(ui_description);

  init_menus_help();

  fill_menu_tables();
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void App_Glom::on_menu_userlevel_developer()
{
  if(m_pFrame)
    m_pFrame->on_menu_userlevel_Developer(m_action_menu_userlevel_developer, m_action_menu_userlevel_operator);
}

void App_Glom::on_menu_userlevel_operator()
{
  if(m_pFrame)
    m_pFrame->on_menu_userlevel_Operator(m_action_menu_userlevel_operator);
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

void App_Glom::on_menu_file_close() //override
{
  // Call the base class implementation:
  Bakery::App_WithDoc_Gtk::on_menu_file_close();
}

static bool uri_is_writable(const Glib::RefPtr<const Gnome::Vfs::Uri>& uri)
{
  if(!uri)
    return false;

#ifdef GLIBMM_EXCEPTIONS_ENABLED
  // TODO: What if this throws?
  Glib::RefPtr<const Gnome::Vfs::FileInfo> file_info = uri->get_file_info(Gnome::Vfs::FILE_INFO_GET_ACCESS_RIGHTS);
#else
  std::auto_ptr<Gnome::Vfs::exception> error;
  Glib::RefPtr<const Gnome::Vfs::FileInfo> file_info = uri->get_file_info(Gnome::Vfs::FILE_INFO_GET_ACCESS_RIGHTS, error);
  if(error.get() != NULL) return false;
#endif
  if(file_info)
  {
    const Gnome::Vfs::FilePermissions permissions = file_info->get_permissions();
    return ((permissions & Gnome::Vfs::PERM_ACCESS_WRITABLE) == Gnome::Vfs::PERM_ACCESS_WRITABLE);
  }
  else
    return true; //Not every URI protocol supports FILE_INFO_GET_ACCESS_RIGHTS, so assume that it's writable and complain later.
}


Glib::ustring App_Glom::get_file_uri_without_extension(const Glib::ustring& uri)
{
  Glib::RefPtr<Gnome::Vfs::Uri> vfs_uri = Gnome::Vfs::Uri::create(uri);
  if(!vfs_uri)
    return uri; //Actually an error.

  const Glib::ustring filename_part = vfs_uri->extract_short_name();
  
  const Glib::ustring::size_type pos_dot = filename_part.rfind(".");
  if(pos_dot == Glib::ustring::npos)
    return uri; //There was no extension, so just return the existing URI.
  else
  {
    const Glib::ustring filename_part_without_ext = filename_part.substr(0, pos_dot);
    const Glib::ustring uri_parent = vfs_uri->extract_dirname();

    Glib::RefPtr<Gnome::Vfs::Uri> vfs_uri_parent = Gnome::Vfs::Uri::create(uri_parent);
    Glib::RefPtr<Gnome::Vfs::Uri> vfs_uri_without_extension = vfs_uri_parent->append_string(filename_part_without_ext);

    return vfs_uri_without_extension->to_string();
  }
}

Glib::ustring App_Glom::ui_file_select_save(const Glib::ustring& old_file_uri) //override
{
  //Reimplement this whole function, just so we can use our custom FileChooserDialog class:
  App& app = *this;

  std::auto_ptr<Gtk::FileChooserDialog> fileChooser_Save;
  Glom::FileChooserDialog* fileChooser_SaveExtras = 0;

  //Create the appropriate dialog, depending on how the caller set m_ui_save_extra_showextras:
  if(m_ui_save_extra_showextras)
  {
    fileChooser_SaveExtras = new Glom::FileChooserDialog(gettext("Save Document"), Gtk::FILE_CHOOSER_ACTION_SAVE);
    fileChooser_Save.reset(fileChooser_SaveExtras);
  }
  else
  {
    fileChooser_Save.reset(new Gtk::FileChooserDialog(gettext("Save Document"), Gtk::FILE_CHOOSER_ACTION_SAVE));
  }

#ifndef GLOM_ENABLE_MAEMO
  // The maemo version is able to run with gtkmm 2.6
  // TODO_maemo: This should probably use Hildon FileChooser API
  fileChooser_Save->set_do_overwrite_confirmation(); //Ask the user if the file already exists.
#endif

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
    Document_Glom* document = dynamic_cast<Document_Glom*>(get_document());
    g_assert(document);
    const Glib::ustring filename = document->get_name(); //Get the filename without the path and extension.

    //Avoid ".". TODO: Find out why it happens:
    if(filename == ".")
      m_ui_save_extra_newdb_title = Glib::ustring();
    else
      m_ui_save_extra_newdb_title = Utils::title_from_string( filename ); //Start with something suitable.

    fileChooser_SaveExtras->set_extra_newdb_title(m_ui_save_extra_newdb_title); 

#ifndef GLOM_ENABLE_CLIENT_ONLY
    fileChooser_SaveExtras->set_extra_newdb_self_hosted(m_ui_save_extra_newdb_selfhosted);
#endif // !GLOM_ENABLE_CLIENT_ONLY
  }


  //Make the save dialog show the existing filename, if any:
  if(!old_file_uri.empty())
  {
    //Just start with the parent folder,
    //instead of the whole name, to avoid overwriting:
    Glib::RefPtr<Gnome::Vfs::Uri> vfs_uri = Gnome::Vfs::Uri::create(old_file_uri);
    if(vfs_uri)
    {
      Glib::ustring uri_parent = vfs_uri->extract_dirname();
      fileChooser_Save->set_uri(uri_parent);
    }
  }


  //bool tried_once_already = false;

  bool try_again = true;
  while(try_again)
  {
    try_again = false;

    //Work around bug #330680 "GtkFileChooserDialog is too small when shown a second time.":
    //(Commented-out because the workaround doesn't work)
    /*
    if(tried_once_already)
    {
      fileChooser_Save->set_default_size(-1, 600); 
    }
    else
      tried_once_already = true;
    */

    const int response_id = fileChooser_Save->run();
    fileChooser_Save->hide();
    if(response_id != Gtk::RESPONSE_CANCEL)
    {
      const Glib::ustring uri_chosen = fileChooser_Save->get_uri();

      //Change the URI, to put the file (and its data folder) in a folder:
      const Glib::ustring uri = get_file_uri_without_extension(uri_chosen);

      //Check whether the file exists, and that we have rights to it:
      Glib::RefPtr<Gnome::Vfs::Uri> vfs_uri = Gnome::Vfs::Uri::create(uri);
      if(!vfs_uri)
        return Glib::ustring(); //Failure.


      //If the file exists (the FileChooser offers a "replace?" dialog, so this is not possible.):
      if(App_WithDoc::file_exists(uri))
      {
        //Check whether we have rights to the file to change it:
        //Really, GtkFileChooser should do this for us.
        if(!uri_is_writable(vfs_uri))
        {
           //Warn the user:
           ui_warning(gettext("Read-only File."), gettext("You may not overwrite the existing file, because you do not have sufficient access rights."));
           try_again = true; //Try again.
           continue;
        }
      }

      //Check whether we have rights to the directory, to create a new file in it:
      //Really, GtkFileChooser should do this for us.
      Glib::RefPtr<const Gnome::Vfs::Uri> vfs_uri_parent = vfs_uri->get_parent();
      if(vfs_uri_parent)
      {
        if(!uri_is_writable(vfs_uri_parent))
        {
          //Warn the user:
           ui_warning(gettext("Read-only Directory."), gettext("You may not create a file in this directory, because you do not have sufficient access rights."));
           try_again = true; //Try again.
           continue;
        }
      }

      if(!try_again && fileChooser_SaveExtras)
      {
        //Get the extra details from the extended save dialog:
        m_ui_save_extra_newdb_title = fileChooser_SaveExtras->get_extra_newdb_title();

#ifndef GLOM_ENABLE_CLIENT_ONLY
        m_ui_save_extra_newdb_selfhosted = fileChooser_SaveExtras->get_extra_newdb_self_hosted();
#endif // !GLOM_ENABLE_CLIENT_ONLY

        if(m_ui_save_extra_newdb_title.empty())
        {
          Frame_Glom::show_ok_dialog(_("Database Title missing"), _("You must specify a title for the new database."), *this, Gtk::MESSAGE_ERROR);

          try_again = true;
          continue;
        }
      }
 
#ifndef GLOM_ENABLE_CLIENT_ONLY
      if(!try_again && fileChooser_SaveExtras && m_ui_save_extra_newdb_selfhosted)
      {
        //Check that the directory does not exist already.
        //The GtkFileChooser could not check for that because it could not know that we would create a directory based on the filename:
        //Note that uri has no extension at this point:
        Glib::RefPtr<Gnome::Vfs::Uri> vfsuri = Gnome::Vfs::Uri::create(uri);
        if(vfsuri->uri_exists())
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
          Gnome::Vfs::Handle::make_directory(uri, 0770 /* leading zero means octal */);
        }
        catch(const Gnome::Vfs::exception&  ex)
        {
          std::cerr << "Error during make_directory(): " << ex.what() << std::endl;
        }

        //Add the filename (Note that the caller will add the extension if necessary, so we don't do it here.)
        Glib::RefPtr<Gnome::Vfs::Uri> uri_with_ext = Gnome::Vfs::Uri::create(uri_chosen);
        const Glib::ustring filename_part = uri_with_ext->extract_short_name();

        //Add the filename part to the newly-created directory:
        Glib::RefPtr<Gnome::Vfs::Uri> uri_whole = vfs_uri->append_string(filename_part);
        return uri_whole->to_string();
      }
#endif // !GLOM_ENABLE_CLIENT_ONLY

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

#ifndef GLOM_ENABLE_CLIENT_ONLY
void App_Glom::stop_self_hosting_of_document_database()
{
  Document_Glom* pDocument = static_cast<Document_Glom*>(get_document());
  if(pDocument && pDocument->get_connection_is_self_hosted())
  {
    ConnectionPool* connection_pool = ConnectionPool::get_instance();
    if(!connection_pool)
      return;

    connection_pool->stop_self_hosting();
  }
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

Bakery::App* App_Glom::new_instance() //Override
{
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "window_main");
#else
  std::auto_ptr<Gnome::Glade::XmlError> error;
  Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "window_main", "", error);
  if(error.get()) return NULL;
#endif
  App_Glom* pApp_Glom = 0;
  refXml->get_widget_derived("window_main", pApp_Glom);

  return pApp_Glom;
}

void App_Glom::init_create_document()
{
  if(!m_pDocument)
  {
    Document_Glom* document_glom = new Document_Glom();
    m_pDocument = document_glom;
    document_glom->set_parent_window(this); //So that it can show a BusyCursor when loading and saving.

    //Tell document about view:
    m_pDocument->set_view(m_pFrame);

    //Tell view about document:
    //(This calls set_document() in the child views too.)
    m_pFrame->set_document(static_cast<Document_Glom*>(m_pDocument));
  }

  type_base::init_create_document(); //Sets window title. Doesn't recreate doc.
}

bool App_Glom::on_document_load()
{
  //Link to the database described in the document.
  //Need to ask user for user/password:
  //m_pFrame->load_from_document();
  Document_Glom* pDocument = static_cast<Document_Glom*>(get_document());
  if(!pDocument)
  {
    return false;
  }
  else
  {
#ifndef GLOM_ENABLE_CLIENT_ONLY
    //Connect signals:
    pDocument->signal_userlevel_changed().connect( sigc::mem_fun(*this, &App_Glom::on_userlevel_changed) );

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

      // Example files are not supported in client only mode because they
      // would need to be saved, but saving support is disabled.
#ifndef GLOM_ENABLE_CLIENT_ONLY
      const bool is_example = pDocument->get_is_example_file();
#endif // !GLOM_ENABLE_CLIENT_ONLY
      if(pDocument->get_is_example_file())
      {
#ifndef GLOM_ENABLE_CLIENT_ONLY
        // Remember the URI to the example file to be able to prevent
        // adding the URI to the recently used files in document_history_add.
        // We want to add the document that is created from the example
        // instead of the example itself.
        m_example_uri = pDocument->get_file_uri();

        pDocument->set_file_uri(Glib::ustring()); //Prevent it from defaulting to the read-only examples directory when offering saveas.

        //m_ui_save_extra_* are used by offer_saveas() if it's not empty:
        m_ui_save_extra_showextras = true;
        m_ui_save_extra_title = _("Creating From Example File");
        m_ui_save_extra_message = _("To use this example file you must save an editable copy of the file. A new database will also be created on the server.");
        m_ui_save_extra_newdb_title = "TODO";
        m_ui_save_extra_newdb_selfhosted = true;
        offer_saveas();
        // Note that bakery will try to add the example file itself to the
        // recently used documents, which is not what we want.
        m_ui_save_extra_message.clear();
        m_ui_save_extra_title.clear();

        //Get the results from the extended save dialog:
        pDocument->set_database_title(m_ui_save_extra_newdb_title);
        pDocument->set_connection_is_self_hosted(m_ui_save_extra_newdb_selfhosted);
        m_ui_save_extra_newdb_selfhosted = false;

        m_ui_save_extra_newdb_title.clear();
        m_ui_save_extra_showextras = false;

        if(get_operation_cancelled())
          return false;
        else
          pDocument->set_is_example_file(false);
#else // !GLOM_ENABLE_CLIENT_ONLY
        // TODO_clientonly: Tell the user that opening example files is
        // not supported. This could alternatively also be done in
        // Document_after::load_after, I am not sure which is better.
        return false;
#endif // GLOM_ENABLE_CLIENT_ONLY
      }

#ifndef GLOM_ENABLE_CLIENT_ONLY
      //Warn about read-only files, because users will otherwise wonder why they can't use Developer mode:
      Document_Glom::userLevelReason reason = Document_Glom::USER_LEVEL_REASON_UNKNOWN;
      const AppState::userlevels userlevel = pDocument->get_userlevel(reason);
      if( (userlevel == AppState::USERLEVEL_OPERATOR) && (reason == Document_Glom::USER_LEVEL_REASON_FILE_READ_ONLY) )
      {
        Gtk::MessageDialog dialog(Bakery::App_Gtk::util_bold_message(_("Opening Read-Only File.")), true,  Gtk::MESSAGE_INFO, Gtk::BUTTONS_NONE);
        dialog.set_secondary_text(_("This file is read only, so you will not be able to enter Developer mode to make design changes."));
        dialog.set_transient_for(*this);
        dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
        dialog.add_button(_("Continue without Developer Mode"), Gtk::RESPONSE_OK); //arbitrary response code.

        const int response = dialog.run();
        dialog.hide();
        if(response == Gtk::RESPONSE_CANCEL)
          return false;
      }
#endif // !GLOM_ENABLE_CLIENT_ONLY

      //Read the connection information from the document:
      ConnectionPool* connection_pool = ConnectionPool::get_instance();
      if(!connection_pool)
        return false; //Impossible anyway.
      else
      {
        //Set the connection details in the ConnectionPool singleton.
        //The ConnectionPool will now use these every time it tries to connect.
#ifndef GLOM_ENABLE_CLIENT_ONLY
        if(pDocument->get_connection_is_self_hosted())
        {
          // TODO: sleep, to give postgres time to start?
          connection_pool->set_self_hosted(pDocument->get_connection_self_hosted_directory_uri());

          if(!is_example) /* It will be started later, after we have asked for the initial db name/title and created the files.*/
          {
            const bool test = connection_pool->start_self_hosting(); //Stopped in on_menu_file_close().
            if(!test)
              return false;
          }
        }
        else
        {
          connection_pool->set_self_hosted(std::string());
        }
#endif // !GLOM_ENABLE_CLIENT_ONLY

        connection_pool->set_host(pDocument->get_connection_server());
        connection_pool->set_user(pDocument->get_connection_user());
        connection_pool->set_database(pDocument->get_connection_database());

        connection_pool->set_ready_to_connect(this); //Box_DB::connect_to_server() will now attempt the connection-> Shared instances of m_Connection will also be usable.

        //Attempt to connect to the specified database:
	bool test = false;
#ifdef GLIBMM_EXCEPTIONS_ENABLED
        try
#else
        std::auto_ptr<ExceptionConnection> error;
#endif
        {
#ifndef GLOM_ENABLE_CLIENT_ONLY
          if(is_example)
          {
            //The user has already had the chance to specify a new filename and database name.
            test = m_pFrame->connection_request_password_and_choose_new_database_name();
          }
          else
#endif // !GLOM_ENABLE_CLIENT_ONLY
#ifdef GLIBMM_EXCEPTIONS_ENABLED
            test = m_pFrame->connection_request_password_and_attempt();
#else
            test = m_pFrame->connection_request_password_and_attempt(error);
#endif
	}
#ifdef GLIBMM_EXCEPTIONS_ENABLED
        catch(const ExceptionConnection& ex)
        {
#else
        if(error.get())
	{
          const ExceptionConnection& ex = *error.get();
#endif
          if(ex.get_failure_type() == ExceptionConnection::FAILURE_NO_DATABASE) //This is the only FAILURE_* type that connection_request_password_and_attempt() throws.
          {
#ifndef GLOM_ENABLE_CLIENT_ONLY
            if(!is_example)
            {
              //The connection to the server is OK, but the database is not there yet.
              Frame_Glom::show_ok_dialog(_("Database Not Found On Server"), _("The database could not be found on the server. Please consult your system administrator."), *this, Gtk::MESSAGE_ERROR);
            }
            else
#endif // !GLOM_ENABLE_CLIENT_ONLY
              std::cerr << "App_Glom::on_document_load(): unexpected ExceptionConnection when opening example." << std::endl;
          }
          else
            std::cerr << "App_Glom::on_document_load(): unexpected ExceptionConnection failure type." << std::endl;
        }
	
        if(!test) //It usually throws an exception instead of returning false.
        {
#ifndef GLOM_ENABLE_CLIENT_ONLY
          //Stop self-hosting, if we are doing that:
          std::cout << "debug: calling stop_self_hosting_of_document_database()" << std::endl;
          stop_self_hosting_of_document_database();
#endif // !GLOM_ENABLE_CLIENT_ONLY

          return false; //Failed. Close the document.
        }

#ifndef GLOM_ENABLE_CLIENT_ONLY
        if(is_example)
        {
          //Create the example database:
          //connection_request_password_and_choose_new_database_name() has already change the database name to a new unused one:
          bool user_cancelled = false;
          const bool test = recreate_database(user_cancelled);
          if(!test)
          {
            // TODO: This is dead code, isn't it? We already bail above if
            // test isn't set. armin.

            //If the database was not successfully recreated:
            if(!user_cancelled)
            {
              //Let the user try again.
              //A warning has already been shown.
              return offer_new_or_existing();
            }
            else
              return false;
          }
          else
          {
            //Make sure that the changes (mark as non example, and save the new database name) are really saved:
            //Change the user level temporarily so that save_changes() actually saves:
            const AppState::userlevels user_level = pDocument->get_userlevel();
            pDocument->set_userlevel(AppState::USERLEVEL_DEVELOPER);
            pDocument->set_modified(true);
            pDocument->set_userlevel(user_level); //Change it back.
          }
        }
#endif // !GLOM_ENABLE_CLIENT_ONLY

        //Switch to operator mode when opening new documents:
        pDocument->set_userlevel(AppState::USERLEVEL_OPERATOR);

#ifndef GLOM_ENABLE_MAEMO
        m_pFrame->show_system_name();
#endif // !GLOM_ENABLE_MAEMO

        //Open default table, or show list of tables instead:
        m_pFrame->do_menu_Navigate_Table(true /* open the default if there is one */);

      }
    }

    //List the non-hidden tables in the menu:
    fill_menu_tables();

#ifndef GLOM_ENABLE_CLIENT_ONLY
    pDocument->set_allow_autosave(true);
#endif // !GLOM_ENABLE_CLIENT_ONLY

    return true; //Loading of the document into the application succeeded.
  }
}

/*
void App_Glom::statusbar_set_text(const Glib::ustring& strText)
{
  m_pStatus->set_text(strText);
}

void App_Glom::statusbar_clear()
{
  statusbar_set_text("");
}
*/


#ifndef GLOM_ENABLE_CLIENT_ONLY
void App_Glom::on_userlevel_changed(AppState::userlevels /* userlevel */)
{
  update_userlevel_ui();
}

void App_Glom::update_userlevel_ui()
{
  AppState::userlevels userlevel = get_userlevel();

  //Disable/Enable developer actions:
  for(type_listActions::iterator iter = m_listDeveloperActions.begin(); iter != m_listDeveloperActions.end(); ++iter)
  {
    Glib::RefPtr<Gtk::Action> action = *iter;
     action->set_sensitive ( userlevel == AppState::USERLEVEL_DEVELOPER );
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
  }
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

Glib::RefPtr<Gtk::UIManager> App_Glom::get_ui_manager()
{
  return m_refUIManager;
}

bool App_Glom::offer_new_or_existing()
{
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  //Offer to load an existing document, or start a new one.
  Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_existing_or_new");
#else
  std::auto_ptr<Gnome::Glade::XmlError> error;
  Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_existing_or_new", "", error);
  if(error.get()) return false;
#endif

  Gtk::Dialog* dialog = 0;
  refXml->get_widget("dialog_existing_or_new", dialog);
  dialog->set_transient_for(*this);

  Gtk::RecentChooserWidget* recent_chooser = NULL;
  refXml->get_widget("existing_or_new_recentchooser", recent_chooser);

  Gtk::RecentFilter filter;
  filter.add_mime_type("application/x-glom");
  recent_chooser->set_filter(filter);

  /* Don't show files that don't exist anymore: */
  recent_chooser->set_show_not_found(FALSE);

  Gtk::Frame* recent_frame = NULL;
  refXml->get_widget("existing_or_new_recentchooser_frame", recent_frame);

  // Hide the recent chooser when they are not any recently used files
  if(recent_chooser->get_items().empty()) recent_frame->hide();
  recent_chooser->signal_item_activated().connect(sigc::bind(sigc::mem_fun(*dialog, &Gtk::Dialog::response), 1)); // Open

#ifdef GLOM_ENABLE_CLIENT_ONLY
  // Don't offer the user to create a new document, because without
  // developer mode he couldn't do anything useful without it, anyway.
  Gtk::Button* new_button;
  refXml->get_widget("existing_or_new_button_new", new_button);
  new_button->hide();

  refXml->get_widget("existing_or_new_button_example", new_button);
  new_button->hide();

  // Show another label that does not ask whether one wants to create a new
  // document because that is not possible in client only mode
  // TODO: Add another text, or simply hide the label?
  Gtk::Label* label;
  refXml->get_widget("existing_or_new_label", label);
  label->set_markup(_("<span weight='bold' size='larger'>Open existing document</span>\n"));
#endif // GLOM_ENABLE_CLIENT_ONLY

#ifdef GLOM_ENABLE_MAEMO
  // Set dialog title to not show <unnamed> (default on maemo for empty title)
  // Strip of terminating newline.
  dialog->set_title(label->get_text().substr(0, label->get_text().length()-1));
  label->hide();

  // This makes the dialog too big in width otherwise
  recent_chooser->set_size_request(500, -1);
#endif

  const int response_id = dialog->run();
  Glib::ustring selected_uri = recent_chooser->get_current_uri();

  delete dialog;
  dialog = 0;

  if(response_id == 1) //Open
  {
    // When a recent document was selected, open that one instead.
    if(selected_uri.empty())
      on_menu_file_open();
    else
      open_document(selected_uri);

    //Check that a document was opened:
    Document_Glom* document = dynamic_cast<Document_Glom*>(get_document());
    if(document->get_file_uri().empty())
    {
      //Ask again:
      return offer_new_or_existing();
    }
  }
#ifndef GLOM_ENABLE_CLIENT_ONLY
  if(response_id == 3) //Open Example
  {
    //Based on on_menu_file_open();

    //Display File Open dialog and respond to choice:

    //Bring document window to front, to make it clear which document is being changed:
    ui_bring_to_front();

    //Ask user to choose file to open:
    //g_warning("GLOM_EXAMPLES_DIR=%s", GLOM_EXAMPLES_DIR);
    Glib::ustring file_uri = ui_file_select_open(GLOM_EXAMPLES_DIR);
    if(!file_uri.empty())
      open_document(file_uri);

    //Check that a document was opened:
    Document_Glom* document = dynamic_cast<Document_Glom*>(get_document());
    if(document->get_file_uri().empty())
    {
      //Ask again:
      return offer_new_or_existing();
    }
  }
  else if(response_id == 2) //New
  {
    //Each document must have a location, so ask the user for one.
    //This will use an extended save dialog that also asks for the database title and some hosting details:
    Glib::ustring db_title;

    m_ui_save_extra_showextras = true; //Offer self-hosting or central hosting, and offer the database title.
    m_ui_save_extra_newdb_selfhosted = true; /* Default to self-hosting */
    m_ui_save_extra_newdb_title.clear();
    offer_saveas();

    //Check that the document was given a location:
    Document_Glom* document = dynamic_cast<Document_Glom*>(get_document());
    if(!document->get_file_uri().empty())
    {
      //Get details from the extended save dialog:
      const Glib::ustring db_title = m_ui_save_extra_newdb_title;
      const bool self_hosted = m_ui_save_extra_newdb_selfhosted;
      m_ui_save_extra_newdb_title.clear();
      m_ui_save_extra_newdb_selfhosted = false;

      //Make sure that the user can do something with his new document:
      document->set_userlevel(AppState::USERLEVEL_DEVELOPER);

      //Each new document must have an associated new database,
      //so ask the user for the name of one to create:
      

      bool keep_asking = true;
      while(keep_asking)
      {
        //Create a database name based on the title.
        //The user will (almost) never see this anyway but it's nicer than using a random number:
        Glib::ustring db_name = Utils::create_name_from_title(db_title);

        //Prefix glom_ to the database name, so it's more obvious
        //for the system administrator.
        //This database name should never be user-visible again, either prefixed or not prefixed.
        db_name = "glom_" + db_name;

        if(!db_name.empty()) //The dialog and prefix prevent this anyway.
        {
          //Connect to the server and choose a variation of this db_name that does not exist yet:
          document->set_connection_database(db_name);
          document->set_connection_is_self_hosted(self_hosted);
               
          const bool connected = m_pFrame->connection_request_password_and_choose_new_database_name();
          if(!connected)
            return false;

          const bool db_created = m_pFrame->create_database(document->get_connection_database(), db_title, false /* do not request password */);
          if(db_created)
          {
            keep_asking = false;

            //document->set_connection_database(db_name); //Select the database that was just created.

            /*
            ConnectionPool* connection_pool = ConnectionPool::get_instance();
            if(connection_pool)
            {
              connection_pool->set_database(db_name); //The rest has been set while creating the database.
            }
            */

            const Glib::ustring database_name_used = document->get_connection_database();
            ConnectionPool::get_instance()->set_database(database_name_used);
            document->set_database_title(db_title);
            m_pFrame->set_databases_selected(database_name_used);
          }
          else
          {
            //Ask again:
            return offer_new_or_existing();
          }
        }
        else
        {
           g_warning(" App_Glom::offer_new_or_existing(): db_name is empty.");
           //And ask again, by going back to the start of the while() loop.
        }

      } /* while() */

      return true; //File successfully created.

    }
    else
    {
      //Ask again:
      return offer_new_or_existing();
    }
  }
#endif // !GLOM_ENABLE_CLIENT_ONLY
  else if(response_id == Gtk::RESPONSE_CANCEL)
  {
    return false; //close the window to close the application, because they need to choose a new or existing document.
  }
  else
  {
    //Do nothing. TODO: Do something?
  }

  return true;
}
void App_Glom::set_mode_data()
{
  m_action_mode_data->activate();
}

void App_Glom::set_mode_find()
{
  m_action_mode_find->activate();
}


void App_Glom::init_menus_help()
{
  //Call base class:
  App_WithDoc_Gtk::init_menus_help();
  m_refHelpActionGroup->add( Gtk::Action::create("BakeryAction_Help_Contents",
                        _("_Contents"), _("Help with the application")),
                        sigc::mem_fun(*this, &App_Glom::on_menu_help_contents) );

  //Build part of the menu structure, to be merged in by using the "PH" plaeholders:
  static const Glib::ustring ui_description =
    "<ui>"
#ifdef GLOM_ENABLE_MAEMO
    "  <popup name='Bakery_MainMenu'>"
#else
    "  <menubar name='Bakery_MainMenu'>"
#endif
    "    <placeholder name='Bakery_MenuPH_Help'>"
    "      <menu action='BakeryAction_Menu_Help'>"
    "        <menuitem action='BakeryAction_Help_Contents' />"
    "      </menu>"
    "    </placeholder>"
#ifdef GLOM_ENABLE_MAEMO
    "  </popup>"
#else
    "  </menubar>"
#endif
    "</ui>";

  //Add menu:
  add_ui_from_string(ui_description);
}

void App_Glom::on_menu_help_contents()
{
  Glom::Utils::show_help();
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
bool App_Glom::recreate_database(bool& user_cancelled)
{
  //Create a database, based on the information in the current document:
  Document_Glom* pDocument = static_cast<Document_Glom*>(get_document());
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
    if(error.get() == NULL)
    {
#endif // GLIBMM_EXCEPTIONS_ENABLED
      g_warning("App_Glom::recreate_database(): Failed because database exists already.");

      return false; //Connection to the database succeeded, because no exception was thrown. so the database exists already.
#ifndef GLIBMM_EXCEPTIONS_ENABLED
    }
#endif // !GLIBMM_EXCEPTIONS_ENABLED
  }
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  catch(const ExceptionConnection& ex)
  {
#else
  if(error.get() != NULL)
  {
    const ExceptionConnection* exptr = dynamic_cast<ExceptionConnection*>(error.get());
    if(exptr)
    {
      const ExceptionConnection& ex = *exptr;
#endif // GLIBMM_EXCEPTIONS_ENABLED
      if(ex.get_failure_type() == ExceptionConnection::FAILURE_NO_SERVER)
      {
        user_cancelled = true; //Eventually, the user will cancel after retrying.
        g_warning("App_Glom::recreate_database(): Failed because connection to server failed, without specifying a database.");
        return false;
      }
#ifndef GLIBMM_EXCEPTIONS_ENABLED
    }
#endif // !GLIBMM_EXCEPTIONS_ENABLED

    //Otherwise continue, because we _expected_ connect() to fail if the db does not exist yet.
  }

  //Show the user that something is happening, because the INSERTS might take time.
  //TOOD: This doesn't actually show up until near the end, even with Gtk::Main::instance()->iteration().
  std::auto_ptr<Dialog_ProgressCreating> dialog_progress;
  Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "window_progress");
  if(refXml)
  {
    Dialog_ProgressCreating* dialog_progress_temp = 0;
    refXml->get_widget_derived("window_progress", dialog_progress_temp);
    if(dialog_progress_temp)
    {
      dialog_progress_temp->set_message(_("Creating Glom Database"), _("Creating Glom database from example file."));
      dialog_progress.reset(dialog_progress_temp); //Put the dialog in an auto_ptr so that it will be deleted (and hidden) when the current function returns.

      dialog_progress->set_transient_for(*this);
      dialog_progress->show();

      //Ensure that the dialog is shown, instead of waiting for the application to be idle:
      while(Gtk::Main::instance()->events_pending())
        Gtk::Main::instance()->iteration();
    }
  }

  dialog_progress->pulse();

  //Create the database: (This will show a connection dialog)
  connection_pool->set_database( Glib::ustring() );
  const bool db_created = m_pFrame->create_database(db_name, pDocument->get_database_title(), false /* Don't ask for password etc again. */);

  if(!db_created)
  {
    return false;
  }
  else
    connection_pool->set_database(db_name); //Specify the new database when connecting from now on.

  dialog_progress->pulse();
  Bakery::BusyCursor busy_cursor(this);

  sharedptr<SharedConnection> sharedconnection;
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
#endif // GLIBMM_EXCEPTIONS_ENABLED
  {
#ifdef GLIBMM_EXCEPTIONS_ENABLED
    sharedconnection = connection_pool->connect();
#else
    sharedconnection = connection_pool->connect(error);
    if(error.get() == NULL)
#endif // GLIBMM_EXCEPTIONS_ENABLED
      connection_pool->set_database(db_name); //The database was successfully created, so specify it when connecting from now on.
  }
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  catch(const ExceptionConnection& ex)
  {
#else
  if(error.get() != NULL)
  {
    const std::exception& ex = *error.get();
#endif // GLIBMM_EXCEPTIONS_ENABLED
    g_warning("App_Glom::recreate_database(): Failed to connect to the newly-created database.");
    return false;
  }

  dialog_progress->pulse();

  //Create each table:
  Document_Glom::type_listTableInfo tables = pDocument->get_tables();
  for(Document_Glom::type_listTableInfo::const_iterator iter = tables.begin(); iter != tables.end(); ++iter)
  {
    sharedptr<const TableInfo> table_info = *iter;

    //Create SQL to describe all fields in this table:
    Glib::ustring sql_fields;
    Document_Glom::type_vecFields fields = pDocument->get_table_fields(table_info->get_name());

    dialog_progress->pulse();
    const bool table_creation_succeeded = m_pFrame->create_table(table_info, fields);
    dialog_progress->pulse();
    if(!table_creation_succeeded)
    {
      g_warning("App_Glom::recreate_database(): CREATE TABLE failed with the newly-created database.");
      return false;
    }
  }

  dialog_progress->pulse();
  m_pFrame->add_standard_tables(); //Add internal, hidden, tables.

  //Create the developer group, and make this user a member of it:
  //If we got this far then the user must really have developer privileges already:
  dialog_progress->pulse();
  const bool test = m_pFrame->add_standard_groups();
  if(!test)
    return false;

  for(Document_Glom::type_listTableInfo::const_iterator iter = tables.begin(); iter != tables.end(); ++iter)
  {
    sharedptr<const TableInfo> table_info = *iter;

    //Add any example data to the table:
    dialog_progress->pulse();

    //try
    //{
      const bool table_insert_succeeded = m_pFrame->insert_example_data(table_info->get_name());
      
      if(!table_insert_succeeded)
      {
        g_warning("App_Glom::recreate_database(): INSERT of example data failed with the newly-created database.");
        return false;
      }
    //}
    //catch(const std::exception& ex)
    //{
    //  std::cerr << "App_Glom::recreate_database(): exception: " << ex.what() << std::endl;
      //HandleError(ex);
    //}

  } //for(tables)

  return true; //All tables created successfully.
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

AppState::userlevels App_Glom::get_userlevel() const
{
  const Document_Glom* document = dynamic_cast<const Document_Glom*>(get_document());
  if(document)
  {
    return document->get_userlevel();
  }
  else
    g_assert_not_reached();
    //return AppState::USERLEVEL_DEVELOPER; //This should never happen.
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void App_Glom::add_developer_action(const Glib::RefPtr<Gtk::Action>& refAction)
{
  //Prevent it from being added twice:
  remove_developer_action(refAction);

  m_listDeveloperActions.push_back(refAction);
}

void App_Glom::remove_developer_action(const Glib::RefPtr<Gtk::Action>& refAction)
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

void App_Glom::fill_menu_tables()
{
  //TODO: There must be a better way than building a ui_string like this:

  m_listNavTableActions.clear();
  if(m_menu_tables_ui_merge_id)
    m_refUIManager->remove_ui(m_menu_tables_ui_merge_id);

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

  Document_Glom* document = dynamic_cast<Document_Glom*>(get_document());
  const Document_Glom::type_listTableInfo tables = document->get_tables();
  for(Document_Glom::type_listTableInfo::const_iterator iter = tables.begin(); iter != tables.end(); ++iter)
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
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
#else
  std::auto_ptr<Glib::Error> error;
#endif // GLIBMM_EXCEPTIONS_ENABLED
  {
#ifdef GLIBMM_EXCEPTIONS_ENABLED
    m_menu_tables_ui_merge_id = m_refUIManager->add_ui_from_string(ui_description);
#else
    m_menu_tables_ui_merge_id = m_refUIManager->add_ui_from_string(ui_description, error);
#endif // GLIBMM_EXCEPTIONS_ENABLED
  }
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  catch(const Glib::Error& ex)
  {
#else
  if(error.get() != NULL)
  {
    const Glib::Error& ex = *error.get();
#endif // GLIBMM_EXCEPTIONS_ENABLED
    std::cerr << " App_Glom::fill_menu_tables(): building menus failed: " <<  ex.what();
  }
}


void App_Glom::fill_menu_reports(const Glib::ustring& table_name)
{
  //TODO: There must be a better way than building a ui_string like this:

  m_listNavReportActions.clear();
  if(m_menu_reports_ui_merge_id)
    m_refUIManager->remove_ui(m_menu_reports_ui_merge_id);

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

  Document_Glom* document = dynamic_cast<Document_Glom*>(get_document());
  const Document_Glom::type_listReports tables = document->get_report_names(table_name);
  for(Document_Glom::type_listReports::const_iterator iter = tables.begin(); iter != tables.end(); ++iter)
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
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
#else
  std::auto_ptr<Glib::Error> error;
#endif // GLIBMM_EXCEPTIONS_ENABLED
  {
#ifdef GLIBMM_EXCEPTIONS_ENABLED
    m_menu_reports_ui_merge_id = m_refUIManager->add_ui_from_string(ui_description);
#else
    m_menu_reports_ui_merge_id = m_refUIManager->add_ui_from_string(ui_description, error);
#endif // GLIBMM_EXCEPTIONS_ENABLED
  }
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  catch(const Glib::Error& ex)
  {
#else
  if(error.get() != NULL)
  {
    const Glib::Error& ex = *error.get();
#endif // GLIBMM_EXCEPTIONS_ENABLED
    std::cerr << " App_Glom::fill_menu_reports(): building menus failed: " <<  ex.what();
  }
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void App_Glom::on_menu_file_save_as_example()
{
  //Based on the implementation of Bakery::App_WithDoc::on_menu_file_saveas()

  //Display File Save dialog and respond to choice:

  //Bring document window to front, to make it clear which document is being saved:
  //This doesn't work: TODO.
  ui_bring_to_front();

  //Show the save dialog:
  Document_Glom* document = dynamic_cast<Document_Glom*>(get_document());
  const Glib::ustring& file_uriOld = document->get_file_uri();

  m_ui_save_extra_showextras = false;
  m_ui_save_extra_title.clear();
  m_ui_save_extra_message.clear();
  m_ui_save_extra_newdb_title.clear();
  m_ui_save_extra_newdb_selfhosted = false;

  Glib::ustring file_uri = ui_file_select_save(file_uriOld); //Also asks for overwrite confirmation.
  if(!file_uri.empty())
  {
    //Enforce the file extension:
    file_uri = document->get_file_uri_with_extension(file_uri);

    bool bUseThisFileUri = true;

    //Check whether file exists already:
    {
      // Try to open the input file.
      Gnome::Vfs::Handle read_handle;
      try
      {
        read_handle.open(file_uri, Gnome::Vfs::OPEN_READ);

        //It does (there was no exception), so ask the user to confirm overwrite:
        const bool bOverwrite = true; //The FileChooser asked already. ui_ask_overwrite(file_uri);

        //Respond to button that was clicked:
        bUseThisFileUri = bOverwrite;
      }
      catch(const Gnome::Vfs::exception& ex)
      {
        bUseThisFileUri = true; //It does not exist.
      }
    }

    //Save to this filepath:
    if(bUseThisFileUri)
    {
      //Prevent saving while we modify the document:
      document->set_allow_autosave(false);

      document->set_file_uri(file_uri, true); //true = enforce file extension
      document->set_is_example_file();

      //Save all data from all tables into the document:
      Document_Glom::type_listTableInfo list_table_info = document->get_tables();
      for(Document_Glom::type_listTableInfo::const_iterator iter = list_table_info.begin(); iter != list_table_info.end(); ++iter)
      {
        const Glib::ustring table_name = (*iter)->get_name();

        //const type_vecFields vec_fields = document->get_table_fields(table_name);

        //export_data_to_stream() needs a type_mapLayoutGroupSequence;
        Document_Glom::type_mapLayoutGroupSequence sequence = document->get_data_layout_groups_default("list", table_name);

        //std::cout << "debug: table_name=" << table_name << std::endl;

        Glib::ustring row_text;
        FoundSet found_set;
        found_set.m_table_name = table_name;
        m_pFrame->export_data_to_string(row_text, found_set, sequence);
        //std::cout << "  debug after row_text=" << row_text << std::endl;

        document->set_table_example_data(table_name, row_text);
      }

      document->set_allow_autosave(true);

      bool bTest  = document->save();

      if(!bTest)
      {
        ui_warning(_("Save failed."), _("There was an error while saving the file. Your changes have not been saved."));
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

void App_Glom::on_menu_developer_changelanguage()
{
  Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_change_language");
  if(refXml)
  {
    Dialog_ChangeLanguage* dialog = 0;
    refXml->get_widget_derived("dialog_change_language", dialog);
    if(dialog)
    {
      dialog->set_transient_for(*this);
      const int response =       Glom::Utils::dialog_run_with_help(dialog, "dialog_change_language");
      dialog->hide();

      if(response == Gtk::RESPONSE_OK)
      {
        TranslatableItem::set_current_locale(dialog->get_locale());

       //Get the translations from the document (in Operator mode, we only load the necessary translations.)
       //This also updates the UI, so we show all the translated titles:
       get_document()->load();

       m_pFrame->show_table_refresh(); //load() doesn't seem to refresh the view.
      }

      delete dialog;
    }
  }
}

void App_Glom::on_menu_developer_translations()
{
  if(!m_window_translations)
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "window_translations");
    refXml->get_widget_derived("window_translations", m_window_translations);
    if(m_window_translations)
    {
      m_pFrame->add_view(m_window_translations);
      m_window_translations->set_transient_for(*this);
      m_window_translations->set_document(static_cast<Document_Glom*>(m_pDocument));
      m_window_translations->load_from_document();
      m_window_translations->show();

      m_window_translations->signal_hide().connect(sigc::mem_fun(*this, &App_Glom::on_window_translations_hide));
    }
  }
  else
  {
    m_window_translations->show();
    m_window_translations->load_from_document();
  }
}

void App_Glom::on_window_translations_hide()
{
  if(m_window_translations)
  {
    m_pFrame->on_developer_dialog_hide();
  }
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

App_Glom* App_Glom::get_application()
{
  return global_application;
}

void App_Glom::document_history_add(const Glib::ustring& file_uri)
{
  // We override this so we can prevent example files from being saved in the recently-used list:

  bool prevent = false;
  if(!file_uri.empty())
  {
    prevent = (file_uri == m_example_uri);
  }

  // Call the base class:
  if(!prevent)
    Bakery::App_WithDoc_Gtk::document_history_add(file_uri);
}

} //namespace Glom


