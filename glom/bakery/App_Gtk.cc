/*
 * Copyright 2000 Murray Cumming
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <config.h>

#include <glom/bakery/App_Gtk.h>
#include <gtkmm/stock.h>
#include <gtkmm/aboutdialog.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/toolbutton.h>
#include <glibmm/i18n-lib.h>
#include <algorithm>
#include <iostream>

namespace GlomBakery
{

//Initialize static member data:


Gtk::Window* App_Gtk::m_pAbout = 0;

App_Gtk::App_Gtk(const Glib::ustring& appname)
: m_pVBox(0)
{
  init_app_name(appname);

#ifndef GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED
  signal_hide().connect(sigc::mem_fun(*this, &App_Gtk::on_hide));
  signal_delete_event().connect(sigc::mem_fun(*this, &App_Gtk::on_delete_event));
#endif
}

/// This constructor can be used with Gtk::Builder::get_derived_widget().
App_Gtk::App_Gtk(BaseObjectType* cobject, const Glib::ustring& appname)
: ParentWindow(cobject),
  m_pVBox(0)
{
  init_app_name(appname);  

#ifndef GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED
  signal_hide().connect(sigc::mem_fun(*this, &App_Gtk::on_hide));
  signal_delete_event().connect(sigc::mem_fun(*this, &App_Gtk::on_delete_event));
#endif
}
  

App_Gtk::~App_Gtk()
{
  if(m_pVBox)
  {
    delete m_pVBox;
    m_pVBox = 0;
  }
  
  //If this was the last instance:
  if(m_AppInstanceManager.get_app_count() == 0)
  {
    //Delete shared static widgets if this was the last instance:
    if(m_pAbout)
    {
      delete m_pAbout;
      m_pAbout = 0;
    }
  }
}

void App_Gtk::on_hide()
{
  ui_signal_hide().emit();
}

void App_Gtk::ui_hide()
{
  hide();  
}

void App_Gtk::ui_bring_to_front()
{
  get_window()->raise();
}
  
void App_Gtk::init_layout()
{
  set_resizable(); //resizable
  set_default_size(640, 400); //A sensible default.

  //This might have been instantiated by Gtk::Builder::get_widget() instead.
  //If not, then we create a default one and add it to the window.
  if(!m_pVBox)
  {
    m_pVBox = new Gtk::VBox();
    Gtk::Window::add(*m_pVBox);
  }

  //Add menu bar at the top:
  //These were defined in init_uimanager().
#ifdef GLOM_ENABLE_MAEMO
  Gtk::Menu* pMenu = static_cast<Gtk::Menu*>(m_refUIManager->get_widget("/Bakery_MainMenu"));
  set_menu(*pMenu);
#else
  Gtk::MenuBar* pMenuBar = static_cast<Gtk::MenuBar*>(m_refUIManager->get_widget("/Bakery_MainMenu"));
  m_pVBox->pack_start(*pMenuBar, Gtk::PACK_SHRINK);
#endif

  Gtk::Toolbar* pToolBar = static_cast<Gtk::Toolbar*>(m_refUIManager->get_widget("/Bakery_ToolBar"));
  if(pToolBar)
  {
#ifdef GLOM_ENABLE_MAEMO
    add_toolbar(*pToolBar);
#else
    m_HandleBox_Toolbar.add(*pToolBar);
    m_HandleBox_Toolbar.show();
    m_pVBox->pack_start(m_HandleBox_Toolbar, Gtk::PACK_SHRINK);
#endif
  }

  add_accel_group(m_refUIManager->get_accel_group());


  //Add placeholder, to be used by add():
  m_pVBox->pack_start(m_VBox_PlaceHolder);
  m_VBox_PlaceHolder.show();
  
  m_pVBox->show(); //Show it last so the child widgets all show at once.
}

void App_Gtk::add_ui_from_string(const Glib::ustring& ui_description)
{
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
    m_refUIManager->add_ui_from_string(ui_description);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << "building menus failed: " <<  ex.what();
  }
#else
  std::auto_ptr<Glib::Error> error;
  m_refUIManager->add_ui_from_string(ui_description, error);
  if(error.get() != NULL) std::cerr << "building menus failed: " << error->what();
#endif
}

void App_Gtk::init()
{
  App::init();
  init_layout(); // start setting up layout after we've gotten all widgets from UIManager
  show();
}

void App_Gtk::init_ui_manager()
{
  using namespace Gtk;

  m_refUIManager = UIManager::create();

  //This is just a skeleton structure.
  //The placeholders allow us to merge the menus and toolbars in later,
  //by adding a us string with one of the placeholders, but with menu items underneath it.
  static const Glib::ustring ui_description =
    "<ui>"
#ifdef GLOM_ENABLE_MAEMO
    "  <popup name='Bakery_MainMenu'>"
#else
    "  <menubar name='Bakery_MainMenu'>"
#endif
    "    <placeholder name='Bakery_MenuPH_File' />"
    "    <placeholder name='Bakery_MenuPH_Edit' />"
    "    <placeholder name='Bakery_MenuPH_Others' />" //Note that extra menus should be inserted before the Help menu, which should always be at the end.
    "    <placeholder name='Bakery_MenuPH_Help' />"
#ifdef GLOM_ENABLE_MAEMO
    "  </popup>"
#else
    "  </menubar>"
#endif
    "  <toolbar name='Bakery_ToolBar'>"
    "    <placeholder name='Bakery_ToolBarItemsPH' />"
    "  </toolbar>"
    "</ui>";
  
  add_ui_from_string(ui_description);
}

void App_Gtk::init_menus()
{
  //Override this to add more menus
  init_menus_file();
  init_menus_edit();
  init_menus_help();

}

void App_Gtk::add(Gtk::Widget& child)
{
  m_VBox_PlaceHolder.pack_start(child);
}


void App_Gtk::init_toolbars()
{
  //Build part of the toolbar structure, to be merged in by using the "PH" placeholders:
  static const Glib::ustring ui_description =
    "<ui>"
    "  <toolbar name='Bakery_ToolBar'>"
    "    <placeholder name='Bakery_ToolBarItemsPH'>"
    "      <toolitem action='BakeryAction_File_New' />"
    "    </placeholder>"
    "  </toolbar>"
    "</ui>";

  add_ui_from_string(ui_description);
}


bool App_Gtk::on_delete_event(GdkEventAny* /* event */)
{
  //Clicking on the [x] in the title bar should be like choosing File|New
  on_menu_file_close();

  return true; // true = don't hide, don't destroy
}

void App_Gtk::init_menus_file()
{
  using namespace Gtk;
  // File menu

  //Build actions:
  m_refFileActionGroup = ActionGroup::create("BakeryFileActions");
  m_refFileActionGroup->add(Action::create("BakeryAction_Menu_File", _("File")));

  m_refFileActionGroup->add(Action::create("BakeryAction_File_New", Gtk::Stock::NEW),
                        sigc::mem_fun((GlomBakery::App&)*this, &App_Gtk::on_menu_file_new));
  m_refFileActionGroup->add(Action::create("BakeryAction_File_Close", Gtk::Stock::CLOSE),
                        sigc::mem_fun((GlomBakery::App&)*this, &App_Gtk::on_menu_file_close));
  m_refFileActionGroup->add(Action::create("BakeryAction_File_Exit", Gtk::Stock::QUIT),
                        sigc::mem_fun((GlomBakery::App&)*this, &App_Gtk::on_menu_file_exit));
  m_refUIManager->insert_action_group(m_refFileActionGroup);

  //Build part of the menu structure, to be merged in by using the "PH" placeholders:
  static const Glib::ustring ui_description =
    "<ui>"
#ifdef GLOM_ENABLE_MAEMO
    "  <popup name='Bakery_MainMenu'>"
#else
    "  <menubar name='Bakery_MainMenu'>"
#endif
    "    <placeholder name='Bakery_MenuPH_File'>" //this has to have the same name as the placeholder above
    "      <menu action='BakeryAction_Menu_File'>" 
    "        <menuitem action='BakeryAction_File_New' />"
    "        <menuitem action='BakeryAction_File_Close' />"
    "        <menuitem action='BakeryAction_File_Exit' />"
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

void App_Gtk::init_menus_edit()
{
  using namespace Gtk;
  //Edit menu
  
  //Build actions:
  m_refEditActionGroup = ActionGroup::create("BakeryEditActions");
  m_refEditActionGroup->add(Action::create("BakeryAction_Menu_Edit", _("_Edit")));
  
  m_refEditActionGroup->add(Action::create("BakeryAction_Edit_Cut", Gtk::Stock::CUT));
  m_refEditActionGroup->add(Action::create("BakeryAction_Edit_Copy", Gtk::Stock::COPY));
  m_refEditActionGroup->add(Action::create("BakeryAction_Edit_Paste", Gtk::Stock::PASTE));
  m_refEditActionGroup->add(Action::create("BakeryAction_Edit_Clear", Gtk::Stock::CLEAR));

  m_refUIManager->insert_action_group(m_refEditActionGroup);
  
  //Build part of the menu structure, to be merged in by using the "PH" placeholders:
  static const Glib::ustring ui_description =
    "<ui>"
#ifdef GLOM_ENABLE_MAEMO
    "  <popup name='Bakery_MainMenu'>"
#else
    "  <menubar name='Bakery_MainMenu'>"
#endif
    "    <placeholder name='Bakery_MenuPH_Edit'>"
    "      <menu action='BakeryAction_Menu_Edit'>"
    "        <menuitem action='BakeryAction_Edit_Cut' />"
    "        <menuitem action='BakeryAction_Edit_Copy' />"
    "        <menuitem action='BakeryAction_Edit_Paste' />"
    "        <menuitem action='BakeryAction_Edit_Clear' />"
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

void App_Gtk::init_menus_help()
{
  using namespace Gtk;
  //Help menu

  //Build actions:
  m_refHelpActionGroup = ActionGroup::create("BakeryHelpActions");
  m_refHelpActionGroup->add(Action::create("BakeryAction_Menu_Help", _("_Help")));

  //TODO: Use stock?
  m_refHelpActionGroup->add(Action::create("BakeryAction_Help_About",
                        _("_About"), _("About the application")),
                        sigc::mem_fun((GlomBakery::App&)*this, &App::on_menu_help_about));

  m_refUIManager->insert_action_group(m_refHelpActionGroup);

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
    "        <menuitem action='BakeryAction_Help_About' />"
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


void App_Gtk::on_menu_help_about()
{
  if(m_pAbout && m_bAboutShown) // "About" box hasn't been closed, so just raise it
  {
    m_pAbout->set_transient_for(*this);

    Glib::RefPtr<Gdk::Window> about_win = m_pAbout->get_window();
    about_win->show();
    about_win->raise();
  }
  else
  {
    //Re-create About box:
    if(m_pAbout)
    {
      delete m_pAbout;
      m_pAbout = 0;
    }

    Gtk::AboutDialog* pDerived = new Gtk::AboutDialog;
    m_pAbout = pDerived;

    pDerived->set_name(m_strAppName);
    pDerived->set_version(m_HelpInfo.m_strVersion);
    pDerived->set_copyright(m_HelpInfo.m_strCopyright);
    pDerived->set_authors(m_HelpInfo.m_vecAuthors);
    pDerived->set_documenters(m_HelpInfo.m_vecDocumenters);
    pDerived->set_translator_credits(m_HelpInfo.m_strTranslatorCredits);

    m_pAbout->signal_hide().connect(sigc::mem_fun((App&)*this, &App::on_about_close));
    m_bAboutShown = true;
    static_cast<Gtk::Dialog*>(m_pAbout)->run(); //show() would be better. see below:
    m_pAbout->hide();
    //m_pAbout->show(); //TODO: respond to the OK button.
  }
}

void App_Gtk::on_about_close()
{
  m_bAboutShown = false;
}

Glib::ustring App_Gtk::util_bold_message(const Glib::ustring& message)
{
  return "<b>" + message + "</b>";
}



} //namespace
