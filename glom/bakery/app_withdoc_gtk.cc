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

#include <glom/bakery/app_withdoc_gtk.h>
#include <glom/bakery/dialog_offersave.h>
//#include <libgnomevfsmm/utils.h> //For escape_path_string()
//#include <libgnomevfsmm/mime-handlers.h> //For type_is_known(). 
#include <gtkmm/toolbutton.h>
#include <gtkmm/stock.h>
#include <gtkmm/recentchoosermenu.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/aboutdialog.h>
#include <giomm.h>
#include <algorithm>

#include <config.h>

#ifdef GLOM_ENABLE_MAEMO
#include <hildon-fmmm/file-chooser-dialog.h>
#include <hildon/hildon-window.h>
#endif // GLOM_ENABLE_MAEMO

#include <glibmm/i18n-lib.h>

namespace GlomBakery
{


//Initialize static member data:

App_WithDoc_Gtk::App_WithDoc_Gtk(const Glib::ustring& appname)
: App_WithDoc(appname),
  m_pVBox(0),
  m_pAbout(0)
{
  init_app_name(appname);
}

/// This constructor can be used with Gtk::Builder::get_derived_widget().
App_WithDoc_Gtk::App_WithDoc_Gtk(BaseObjectType* cobject, const Glib::ustring& appname)
: App_WithDoc(appname),
  ParentWindow(cobject), 
  m_pVBox(0),
  m_pAbout(0)
{
  init_app_name(appname);

  #ifdef GLOM_ENABLE_MAEMO
  //The .glade file needs to specify HildonWindow or features such as HildonAppMenu won't work.
  g_assert(HILDON_IS_WINDOW(gobj()));
  #endif //GLOM_ENABLE_MAEMO
}

  
App_WithDoc_Gtk::~App_WithDoc_Gtk()
{
  if(m_pVBox)
  {
    delete m_pVBox;
    m_pVBox = 0;
  }
  
  if(m_pAbout)
  {
    delete m_pAbout;
    m_pAbout = 0;
  }
}


void App_WithDoc_Gtk::on_hide()
{
  ui_signal_hide().emit();
}

void App_WithDoc_Gtk::ui_hide()
{
  hide();  
}

void App_WithDoc_Gtk::ui_bring_to_front()
{
  get_window()->raise();
}


void App_WithDoc_Gtk::init_layout()
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
  //TODO: Use Hildon::AppMenu
  //Gtk::Menu* pMenu = static_cast<Gtk::Menu*>(m_refUIManager->get_widget("/Bakery_MainMenu"));
  //set_menu(*pMenu);
#else
  Gtk::MenuBar* pMenuBar = static_cast<Gtk::MenuBar*>(m_refUIManager->get_widget("/Bakery_MainMenu"));
  m_pVBox->pack_start(*pMenuBar, Gtk::PACK_SHRINK);
#endif

  add_accel_group(m_refUIManager->get_accel_group());


  //Add placeholder, to be used by add():
  m_pVBox->pack_start(m_VBox_PlaceHolder);
  m_VBox_PlaceHolder.show();
  
  m_pVBox->show(); //Show it last so the child widgets all show at once.
}

void App_WithDoc_Gtk::add_ui_from_string(const Glib::ustring& ui_description)
{
  try
  {
    m_refUIManager->add_ui_from_string(ui_description);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << "building menus failed: " <<  ex.what();
  }
}

void App_WithDoc_Gtk::init()
{  
  App_WithDoc::init(); //Create document and ask to show it in the UI.
  init_layout(); // start setting up layout after we've gotten all widgets from UIManager
  show();
}


void App_WithDoc_Gtk::init_ui_manager()
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

void App_WithDoc_Gtk::init_toolbars()
{
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
}

void App_WithDoc_Gtk::init_menus()
{
  //Override this to add more menus
  init_menus_file();
  init_menus_edit();
  init_menus_help();

}
void App_WithDoc_Gtk::init_menus_file_recentfiles(const Glib::ustring& path)
{
  if(!m_mime_types.empty()) //"Recent-files" is useless unless it knows what documents (which MIME-types) to show.
  {
    //Add recent-files submenu:
    Gtk::MenuItem* pMenuItem = dynamic_cast<Gtk::MenuItem*>(m_refUIManager->get_widget(path));
    if(pMenuItem)
    {
      Gtk::RecentFilter filter;

      //Add the mime-types, so that it only shows those documents:
      for(type_list_strings::iterator iter = m_mime_types.begin(); iter != m_mime_types.end(); ++iter)
      {
        const Glib::ustring mime_type = *iter;

        //TODO: Find a gio equivalent for gnome_vfs_mime_type_is_known(). murrayc.
        filter.add_mime_type(mime_type);
      }

      Gtk::RecentChooserMenu* menu = Gtk::manage(new Gtk::RecentChooserMenu);
      menu->set_filter(filter);
      menu->set_limit(10 /* this should be a global GNOME preference, I think. */);
      menu->set_show_numbers(false);
      menu->set_sort_type(Gtk::RECENT_SORT_MRU);
      menu->signal_item_activated().connect(sigc::bind(sigc::mem_fun(*this, static_cast<void(App_WithDoc_Gtk::*)(Gtk::RecentChooser&)>(&App_WithDoc_Gtk::on_recent_files_activate)), sigc::ref(*menu)));

      pMenuItem->set_submenu(*menu);
    }
    else
    {
      std::cout << "debug: recent files menu not found" << std::endl;
    }
  }
  else
  {
    //std::cout << "debug: " << G_STRFUNC << ": No recent files sub-menu added, because no MIME types are specified." << std::endl;
  }
}

void App_WithDoc_Gtk::init_menus_file()
{
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

  //Remember thes ones for later, so we can disable Save menu and toolbar items:
  m_action_save = Gtk::Action::create("BakeryAction_File_Save", Gtk::Stock::SAVE);
  m_refFileActionGroup->add(m_action_save,
                        sigc::mem_fun((App_WithDoc&)*this, &App_WithDoc::on_menu_file_save));

  m_action_saveas = Gtk::Action::create("BakeryAction_File_SaveAs", Gtk::Stock::SAVE_AS);                   
  m_refFileActionGroup->add(m_action_saveas,
                        sigc::mem_fun((App_WithDoc&)*this, &App_WithDoc::on_menu_file_saveas));
                        
  m_refFileActionGroup->add(Gtk::Action::create("BakeryAction_File_Close", Gtk::Stock::CLOSE),
                        sigc::mem_fun((App_WithDoc&)*this, &App_WithDoc::on_menu_file_close));
                        
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
    "        <menuitem action='BakeryAction_File_Save' />"
    "        <menuitem action='BakeryAction_File_SaveAs' />"
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

void App_WithDoc_Gtk::init_menus_edit()
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

void App_WithDoc_Gtk::init_menus_help()
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


void App_WithDoc_Gtk::on_menu_help_about()
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

void App_WithDoc_Gtk::on_about_close()
{
  m_bAboutShown = false;
}

void App_WithDoc_Gtk::add(Gtk::Widget& child)
{
  m_VBox_PlaceHolder.pack_start(child);
}

bool App_WithDoc_Gtk::on_delete_event(GdkEventAny* /* event */)
{
  //Clicking on the [x] in the title bar should be like choosing File|Close
  on_menu_file_close();

  return true; // true = don't hide, don't destroy
}


void App_WithDoc_Gtk::update_window_title()
{
  //Set application's main window title:

  Document* pDoc = get_document();
  if(!pDoc)
    return;

  Glib::ustring strTitle = m_strAppName;
  strTitle += " - " + pDoc->get_name();

  //Indicate unsaved changes:
  if(pDoc->get_modified())
    strTitle += " *";

  //Indicate read-only files:
  if(pDoc->get_read_only())
    strTitle += _(" (read-only)");

  set_title(strTitle);
}

void App_WithDoc_Gtk::ui_warning(const Glib::ustring& text, const Glib::ustring& secondary_text)
{
  Gtk::Window* pWindow = this;

#ifdef GLOM_ENABLE_MAEMO
  Hildon::Note dialog(Hildon::NOTE_TYPE_INFORMATION, *pWindow, text);
#else
  Gtk::MessageDialog dialog(App_WithDoc_Gtk::util_bold_message(text), true /* use markup */, Gtk::MESSAGE_WARNING);
  dialog.set_secondary_text(secondary_text);

  dialog.set_title(""); //The HIG says that alert dialogs should not have titles. The default comes from the message type.
#endif

  if(pWindow)
    dialog.set_transient_for(*pWindow);

  dialog.run();
}


Glib::ustring App_WithDoc_Gtk::util_bold_message(const Glib::ustring& message)
{
  return "<b>" + message + "</b>";
}

Glib::ustring App_WithDoc_Gtk::ui_file_select_open(const Glib::ustring& starting_folder_uri)
{
  Gtk::Window* pWindow = this;

#ifdef GLOM_ENABLE_MAEMO
  Hildon::FileChooserDialog fileChooser_Open(Gtk::FILE_CHOOSER_ACTION_OPEN);
#else
  Gtk::FileChooserDialog fileChooser_Open(_("Open Document"), Gtk::FILE_CHOOSER_ACTION_OPEN);
  fileChooser_Open.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  fileChooser_Open.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);
  fileChooser_Open.set_default_response(Gtk::RESPONSE_OK);
#endif // GLOM_ENABLE_MAEMO

  if(pWindow)
    fileChooser_Open.set_transient_for(*pWindow);

  if(!starting_folder_uri.empty())
    fileChooser_Open.set_current_folder_uri(starting_folder_uri);

  const int response_id = fileChooser_Open.run();
  fileChooser_Open.hide();
  if(response_id != Gtk::RESPONSE_CANCEL)
  {
    return fileChooser_Open.get_uri();
  }
  else
    return Glib::ustring();
}

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

Glib::ustring App_WithDoc_Gtk::ui_file_select_save(const Glib::ustring& old_file_uri)
{
 Gtk::Window* pWindow = this;

#ifdef GLOM_ENABLE_MAEMO
  Hildon::FileChooserDialog fileChooser_Save(Gtk::FILE_CHOOSER_ACTION_SAVE);
#else
  Gtk::FileChooserDialog fileChooser_Save(_("Save Document"), Gtk::FILE_CHOOSER_ACTION_SAVE);
  fileChooser_Save.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  fileChooser_Save.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);
  fileChooser_Save.set_default_response(Gtk::RESPONSE_OK);
#endif // GLOM_ENABLE_MAEMO

 if(pWindow)
    fileChooser_Save.set_transient_for(*pWindow);

  fileChooser_Save.set_do_overwrite_confirmation(); //Ask the user if the file already exists.

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
        fileChooser_Save.set_uri(uri_parent);
      }
    }
  }

  bool try_again = true;
  while(try_again)
  {
    try_again = false;

    const int response_id = fileChooser_Save.run();
    fileChooser_Save.hide();
    if(response_id != Gtk::RESPONSE_CANCEL)
    {
      const Glib::ustring uri = fileChooser_Save.get_uri();

      Glib::RefPtr<Gio::File> gio_file = Gio::File::create_for_uri(uri);

      //If the file exists (the FileChooser offers a "replace?" dialog, so this is possible.):
      if(App_WithDoc::file_exists(uri))
      {
        //Check whether we have rights to the file to change it:
        //Really, GtkFileChooser should do this for us.
        if(!uri_is_writable(gio_file))
        {
           //Warn the user:
           ui_warning(_("Read-only File."), _("You may not overwrite the existing file, because you do not have sufficient access rights."));
           try_again = true; //Try again.
           continue;
        }
      }

      //Check whether we have rights to the directory, to create a new file in it:
      //Really, GtkFileChooser should do this for us.
      Glib::RefPtr<const Gio::File> gio_file_parent = gio_file->get_parent();
      if(gio_file_parent)
      {
        if(!uri_is_writable(gio_file_parent))
        {
          //Warn the user:
           ui_warning(_("Read-only Directory."), _("You may not create a file in this directory, because you do not have sufficient access rights."));
           try_again = true; //Try again.
           continue;
        }
      }

      if(!try_again)
        return uri;
    }
    else
      return Glib::ustring(); //The user cancelled.
  }

  return Glib::ustring();
}

void App_WithDoc_Gtk::ui_show_modification_status()
{
  const bool modified = m_pDocument->get_modified();

  //Enable Save and SaveAs menu items:
  if(m_action_save)
    m_action_save->set_sensitive(modified);

  if(m_action_saveas)
    m_action_saveas->set_sensitive(modified);
}

App_WithDoc_Gtk::enumSaveChanges App_WithDoc_Gtk::ui_offer_to_save_changes()
{
  App_WithDoc::enumSaveChanges result = App_WithDoc::SAVECHANGES_Cancel;

  if(!m_pDocument)
    return result;

  GlomBakery::Dialog_OfferSave* pDialogQuestion 
    = new GlomBakery::Dialog_OfferSave( m_pDocument->get_file_uri() );

  Gtk::Window* pWindow = this;
  if(pWindow)
    pDialogQuestion->set_transient_for(*pWindow);

  GlomBakery::Dialog_OfferSave::enumButtons buttonClicked = (GlomBakery::Dialog_OfferSave::enumButtons)pDialogQuestion->run();
  delete pDialogQuestion;
  pDialogQuestion = 0;

  if(buttonClicked == GlomBakery::Dialog_OfferSave::BUTTON_Save)
     result = App_WithDoc::SAVECHANGES_Save;
  else if(buttonClicked == GlomBakery::Dialog_OfferSave::BUTTON_Discard)
     result = App_WithDoc::SAVECHANGES_Discard;
  else
     result = App_WithDoc::SAVECHANGES_Cancel;

  return result;
}

void App_WithDoc_Gtk::document_history_add(const Glib::ustring& file_uri)
{
  if(file_uri.empty())
    return;

  //This can sometimes be called for a file that does not yet exist on disk.
  //Avoid warning in RecentManager if that is the case.
  //For instance, Glom does this when the user chooses a new filename, 
  //but before Glom has enough information to save a useful file.
  if(!file_exists(file_uri))
    return;

  {
    //TODO: Wrap gnome_vfs_escape_path_string() in gnome-vfsmm.
    //Glib::ustring filename_e = Gnome::Vfs::escape_path_string(file_uri);
    const Glib::ustring uri = file_uri; // "file://" + filename_e;

    Gtk::RecentManager::get_default()->add_item(uri);
  }
}

void App_WithDoc_Gtk::document_history_remove(const Glib::ustring& file_uri)
{
  if(!file_uri.empty())
  {
    //Glib::ustring filename_e = Gnome::Vfs::escape_path_string(file_uri.c_str());
    const Glib::ustring uri = file_uri; //"file://" + filename_e;

    Gtk::RecentManager::get_default()->remove_item(uri);
  }
}

void App_WithDoc_Gtk::on_recent_files_activate(Gtk::RecentChooser& chooser)
{
  const Glib::ustring uri = chooser.get_current_uri();
  const bool bTest = open_document(uri);
  if(!bTest)
    document_history_remove(uri);
}

} //namespace
