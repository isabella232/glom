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

#include "frame_glom.h"
#include "application.h"
#include "appstate.h"
#include <libintl.h>

Frame_Glom::Frame_Glom(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: PlaceHolder(cobject, refGlade),
  m_pLabel_Table(0),
  m_pLabel_Mode(0),
  m_pLabel_UserLevel(0),
  m_pBox_Mode(0),
  m_pBox_Databases(0),
  m_pBox_Tables(0),
  m_pDialog_Databases(0),
  m_pDialog_Tables(0),
  m_pDialog_Fields(0),
  m_pDialog_Relationships(0)

{
  //Load widgets from glade file:
  refGlade->get_widget("label_table_name", m_pLabel_Table);
  refGlade->get_widget("label_mode", m_pLabel_Mode);
  refGlade->get_widget("label_user_level", m_pLabel_UserLevel);
  refGlade->get_widget_derived("vbox_mode", m_pBox_Mode);

  //m_pLabel_Mode->set_text(gettext("No database selected.\n Use the Navigation menu, or open a previous Glom document."));
  
  //Load the Glade file and instantiate its widgets to get the dialog stuff:
  try
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "box_navigation_connection");

    refXml->get_widget_derived("box_navigation_connection", m_pBox_Databases);
    m_pDialog_Databases = new Dialog_Glom(m_pBox_Databases);
  }
  catch(const Gnome::Glade::XmlError& ex)
  {
    std::cerr << ex.what() << std::endl;
  }

  try
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "box_navigation_tables");

    refXml->get_widget_derived("box_navigation_tables", m_pBox_Tables);
    m_pDialog_Tables = new Dialog_Glom(m_pBox_Tables);
  }
  catch(const Gnome::Glade::XmlError& ex)
  {
    std::cerr << ex.what() << std::endl;
  }

  try
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "window_design");

    refXml->get_widget_derived("window_design", m_pDialog_Fields);
  }
  catch(const Gnome::Glade::XmlError& ex)
  {
    std::cerr << ex.what() << std::endl;
  }

  try
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "window_design");

    refXml->get_widget_derived("window_design", m_pDialog_Relationships);
  }
  catch(const Gnome::Glade::XmlError& ex)
  {
    std::cerr << ex.what() << std::endl;
  }

  m_Mode = MODE_None;
  m_Mode_Previous = MODE_None;

  Gtk::Window* pWindow = get_app_window();
  if(pWindow)
    m_pDialog_Databases->set_transient_for(*pWindow);

  m_pDialog_Databases->get_vbox()->pack_start(*m_pBox_Databases);
  m_pBox_Databases->show_all();

  if(pWindow)
    m_pDialog_Tables->set_transient_for(*pWindow);

  m_pDialog_Tables->get_vbox()->pack_start(*m_pBox_Tables);
  m_pDialog_Tables->set_default_size(300, 400);
  m_pBox_Tables->show_all();

  //Connect signals:
  m_pBox_Databases->signal_selected.connect(sigc::mem_fun(*this, &Frame_Glom::on_Box_Databases_selected));
  m_pBox_Tables->signal_selected.connect(sigc::mem_fun(*this, &Frame_Glom::on_Box_Tables_selected));

  
  m_Notebook_Find.signal_find.connect(sigc::mem_fun(*this, &Frame_Glom::on_Notebook_Find));

  //Fill Composite View:
  //This means that set_document and load/save are delegated to these children:
  add_view(m_pBox_Databases);
  add_view(m_pBox_Tables);
  add_view(m_pDialog_Fields); //Also a composite view.
  add_view(m_pDialog_Relationships); //Also a composite view.
  add_view(&m_Notebook_Data); //Also a composite view.

  on_userlevel_changed(AppState::USERLEVEL_OPERATOR); //A default to show before a document is created or loaded.
  show_all();
}

Frame_Glom::~Frame_Glom()
{
  if(m_pDialog_Databases)
  {
      delete m_pDialog_Databases;
      m_pDialog_Databases = 0;
  }

  if(m_pDialog_Tables)
  {
      delete m_pDialog_Tables;
      m_pDialog_Tables = 0;
  }
}

void Frame_Glom::on_Box_Databases_selected(Glib::ustring strName)
{
  m_pDialog_Databases->hide(); //cause_close();

  get_document()->set_connection_database(strName);

  //Enable more menus:
  App_Glom* pApp = dynamic_cast<App_Glom*>(get_app_window());
  if(pApp)
    pApp->on_database_selected(true);

  on_menu_Navigate_Table();
}

void Frame_Glom::on_Box_Tables_selected(Glib::ustring strName)
{
  m_pDialog_Tables->hide(); //cause_close();

  show_table(strName);
}

void Frame_Glom::set_mode_widget(Gtk::Widget& widget)
{
  //Remove current contents.
  //I wish that there was a better way to do this:
  //Trying to remove all of them leads to warnings,
  //and I don't see a way to get a list of children.

  App_Glom* pApp = dynamic_cast<App_Glom*>(get_app_window());
  if(pApp)
  {
    Glib::RefPtr<Gtk::UIManager> ui_manager = pApp->get_ui_manager();

    Notebook_Glom* notebook_current = dynamic_cast<Notebook_Glom*>(m_pBox_Mode->get_child());
    if(notebook_current)
    {
      m_pBox_Mode->remove();
    }

    m_pBox_Mode->add(widget);
    widget.show();

    //Show help text:
    Notebook_Glom* pNotebook = dynamic_cast<Notebook_Glom*>(&widget);
    if(pNotebook)
    {
      pNotebook->show_hint();
    }
  }
}

bool Frame_Glom::set_mode(enumModes mode)
{
  bool bChanged = (m_Mode != mode);

  //Choose a default mode, if necessary:
  if(mode == MODE_None)
    mode = MODE_Data;

  m_Mode_Previous = m_Mode;
  m_Mode = mode;

  return bChanged;
}

void Frame_Glom::alert_no_table()
{
  //Ask user to choose a table first:
  show_ok_dialog(gettext("You must choose a database table first.\n Use the Navigation menu, or load a previous document."));
}

void Frame_Glom::show_table(const Glib::ustring& strTableName)
{
  //Check that there is a table to show:
  if(strTableName.empty())
  {
    alert_no_table();
  }
  else
  {
    //Choose a default mode, if necessary:
    if(m_Mode == MODE_None)
      set_mode(m_Mode);

    //Show the table:
    m_strTableName = strTableName;
    Glib::ustring strMode;

    //Update the document with any new information in the database if necessary (though the database _should never have changed information)
    update_table_in_document_from_database();

    //Update user-level dependent UI:
    App_Glom* pApp = dynamic_cast<App_Glom*>(get_app_window());
    if(pApp)
      on_userlevel_changed(pApp->get_userlevel());
    
    switch(m_Mode)
    {
      case(MODE_Data):
      {
        strMode = gettext("Data");
        m_Notebook_Data.init_db_details( m_pBox_Databases->get_database_name(), m_strTableName);
        set_mode_widget(m_Notebook_Data);
        break;
      }
      case(MODE_Find):
      {
        strMode = gettext("Find");
        m_Notebook_Find.init_db_details( m_pBox_Databases->get_database_name(), m_strTableName);
        set_mode_widget(m_Notebook_Find);
        break;
      }
      default:
      {
        std::cout << "Frame_Glom::on_Box_Tables_selected(): Unexpected mode" << std::endl;
        strMode = gettext("Unknown");
        break;
      }
    }

    m_pLabel_Mode->set_text(strMode);
  }

  show_table_title();
  
  show_all();
}

void Frame_Glom::on_menu_UserLevel_Developer(Glib::RefPtr<Gtk::RadioAction> action)
{
  if(action->get_active())
  {
    Document_Glom* document = dynamic_cast<Document_Glom*>(get_document());
    if(document)
    {
      //Avoid double signals:
      //if(document->get_userlevel() != AppState::USERLEVEL_DEVELOPER)
        document->set_userlevel(AppState::USERLEVEL_DEVELOPER);
    }
  }
}

void Frame_Glom::on_menu_UserLevel_Operator(Glib::RefPtr<Gtk::RadioAction> action)
{
  if(action->get_active())
  {
    Document_Glom* document = dynamic_cast<Document_Glom*>(get_document());
    if(document)
    {
      //Avoid double signals:
      //if(document->get_userlevel() != AppState::USERLEVEL_OPERATOR)
        document->set_userlevel(AppState::USERLEVEL_OPERATOR);
    }
  }
}

void Frame_Glom::on_menu_Mode_Data()
{
  if(set_mode(MODE_Data))
    show_table(m_strTableName);
}

void Frame_Glom::on_menu_Mode_Find()
{
  if(set_mode(MODE_Find))
    show_table(m_strTableName);
}

void Frame_Glom::on_menu_Navigate_Database()
{
  do_menu_Navigate_Database();
}

void Frame_Glom::do_menu_Navigate_Database(bool bUseList /* = true */)
{
  m_pBox_Databases->set_use_list(bUseList);
  m_pBox_Databases->load_from_document();

  //guint dialog_height = 400;
  bool bAutoShrink = false;
  if(!bUseList)
    bAutoShrink = true;

  //m_pDialog_Databases->set_default_size(300, 400);

  //m_pDialog_Databases->set_policy(false, true, bAutoShrink);  //TPDO_port
  m_pDialog_Databases->show();
  //m_frame.remove();
  //m_frame.add(*m_pBox_Databases);
  //m_frame.show_all();
}

void Frame_Glom::on_menu_Navigate_Table()
{
  if(get_document()->get_connection_database().size() == 0)
  {
    show_ok_dialog(gettext("You must choose a database first.\n Use the Navigation|Database menu item, or load a previous document."));
  }
  else
  {
    m_pBox_Tables->init_db_details( get_document()->get_connection_database());
    //m_pDialog_Tables->set_policy(false, true, false); //TODO_port
    m_pDialog_Tables->show();
  }
}

const Gtk::Window* Frame_Glom::get_app_window() const
{
  Frame_Glom* nonconst = const_cast<Frame_Glom*>(this);
  return nonconst->get_app_window();
}

Gtk::Window* Frame_Glom::get_app_window()
{
  Gtk::Widget* pWidget = get_parent();
  while(pWidget)
  {
    //Is this widget a Gtk::Window?:
    Gtk::Window* pWindow = dynamic_cast<Gtk::Window*>(pWidget);
    if(pWindow)
    {
      //Yes, return it.
      return pWindow;
    }
    else
    {
      //Try the parent's parent:
      pWidget = pWidget->get_parent();
    }
  }

  return 0; //not found.

}

void Frame_Glom::show_ok_dialog(const Glib::ustring& strMessage)
{
  //I wrote a method for this so that I wouldn't have to repeat this code.
  //However, not all of this code is really necessary,
  //because get_app_window() should always succeed.

  Gtk::MessageDialog dialog(strMessage);

  Gtk::Window* pWindowApp = get_app_window();
  if(pWindowApp)
   dialog.set_transient_for(*pWindowApp);

  dialog.run();
}

void Frame_Glom::on_Notebook_Find(Glib::ustring strWhereClause)
{
  on_menu_Mode_Data();
  m_Notebook_Data.init_db_details( m_pBox_Databases->get_database_name(), m_strTableName, strWhereClause);
  m_Notebook_Data.select_page_for_find_results();
}

void Frame_Glom::on_userlevel_changed(AppState::userlevels userlevel)
{
  //show user level:
  Glib::ustring user_level_name = gettext("Operator");
  if(userlevel == AppState::USERLEVEL_DEVELOPER)
    user_level_name = gettext("Developer");

    
  if(m_pLabel_UserLevel)
    m_pLabel_UserLevel->set_text(user_level_name);

  show_table_title(); 
}

void Frame_Glom::show_table_title()
{
  if(get_document())
  {
    //Show the table title:
    Glib::ustring table_label = get_document()->get_table_title(m_strTableName);
    if(!table_label.empty())
    {
      Document_Glom* document = dynamic_cast<Document_Glom*>(get_document());
      if(document)
      {
        if(document->get_userlevel() == AppState::USERLEVEL_DEVELOPER)
          table_label += " (" + m_strTableName + ")"; //Show the table name as well, if in developer mode.
      }
    }
    else //Use the table name if there is no table title.
      table_label = m_strTableName;

    m_pLabel_Table->set_text(table_label);
  }
}

void Frame_Glom::update_table_in_document_from_database()
{
  //Add any new/changed information from the database to the document
  //The database should never change without the knowledge of the document anyway, so this should be unnecessary.

  //TODO_performance: There are a lot of temporary Field and FieldAttributes instances here, with a lot of string copying.
  
  //For instance, changed field details, or new fields, or removed fields.
  typedef Box_DB_Table::type_vecFields type_vecFields;

  //Get the fields information from the database:
  type_vecFields fieldsDatabase = Box_DB_Table::get_fields_for_table_from_database(m_strTableName);

  Document_Glom* pDoc = dynamic_cast<const Document_Glom*>(get_document());
  if(pDoc)
  {
    bool document_must_to_be_updated = false;
    
    //Get the fields information from the document.
    //and add to, or update Document's list of fields:
    type_vecFields fieldsDocument = pDoc->get_table_fields(m_strTableName);
    
    for(type_vecFields::const_iterator iter = fieldsDatabase.begin(); iter != fieldsDatabase.end(); ++iter)
    {
      const Field& field_database = *iter;

      //Is the field already in the document?
      type_vecFields::iterator iterFindDoc = std::find_if( fieldsDocument.begin(), fieldsDocument.end(), predicate_FieldHasName<Field>( field_database.get_name() ) );
      if(iterFindDoc == fieldsDocument.end()) //If it was not found:
      {
        //Add it
        fieldsDocument.push_back(field_database);
        document_must_to_be_updated = true;
      }
      else //if it was found.
      {
        //Compare the information:
        Gnome::Gda::FieldAttributes field_info_db =  field_database.get_field_info();
        Field field_document =  *iterFindDoc;
        if(field_document.field_info_from_database_is_equal( field_info_db )) //ignores auto_increment because libgda does not report it from the database properly.
        {
          //The database has different information. We assume that the information in the database is newer.

          //Update the field information:
          field_info_db.set_auto_increment( field_document.get_field_info().get_auto_increment() ); //libgda does not report it from the database properly.
          iterFindDoc->set_field_info( field_info_db );

          document_must_to_be_updated = true;
        }
      }
    }

    //Remove fields that are no longer in the database:
    //TODO_performance: This is incredibly inefficient - but it's difficut to erase() items while iterating over them.
    type_vecFields fieldsActual;
    for(type_vecFields::const_iterator iter = fieldsDocument.begin(); iter != fieldsDocument.end(); ++iter)
    {
      const Field& field = *iter;

      //Check whether it's in the database:
      type_vecFields::iterator iterFindDatabase = std::find_if( fieldsDatabase.begin(), fieldsDatabase.end(), predicate_FieldHasName<Field>( field.get_name() ) );
      if(iterFindDatabase != fieldsDatabase.end()) //If it was found
      {
        fieldsActual.push_back(field);
      }
      else
        document_must_to_be_updated = true; //Something changed.
      
    }
          
    if(document_must_to_be_updated)
      pDoc->set_table_fields(m_strTableName, fieldsActual);
  }
}

void Frame_Glom::set_document(Document_Glom* pDocument)
{
  View_Composite_Glom::set_document(pDocument);

  if(m_pDocument)
  {
    //Connect to a signal that is only on the derived document class:
    m_pDocument->signal_userlevel_changed().connect( sigc::mem_fun(*this, &Frame_Glom::on_userlevel_changed) );

    //Show the appropriate UI for the user level that is specified by this new document:
    on_userlevel_changed(m_pDocument->get_userlevel());
  }
}

void Frame_Glom::load_from_document()
{
  Document_Glom* document = dynamic_cast<Document_Glom*>(get_document());
  if(document)
  {
    //Call base class:
    View_Composite_Glom::load_from_document();
  }
}

void Frame_Glom::on_menu_developer_fields()
{
  //Check that there is a table to show:
  if(m_strTableName.empty())
  {
    alert_no_table(); //TODO: Disable the menu item instead.
  }
  else
  {
    m_pDialog_Fields->set_transient_for(*get_app_window());
    m_pDialog_Fields->init_db_details( m_pBox_Databases->get_database_name(), m_strTableName);
    m_pDialog_Fields->show();
  }
}

void Frame_Glom::on_menu_developer_relationships()
{
  //Check that there is a table to show:
  if(m_strTableName.empty())
  {
    alert_no_table(); //TODO: Disable the menu item instead.
  }
  else
  {
    m_pDialog_Relationships->set_transient_for(*get_app_window());
    m_pDialog_Relationships->init_db_details( m_pBox_Databases->get_database_name(), m_strTableName);
    m_pDialog_Relationships->show();
  }
}

void Frame_Glom::on_menu_developer_users()
{

}

void Frame_Glom::on_menu_developer_layout()
{
  //Check that there is a table to show:
  if(m_strTableName.empty())
  {
    alert_no_table(); //TODO: Disable the menu item instead.
  }
  else
  {
    Notebook_Glom* notebook_current = dynamic_cast<Notebook_Glom*>(m_pBox_Mode->get_child());
    if(notebook_current)
      notebook_current->do_menu_developer_layout();
  }
}


