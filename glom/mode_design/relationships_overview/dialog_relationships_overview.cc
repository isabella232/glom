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

#include "config.h"
#include <gtkmm.h>
#include "dialog_relationships_overview.h"
#include "glom/utility_widgets/canvas/canvas_line_movable.h"
#include "glom/utility_widgets/canvas/canvas_text_movable.h"
#include <glom/mode_design/layout/dialog_choose_relationship.h>
#include "printoperation_relationshipsoverview.h"
#include "glom/application.h"
#include <goocanvas.h>
#include <glibmm/i18n.h>
#include <iostream>

namespace Glom
{

//static:
int Dialog_RelationshipsOverview::m_last_size_x = 0;
int Dialog_RelationshipsOverview::m_last_size_y = 0;

const char* Dialog_RelationshipsOverview::glade_id("dialog_relationships_overview");
const bool Dialog_RelationshipsOverview::glade_developer(true);

Dialog_RelationshipsOverview::Dialog_RelationshipsOverview(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
  : Gtk::Dialog(cobject),
    m_menu(0),
    m_modified(false),
    m_scrolledwindow_canvas(0)
{
  m_refPageSetup = Gtk::PageSetup::create();
  m_refSettings = Gtk::PrintSettings::create();

  //Add a menu:
  Gtk::VBox* vbox = 0;
  builder->get_widget("vbox_placeholder_menubar", vbox);

  m_refActionGroup = Gtk::ActionGroup::create();

  m_refActionGroup->add(Gtk::Action::create("Overview_MainMenu_File", _("_File")) );
  m_refActionGroup->add(Gtk::Action::create("Overview_MainMenu_File_PageSetup", _("Page _Setup")),
    sigc::mem_fun(*this, &Dialog_RelationshipsOverview::on_menu_file_page_setup) );
  m_refActionGroup->add(Gtk::Action::create("Overview_MainMenu_File_Print", Gtk::Stock::PRINT),
    sigc::mem_fun(*this, &Dialog_RelationshipsOverview::on_menu_file_print) );

  m_refActionGroup->add(Gtk::Action::create("Overview_MainMenu_View", _("_View")) );
  m_action_showgrid = Gtk::ToggleAction::create("Overview_MainMenu_View_Grid", _("Show _Grid"));
  m_refActionGroup->add(m_action_showgrid,
    sigc::mem_fun(*this, &Dialog_RelationshipsOverview::on_menu_view_showgrid) );

  Glib::RefPtr<Gtk::UIManager> m_refUIManager = Gtk::UIManager::create();

  m_refUIManager->insert_action_group(m_refActionGroup);
  add_accel_group(m_refUIManager->get_accel_group());

  try
  {
    static const Glib::ustring ui_description =
    "<ui>"
#ifdef GLOM_ENABLE_MAEMO
    "  <popup name='Overview_MainMenu'>"
#else
    "  <menubar name='Overview_MainMenu'>"
#endif
    "    <menu action='Overview_MainMenu_File'>"
    "      <menuitem action='Overview_MainMenu_File_PageSetup' />"
    "      <menuitem action='Overview_MainMenu_File_Print' />"
    "    </menu>"
    "    <menu action='Overview_MainMenu_View'>"
    "      <menuitem action='Overview_MainMenu_View_Grid' />"
    "    </menu>"
#ifdef GLOM_ENABLE_MAEMO
    "  </popup>"
#else
    "  </menubar>"
#endif
    "</ui>";

    m_refUIManager->add_ui_from_string(ui_description);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << "building menus failed: " <<  ex.what();
  }

  //Get the menu:
  m_menu = dynamic_cast<Gtk::MenuBar*>( m_refUIManager->get_widget("/Overview_MainMenu") );
  if(!m_menu)
    g_warning("menu not found");

  vbox->pack_start(*m_menu, Gtk::PACK_SHRINK);
  m_menu->show();


  //Get the scolled window and add the canvas to it:
  m_scrolledwindow_canvas = 0;
  builder->get_widget("scrolledwindow_canvas", m_scrolledwindow_canvas);

  m_scrolledwindow_canvas->add(m_canvas);
  m_canvas.show();

  //Restore the previous window size, to avoid annoying the user:
  if(m_last_size_x != 0 && m_last_size_y != 0 )
  {
    set_default_size(m_last_size_x, m_last_size_y);
  }

  m_group_tables = Goocanvas::Group::create();
  m_canvas.add_item(m_group_tables);
  m_group_lines = Goocanvas::Group::create();
  m_canvas.add_item(m_group_lines);
  m_group_lines->lower(); //Make sure that the lines are below the tables.

  //Respond to changes of window size,
  //so we always make the canvas bounds big enough:
  m_scrolledwindow_canvas->get_hadjustment()->signal_changed().connect(
    sigc::mem_fun(*this, &Dialog_RelationshipsOverview::on_scroll_value_changed) );
  m_scrolledwindow_canvas->get_vadjustment()->signal_changed().connect(
    sigc::mem_fun(*this, &Dialog_RelationshipsOverview::on_scroll_value_changed) );

  setup_context_menu();
}

Dialog_RelationshipsOverview::~Dialog_RelationshipsOverview()
{
  get_size(m_last_size_x, m_last_size_y);

  //Disconnect signal handlers for all current items,
  //so sigc::bind()ed RefPtrs can be released.
  while(m_list_table_connections.size())
  {
     type_list_connections::iterator iter = m_list_table_connections.begin();
     sigc::connection the_connection = *iter;
     the_connection.disconnect();
     m_list_table_connections.erase(iter);
  }

  //Remove all current items:
  //while(m_group_tables->get_n_children() > 0)
  //  m_group_tables->remove_child(0);
}


void Dialog_RelationshipsOverview::draw_tables()
{
  //Disconnect signal handlers for all current items,
  //so sigc::bind()ed RefPtrs can be released.
  while(m_list_table_connections.size())
  {
     type_list_connections::iterator iter = m_list_table_connections.begin();
     sigc::connection the_connection = *iter;
     the_connection.disconnect();
     m_list_table_connections.erase(iter);
  }

  //Remove all current items:
  while(m_group_tables->get_n_children() > 0)
    m_group_tables->remove_child(0);

  Document* document = dynamic_cast<Document*>(get_document());
  if(document)
  {
    double max_table_height = 0;
    double sizex = 10;
    double sizey = 10;

    //Create tables canvas items, with lists of fields:
    Document::type_listTableInfo tables = document->get_tables();
    for(Document::type_listTableInfo::iterator iter = tables.begin(); iter != tables.end(); ++iter)
    {
      sharedptr<TableInfo> info = *iter;
      const Glib::ustring table_name = info->get_name();

      float table_x = 0;
      float table_y = 0;
      //Get the x and y position from the document:
      if(!document->get_table_overview_position(table_name, table_x, table_y))
      {
        table_x = sizex;
        table_y = sizey;
        document->set_table_overview_position(table_name, table_x, table_y);
        m_modified = true;
      }

      Document::type_vec_fields fields = document->get_table_fields(table_name);

      Glib::RefPtr<CanvasGroupDbTable> table_group =
        CanvasGroupDbTable::create(info->get_name(), info->get_title_or_name(), fields, table_x, table_y);
      m_group_tables->add_child(table_group);
      m_canvas.associate_with_grid(table_group); //Make snapping work.

      table_group->signal_moved().connect(
        sigc::mem_fun(*this, &Dialog_RelationshipsOverview::on_table_moved));

      sigc::connection the_connection = table_group->signal_show_context().connect( sigc::bind(
        sigc::mem_fun(*this, &Dialog_RelationshipsOverview::on_table_show_context),
        table_group) );
      m_list_table_connections.push_back(the_connection);

      //tv->x2 = tv->x1 + table_width;
      //tv->y2 = tv->y1 + table_height;

      sizex += table_group->get_table_width() + 10;

      max_table_height = std::max(max_table_height, table_group->get_table_height());
    }

    m_canvas.set_bounds(0, 0, sizex, max_table_height * tables.size());
  }
}

void Dialog_RelationshipsOverview::draw_lines()
{
  //Remove all current items:
  while(m_group_lines->get_n_children() > 0)
    m_group_lines->remove_child(0);

  Document* document = dynamic_cast<Document*>(get_document());
  if(document)
  {
    //Create the lines linking tables to show relationships:
    Document::type_listTableInfo tables = document->get_tables();
    for(Document::type_listTableInfo::iterator iter = tables.begin(); iter != tables.end(); ++iter)
    {
      sharedptr<TableInfo> info = *iter;
      const Glib::ustring table_name = info->get_name();

      Document::type_vec_relationships m_relationships = document->get_relationships(table_name);
      Document::type_vec_fields fields = document->get_table_fields(table_name);

      for(Document::type_vec_relationships::const_iterator rit = m_relationships.begin(); rit != m_relationships.end(); ++rit)
      {
        sharedptr<const Relationship> relationship = *rit;
        if(!relationship)
          continue;

        Glib::RefPtr<CanvasGroupDbTable> group_from = get_table_group(relationship->get_from_table());

        double from_field_x = 0.0;
        double from_field_y = 0.0;

        if(group_from)
        {
          double temp_x = 0.0;
          double temp_y = 0.0;
          group_from->get_xy(temp_x, temp_y);

          from_field_x = temp_x;
          from_field_y = temp_y + group_from->get_field_y(relationship->get_from_field());
        }

        //Only primary keys can be to fields:
        if(true) //document->get_field(relationship->get_to_table(), relationship->get_to_field())->get_primary_key())
        {
          Glib::RefPtr<CanvasGroupDbTable> group_to = get_table_group(relationship->get_to_table());

          double to_field_x = 0.0;
          double to_field_y = 0.0;

          if(group_to)
          {
            double temp_x = 0.0;
            double temp_y = 0.0;
            group_to->get_xy(temp_x, temp_y);
            to_field_x = temp_x;
            to_field_y = temp_y + group_to->get_field_y(relationship->get_to_field());
          }

          //Start the line from the right of the from table instead of the left, if the to table is to the right:
          double extra_line = 0; //An extra horizontal line before the real diagonal line starts.
          if(to_field_x > from_field_x)
          {
            from_field_x += group_from->get_table_width();
            extra_line = 20;
          }
          else
          {
            to_field_x += group_to->get_table_width();
            extra_line = -20;
          }

          //Create the line:
          Glib::RefPtr<CanvasLineMovable> line = CanvasLineMovable::create();
          double points_coordinates[] = {from_field_x, from_field_y,
            from_field_x + extra_line, from_field_y,
            to_field_x - extra_line, to_field_y,
            to_field_x, to_field_y};
          Goocanvas::Points points(4, points_coordinates);
          line->property_points() = points;
          line->property_stroke_color() = "black";
          line->property_line_width() = 1.0;
          line->property_start_arrow() = false;
          line->property_end_arrow() = true;
          line->property_arrow_width() = 10.0;
          line->property_arrow_length() = 10.0;
          line->set_movement_allowed(false, false); //Don't let the user move this by dragging.
          m_group_lines->add_child(line);

          //Create a text item, showing the name of the relationship on the line:
          //
          //Raise or lower the text slightly to make it show above the line when horizontal,
          //and to avoid overwriting a relationship in the other direction:
          //TODO: This is not very clear. Investigate how other systems show this.
          double y_offset = (from_field_x < to_field_x) ? -10 : +10;
          if(from_field_x == to_field_x)
            y_offset = (from_field_y < to_field_y) ? -10 : +10;

          const double text_x = (from_field_x + to_field_x) / 2;
          const double text_y = ((from_field_y + to_field_y) / 2) + y_offset;
          Glib::RefPtr<CanvasTextMovable> text = CanvasTextMovable::create(relationship->get_title_or_name(),
            text_x, text_y, -1, //TODO: Calc a suitable width.
            Goocanvas::ANCHOR_CENTER);
          text->property_font() = "Sans 10";
          text->property_use_markup() = true;
          text->set_movement_allowed(false, false); //Move only as part of the parent group.
          m_group_lines->add_child(text);
        }
      }
    }
  }
  else
  {
    std::cout << "ERROR: Could not retrieve the Glom document." << std::endl;
  }
}

void Dialog_RelationshipsOverview::load_from_document()
{
  draw_tables();
  draw_lines();
}

void Dialog_RelationshipsOverview::on_response(int /* id */)
{
  if(m_modified && get_document())
    get_document()->set_modified();

  hide();
}

void Dialog_RelationshipsOverview::on_menu_file_print()
{
  print_or_preview(Gtk::PRINT_OPERATION_ACTION_PRINT_DIALOG);
}

void Dialog_RelationshipsOverview::on_menu_file_page_setup()
{
  //Show the page setup dialog, asking it to start with the existing settings:
  Glib::RefPtr<Gtk::PageSetup> new_page_setup =
      Gtk::run_page_setup_dialog(*this, m_refPageSetup, m_refSettings);

  //Save the chosen page setup dialog for use when printing, previewing, or
  //showing the page setup dialog again:
  m_refPageSetup = new_page_setup;
}

void Dialog_RelationshipsOverview::on_menu_view_showgrid()
{
  if(m_action_showgrid->get_active())
  {
    m_canvas.set_grid_gap(40);
  }
  else
  {
    m_canvas.remove_grid();
  }
}

void Dialog_RelationshipsOverview::on_menu_file_save()
{
}

void Dialog_RelationshipsOverview::print_or_preview(Gtk::PrintOperationAction print_action)
{
  //Create a new PrintOperation with our PageSetup and PrintSettings:
  //(We use our derived PrintOperation class)
  Glib::RefPtr<PrintOperationRelationshipsOverview> print = PrintOperationRelationshipsOverview::create();
  print->set_canvas(&m_canvas);

  print->set_track_print_status();
  print->set_default_page_setup(m_refPageSetup);
  print->set_print_settings(m_refSettings);

  //print->signal_done().connect(sigc::bind(sigc::mem_fun(*this,
  //                &ExampleWindow::on_printoperation_done), print));

  try
  {
    print->run(print_action /* print or preview */, *this);
  }
  catch (const Gtk::PrintError& ex)
  {
    //See documentation for exact Gtk::PrintError error codes.
    std::cerr << "An error occurred while trying to run a print operation:"
        << ex.what() << std::endl;
  }
}

Glib::RefPtr<CanvasGroupDbTable> Dialog_RelationshipsOverview::get_table_group(const Glib::ustring& table_name)
{
  const int count = m_group_tables->get_n_children();
  for(int i = 0; i < count; ++i)
  {
    Glib::RefPtr<Goocanvas::Item> item = m_group_tables->get_child(i);
    Glib::RefPtr<CanvasGroupDbTable> table_item = Glib::RefPtr<CanvasGroupDbTable>::cast_dynamic(item);
    if(table_item && (table_item->get_table_name() == table_name))
    {
      return table_item;
    }

  }

  return Glib::RefPtr<CanvasGroupDbTable>();
}

void Dialog_RelationshipsOverview::on_table_moved(const Glib::RefPtr<CanvasItemMovable>& item, double /* x_offset */, double /* y_offset */)
{
  Glib::RefPtr<CanvasGroupDbTable> table = 
    Glib::RefPtr<CanvasGroupDbTable>::cast_dynamic(item);
  if(!table)
    return;

  Document* document = dynamic_cast<Document*>(get_document());
  if(document && table)
  {
    //Save the new position in the document:
    double x = 0;
    double y = 0;
    table->get_xy(x, y);
    document->set_table_overview_position(table->get_table_name(), x, y);
  }

  //It is probably incredibly inefficient to recreate the lines repeatedly while dragging a table,
  //but it seems to work OK, and it makes the code much simpler.
  //If this is a problem, we should just change the start/end coordinates of any lines connected to the moved table.
  draw_lines();
}

void Dialog_RelationshipsOverview::on_table_show_context(guint button, guint32 activate_time, Glib::RefPtr<CanvasGroupDbTable> table)
{
  if(m_action_edit_fields)
  {
    // Disconnect the previous handler,
    // and connect a new one, with the correct table as a bound parameter:
    m_connection_edit_fields.disconnect();
    m_connection_edit_fields = m_action_edit_fields->signal_activate().connect(
      sigc::bind( sigc::mem_fun(*this, &Dialog_RelationshipsOverview::on_context_menu_edit_fields), table ));

    m_connection_edit_relationships.disconnect();
    m_connection_edit_relationships = m_action_edit_relationships->signal_activate().connect(
      sigc::bind( sigc::mem_fun(*this, &Dialog_RelationshipsOverview::on_context_menu_edit_relationships), table ));
  }

  if(m_context_menu)
    m_context_menu->popup(button, activate_time);

}

void Dialog_RelationshipsOverview::setup_context_menu()
{
  m_context_menu_action_group = Gtk::ActionGroup::create();

  m_context_menu_action_group->add(Gtk::Action::create("ContextMenu", "Context Menu") );

  m_action_edit_fields = Gtk::Action::create("ContextEditFields", _("Edit _Fields"));
  m_context_menu_action_group->add(m_action_edit_fields);

  m_action_edit_relationships = Gtk::Action::create("ContextEditRelationships", _("Edit _Relationships"));
  m_context_menu_action_group->add(m_action_edit_relationships);

  m_context_menu_uimanager = Gtk::UIManager::create();
  m_context_menu_uimanager->insert_action_group(m_context_menu_action_group);

  try
  {
    Glib::ustring ui_info =
    "<ui>"
    "  <popup name='ContextMenu'>"
    "  <menuitem action='ContextEditFields'/>"
    "  <menuitem action='ContextEditRelationships'/>"
    "  </popup>"
    "</ui>";

    m_context_menu_uimanager->add_ui_from_string(ui_info);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << "building menus failed: " <<  ex.what();
  }

  //Get the menu:
  m_context_menu = dynamic_cast<Gtk::Menu*>( m_context_menu_uimanager->get_widget("/ContextMenu") );
}

void Dialog_RelationshipsOverview::on_context_menu_edit_fields(Glib::RefPtr<CanvasGroupDbTable> table)
{
  Application* pApp = Application::get_application();
  if(pApp && table)
  {
    pApp->do_menu_developer_fields(*this, table->get_table_name());
    //draw_tables();
    //draw_lines();
  }
}

void Dialog_RelationshipsOverview::on_context_menu_edit_relationships(Glib::RefPtr<CanvasGroupDbTable> table)
{
  Application* pApp = Application::get_application();
  if(pApp && table)
  {
    pApp->do_menu_developer_relationships(*this, table->get_table_name());
    //draw_tables();
    //draw_lines();
  }
}


void Dialog_RelationshipsOverview::on_scroll_value_changed()
{
  if(!m_scrolledwindow_canvas)
    return;

  double width = m_scrolledwindow_canvas->get_hadjustment()->get_page_size();
  double height = m_scrolledwindow_canvas->get_vadjustment()->get_page_size();
  //double x = m_scrolledwindow_canvas->get_hadjustment()->get_value();
  //double y = m_scrolledwindow_canvas->get_vadjustment()->get_value();

  //Make sure that the canvas bounds are as big as the scrollable area:
  double old_left = 0;
  double old_top = 0;
  double old_right = 0;
  double old_bottom = 0;
  m_canvas.get_bounds(old_left, old_top, old_right, old_bottom);

  const double old_height = old_bottom - old_top;
  const double old_width = old_right - old_left;

  if( (width > old_width) ||
      (height > old_height) )
  {
    m_canvas.set_bounds(0, 0, width, height);
  }
}

} //namespace Glom
