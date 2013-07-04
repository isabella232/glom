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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#include <glom/utils_ui.h>
#include "canvas_layout_item.h"
#include <glom/utility_widgets/canvas/canvas_rect_movable.h>
#include <glom/utility_widgets/canvas/canvas_text_movable.h>
#include <glom/utility_widgets/canvas/canvas_image_movable.h>
#include <glom/utility_widgets/canvas/canvas_line_movable.h>
#include <glom/utility_widgets/canvas/canvas_group_movable.h>
#include <glom/utility_widgets/canvas/canvas_table_movable.h>
#include <glom/appwindow.h>
#include <libglom/data_structure/layout/layoutitem_button.h>
#include <libglom/data_structure/layout/layoutitem_text.h>
#include <libglom/data_structure/layout/layoutitem_image.h>
#include <libglom/data_structure/layout/layoutitem_field.h>
#include <libglom/data_structure/layout/layoutitem_line.h>
#include <libglom/data_structure/layout/layoutitem_portal.h>
#include <libglom/data_structure/layout/report_parts/layoutitem_fieldsummary.h>
#include <libglom/data_structure/glomconversions.h>
#include <glibmm/i18n.h>
#include <cmath>
#include <algorithm> //For std::max().
#include <iostream>

namespace Glom
{

CanvasLayoutItem::CanvasLayoutItem()
{
  //Rescale images when the canvas item is resized:
  signal_resized().connect( sigc::mem_fun(*this, &CanvasLayoutItem::on_resized) );
}

CanvasLayoutItem::~CanvasLayoutItem()
{
}

Glib::RefPtr<CanvasLayoutItem> CanvasLayoutItem::create()
{
  return Glib::RefPtr<CanvasLayoutItem>(new CanvasLayoutItem());
}

std::shared_ptr<LayoutItem> CanvasLayoutItem::get_layout_item()
{
  return m_layout_item;
}

void CanvasLayoutItem::apply_formatting(const Glib::RefPtr<CanvasTextMovable>& canvas_item, const std::shared_ptr<const LayoutItem_WithFormatting>& layout_item)
{
  if(!canvas_item)
    return;

  if(!layout_item)
    return;

  //Horizontal alignment:
  const Formatting::HorizontalAlignment alignment =
    layout_item->get_formatting_used_horizontal_alignment();
  const Pango::Alignment x_align = (alignment == Formatting::HORIZONTAL_ALIGNMENT_LEFT ? Pango::ALIGN_LEFT : Pango::ALIGN_RIGHT);
  canvas_item->property_alignment() = x_align;

  const Formatting& formatting = layout_item->get_formatting_used();

  Glib::ustring font = formatting.get_text_format_font();
  if(font.empty())
  {
    //Just a sanity-check default that should never actually be used:
    font = "Serif 9";
  }

  canvas_item->set_font_points(font);

  //TODO: Handle horizontal alignment.

  //TODO: Are these sensible properties? Maybe we need to use markup:
  //TODO: Use the negative color.
  const Glib::ustring fg = formatting.get_text_format_color_foreground();
  if(!fg.empty())
  {
    //GooCanvasText uses fill-color for the text foreground color.
    //Presumably stroke-color would be an outline, if we had a line-width of >0 width.
    canvas_item->property_fill_color() = fg;
  }
  const Glib::ustring bg = formatting.get_text_format_color_background();
  if(!bg.empty())
  {
    //TODO: Add a filled rectangle.
  }
}

void CanvasLayoutItem::on_resized()
{
  Glib::RefPtr<CanvasImageMovable> canvas_image = Glib::RefPtr<CanvasImageMovable>::cast_dynamic(get_child());
  if(canvas_image)
    canvas_image->scale_to_size();
}

void CanvasLayoutItem::set_layout_item(const std::shared_ptr<LayoutItem>& layout_item)
{
  //TODO: If we can ever avoid this the also update the CanvasLayoutItem class documentation.
  if(!get_canvas())
  {
    std::cerr << G_STRFUNC << ": get_canvas() returned null. This should not be called before the CanvasLayoutItem is in a canvas due to goocanvas bug https://bugzilla.gnome.org/show_bug.cgi?id=657592#c16 ." << std::endl;
  }
  
  //Add the new child:
  m_layout_item = layout_item;

  if(!m_layout_item)
    std::cerr << G_STRFUNC << ": item was NULL." << std::endl;

  Glib::RefPtr<CanvasItemMovable> child_item = create_canvas_item_for_layout_item(m_layout_item);

  if(child_item)
  {
    //child_item->property_pointer_events() =
    //  (Goocanvas::PointerEvents)(Goocanvas::EVENTS_VISIBLE_FILL & GOO_CANVAS_EVENTS_VISIBLE_STROKE);

    //Set the position and dimensions of this group to match the child:
    double x = 0;
    double y = 0;
    double width = 0;
    double height = 0;
    m_layout_item->get_print_layout_position(x, y, width, height);

    set_xy(x, y);
    set_width_height(width, height);
    //std::cout << "debug: " << G_STRFUNC << ": item x=" << x << std::endl;

    set_child(child_item);
  }

  //Scale images.
  //This can only be done after setting the size:
  Glib::RefPtr<CanvasImageMovable> canvas_image = Glib::RefPtr<CanvasImageMovable>::cast_dynamic(child_item);
  if(canvas_image)
  {
    canvas_image->scale_to_size();
  }
}

//TODO: Remove this?
int CanvasLayoutItem::get_rows_count_for_portal(const std::shared_ptr<const LayoutItem_Portal>& portal, double& row_height)
{
  if(!portal)
  {
    row_height = 0;
    return 0;
  }

  row_height = std::max(portal->get_print_layout_row_height(), (double)1); //Avoid 0, because that makes the whole thing zero sized.

  double ignore_x = 0;
  double ignore_y = 0;
  double total_width = 0;
  double total_height = 0;
  portal->get_print_layout_position(ignore_x, ignore_y, total_width, total_height);

  const double max_rows_fraction = total_height / row_height;
  double max_rows = 0;
  modf(max_rows_fraction, &max_rows);
  std::cout << "debug: max_rows=" << max_rows << ", for total_height=" << total_height << ", row_height=" << row_height << std::endl;

  return max_rows;
}

Glib::RefPtr<CanvasItemMovable> CanvasLayoutItem::create_canvas_item_for_layout_item(const std::shared_ptr<LayoutItem>& layout_item)
{
  Glib::RefPtr<CanvasItemMovable> child;
  Glib::RefPtr<Goocanvas::Item> child_item;
  std::shared_ptr<LayoutItem_Text> text = std::dynamic_pointer_cast<LayoutItem_Text>(layout_item);
  if(text)
  {
    Glib::RefPtr<CanvasTextMovable> canvas_item = CanvasTextMovable::create();
    canvas_item->property_line_width() = 0;

    apply_formatting(canvas_item, text);

    canvas_item->set_text(text->get_text(AppWindow::get_current_locale()));
    child = canvas_item;
    child_item = canvas_item;
  }
  else
  {
    std::shared_ptr<LayoutItem_Image> image = std::dynamic_pointer_cast<LayoutItem_Image>(layout_item);
    if(image)
    {
      Glib::RefPtr<CanvasImageMovable> canvas_item = CanvasImageMovable::create();
      Glib::RefPtr<Gdk::Pixbuf> pixbuf = UiUtils::get_pixbuf_for_gda_value(image->m_image);
      if(pixbuf)
        canvas_item->set_image(pixbuf);
      else
        canvas_item->set_image_empty(); //show a no-image picture.

      //canvas_item->property_fill_color() = "white"; //This makes the whole area clickable, not just the outline stroke.
      //canvas_item->property_fill_color_rgba() = 0xFFFFFF00;

      child = canvas_item;
      child_item = canvas_item;
    }
    else
    {
      std::shared_ptr<LayoutItem_Line> line = std::dynamic_pointer_cast<LayoutItem_Line>(layout_item);
      if(line)
      {
        double start_x = 0;
        double start_y = 0;
        double end_x  = 0;
        double end_y = 0;
        line->get_coordinates(start_x, start_y, end_x, end_y);

        Glib::RefPtr<CanvasLineMovable> canvas_item = CanvasLineMovable::create();
        canvas_item->property_line_width() = line->get_line_width();
        canvas_item->property_stroke_color() = line->get_line_color();

        Goocanvas::Points points(2);
        points.set_coordinate(0, start_x, start_y);
        points.set_coordinate(0, end_x, end_y);
        canvas_item->property_points() = points;
        child = canvas_item;
        child_item = canvas_item;
      }
      else
      {
        std::shared_ptr<LayoutItem_Field> field = std::dynamic_pointer_cast<LayoutItem_Field>(layout_item);
        if(field)
        {
          //Create an appropriate canvas item for the field type:
          if(field->get_glom_type() == Field::TYPE_IMAGE)
          {
            Glib::RefPtr<CanvasImageMovable> canvas_item = CanvasImageMovable::create();
            canvas_item->set_image_empty();

            child = canvas_item;
            child_item = canvas_item;
          }
          else //text, numbers, date, time, boolean:
          {
            Glib::RefPtr<CanvasTextMovable> canvas_item = CanvasTextMovable::create();
            canvas_item->property_line_width() = 0;
            apply_formatting(canvas_item, field);

            Glib::ustring name = field->get_name();
            if(name.empty())
              name = _("Choose Field");

            canvas_item->set_text(name);

            child = canvas_item;
            child_item = canvas_item;
          }
        }
        else
        {
          std::shared_ptr<LayoutItem_Portal> portal = std::dynamic_pointer_cast<LayoutItem_Portal>(layout_item);
          if(portal)
          {
            Glib::RefPtr<CanvasTableMovable> canvas_item = CanvasTableMovable::create();

            canvas_item->set_lines_details(
              portal->get_print_layout_row_line_width(),
              portal->get_print_layout_column_line_width(),
              portal->get_print_layout_line_color());

            gulong rows_count_min = 0;
            gulong rows_count_max = 0;
            portal->get_rows_count(rows_count_min, rows_count_max);
            add_portal_rows_if_necessary(canvas_item, portal, rows_count_min);

            child = canvas_item;
            child_item = canvas_item;
          }
          else if(layout_item)
          {
            std::cerr << G_STRFUNC << ": Unhandled LayoutItem type. part type=" << layout_item->get_part_type_name() << std::endl;
          }
          else
          {
            std::cerr << G_STRFUNC << ": NULL LayoutItem type." << std::endl;
          }
        }
      }
    }
  }

  return child;
}


Glib::RefPtr<Goocanvas::Item> CanvasLayoutItem::get_canvas_table_cell_child(const Glib::RefPtr<Goocanvas::Table>& table, int row, int col)
{
  Glib::RefPtr<Goocanvas::Item> result;

  if(!table)
    return result;

  const int count = table->get_n_children();
  for(int i = 0; i < count; ++i)
  {
    Glib::RefPtr<Goocanvas::Item> child = table->get_child(i);
    if(!child)
      continue;

    int column_value = 0;
    table->get_child_property(child, "column", column_value);
    int row_value = 0;
    table->get_child_property(child, "row", row_value);

    //This assumes that all items occupy only one cell:
    if( (column_value == col) &&
        (row_value == row) )
    {
      return child;
    }
  }

  return result;
}

void CanvasLayoutItem::add_portal_rows_if_necessary(guint rows_count)
{
  Glib::RefPtr<CanvasItemMovable> child = get_child();
  Glib::RefPtr<CanvasTableMovable> canvas_table = 
    Glib::RefPtr<CanvasTableMovable>::cast_dynamic(child);
  if(!canvas_table)
    return;

  std::shared_ptr<LayoutItem_Portal> portal = 
    std::dynamic_pointer_cast<LayoutItem_Portal>(get_layout_item());
  if(!portal)
  {
    std::cerr << G_STRFUNC << ": The layout item was not a portal." << std::endl;
    return;
  }

  add_portal_rows_if_necessary(canvas_table, portal, rows_count);
}

void CanvasLayoutItem::add_portal_rows_if_necessary(const Glib::RefPtr<CanvasTableMovable>& canvas_table, const std::shared_ptr<LayoutItem_Portal>& portal, guint rows_count)
{
  const double row_height = portal->get_print_layout_row_height();
  const LayoutGroup::type_list_items child_items = portal->get_items();

  for(guint row = 0; row < rows_count; ++row)
  {
    guint col = 0;
    const guint num_cols = child_items.size();
    bool something_expanded = false;
    for(LayoutGroup::type_list_items::const_iterator iter = child_items.begin(); iter != child_items.end(); ++iter)
    {
      //std::cout << "  row=" << row << ", col=" << col << std::endl;
      std::shared_ptr<LayoutItem> layout_item = *iter;

      //Check if a child already exists:
      Glib::RefPtr<Goocanvas::Item> existing_child = 
        get_canvas_table_cell_child(canvas_table, row, col);
      if(existing_child)
      {
        //std::cout << "    existing child" << std::endl;
        col++;
        continue;
      }

      //We use create_canvas_item_for_layout_item() instead of just
      //creating another CanvasLayoutItem, because that would be a group,
      //but goocanvas cannot yet support Groups inside Tables. murrayc.
      //TODO: Bug number.
      Glib::RefPtr<CanvasItemMovable> cell = create_canvas_item_for_layout_item(layout_item);
      Glib::RefPtr<Goocanvas::Item> cell_as_item = CanvasItemMovable::cast_to_item(cell);

      if(cell && cell_as_item)
      {
        const guint width = layout_item->get_display_width();
        bool expand = (width == 0);
        
        //If this is the last item, and no other item has expanded,
        //let this one expand,
        //Otherwise, we could allocate less space than the table has left.
        //TODO: Prevent the user from allocating _more_ space than the table has.
        if(!something_expanded && (col == (num_cols - 1)))
        {
          expand = true;
        }
        
        if(expand)
        {
            //Let the table allocate the width automatically:

            cell->set_width_height(-1, row_height);
            canvas_table->attach(cell_as_item,
              col /* left_attach */, col + 1 /* right_attach */,
              row /* top_attach */, row + 1 /* right_attach */,
              (Gtk::AttachOptions)(Gtk::FILL | Gtk::EXPAND), (Gtk::AttachOptions)(Gtk::FILL | Gtk::EXPAND));
  
            something_expanded = true;
        }
        else
        {
            //Enforce a width:

            //TODO: Add/Remove rows when resizing, instead of resizing the rows:
            cell->set_width_height(width, row_height);

            canvas_table->attach(cell_as_item,
              col /* left_attach */, col + 1 /* right_attach */,
              row /* top_attach */, row + 1 /* right_attach */,
              (Gtk::FILL), (Gtk::AttachOptions)(Gtk::FILL | Gtk::EXPAND));
  
            //Add a second item (an invisible rect) to make sure that the size is really used:
            Glib::RefPtr<Goocanvas::Rect> rect = 
              Goocanvas::Rect::create(0, 0, width, row_height);
            //TODO: Find out why this doesn't work: rect->property_stroke_pattern() = Cairo::RefPtr<Cairo::Pattern>();
            g_object_set(rect->gobj(), "stroke-pattern", (void*)0, (void*)0);
 
            canvas_table->attach(rect,
              col /* left_attach */, col + 1 /* right_attach */,
              row /* top_attach */, row + 1 /* right_attach */,
              Gtk::SHRINK, Gtk::SHRINK);
        }
      }

      ++col;
    }
  }
}

void CanvasLayoutItem::set_db_data(const Gnome::Gda::Value& value)
{
  std::shared_ptr<LayoutItem_Field> field = std::dynamic_pointer_cast<LayoutItem_Field>(m_layout_item);
  if(!field)
    return;

  Glib::RefPtr<CanvasItemMovable> child = get_child();
  if(!child)
    return;

  const Field::glom_field_type field_type = field->get_glom_type();
  switch(field->get_glom_type())
  {
    case(Field::TYPE_TEXT):
    case(Field::TYPE_NUMERIC):
    case(Field::TYPE_BOOLEAN):
    case(Field::TYPE_TIME):
    case(Field::TYPE_DATE):
    {
      Glib::RefPtr<CanvasTextMovable> canvas_item = Glib::RefPtr<CanvasTextMovable>::cast_dynamic(child);
      if(!canvas_item)
        return;

      Glib::ustring text_value = Conversions::get_text_for_gda_value(field_type, value, field->get_formatting_used().m_numeric_format);

      //The Postgres summary functions return NULL when summarising NULL records, but 0 is more sensible:
      if(text_value.empty() && std::dynamic_pointer_cast<const LayoutItem_FieldSummary>(field) && (field_type == Field::TYPE_NUMERIC))
      {
        //Use get_text_for_gda_value() instead of "0" so we get the correct numerical formatting:
        const Gnome::Gda::Value value = Conversions::parse_value(0);
        text_value = Conversions::get_text_for_gda_value(field_type, value, field->get_formatting_used().m_numeric_format);
      }

      canvas_item->set_text(text_value);
      break;
    }
    case(Field::TYPE_IMAGE):
    {
      Glib::RefPtr<CanvasImageMovable> canvas_item = Glib::RefPtr<CanvasImageMovable>::cast_dynamic(child);
      if(!canvas_item)
        return;

      //Get the height of the item (not of the pixbuf),
      //so we can scale the pixbuf:
      double width = 0;
      double height = 0;
      canvas_item->get_width_height(width, height);

      Glib::RefPtr<Gdk::Pixbuf> pixbuf = UiUtils::get_pixbuf_for_gda_value(value);
      if(pixbuf)
        canvas_item->set_image(pixbuf);
      else
        canvas_item->set_image_empty();

      break;
    }
    default:
      std::cerr << G_STRFUNC << ": unhandled field type." << std::endl;
  }

}

void CanvasLayoutItem::remove_empty_indicators()
{
  Glib::RefPtr<CanvasItemMovable> child = get_child();
  Glib::RefPtr<CanvasImageMovable> canvas_image = Glib::RefPtr<CanvasImageMovable>::cast_dynamic(child);
  if(canvas_image)
  {
    //Clear the no-image pixbuf from images:
    if(canvas_image->get_image_empty())
    {
      Glib::RefPtr<Gdk::Pixbuf> really_empty;
      canvas_image->property_pixbuf() = really_empty;
    }
  }
}

void CanvasLayoutItem::update_layout_position_from_canvas()
{
  std::shared_ptr<LayoutItem> layout_item = get_layout_item();
  if(!layout_item)
    return;

  //Get the actual position:
  double x = 0;
  double y = 0;
  get_xy(x, y);
  //std::cout << "debug: " << G_STRFUNC << ": x=" << x << std::endl;

  double width = 0;
  double height = 0;
  get_width_height(width, height);

  layout_item->set_print_layout_position(x, y, width, height);
}

} //namespace Glom
