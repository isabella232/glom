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
#include "mode_design/users/dialog_groups_list.h"
#include "dialog_database_preferences.h"
#include "dialog_layout_report.h"
#include "utils.h"
#include "data_structure/glomconversions.h"
#include "data_structure/layout/report_parts/layoutitem_summary.h"
#include "data_structure/layout/report_parts/layoutitem_fieldsummary.h"
#include "relationships_overview/dialog_relationships_overview.h"
#include <glibmm/i18n.h>

Frame_Glom::Frame_Glom(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: PlaceHolder(cobject, refGlade),
  m_pLabel_Name(0),
  m_pLabel_Table(0),
  m_pLabel_Mode(0),
  m_pLabel_userlevel(0),
  m_pBox_Mode(0),
  m_pBox_Tables(0),
  m_pBox_Reports(0),
  m_pDialog_Tables(0),
  m_pDialog_Reports(0),
  m_pDialog_Fields(0),
  m_pDialog_Relationships(0),
  m_pDialogConnection(0),
  m_pDialogConnectionFailed(0),
  m_pDialogLayoutReport(0)
{
  //Load widgets from glade file:
  refGlade->get_widget("label_name", m_pLabel_Name);
  refGlade->get_widget("label_table_name", m_pLabel_Table);
  refGlade->get_widget("label_mode", m_pLabel_Mode);
  refGlade->get_widget("label_user_level", m_pLabel_userlevel);
  refGlade->get_widget_derived("vbox_mode", m_pBox_Mode);

  //m_pLabel_Mode->set_text(_("No database selected.\n Use the Navigation menu, or open a previous Glom document."));

  //Load the Glade file and instantiate its widgets to get the dialog stuff:

  try
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_connection");

    refXml->get_widget_derived("dialog_connection", m_pDialogConnection);
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
    
    //Respond to window close:
    m_pDialog_Tables->signal_hide().connect(sigc::mem_fun(*this, &Frame_Glom::on_dialog_tables_hide));
  }
  catch(const Gnome::Glade::XmlError& ex)
  {
    std::cerr << ex.what() << std::endl;
  }

  try
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "box_reports");

    refXml->get_widget_derived("box_reports", m_pBox_Reports);
    m_pDialog_Reports = new Dialog_Glom(m_pBox_Reports);
    //add_view(m_pBox_Reports);
  }
  catch(const Gnome::Glade::XmlError& ex)
  {
    std::cerr << ex.what() << std::endl;
  }

  try
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "window_report_layout");

    refXml->get_widget_derived("window_report_layout", m_pDialogLayoutReport);

    add_view(m_pDialogLayoutReport);
    m_pDialogLayoutReport->signal_hide().connect( sigc::mem_fun(*this, &Frame_Glom::on_dialog_layout_report_hide) );

    m_pDialog_Reports = new Dialog_Glom(m_pBox_Reports);
    m_pDialog_Reports->signal_hide().connect( sigc::mem_fun(*this, &Frame_Glom::on_dialog_reports_hide) );
  }
  catch(const Gnome::Glade::XmlError& ex)
  {
    std::cerr << ex.what() << std::endl;
  }

  try
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "window_design");

    refXml->get_widget_derived("window_design", m_pDialog_Fields);
    m_pDialog_Fields->signal_hide().connect( sigc::mem_fun(*this, &Frame_Glom::on_developer_dialog_hide));
  }
  catch(const Gnome::Glade::XmlError& ex)
  {
    std::cerr << ex.what() << std::endl;
  }

  try
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "window_design");

    refXml->get_widget_derived("window_design", m_pDialog_Relationships);
    m_pDialog_Relationships->set_title("Relationships");
    m_pDialog_Relationships->signal_hide().connect( sigc::mem_fun(*this, &Frame_Glom::on_developer_dialog_hide));
  }
  catch(const Gnome::Glade::XmlError& ex)
  {
    std::cerr << ex.what() << std::endl;
  }

  try
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_error_connection");

    refXml->get_widget("dialog_error_connection", m_pDialogConnectionFailed);
  }
  catch(const Gnome::Glade::XmlError& ex)
  {
    std::cerr << ex.what() << std::endl;
  }


  m_Mode = MODE_None;
  m_Mode_Previous = MODE_None;

  Gtk::Window* pWindow = get_app_window();

  if(pWindow)
    m_pDialog_Tables->set_transient_for(*pWindow);

  m_pDialog_Tables->get_vbox()->pack_start(*m_pBox_Tables);
  m_pDialog_Tables->set_default_size(300, 400);
  m_pBox_Tables->show_all();

  //Connect signals:
  m_pBox_Tables->signal_selected.connect(sigc::mem_fun(*this, &Frame_Glom::on_box_tables_selected));

  m_pDialog_Reports->get_vbox()->pack_start(*m_pBox_Reports);
  m_pDialog_Reports->set_default_size(300, 400);
  m_pBox_Reports->show_all();

  m_pBox_Reports->signal_selected.connect(sigc::mem_fun(*this, &Frame_Glom::on_box_reports_selected));

  m_Notebook_Find.signal_find_criteria.connect(sigc::mem_fun(*this, &Frame_Glom::on_notebook_find_criteria));

  m_Notebook_Data.signal_record_details_requested().connect(sigc::mem_fun(*this, &Frame_Glom::on_notebook_data_record_details_requested));
  //Fill Composite View:
  //This means that set_document and load/save are delegated to these children:
  add_view(m_pBox_Tables);
  add_view(m_pBox_Reports);
  add_view(m_pDialog_Fields); //Also a composite view.
  add_view(m_pDialog_Relationships); //Also a composite view.
  add_view(m_pDialogConnection); //Also a composite view.
  add_view(&m_Notebook_Data); //Also a composite view.
  add_view(&m_Notebook_Find); //Also a composite view.

  on_userlevel_changed(AppState::USERLEVEL_OPERATOR); //A default to show before a document is created or loaded.

  show_all();
}

Frame_Glom::~Frame_Glom()
{
  remove_view(m_pBox_Tables);
  remove_view(m_pBox_Reports);
  remove_view(&m_Notebook_Data); //Also a composite view.
  remove_view(&m_Notebook_Find); //Also a composite view.

  if(m_pDialog_Tables)
  {
    delete m_pDialog_Tables;
    m_pDialog_Tables = 0;
  }

  if(m_pDialogConnection)
  {
    remove_view(m_pDialogConnection);
    delete m_pDialogConnection;
    m_pDialogConnection = 0;
  }

  if(m_pDialogConnectionFailed)
  {
    delete m_pDialogConnectionFailed;
    m_pDialogConnectionFailed = 0;
  }

  if(m_pDialog_Relationships)
  {
    remove_view(m_pDialog_Relationships);
    delete m_pDialog_Relationships;
    m_pDialog_Relationships = 0;
  }

  if(m_pDialog_Relationships)
  {
    delete m_pDialog_Relationships;
    m_pDialog_Relationships = 0;
  }

  if(m_pDialogLayoutReport)
  {
    remove_view(m_pDialogLayoutReport);
    delete m_pDialogLayoutReport;
    m_pDialogLayoutReport = 0;
  }

  if(m_pDialog_Fields)
  {
    remove_view(m_pDialog_Fields);
    delete m_pDialog_Fields;
    m_pDialog_Fields = 0;
  }
}

void Frame_Glom::set_databases_selected(const Glib::ustring& strName)
{
  //m_pDialog_Databases->hide(); //cause_close();

  get_document()->set_connection_database(strName);

  do_menu_Navigate_Table(true /* open default */);
}

void Frame_Glom::on_box_tables_selected(const Glib::ustring& strName)
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
    //Notebook_Glom* pNotebook = dynamic_cast<Notebook_Glom*>(&widget);
    //if(pNotebook)
   // {
   //   pNotebook->show_hint();
   // }
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
  Gtk::Window* pWindowApp = get_app_window();
  if(pWindowApp)
  {
    //TODO: Obviously this document should have been deleted when the database-creation was cancelled.
    /* Note that "canceled" is the correct US spelling. */
    show_ok_dialog(_("No database"), _("This document does not specify any database. Maybe the document creation was canceled before the database could be created."), *pWindowApp);
  }
}

void Frame_Glom::show_table(const Glib::ustring& strTableName)
{
  App_Glom* pApp = dynamic_cast<App_Glom*>(get_app_window());

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
    if(pApp)
      on_userlevel_changed(pApp->get_userlevel());
 
    switch(m_Mode)
    {
      case(MODE_Data):
      {
        strMode = _("Data");
        m_Notebook_Data.init_db_details(m_strTableName);
        set_mode_widget(m_Notebook_Data);
        break;
      }
      case(MODE_Find):
      {
        strMode = _("Find");
        m_Notebook_Find.init_db_details(m_strTableName);
        set_mode_widget(m_Notebook_Find);
        break;
      }
      default:
      {
        std::cout << "Frame_Glom::on_box_tables_selected(): Unexpected mode" << std::endl;
        strMode = _("Unknown");
        break;
      }
    }

    m_pLabel_Mode->set_text(strMode);
  }

  show_table_title();

  //List the reports in the menu:
  pApp->fill_menu_reports(strTableName);

  show_all();
}

void Frame_Glom::on_menu_userlevel_Developer(const Glib::RefPtr<Gtk::RadioAction>& action, const Glib::RefPtr<Gtk::RadioAction>& operator_action)
{
  if(action && action->get_active())
  {
    Document_Glom* document = dynamic_cast<Document_Glom*>(get_document());
    if(document)
    {

      //Check whether the current user has developer privileges:
      const ConnectionPool* connection_pool = ConnectionPool::get_instance();
      bool test = get_user_is_in_group(connection_pool->get_user(), GLOM_STANDARD_GROUP_NAME_DEVELOPER);
      if(test)
      {
        //Avoid double signals:
        //if(document->get_userlevel() != AppState::USERLEVEL_DEVELOPER)
        test = document->set_userlevel(AppState::USERLEVEL_DEVELOPER);
      }

      //If this was not possible then revert the menu:
      if(!test)
      {
        Gtk::MessageDialog dialog(Bakery::App_Gtk::util_bold_message(_("Developer mode not available.")), true, Gtk::MESSAGE_WARNING);
        dialog.set_secondary_text(_("Developer mode is not available. Check that you have sufficient database access rights and that the glom file is not read-only."));
        dialog.set_transient_for(*get_app_window());
        dialog.run();

        //This causes an endless loop, but it is not recursive so we can't block it.
        //TODO: Submit GTK+ bug.
        //action->set_active(false);
        operator_action->set_active();
      }
    }
  }
}

void Frame_Glom::on_menu_userlevel_Operator(const Glib::RefPtr<Gtk::RadioAction>& action)
{
  if(action &&  action->get_active())
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

void Frame_Glom::on_menu_file_print()
{
 Notebook_Glom* notebook_current = dynamic_cast<Notebook_Glom*>(m_pBox_Mode->get_child());
 if(notebook_current)
   notebook_current->do_menu_file_print();
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

/*
void Frame_Glom::on_menu_Navigate_Database()
{
  do_menu_Navigate_Database();
}

void Frame_Glom::do_menu_Navigate_Database(bool bUseList)
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
*/

void Frame_Glom::on_menu_Tables_EditReports()
{
  on_menu_developer_reports();
}

void Frame_Glom::on_menu_Tables_EditTables()
{
  do_menu_Navigate_Table(); 
}

void Frame_Glom::do_menu_Navigate_Table(bool open_default)
{
  if(get_document()->get_connection_database().empty())
  {
    alert_no_table();
  }
  else
  {
    m_pBox_Tables->init_db_details();

    Glib::ustring default_table_name;
    if(open_default)
    {
      default_table_name = get_document()->get_default_table();
    }

    if(!default_table_name.empty())
    {
      //Show the default table, and let the user navigate to another table manually if he wants:
      show_table(default_table_name);
    }
    else
    {
      //Let the user choose a table:
      //m_pDialog_Tables->set_policy(false, true, false); //TODO_port
      //m_pDialog_Tables->load_from_document(); //Refresh
      m_pDialog_Tables->show();
    }
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

void Frame_Glom::show_ok_dialog(const Glib::ustring& title, const Glib::ustring& message, Gtk::Window& parent)
{
  Gtk::MessageDialog dialog("<b>" + title + "</b>", true /* markup */, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK);
  dialog.set_secondary_text(message);
  dialog.set_transient_for(parent);
  dialog.run();
}

void Frame_Glom::on_notebook_find_criteria(const Glib::ustring& strWhereClause)
{
  //on_menu_Mode_Data();

  App_Glom* pApp = dynamic_cast<App_Glom*>(get_app_window());
  if(pApp)
  {
    pApp->set_mode_data();

    bool records_found = m_Notebook_Data.init_db_details(m_strTableName, strWhereClause);
    m_Notebook_Data.select_page_for_find_results();

    if(!records_found)
    {
      bool find_again = show_warning_no_records_found(*get_app_window());

      if(find_again)
        pApp->set_mode_find();
    }
  }
}

void Frame_Glom::on_userlevel_changed(AppState::userlevels userlevel)
{
  //show user level:
  Glib::ustring user_level_name = _("Operator");
  if(userlevel == AppState::USERLEVEL_DEVELOPER)
    user_level_name = _("Developer");

  if(m_pLabel_userlevel)
    m_pLabel_userlevel->set_text(user_level_name);

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

    m_pLabel_Table->set_markup("<b><span size=\"xx-large\">" + table_label + "</span></b>"); //Show the table title in large text, because it's very important to the user.
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
  Base_DB::type_vecFields fieldsDatabase = Base_DB::get_fields_for_table_from_database(m_strTableName);

  Document_Glom* pDoc = dynamic_cast<const Document_Glom*>(get_document());
  if(pDoc)
  {
    bool document_must_to_be_updated = false;

    //Get the fields information from the document.
    //and add to, or update Document's list of fields:
    type_vecFields fieldsDocument = pDoc->get_table_fields(m_strTableName);

    for(Base_DB::type_vecFields::const_iterator iter = fieldsDatabase.begin(); iter != fieldsDatabase.end(); ++iter)
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
        Gnome::Gda::FieldAttributes field_info_db = field_database.get_field_info();
        Field field_document =  *iterFindDoc;
        if(field_document.field_info_from_database_is_equal( field_info_db )) //ignores auto_increment because libgda does not report it from the database properly.
        {
          //The database has different information. We assume that the information in the database is newer.

          //Update the field information:
          field_info_db.set_auto_increment( field_document.get_auto_increment() ); //libgda does not report it from the database properly.
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
      {
        document_must_to_be_updated = true; //Something changed.
      }
    }

    if(document_must_to_be_updated)
      pDoc->set_table_fields(m_strTableName, fieldsActual);
  }
}

void Frame_Glom::set_document(Document_Glom* pDocument)
{
  View_Composite_Glom::set_document(pDocument);

  Document_Glom* document = get_document();
  if(document)
  {
    //Connect to a signal that is only on the derived document class:
    document->signal_userlevel_changed().connect( sigc::mem_fun(*this, &Frame_Glom::on_userlevel_changed) );

    //Show the appropriate UI for the user level that is specified by this new document:
    on_userlevel_changed(document->get_userlevel());
  }
}

void Frame_Glom::show_system_name()
{
  const SystemPrefs prefs = get_database_preferences();
  const Glib::ustring org = prefs.m_org_name;
  const Glib::ustring name = prefs.m_name;

  Glib::ustring system_name = org;
  if(!system_name.empty() && !name.empty())
    system_name += ": ";

  system_name += name;

  m_pLabel_Name->set_text ( Bakery::App_Gtk::util_bold_message(system_name) );
  m_pLabel_Name->set_use_markup();
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

void Frame_Glom::on_menu_developer_database_preferences()
{
  Dialog_Database_Preferences* dialog = 0;
  try
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_database_preferences");
    refXml->get_widget_derived("dialog_database_preferences", dialog);
    if(dialog)
    {
      dialog->set_transient_for(*(get_app_window()));
      add_view(dialog);
      dialog->load_from_document();

      dialog->run();

      remove_view(dialog);
      delete dialog;

      show_system_name(); //In case it has changed.
    }
  }

  catch(const Gnome::Glade::XmlError& ex)
  {
    std::cerr << ex.what() << std::endl;
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
    m_pDialog_Fields->init_db_details(m_strTableName);
    m_pDialog_Fields->show();
  }
}

void Frame_Glom::on_menu_developer_relationships_overview()
{
  Dialog_RelationshipsOverview* dialog = 0;
  try
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_relationships_overview");
    refXml->get_widget_derived("dialog_relationships_overview", dialog);
    if(dialog)
    {
      dialog->set_transient_for(*(get_app_window()));
      add_view(dialog);
      dialog->load_from_document();

      dialog->run();

      remove_view(dialog);
      delete dialog;
    }
  }

  catch(const Gnome::Glade::XmlError& ex)
  {
    std::cerr << ex.what() << std::endl;
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
    m_pDialog_Relationships->init_db_details(m_strTableName);
    m_pDialog_Relationships->show();
  }
}

void Frame_Glom::on_menu_developer_users()
{
  Dialog_GroupsList* dialog = 0;
  try
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "window_groups");

    refXml->get_widget_derived("window_groups", dialog);
  }
  catch(const Gnome::Glade::XmlError& ex)
  {
    std::cerr << ex.what() << std::endl;
  }

  dialog->set_transient_for(*get_app_window());

  add_view(dialog); //Give it access to the document.
  dialog->load_from_document(); //Update the UI now that it has the document.

  dialog->run();
  remove_view(dialog);
  delete dialog;

  //Update the Details and List layouts, in case the permissions have changed:
  //TODO: Also update them somehow if another user has changed them,
  //or respond to the failed SQL nicely.
  show_table(m_strTableName);
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

void Frame_Glom::on_menu_developer_reports()
{
  //Check that there is a table to show:
  if(m_strTableName.empty())
  {
    alert_no_table(); //TODO: Disable the menu item instead.
  }
  else
  {
    //Gtk::MessageDialog dialog("This is not working yet. It's just some test code."); //TODO: Remove this.
    //dialog.run();

    m_pBox_Reports->init_db_details(m_strTableName);
    m_pDialog_Reports->show();
  }
}


void Frame_Glom::on_box_reports_selected(const Glib::ustring& report_name)
{
  m_pDialog_Reports->hide();

  Report report;
  bool found = get_document()->get_report(m_strTableName, report_name, report);
  if(found)
  {
    m_pDialogLayoutReport->set_report(m_strTableName, report);
    m_pDialogLayoutReport->show();
  }
}


void Frame_Glom::on_developer_dialog_hide()
{
  //The dababase structure might have changed, so refresh the data view:
  show_table(m_strTableName);
}

bool Frame_Glom::connection_request_password_and_attempt()
{
  while(true) //Loop until a return
  {
    //Ask for connection details:
    m_pDialogConnection->load_from_document(); //Get good defaults.
    m_pDialogConnection->set_transient_for(*get_app_window());
    int response = m_pDialogConnection->run();
    m_pDialogConnection->hide();

    if(response == Gtk::RESPONSE_OK)
    {
      try
      {
        //TODO: Remove any previous database setting?
        sharedptr<SharedConnection> sharedconnection = m_pDialogConnection->connect_to_server_with_connection_settings();
        return true; //Succeeeded, because no exception was thrown.
      }
      catch(const ExceptionConnection& ex)
      {
        g_warning("Frame_Glom::connection_request_password_and_attempt(): caught exception.");

        if(ex.get_failure_type() == ExceptionConnection::FAILURE_NO_SERVER)
        {
          //Warn the user, and let him try again:
          m_pDialogConnectionFailed->set_transient_for(*get_app_window());
          int response = m_pDialogConnectionFailed->run();
          m_pDialogConnectionFailed->hide();

          if(response != Gtk::RESPONSE_OK)
            return false; //The user cancelled.
        }
        else
        {
          g_warning("Frame_Glom::connection_request_password_and_attempt(): rethrowing exception.");

          //The connection to the server is OK, but the specified database does not exist:
          throw ex; //Pass it on for the caller to handle.
          return false;
        }
      }

      //Try again.
    }
    else
      return false; //The user cancelled.
  }
}

bool Frame_Glom::create_database(const Glib::ustring& database_name, bool request_password)
{
  //Ask for connection details:
  bool connection_possible = false;
  try
  {
    if(request_password)
      connection_possible = connection_request_password_and_attempt(); //If it succeeded and the user did not cancel.
    else
      connection_possible = true; //Assume that connection details are already coorect.
  }
  catch(const ExceptionConnection& ex)
  {
     connection_possible = false;
     g_warning("debug Frame_Glom::create_database() exception caught: connection failed be");
  }

  if(!connection_possible)
  {
    g_warning("debug Frame_Glom::create_database(): connection was not possible.");
    return false;
  }
  else
  {
    //This must now succeed, because we've already tried it once:
    sharedptr<SharedConnection> sharedconnection;
    try
    {
      if(request_password)
        sharedconnection = m_pDialogConnection->connect_to_server_with_connection_settings();
      else
        sharedconnection = ConnectionPool::get_instance()->connect();
    }
    catch(const ExceptionConnection& ex)
    {
      g_warning("debug Frame_Glom::create_database() Connection failed.");

      return false;
    }

    if(sharedconnection)
    {
      Bakery::BusyCursor(*get_app_window());

      Glib::RefPtr<Gnome::Gda::Connection> connection = sharedconnection->get_gda_connection();
      if(connection)
      {
        bool result = connection->create_database(database_name);
        if(result)
        {
          g_warning("Frame_Glom::create_database(): Succeeded: database_name=%s", database_name.c_str());
          return true; // It succeeded.
        }
        else
        {
          //Tell the user:
          Gtk::Dialog* dialog = 0;
          try
          {
            Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_error_create_database");

            refXml->get_widget("dialog_error_create_database", dialog);
          }
          catch(const Gnome::Glade::XmlError& ex)
          {
            std::cerr << ex.what() << std::endl;
          }

          //TODO: dialog->set_transient_for(*get_app_window());
          dialog->run();
          delete dialog;
          return false; //Failed. //TODO: Allow the user to try with a different user name?
        }
      }
    }
  }

  return false;
}


void Frame_Glom::on_menu_report_selected(const Glib::ustring& report_name)
{
  const Privileges table_privs = get_current_privs(m_strTableName);

  //Don't try to print tables that the user can't view.
  if(!table_privs.m_view)
  {
    //TODO: Warn the user.
    return;
  }

  Report report;
  bool found = get_document()->get_report(m_strTableName, report_name, report);
  if(!found)
    return;

  report_build(m_strTableName, report, ""); //TODO: Use found set's where_claus.
}

void Frame_Glom::on_dialog_layout_report_hide()
{
  Document_Glom* document = get_document();

  if(true) //m_pDialogLayoutReport->get_modified())
  {
    const Glib::ustring original_name = m_pDialogLayoutReport->get_original_report_name();
    Report report = m_pDialogLayoutReport->get_report();
    if(original_name != report.get_name())
      document->remove_report(m_strTableName, original_name);

    document->set_report(m_strTableName, report);
  }

  //Update the reports menu:
  App_Glom* pApp = dynamic_cast<App_Glom*>(get_app_window());
  if(pApp)
    pApp->fill_menu_reports(m_strTableName);
}

void Frame_Glom::on_dialog_reports_hide()
{
  //Update the reports menu:
  App_Glom* pApp = dynamic_cast<App_Glom*>(get_app_window());
  if(pApp)
    pApp->fill_menu_reports(m_strTableName);
}

void Frame_Glom::on_dialog_tables_hide()
{
  //If tables could have been added or removed, update the tables menu:
  Document_Glom* document = dynamic_cast<Document_Glom*>(get_document());
  if(document)
  {
    if(document->get_userlevel() == AppState::USERLEVEL_DEVELOPER)
    {
      App_Glom* pApp = dynamic_cast<App_Glom*>(get_app_window());
      if(pApp)
        pApp->fill_menu_tables();
        
      //Select a different table if the current one no longer exists:
      if(!document->get_table_is_known(m_strTableName))
      {
        //Open the default table, or the first table if there is no default: 
        Glib::ustring table_name = document->get_default_table();
        if(table_name.empty())
          table_name = document->get_first_table();
    
        show_table(table_name);
      }
    }
  }
}

void Frame_Glom::on_notebook_data_record_details_requested(const Glib::ustring& table_name, Gnome::Gda::Value primary_key_value)
{
  show_table(table_name);
  m_Notebook_Data.show_details(primary_key_value);
}

