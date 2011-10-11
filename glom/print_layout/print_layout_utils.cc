/* Glom
 *
 * Copyright (C) 2001-2011 Openismus GmbH
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

#include <glom/print_layout/print_layout_utils.h>
#include <glom/print_layout/canvas_print_layout.h>
#include <glom/print_layout/printoperation_printlayout.h>
#include <iostream>

namespace Glom
{

namespace PrintLayoutUtils
{

static Gtk::Unit get_units()
{
  //TODO: Deal with this properly if we ever allow non-MM units in the UI:
  //m_canvas.property_units()
  return Gtk::UNIT_MM;
}

/* Get the start and end of the page, inside the margins.
 */
static void get_page_y_start_and_end(const Glib::RefPtr<const Gtk::PageSetup>& page_setup, guint page_number, double& y1, double& y2)
{
  y1 = 0;
  y2 = 0;
  
  const Gtk::PaperSize paper_size = page_setup->get_paper_size();
  const Gtk::Unit units = get_units();
  
  double page_height = 0;
  if(page_setup->get_orientation() == Gtk::PAGE_ORIENTATION_PORTRAIT) //TODO: Handle the reverse orientations too?
    page_height = paper_size.get_height(units);
  else
    page_height = paper_size.get_width(units);
    
  //y1:
  y1 = page_height * (page_number);  
  double y_border = page_setup->get_top_margin(units);
  while(y1 <= y_border)
    y1 += GRID_GAP;
  
  //y2:
  y2 = page_height * (page_number + 1);  
  y2 -= page_setup->get_bottom_margin(units); //TODO: Handle orientation here and wherever else we use the margin?

  //std::cout << G_STRFUNC << "page_number=" << page_number << ", y1=" << y1 << "y2=" << y2 << std::endl;
}


static void create_standard(const sharedptr<const LayoutGroup>& layout_group, const sharedptr<LayoutGroup>& print_layout_group, const Glib::RefPtr<const Gtk::PageSetup>& page_setup, double x, double& y, guint& page_number)
{
  if(!layout_group || !print_layout_group)
  {
    return;
  }

  double min_y = 0; //ignored;
  double max_y = 0;
  get_page_y_start_and_end(page_setup, page_number, min_y, max_y);

  const double gap = GRID_GAP;

  const Gtk::Unit units = get_units();
  const Gtk::PaperSize paper_size = page_setup->get_paper_size();
  const double item_width = paper_size.get_width(units) - x -
    page_setup->get_right_margin(units) - gap;
  const double field_height = ITEM_HEIGHT;

  //Show the group's title
  //(but do not fall back to the name, because unnamed groups are really wanted sometimes.)
  const Glib::ustring title = layout_group->get_title();
  if(!title.empty())
  {
    sharedptr<LayoutItem_Text> text = sharedptr<LayoutItem_Text>::create();
    text->set_text(title);
    text->m_formatting.set_text_format_font("Sans Bold 10");

    text->set_print_layout_position(x, y, item_width, field_height); //TODO: Enough and no more.
    y += field_height + gap; //padding.

    print_layout_group->add_item(text);

    //Start on the next page, if necessary:
    //TODO: Add a page if necessary:
    if( y >= max_y )
    {
      page_number += 1;
      get_page_y_start_and_end(page_setup, page_number, y, max_y);
    }
  }

  //Deal with a portal group: 
  const sharedptr<const LayoutItem_Portal> portal = sharedptr<const LayoutItem_Portal>::cast_dynamic(layout_group); 
  if(portal)
  {
    sharedptr<LayoutItem_Portal> portal_clone = glom_sharedptr_clone(portal);
    portal_clone->set_print_layout_row_height(field_height); //Otherwise it will be 0, which is useless.

    double height = ITEM_HEIGHT;
    const double rows_count = portal->get_rows_count();
    if(rows_count)
      height = ITEM_HEIGHT * rows_count;

    portal_clone->set_print_layout_position(x, y, item_width, height); //TODO: Enough and no more.
    y += height + gap; //padding.

    print_layout_group->add_item(portal_clone);

    //Start on the next page, if necessary:
    //TODO: Add a page if necessary:
    if( y >= max_y )
    {
      page_number += 1;
      get_page_y_start_and_end(page_setup, page_number, y, max_y);
    }

    return;
  }

  //Deal with a regular group:
  //Recurse into the group's child items:
  for(LayoutGroup::type_list_items::const_iterator iter = layout_group->m_list_items.begin(); iter != layout_group->m_list_items.end(); ++iter)
  {
    const sharedptr<const LayoutItem> item = *iter;
    if(!item)
      continue;

    const sharedptr<const LayoutGroup> group = sharedptr<const LayoutGroup>::cast_dynamic(item);
    if(group && !portal)
    {
      //Recurse: //TODO: Handle portals separately:
      create_standard(group, print_layout_group, page_setup, x, y, page_number);
    }
    else
    {
      //Add field titles, if necessary:
      const double title_width = ITEM_WIDTH_WIDE; //TODO: Calculate it based on the widest in the column. Or just halve the column to start.
      const sharedptr<const LayoutItem_Field> field = sharedptr<const LayoutItem_Field>::cast_dynamic(item);
      if(field)
      {
        sharedptr<LayoutItem_Text> text = sharedptr<LayoutItem_Text>::create();
        text->set_text(field->get_title_or_name() + ":");
        text->set_print_layout_position(x, y, title_width, field_height); //TODO: Enough and no more.
        text->m_formatting.set_text_format_font("Sans 10");

        print_layout_group->add_item(text);
      }

      //Add the item, such as a field:
      sharedptr<LayoutItem> clone = glom_sharedptr_clone(item);

      double item_x = x;
      if(field)
        item_x += (title_width + gap);

      const double field_width = item_width - title_width - gap;
      clone->set_print_layout_position(item_x, y, field_width, field_height); //TODO: Enough and no more.
      y += field_height + gap; //padding.

      print_layout_group->add_item(clone);

      //Start on the next page, if necessary:
      //TODO: Add a page if necessary:
      if( y >= max_y )
      {
        page_number += 1;
        get_page_y_start_and_end(page_setup, page_number, y, max_y);
      }
    }
  }
}

sharedptr<PrintLayout> create_standard(const Glib::RefPtr<const Gtk::PageSetup>& page_setup, const Glib::ustring& table_name, const Document* document)
{
  sharedptr<PrintLayout> print_layout = sharedptr<PrintLayout>::create();  
  
  //Start inside the border, on the next grid line:
  double y = 0;
  double max_y = 0; //ignored
  guint page_number = 0;
  get_page_y_start_and_end(page_setup, page_number, y, max_y);
  
  double x = 0;
  double x_border = 0;
  if(page_setup)
    x_border = page_setup->get_left_margin(get_units());
  while(x <= x_border)
    x += GRID_GAP;
  
  //The table title:
  const Glib::ustring title = document->get_table_title_singular(table_name);
  if(!title.empty())
  {
    sharedptr<LayoutItem_Text> text = sharedptr<LayoutItem_Text>::create();
    text->set_text(title);
    text->m_formatting.set_text_format_font("Sans Bold 12");

    const double field_height = ITEM_HEIGHT;
    text->set_print_layout_position(x, y, ITEM_WIDTH_WIDE, field_height); //TODO: Enough and no more.
    y += field_height + GRID_GAP; //padding.

    print_layout->m_layout_group->add_item(text);
  }

  //The layout:
  //TODO: Use fill_layout_group_field_info()?
  const Document::type_list_layout_groups layout_groups = 
    document->get_data_layout_groups("details", table_name); //TODO: layout_platform.
  for(Document::type_list_layout_groups::const_iterator iter = layout_groups.begin(); iter != layout_groups.end(); ++iter)
  {
    const sharedptr<const LayoutGroup> group = *iter;
    if(!group)
      continue;

    create_standard(group, print_layout->m_layout_group, page_setup, x, y, page_number);
  }

  //Add extra pages if necessary:
  if(page_number >= print_layout->get_page_count())
  {
    print_layout->set_page_count(page_number + 1);
  }
  
  return print_layout;
}

void do_print_layout(const sharedptr<const PrintLayout>& print_layout, const FoundSet& found_set, bool preview, const Document* document, Gtk::Window* transient_for)
{
  if(!print_layout)
  {
    std::cerr << G_STRFUNC << ": print_layout was null" << std::endl;
    return;
  }
  
  if(!document)
  {
    std::cerr << G_STRFUNC << ": document was null" << std::endl;
    return;
  }
  
  //TODO: All this to be null when we allow that in Gtk::PrintOperation::run().
  if(!transient_for)
  {
    std::cerr << G_STRFUNC << ":  transient_for was null" << std::endl;
    return;
  }

  Canvas_PrintLayout canvas;
  canvas.set_document(const_cast<Document*>(document)); //We const_cast because, for this use, it will not be changed.
  canvas.set_print_layout(found_set.m_table_name, print_layout);

  //Do not show things that are only for editing the print layout:
  canvas.remove_grid();
  canvas.set_rules_visibility(false);
  canvas.set_outlines_visibility(false);

  //Create a new PrintOperation with our PageSetup and PrintSettings:
  //(We use our derived PrintOperation class)
  Glib::RefPtr<PrintOperationPrintLayout> print = PrintOperationPrintLayout::create();
  print->set_canvas(&canvas);

  print->set_track_print_status();

  //TODO: Put this in a utility function.
  Glib::RefPtr<Gtk::PageSetup> page_setup;
  const Glib::ustring key_file_text = print_layout->get_page_setup();
  if(!key_file_text.empty())
  {
    Glib::KeyFile key_file;
    key_file.load_from_data(key_file_text);
    page_setup = Gtk::PageSetup::create_from_key_file(key_file);
  }

  print->set_default_page_setup(page_setup);

  //print->set_print_settings(m_refSettings);

  //print->signal_done().connect(sigc::bind(sigc::mem_fun(*this,
  //                &ExampleWindow::on_printoperation_done), print));

  canvas.fill_with_data(found_set);

  try
  {
    print->run(
      (preview ? Gtk::PRINT_OPERATION_ACTION_PREVIEW : Gtk::PRINT_OPERATION_ACTION_PRINT_DIALOG),
      *transient_for);
  }
  catch (const Gtk::PrintError& ex)
  {
    //See documentation for exact Gtk::PrintError error codes.
    std::cerr << "An error occurred while trying to run a print operation:"
        << ex.what() << std::endl;
  }
}

} //namespace PrintLayoutUtils

} //namespace Glom

