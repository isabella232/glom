
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

#include "window_print_layout_edit.h"
#include <glom/box_db_table.h>
#include "canvas_layout_item.h"
//#include <libgnome/gnome-i18n.h>
#include <bakery/App/App_Gtk.h> //For util_bold_message().
#include <gtkmm/scrolledwindow.h>
#include <glibmm/i18n.h>

namespace Glom
{

Window_PrintLayout_Edit::Window_PrintLayout_Edit(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::Window(cobject),
  m_entry_name(0),
  m_entry_title(0),
  m_label_table_name(0),
  m_label_table(0),
  m_button_close(0),
  m_box(0)
{
  set_default_size(640, 480);

  add_view(&m_canvas);

  refGlade->get_widget("vbox_menu", m_box_menu);
  refGlade->get_widget("vbox_canvas", m_box_canvas);
  refGlade->get_widget("vbox_inner", m_box);

  //refGlade->get_widget("label_name", m_label_name);
  refGlade->get_widget("label_table_name", m_label_table_name);
  refGlade->get_widget("entry_name", m_entry_name);
  refGlade->get_widget("entry_title", m_entry_title);

  refGlade->get_widget("button_close", m_button_close);
  m_button_close->signal_clicked().connect( sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_button_close) );

  init_menu();

  Gtk::ScrolledWindow* scrolled = Gtk::manage(new Gtk::ScrolledWindow());
  scrolled->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  scrolled->add(m_canvas);
  scrolled->show_all();
  m_box_canvas->pack_start(*scrolled);
  m_canvas.show();

  //Fill composite view:
  //add_view(m_box);

  setup_context_menu();
  m_canvas.signal_show_context().connect(sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_canvas_show_context_menu));

  show_all_children();
}

void Window_PrintLayout_Edit::init_menu()
{
  m_action_group = Gtk::ActionGroup::create();

  m_action_group->add(Gtk::Action::create("Menu_File", _("_File")));
  m_action_group->add(Gtk::Action::create("Action_Menu_File_PageSetup", _("_Page Setup")),
    sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_file_page_setup));

  m_action_group->add(Gtk::Action::create("Menu_Edit", Gtk::Stock::EDIT));
  m_action_group->add(Gtk::Action::create("Action_Menu_Edit_Cut", Gtk::Stock::CUT));
  m_action_group->add(Gtk::Action::create("Action_Menu_Edit_Copy", Gtk::Stock::COPY));
  m_action_group->add(Gtk::Action::create("Action_Menu_Edit_Paste", Gtk::Stock::PASTE));
  m_action_group->add(Gtk::Action::create("Action_Menu_Edit_Delete", Gtk::Stock::DELETE));

  m_action_group->add(Gtk::Action::create("Menu_Insert", _("_Insert")));
  m_action_group->add(Gtk::Action::create("Action_Menu_Insert_Field", _("Insert _Field")),
                        sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_insert_field) );
  m_action_group->add(Gtk::Action::create("Action_Menu_Insert_Text", _("Insert _Text")),
                        sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_insert_text) );
  m_action_group->add(Gtk::Action::create("Action_Menu_Insert_Image", _("Insert _Image")),
                        sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_insert_image) );

  m_action_group->add(Gtk::Action::create("Menu_View", _("_View")));
  m_action_showgrid = Gtk::ToggleAction::create("Action_Menu_View_ShowGrid", _("Show Grid"));
  m_action_group->add(m_action_showgrid, sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_view_show_grid));
  m_action_showrules = Gtk::ToggleAction::create("Action_Menu_View_ShowRules", _("Show Rules"));
  m_action_group->add(m_action_showrules, sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_view_show_rules));

  Gtk::RadioAction::Group group_zoom;
  m_action_group->add(Gtk::RadioAction::create(group_zoom, "Action_Menu_View_Zoom200", _("Zoom 200%")),
   sigc::bind( sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_view_zoom), 200));
  m_action_group->add(Gtk::RadioAction::create(group_zoom, "Action_Menu_View_Zoom100", Gtk::Stock::ZOOM_100),
   sigc::bind( sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_view_zoom), 100));

  Glib::RefPtr<Gtk::Action> action_50 = Gtk::RadioAction::create(group_zoom, "Action_Menu_View_Zoom50", _("Zoom 50%"));
  m_action_group->add(action_50,
   sigc::bind( sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_view_zoom), 50));
  action_50->activate();

  m_action_group->add(Gtk::RadioAction::create(group_zoom, "Action_Menu_View_Zoom25", _("Zoom 25%")),
   sigc::bind( sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_menu_view_zoom), 25));

  //Build part of the menu structure, to be merged in by using the "PH" placeholders:
  static const Glib::ustring ui_description =
    "<ui>"
    "  <menubar name='Menubar'>"
    "      <menu action='Menu_File'>"
    "        <menuitem action='Action_Menu_File_PageSetup' />"
    "      </menu>"
    "      <menu action='Menu_Edit'>"
    "        <menuitem action='Action_Menu_Edit_Cut' />"
    "        <menuitem action='Action_Menu_Edit_Copy' />"
    "        <menuitem action='Action_Menu_Edit_Paste' />"
    "        <menuitem action='Action_Menu_Edit_Delete' />"
    "      </menu>"
    "      <menu action='Menu_Insert'>"
    "        <menuitem action='Action_Menu_Insert_Field' />"
    "        <menuitem action='Action_Menu_Insert_Text' />"
    "        <menuitem action='Action_Menu_Insert_Image' />"
    "      </menu>"
    "      <menu action='Menu_View'>"
    "        <menuitem action='Action_Menu_View_ShowGrid' />"
    "        <menuitem action='Action_Menu_View_ShowRules' />"
    "        <separator />"
    "        <menuitem action='Action_Menu_View_Zoom200' />"
    "        <menuitem action='Action_Menu_View_Zoom100' />"
    "        <menuitem action='Action_Menu_View_Zoom50' />"
    "        <menuitem action='Action_Menu_View_Zoom25' />"
    "      </menu>"
    "  </menubar>"
    "</ui>";

  //Add menu:
  m_uimanager = Gtk::UIManager::create();
  m_uimanager->insert_action_group(m_action_group);
  m_uimanager->add_ui_from_string(ui_description);

  //Menubar:
  Gtk::MenuBar* pMenuBar = static_cast<Gtk::MenuBar*>(m_uimanager->get_widget("/Menubar"));
  m_box_menu->pack_start(*pMenuBar, Gtk::PACK_SHRINK);
  pMenuBar->show();

  //TODO: Add a toolbar if it would be useful:
  //Gtk::Toolbar* pToolBar = static_cast<Gtk::Toolbar*>(m_uimanager->get_widget("/Bakery_ToolBar"));
  //m_HandleBox_Toolbar.add(*pToolBar);
  //m_HandleBox_Toolbar.show();

  add_accel_group(m_uimanager->get_accel_group());
}


Window_PrintLayout_Edit::~Window_PrintLayout_Edit()
{
  remove_view(&m_canvas);
}

bool Window_PrintLayout_Edit::init_db_details(const Glib::ustring& table_name)
{
  Document_Glom* document = dynamic_cast<Document_Glom*>(get_document());
  if(!document)
    return false;

  Glib::ustring table_label = _("None selected");

  //Show the table title (if any) and name:
  Glib::ustring table_title = document->get_table_title(table_name);
  if(table_title.empty())
    table_label = table_name;
  else
    table_label = table_title + " (" + table_name + ")";

  if(m_label_table)
    m_label_table->set_text(table_label);

  return true;

/*
  if(m_box)
  {
    m_box->load_from_document();

    Dialog_Design::init_db_details(table_name);

    m_box->init_db_details(table_name);
  }
*/
  return true;
}


Glib::ustring Window_PrintLayout_Edit::get_original_name() const
{
  return m_name_original;
}

void Window_PrintLayout_Edit::set_print_layout(const Glib::ustring& table_name, const sharedptr<const PrintLayout>& print_layout)
{
  m_modified = false;

  m_name_original = print_layout->get_name();
  m_print_layout = sharedptr<PrintLayout>(new PrintLayout(*print_layout)); //Copy it, so we only use the changes when we want to.
  m_canvas.set_print_layout(table_name, m_print_layout);
  m_table_name = table_name;

  //Dialog_Layout::set_document(layout, document, table_name, table_fields);

  //Set the table name and title:
  m_label_table_name->set_text(table_name);

  m_entry_name->set_text(print_layout->get_name()); 
  m_entry_title->set_text(print_layout->get_title());

  m_modified = false;
}



void Window_PrintLayout_Edit::enable_buttons()
{

}

sharedptr<PrintLayout> Window_PrintLayout_Edit::get_print_layout()
{
  m_print_layout = m_canvas.get_print_layout();
  m_print_layout->set_name( m_entry_name->get_text() );
  m_print_layout->set_title( m_entry_title->get_text() );

/*
  m_print_layout->m_layout_group->remove_all_items();

  m_print_layout->m_layout_group->remove_all_items();

  //The Header and Footer parts are implicit (they are the whole header or footer treeview)
  sharedptr<LayoutItem_Header> header = sharedptr<LayoutItem_Header>::create();
  sharedptr<LayoutGroup> group_temp = header;
  fill_print_layout_parts(group_temp, m_model_parts_header);
  if(header->get_items_count())
    m_print_layout->m_layout_group->add_item(header);

  fill_print_layout_parts(m_print_layout->m_layout_group, m_model_parts_main);

  sharedptr<LayoutItem_Footer> footer = sharedptr<LayoutItem_Footer>::create();
  group_temp = footer;
  fill_print_layout_parts(group_temp, m_model_parts_footer);
  if(footer->get_items_count())
    m_print_layout->m_layout_group->add_item(footer);

*/
  return m_print_layout;
}

void Window_PrintLayout_Edit::on_context_menu_insert_field()
{
  on_menu_insert_field();
}

void Window_PrintLayout_Edit::on_context_menu_insert_text()
{
  on_menu_insert_text();
}

void Window_PrintLayout_Edit::setup_context_menu()
{
  m_context_menu_action_group = Gtk::ActionGroup::create();

  m_context_menu_action_group->add(Gtk::Action::create("ContextMenu", "Context Menu") );
  m_context_menu_action_group->add(Gtk::Action::create("ContextMenuInsert", _("Insert")) );

  Glib::RefPtr<Gtk::Action> action =  Gtk::Action::create("ContextInsertField", _("Field"));
  m_context_menu_action_group->add(action,
    sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_context_menu_insert_field) );

  action =  Gtk::Action::create("ContextInsertText", _("Text"));
  m_context_menu_action_group->add(action,
    sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_context_menu_insert_text) );

  /*
  action =  Gtk::Action::create("ContextDelete", Gtk::Stock::DELETE);
  m_context_menu_action_group->add(action,
    sigc::mem_fun(*this, &Window_PrintLayout_Edit::on_context_menu_delete) );
  */

  m_context_menu_uimanager = Gtk::UIManager::create();
  m_context_menu_uimanager->insert_action_group(m_context_menu_action_group);

  #ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
  #endif
    Glib::ustring ui_info = 
      "<ui>"
      "  <popup name='ContextMenu'>"
      "    <menu action='ContextMenuInsert'>"
      "      <menuitem action='ContextInsertField'/>"
      "      <menuitem action='ContextInsertText'/>"
      "    </menu>"
      "  </popup>"
      "</ui>";

  #ifdef GLIBMM_EXCEPTIONS_ENABLED
    m_context_menu_uimanager->add_ui_from_string(ui_info);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << "building menus failed: " <<  ex.what();
  }
  #else
  std::auto_ptr<Glib::Error> error;
  m_context_menu_uimanager->add_ui_from_string(ui_info, error);
  if(error.get() != NULL)
  {
    std::cerr << "building menus failed: " << error->what();
  }
  #endif

  //Get the menu:
  m_context_menu = dynamic_cast<Gtk::Menu*>( m_context_menu_uimanager->get_widget("/ContextMenu") ); 
}


void Window_PrintLayout_Edit::on_canvas_show_context_menu(guint button, guint32 activate_time)
{
  if(m_context_menu)
    m_context_menu->popup(button, activate_time);
}

void Window_PrintLayout_Edit::set_default_position(const sharedptr<LayoutItem>& item)
{
  item->set_print_layout_position(10, 10, 100, 100);
}

void Window_PrintLayout_Edit::on_menu_insert_field()
{
  sharedptr<LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::create();

  // Note to translators: This is the default contents of a text item on a print layout: 
  set_default_position(layout_item);

  Glib::RefPtr<CanvasLayoutItem> item = CanvasLayoutItem::create(layout_item);
  m_canvas.add_item(item);
}

void Window_PrintLayout_Edit::on_menu_insert_text()
{
  sharedptr<LayoutItem_Text> layout_item = sharedptr<LayoutItem_Text>::create();

  // Note to translators: This is the default contents of a text item on a print layout: 
  layout_item->set_text(_("text"));
  set_default_position(layout_item);

  Glib::RefPtr<CanvasLayoutItem> item = CanvasLayoutItem::create(layout_item);
  m_canvas.add_item(item);
}

void Window_PrintLayout_Edit::on_menu_insert_image()
{
  sharedptr<LayoutItem_Image> layout_item = sharedptr<LayoutItem_Image>::create();
  // Note to translators: This is the default contents of a text item on a print layout: 
  //layout_item->set_text(_("text"));
  set_default_position(layout_item);

  Glib::RefPtr<CanvasLayoutItem> item = CanvasLayoutItem::create(layout_item);
  m_canvas.add_item(item);
}

void Window_PrintLayout_Edit::on_button_close()
{
  hide();
}

void Window_PrintLayout_Edit::on_menu_view_show_grid()
{
 if(m_action_showgrid->get_active())
  {
    std::cout << "showing" << std::endl;
    m_canvas.set_grid_gap(20);
  }
  else
  {
    std::cout << "hiding" << std::endl;
    m_canvas.remove_grid();
  }
}

void Window_PrintLayout_Edit::on_menu_view_show_rules()
{
  //TODO:
}

void Window_PrintLayout_Edit::on_menu_view_zoom(guint percent)
{
  m_canvas.set_zoom_percent(percent);
}

void Window_PrintLayout_Edit::on_menu_file_page_setup()
{
  Glib::RefPtr<Gtk::PageSetup> page_setup = m_canvas.get_page_setup();

  //Show the page setup dialog, asking it to start with the existing settings:
  Glib::RefPtr<Gtk::PrintSettings> print_settings = Gtk::PrintSettings::create(); //TODO: Do we really need to get this from the user and store it?
  page_setup = Gtk::run_page_setup_dialog(*this, page_setup, print_settings);

  //Save the chosen page setup dialog for use when printing, previewing, or
  //showing the page setup dialog again:
  m_canvas.set_page_setup(page_setup);
}



} //namespace Glom


