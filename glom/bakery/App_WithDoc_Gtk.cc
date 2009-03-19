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

#include <glom/bakery/App_WithDoc_Gtk.h>
#include <glom/bakery/GtkDialogs.h>
//#include <libgnomevfsmm/utils.h> //For escape_path_string()
//#include <libgnomevfsmm/mime-handlers.h> //For type_is_known(). 
#include <gtkmm/toolbutton.h>
#include <gtkmm/stock.h>
#ifdef GTKMM_GEQ_2_10
#include <gtkmm/recentchoosermenu.h>
#endif // GTKMM_GEQ_2_10
#include <algorithm>
#include <glibmm/i18n-lib.h>

//#include <gtk/gtkfilesel.h>


namespace GlomBakery
{


//Initialize static member data:

App_WithDoc_Gtk::App_WithDoc_Gtk(const Glib::ustring& appname)
: App_WithDoc(appname),
  App_Gtk(appname)
{
}

/// This constructor can be used with Gtk::Builder::get_derived_widget().
App_WithDoc_Gtk::App_WithDoc_Gtk(BaseObjectType* cobject, const Glib::ustring& appname)
: ParentWindow(cobject), //This is a virtual base class (not a direct base), so we must specify a constructor or the default constructor will be called, regardless of what the App_Gtk(cobject) constructor does. Derived classes must do this as well.
  App_WithDoc(appname),
  App_Gtk(cobject, appname)
{
  //TODO: appname.
}

  
App_WithDoc_Gtk::~App_WithDoc_Gtk()
{
}


void App_WithDoc_Gtk::init()
{  
  App_WithDoc::init(); //Create document and ask to show it in the UI.
  
  init_layout();
    
  show();
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

void App_WithDoc_Gtk::init_menus_file_recentfiles(const Glib::ustring& path)
{
  if(!m_mime_types.empty()) //"Recent-files" is useless unless it knows what documents (which MIME-types) to show.
  {
    //Add recent-files submenu:
    Gtk::MenuItem* pMenuItem = dynamic_cast<Gtk::MenuItem*>(m_refUIManager->get_widget(path));
    if(pMenuItem)
    {
#ifdef GTKMM_GEQ_2_10
      Gtk::RecentFilter filter;

      //Add the mime-types, so that it only shows those documents:
      for(type_list_strings::iterator iter = m_mime_types.begin(); iter != m_mime_types.end(); ++iter)
      {
        const Glib::ustring mime_type = *iter;

        //TODO: Find a gio equivalent for gnome_vfs_mime_type_is_known(). murrayc.
//#ifndef G_OS_WIN32
//        if( Gnome::Vfs::Mime::type_is_known(mime_type) )
//#endif // !G_OS_WIN32
//        {
          filter.add_mime_type(mime_type);
//        }
//#ifndef G_OS_WIN32
//        else
//        {
//          g_warning("App_WithDoc_Gtk::init_menus_file_recentfiles(): MIME-type %s is not known to gnome-vfs", mime_type.c_str());
//        }
//#endif // !G_OS_WIN32
      }

      Gtk::RecentChooserMenu* menu = Gtk::manage(new Gtk::RecentChooserMenu);
      menu->set_filter(filter);
      menu->set_limit(10 /* this should be a global GNOME preference, I think. */);
      menu->set_show_numbers(false);
      menu->set_sort_type(Gtk::RECENT_SORT_MRU);
      menu->signal_item_activated().connect(sigc::bind(sigc::mem_fun(*this, static_cast<void(App_WithDoc_Gtk::*)(Gtk::RecentChooser&)>(&App_WithDoc_Gtk::on_recent_files_activate)), sigc::ref(*menu)));

      pMenuItem->set_submenu(*menu);
#else
      // TODO: Resurrect libegg? Ignore?
#endif // GTKMM_GEQ_2_10
    }
    else
    {
      std::cout << "debug: recent files menu not found" << std::endl;
    }
  }
  else
  {
    //std::cout << "debug: GlomBakery::App_WithDoc_Gtk::init_menus_file_recentfiles(): No recent files sub-menu added, because no MIME types are specified." << std::endl;
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


void App_WithDoc_Gtk::update_window_title()
{
  //Set application's main window title:

  Glib::ustring strTitle = m_strAppName;
  Document* pDoc = get_document();
  if(pDoc)
  {
    strTitle += " - " + pDoc->get_name();

    //Indicate unsaved changes:
    if(pDoc->get_modified())
      strTitle += " *";

    //Indicate read-only files:
    if(pDoc->get_read_only())
      strTitle += _(" (read-only)");

    set_title(strTitle);
  }
}

void App_WithDoc_Gtk::ui_warning(const Glib::ustring& text, const Glib::ustring& secondary_text)
{
  GtkDialogs::ui_warning(*this, text, secondary_text);
}

Glib::ustring App_WithDoc_Gtk::ui_file_select_open(const Glib::ustring& starting_folder_uri)
{
  return GtkDialogs::ui_file_select_open(*this, starting_folder_uri);
}

Glib::ustring App_WithDoc_Gtk::ui_file_select_save(const Glib::ustring& old_file_uri)
{
  return GtkDialogs::ui_file_select_save(*this, old_file_uri);
}

void App_WithDoc_Gtk::ui_show_modification_status()
{
  bool modified = m_pDocument->get_modified();

  //Enable Save and SaveAs menu items:
  if(m_action_save)
    g_object_set(G_OBJECT(m_action_save->gobj()), "sensitive", modified, NULL); // TODO: Use a set_sensitive(modified)?

  if(m_action_saveas)
    g_object_set(G_OBJECT(m_action_saveas->gobj()), "sensitive", modified, NULL); // TODO: Use a set_sensitive(modified)?

}

App_WithDoc_Gtk::enumSaveChanges App_WithDoc_Gtk::ui_offer_to_save_changes()
{
  return GtkDialogs::ui_offer_to_save_changes(*this, m_pDocument->get_file_uri());
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

#ifdef GLIBMM_EXCEPTIONS_ENABLED
    Gtk::RecentManager::get_default()->add_item(uri);
#else
    std::auto_ptr<Glib::Error> error;
    Gtk::RecentManager::get_default()->add_item(uri, error);
    // Ignore error
#endif
  }
}

void App_WithDoc_Gtk::document_history_remove(const Glib::ustring& file_uri)
{
  if(!file_uri.empty())
  {
    //Glib::ustring filename_e = Gnome::Vfs::escape_path_string(file_uri.c_str());
    const Glib::ustring uri = file_uri; //"file://" + filename_e;

#ifdef GLIBMM_EXCEPTIONS_ENABLED
    Gtk::RecentManager::get_default()->remove_item(uri);
#else
    std::auto_ptr<Glib::Error> error;
    Gtk::RecentManager::get_default()->remove_item(uri, error);
    // Ignore error
#endif
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
