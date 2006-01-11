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
#include "dialog_new_database.h"
#include <libgnome/gnome-help.h> //For gnome_help_display
#include "config.h" //For VERSION.
#include <cstdio>
#include <memory> //For std::auto_ptr<>
#include <glibmm/i18n.h>


App_Glom::App_Glom(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::Window(cobject), //It's a virtual base class, so we have to call the specific constructor to prevent the default constructor from being called.
  type_base(cobject, "Glom"),
  m_pBoxTop(0),
  m_pFrame(0),
  m_menu_tables_ui_merge_id(0),
  m_menu_reports_ui_merge_id(0)
{
  //Load widgets from glade file:
  refGlade->get_widget("bakery_vbox", m_pBoxTop);
  refGlade->get_widget_derived("vbox_frame", m_pFrame); //This one is derived. There's a lot happening here.

  add_mime_type("application/x-glom"); //TODO: make this actually work - we need to register it properly.

  //Hide the toolbar because it doesn't contain anything useful for this app.
  m_HandleBox_Toolbar.hide();
}

App_Glom::~App_Glom()
{

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
    bool test = open_document(document_uri);
    if(!test)
      return offer_new_or_existing();
  }
  
  return true;
  //show_all();
}

void App_Glom::init_layout()
{
  //We override this method so that we can put everything in the vbox from the glade file, instead of the vbox from App_Gtk.

  //Add menu bar at the top:
  //These were defined in init_uimanager().
  Gtk::MenuBar* pMenuBar = static_cast<Gtk::MenuBar*>(m_refUIManager->get_widget("/Bakery_MainMenu"));
  m_pBoxTop->pack_start(*pMenuBar, Gtk::PACK_SHRINK);

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

  m_refFileActionGroup->add(Gtk::Action::create("GlomAction_File_Print", Gtk::Stock::PRINT),
                        sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_file_print) );

  m_refFileActionGroup->add(Gtk::Action::create("BakeryAction_File_Close", Gtk::Stock::CLOSE),
                        sigc::mem_fun((App_WithDoc&)*this, &App_WithDoc::on_menu_file_close));
  m_refFileActionGroup->add(Gtk::Action::create("BakeryAction_File_Exit", Gtk::Stock::QUIT),
                        sigc::mem_fun((App&)*this, &App::on_menu_file_exit));

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
    "        <separator/>"
    "        <menuitem action='GlomAction_File_Print' />"
    "        <separator/>"
    "        <menuitem action='BakeryAction_File_Close' />"
    "        <menuitem action='BakeryAction_File_Exit' />"
    "      </menu>"
    "    </placeholder>"
    "  </menubar>"
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


  Glib::RefPtr<Gtk::Action> action = Gtk::Action::create("GlomAction_Menu_EditTables", _("_Edit Tables"));
  m_refActionGroup_Others->add(action,
                        sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_Tables_EditTables) );
  m_listDeveloperActions.push_back(action);


  //"Reports" menu:
  m_refActionGroup_Others->add( Gtk::Action::create("Glom_Menu_Reports", _("_Reports")) );

  action = Gtk::Action::create("GlomAction_Menu_EditReports", _("_Edit Reports"));
  m_refActionGroup_Others->add(action,
                        sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_Tables_EditReports) );
  m_listDeveloperActions.push_back(action);

  //"UserLevel" menu:
  m_refActionGroup_Others->add(Gtk::Action::create("Glom_Menu_userlevel", _("_User Level")));
  Gtk::RadioAction::Group group_userlevel;

  m_action_menu_userlevel_developer = Gtk::RadioAction::create(group_userlevel, "GlomAction_Menu_userlevel_Developer", _("_Developer"));
  m_refActionGroup_Others->add(m_action_menu_userlevel_developer,
                        sigc::mem_fun(*this, &App_Glom::on_menu_userlevel_developer) );

  m_action_menu_userlevel_operator =  Gtk::RadioAction::create(group_userlevel, "GlomAction_Menu_userlevel_Operator", _("_Operator"));
  m_refActionGroup_Others->add(m_action_menu_userlevel_operator,
                          sigc::mem_fun(*this, &App_Glom::on_menu_userlevel_operator) );

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

  action = Gtk::Action::create("GlomAction_Menu_Developer_Layout", _("_Layout"));
  m_listDeveloperActions.push_back(action);
  m_refActionGroup_Others->add(action, sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_developer_layout));

  m_refUIManager->insert_action_group(m_refActionGroup_Others);

  //Build part of the menu structure, to be merged in by using the "Bakery_MenuPH_Others" placeholder:
  static const Glib::ustring ui_description =
    "<ui>"
    "  <menubar name='Bakery_MainMenu'>"
    "    <placeholder name='Bakery_MenuPH_Others'>"
    "      <menu action='Glom_Menu_Tables'>"
    "        <placeholder name='Menu_Tables_Dynamic' />"
    "        <separator />"
    "        <menuitem action='GlomAction_Menu_EditTables' />"
    "     </menu>"
    "     <menu action='Glom_Menu_Reports'>"
    "        <placeholder name='Menu_Reports_Dynamic' />"
    "        <separator />"
    "        <menuitem action='GlomAction_Menu_EditReports' />"
    "     </menu>"
    "      <menu action='Glom_Menu_Mode'>"
    "        <menuitem action='GlomAction_Menu_Mode_Data' />"
    "        <menuitem action='GlomAction_Menu_Mode_Find' />"
    "      </menu>"
    "      <menu action='Glom_Menu_userlevel'>"
    "        <menuitem action='GlomAction_Menu_userlevel_Developer' />"
    "        <menuitem action='GlomAction_Menu_userlevel_Operator' />"
    "      </menu>"
    "      <menu action='Glom_Menu_Developer'>"
    "        <menuitem action='GlomAction_Menu_Developer_Database_Preferences' />"
    "        <menuitem action='GlomAction_Menu_Developer_Fields' />"
    "        <menuitem action='GlomAction_Menu_Developer_Relationships' />"
    "        <menuitem action='GlomAction_Menu_Developer_Layout' />"
    "        <menuitem action='GlomAction_Menu_Developer_Users' />"
    "        <menuitem action='GlomAction_Menu_Developer_Reports' />"
    "      </menu>"
    "    </placeholder>"
    "  </menubar>"
    "</ui>";

/*  "        <menuitem action='GlomAction_Menu_Developer_RelationshipsOverview' />" */

  //Add menu:
  add_ui_from_string(ui_description);

  init_menus_help();

  fill_menu_tables();
}

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

Bakery::App* App_Glom::new_instance() //Override
{
  Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "window_main");
  App_Glom* pApp_Glom = 0;
  refXml->get_widget_derived("window_main", pApp_Glom);

  return pApp_Glom;
}

void App_Glom::init_create_document()
{
  if(!m_pDocument)
  {
    m_pDocument = new Document_Glom();

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
    return false;
  else
  {
    //Connect signals:
    pDocument->signal_userlevel_changed().connect( sigc::mem_fun(*this, &App_Glom::on_userlevel_changed) );

    //Disable/Enable actions, depending on userlevel:
    pDocument->emit_userlevel_changed();
 
    if(pDocument->get_connection_database().empty()) //If it is a new (default) document.
    {
      //offer_new_or_existing();
    }
    else
    {
      //Read the connection information from the document:
      ConnectionPool* connection_pool = ConnectionPool::get_instance();
      if(!connection_pool)
        return false; //Impossible anyway.
      else
      {
        //Set the connection details in the ConnectionPool singleton.
        //The ConnectionPool will now use these every time it tries to connect.
        connection_pool->set_host(pDocument->get_connection_server());
        connection_pool->set_user(pDocument->get_connection_user());
        connection_pool->set_database(pDocument->get_connection_database());

        connection_pool->set_ready_to_connect(); //Box_DB::connect_to_server() will now attempt the connection-> Shared instances of m_Connection will also be usable.

        //Attempt to connect to the specified database:
        try
        {
          bool test = m_pFrame->connection_request_password_and_attempt();
          if(!test)
            return false; //Failed. Close the document.
        }
        catch(const ExceptionConnection& ex)
        {
          if(ex.get_failure_type() == ExceptionConnection::FAILURE_NO_DATABASE) //This is the only FALURE_* type that connection_request_password_and_attempt() throws.
          {
            //The connection to the server is OK, but the database is not there yet.
            //Ask the user if he wants to create it.
            //For instance, it might be an example document, whose database does not exist on his server yet.

            Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_recreate_database");
            Gtk::Dialog* dialog = 0;
            refXml->get_widget("dialog_recreate_database", dialog);
            if(dialog)
            {
              int response = dialog->run();
              dialog->set_transient_for(*this);
              delete dialog;
              dialog = 0;

              if(response == Gtk::RESPONSE_CANCEL)
                return false; //Close the document.
              else
              {
                bool user_cancelled = false;
                bool test = recreate_database(user_cancelled);

                if(test)
                {
                  //If the database was successfully recreated.

                  //Warn about read-only files, such as installed example files.
                  Document_Glom::userLevelReason reason = Document_Glom::USER_LEVEL_REASON_UNKNOWN;
                  AppState::userlevels userlevel = pDocument->get_userlevel(reason);
                  if( (userlevel == AppState::USERLEVEL_OPERATOR) && (reason == Document_Glom::USER_LEVEL_REASON_FILE_READ_ONLY) )
                  {
                    Gtk::MessageDialog dialog(Bakery::App_Gtk::util_bold_message(_("Creating from read-only file.")), true,  Gtk::MESSAGE_INFO, Gtk::BUTTONS_NONE);
                    dialog.set_secondary_text(_("This file is read only, so you will not be able to enter Developer mode to make design changes. Maybe this file is an installed example file. Therefore, you might want to create your own writeable copy of this file."));
                    dialog.set_transient_for(*this);

                    dialog.add_button(Gtk::Stock::SAVE_AS, Gtk::RESPONSE_OK); //arbitrary response code.
                    dialog.add_button(_("Coninue without Developer Mode"), Gtk::RESPONSE_ACCEPT); //arbitrary response code.

                    int response = dialog.run();
                    if(response == Gtk::RESPONSE_OK)
                    {
                      //TODO: Offer again if they choose cancel.
                      offer_saveas();
                    }
                    //TODO: Store a magic number in the database (a special table) and the file to check for mismatches.
                  }
                }
                else
                {
                  //If the database was not successfully recreated:

                  if(!user_cancelled)
                  {
                    //Tell the user:
                    Gtk::Dialog* dialog = 0;
                    try
                    {
                      Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_error_create_database");
                      refXml->get_widget("dialog_error_create_database", dialog);
                      dialog->set_transient_for(*this);
                      dialog->run();
                      delete dialog;
                    }
                    catch(const Gnome::Glade::XmlError& ex)
                    {
                      std::cerr << ex.what() << std::endl;
                    }
                  }

                  return false;
                }
              }
            }
          }
        }

        //Switch to operator mode when opening new documents:
        pDocument->set_userlevel(AppState::USERLEVEL_OPERATOR);

        m_pFrame->show_system_name();

        //Open default table, or show list of tables instead:
        m_pFrame->do_menu_Navigate_Table(true /* open the default if there is one */);

      }
    }

    //List the non-hidden tables in the menu:
    fill_menu_tables();

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

Glib::RefPtr<Gtk::UIManager> App_Glom::get_ui_manager()
{
  return m_refUIManager;
}

bool App_Glom::offer_new_or_existing()
{
  //Offer to load an existing document, or start a new one.
  Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_existing_or_new");
  Gtk::Dialog* dialog = 0;
  refXml->get_widget("dialog_existing_or_new", dialog);
  dialog->set_transient_for(*this);

  int response_id = dialog->run();
  delete dialog;
  dialog = 0;

  if(response_id == 1) //Open
  {
    on_menu_file_open();

    //Check that a document was opened:
    Document_Glom* document = dynamic_cast<Document_Glom*>(get_document());
    if(document->get_file_uri().empty())
    {
      //Ask again:
      return offer_new_or_existing();
    }
  }
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
    offer_saveas();

    //Check that the document was given a location:
    Document_Glom* document = dynamic_cast<Document_Glom*>(get_document());
    if(!document->get_file_uri().empty())
    {
      //Make sure that the user can do something with his new document:
      document->set_userlevel(AppState::USERLEVEL_DEVELOPER);

      //Each new document must have an associated new database,
      //so ask the user for the name of one to create:
      Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_new_database");
      if(refXml)
      {
        Dialog_NewDatabase* dialog = 0;
        refXml->get_widget_derived("dialog_new_database", dialog);
        if(dialog)
        {
          std::auto_ptr<Dialog_NewDatabase> dialog_owner(dialog); //This will delete the dialog even when we return in the middle of this function.
          dialog->set_transient_for(*this);

          //Set suitable defaults:
          const Glib::ustring filename = document->get_name(); //Get the filename without the path and extension.
          dialog->set_input(filename, Base_DB::util_title_from_string( filename ) );

          bool keep_asking = true;
          while(keep_asking)
          {
            int response = dialog->run();

            if(response == Gtk::RESPONSE_OK)
            {
              Glib::ustring db_name;
              Glib::ustring db_title;
              dialog->get_input(db_name, db_title);

              //Prefix glom_ to the database name, so it's more obvious
              //for the system administrator.
              //This database name should never be user-visible again, either prefixed or not prefixed.
              db_name = "glom_" + db_name;

              if(!db_name.empty()) //The dialog prevents this anyway.
              {
                bool db_created = m_pFrame->create_database(db_name);
                if(db_created)
                {
                  keep_asking = false;

                  document->set_connection_database(db_name); //Select the database that was just created.

                  ConnectionPool* connection_pool = ConnectionPool::get_instance();
                  if(connection_pool)
                  {
                    connection_pool->set_database(db_name); //The rest has been set while creating the database.
                  }

                  document->set_database_title(db_title);
                  m_pFrame->set_databases_selected(db_name);
                }
                else
                {
                  return false;
                }
              }
              else
              {
                g_warning(" App_Glom::offer_new_or_existing(): db_name is empty.");
                //And ask again, by going back to the start of the while() loop.
              }
            } /* if(response) */
            else
            {
              return false; //The user cancelled.
            }            
          } /* while() */
            
          return true; //File successfully created.
        }
       }
  
      return false; //Creation of new document failed.
    }
    else
    {
      //Ask again:
      return offer_new_or_existing();
    }
  }
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
  m_action_mode_data->activate();
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

void App_Glom::on_menu_help_contents()
{
  gnome_help_display("glom", 0, 0);
}

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
  try
  {
    connection_pool->set_ready_to_connect(); //This has succeeded already.
    sharedptr<SharedConnection> sharedconnection = connection_pool->connect();
    g_warning("App_Glom::recreate_database(): Failed because database exists already.");

    return false; //Connection to the database succeeded, because no exception was thrown. so the database exists already.
  }
  catch(const ExceptionConnection& ex)
  {
    if(ex.get_failure_type() == ExceptionConnection::FAILURE_NO_SERVER)
    { 
      user_cancelled = true; //Eventually, the user will cancel after retrying.
      g_warning("App_Glom::recreate_database(): Failed because connection to server failed, without specifying a databse.");
      return false;
    }

    //Otherwise continue, because we expected connect() to fail if the db does not exist yet.
  }

 //Create the database:
  connection_pool->set_database( Glib::ustring() );
  bool db_created = m_pFrame->create_database(db_name, false /* Don't ask for password etc again. */);

  if(!db_created)
  {
    return false;
  }
  else
    connection_pool->set_database(db_name); //Specify the new database when connection from now on.

  sharedptr<SharedConnection> sharedconnection;
  try
  {
    sharedconnection = connection_pool->connect();
    connection_pool->set_database(db_name); //The database was successfully created, so specify it when connection from now on.
  }
  catch(const ExceptionConnection& ex)
  {
    g_warning("App_Glom::recreate_database(): Failed to connect to the newly-created database.");
    return false;
  }

  Bakery::BusyCursor(*this);

  //Create each table:
  Document_Glom::type_listTableInfo tables = pDocument->get_tables();
  for(Document_Glom::type_listTableInfo::const_iterator iter = tables.begin(); iter != tables.end(); ++iter)
  {
    const TableInfo& table_info = *iter;

    //Create SQL to describe all fields in this table:
    Glib::ustring sql_fields;
    Document_Glom::type_vecFields fields = pDocument->get_table_fields(table_info.get_name());

    bool table_creation_succeeded = m_pFrame->create_table(table_info, fields);
    if(!table_creation_succeeded)
    {
      g_warning("App_Glom::recreate_database(): CREATE TABLE failed with the newly-created database.");
      return false;
    }

  } //for(tables)


  m_pFrame->add_standard_tables(); //Add internal, hidden, tables.

  //Create the developer group, and make this user a member of it:
  //If we got this far then the user must really have developer privileges already:
  m_pFrame->add_standard_groups();

  Glib::ustring strQuery = "ALTER GROUP " GLOM_STANDARD_GROUP_NAME_DEVELOPER " ADD USER " + connection_pool->get_user();
  m_pFrame->Query_execute(strQuery);

  return true; //All tables created successfully.
}

  

AppState::userlevels App_Glom::get_userlevel() const
{
  const Document_Glom* document = dynamic_cast<const Document_Glom*>(get_document());
  if(document)
  {
    return document->get_userlevel();
  }
  else
    return AppState::USERLEVEL_DEVELOPER; //This should never happen.
}

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

void App_Glom::fill_menu_tables()
{
  //TODO: There must be a better way than building a ui_string like this:

  m_listNavTableActions.clear();
  if(m_menu_tables_ui_merge_id)
    m_refUIManager->remove_ui(m_menu_tables_ui_merge_id);

  m_refNavTablesActionGroup = Gtk::ActionGroup::create("NavTablesActions");

  Glib::ustring ui_description =
    "<ui>"
    "  <menubar name='Bakery_MainMenu'>"
    "    <placeholder name='Bakery_MenuPH_Others'>"
    "      <menu action='Glom_Menu_Tables'>"
    "        <placeholder name='Menu_Tables_Dynamic'>";

  Document_Glom* document = dynamic_cast<Document_Glom*>(get_document());
  const Document_Glom::type_listTableInfo tables = document->get_tables();
  for(Document_Glom::type_listTableInfo::const_iterator iter = tables.begin(); iter != tables.end(); ++iter)
  {
    const TableInfo& table_info = *iter;
    if(!table_info.m_hidden)
    {
      const Glib::ustring action_name = "NavTableAction_" + table_info.get_name();

      ui_description += "<menuitem action='" + action_name + "' />";

      Glib::RefPtr<Gtk::Action> refAction = Gtk::Action::create(action_name, table_info.get_title_or_name());
      m_refNavTablesActionGroup->add(refAction,
        sigc::bind( sigc::mem_fun(*m_pFrame, &Frame_Glom::on_box_tables_selected), table_info.m_name) );

      m_listNavTableActions.push_back(refAction);

      //m_refUIManager->add_ui(merge_id, path, table_info.m_title, refAction, UI_MANAGER_MENUITEM);
    }
  }

  m_refUIManager->insert_action_group(m_refNavTablesActionGroup);


  ui_description +=
    "     </placeholder>"
    "    </menu>"
    "    </placeholder>"
    "  </menubar>"
    "</ui>";

  //Add menus:
  try
  {
    m_menu_tables_ui_merge_id = m_refUIManager->add_ui_from_string(ui_description);
  }
  catch(const Glib::Error& ex)
  {
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
    "  <menubar name='Bakery_MainMenu'>"
    "    <placeholder name='Bakery_MenuPH_Others'>"
    "     <menu action='Glom_Menu_Reports'>"
    "        <placeholder name='Menu_Reports_Dynamic'>";

  Document_Glom* document = dynamic_cast<Document_Glom*>(get_document());
  const Document_Glom::type_listReports tables = document->get_report_names(table_name);
  for(Document_Glom::type_listReports::const_iterator iter = tables.begin(); iter != tables.end(); ++iter)
  {
    Report report;
    bool found =  document->get_report(table_name, *iter, report);
    if(found)
    {
      const Glib::ustring report_name = report.get_name();
      if(!report_name.empty())
      {
        const Glib::ustring action_name = "NavReportAction_" + report_name;

        ui_description += "<menuitem action='" + action_name + "' />";

        Glib::RefPtr<Gtk::Action> refAction = Gtk::Action::create(action_name, report.get_title_or_name());
        m_refNavReportsActionGroup->add(refAction,
          sigc::bind( sigc::mem_fun(*m_pFrame, &Frame_Glom::on_menu_report_selected), report.m_name) );

        m_listNavReportActions.push_back(refAction);

        //m_refUIManager->add_ui(merge_id, path, table_info.m_title, refAction, UI_MANAGER_MENUITEM);
      }
    }
  }

  m_refUIManager->insert_action_group(m_refNavReportsActionGroup);


  ui_description +=
    "     </placeholder>"
    "    </menu>"
    "    </placeholder>"
    "  </menubar>"
    "</ui>";

  //Add menus:
  try
  {
    m_menu_reports_ui_merge_id = m_refUIManager->add_ui_from_string(ui_description);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << " App_Glom::fill_menu_reports(): building menus failed: " <<  ex.what();
  }
}
