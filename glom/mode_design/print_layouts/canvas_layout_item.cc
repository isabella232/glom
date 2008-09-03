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

#include "canvas_layout_item.h"
#include <glom/utility_widgets/canvas/canvas_rect_movable.h>
#include <glom/utility_widgets/canvas/canvas_text_movable.h>
#include <glom/utility_widgets/canvas/canvas_image_movable.h>
#include <glom/utility_widgets/canvas/canvas_line_movable.h>
#include <glom/utility_widgets/canvas/canvas_group_movable.h>
#include <glom/utility_widgets/canvas/canvas_table_movable.h>
#include <glom/libglom/data_structure/layout/layoutitem_button.h>
#include <glom/libglom/data_structure/layout/layoutitem_text.h>
#include <glom/libglom/data_structure/layout/layoutitem_image.h>
#include <glom/libglom/data_structure/layout/layoutitem_field.h>
#include <glom/libglom/data_structure/layout/layoutitem_line.h>
#include <glom/libglom/data_structure/layout/layoutitem_portal.h>
#include <glom/libglom/data_structure/layout/report_parts/layoutitem_fieldsummary.h>
#include <glom/libglom/data_structure/glomconversions.h>
#include <glibmm/i18n.h>
#include <math.h>

namespace Glom
{

CanvasLayoutItem::CanvasLayoutItem()
{
  //Rescale images when the canvas item is resized:
  signal_resized().connect( sigc::mem_fun(*this, &CanvasLayoutItem::on_resized) );
}

CanvasLayoutItem::CanvasLayoutItem(const sharedptr<LayoutItem>& layout_item)
{
  set_layout_item(layout_item);

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

Glib::RefPtr<CanvasLayoutItem> CanvasLayoutItem::create(const sharedptr<LayoutItem>& layout_item)
{
  return Glib::RefPtr<CanvasLayoutItem>(new CanvasLayoutItem(layout_item));
}

sharedptr<LayoutItem> CanvasLayoutItem::get_layout_item()
{
  return m_layout_item;
}

void CanvasLayoutItem::check_and_apply_formatting(const Glib::RefPtr<CanvasTextMovable>& canvas_item, FieldFormatting& formatting)
{
  if(!canvas_item)
    return;

  Glib::ustring font = formatting.get_text_format_font();
  if(font.empty())
  {
    font = "Serif 9";

    //Set it in the input parameter,
    //so that this is the default:
    formatting.set_text_format_font(font);    
  }

  canvas_item->set_font_points(font);

  //TODO: Are these sensible properties? Maybe we need to use markup:
  const Glib::ustring fg = formatting.get_text_format_color_foreground();
  if(!fg.empty())
    canvas_item->property_stroke_color() = fg;

  const Glib::ustring bg = formatting.get_text_format_color_background();
  if(!bg.empty())
  canvas_item->property_fill_color() = bg;
}

void CanvasLayoutItem::on_resized()
{
  Glib::RefPtr<CanvasImageMovable> canvas_image = Glib::RefPtr<CanvasImageMovable>::cast_dynamic(get_child());
  if(canvas_image)
    canvas_image->scale_to_size();
}

void CanvasLayoutItem::set_layout_item(const sharedptr<LayoutItem>& item)
{
  //Add the new child:
  m_layout_item = item;

  if(!m_layout_item)
    std::cerr << "CanvasLayoutItem::set_layout_item(): item was NULL." << std::endl;

  sharedptr<LayoutItem_Line> line;

  Glib::RefPtr<CanvasItemMovable> child;
  Glib::RefPtr<Goocanvas::Item> child_item;
  sharedptr<LayoutItem_Text> text = sharedptr<LayoutItem_Text>::cast_dynamic(m_layout_item);
  if(text)
  {
    Glib::RefPtr<CanvasTextMovable> canvas_item = CanvasTextMovable::create();
    canvas_item->property_line_width() = 0;

    FieldFormatting& formatting = text->m_formatting;
    check_and_apply_formatting(canvas_item, formatting);

    canvas_item->set_text(text->get_text());
    child = canvas_item;
    child_item = canvas_item;
  }
  else
  {
    sharedptr<LayoutItem_Image> image = sharedptr<LayoutItem_Image>::cast_dynamic(m_layout_item);
    if(image)
    {
      Glib::RefPtr<CanvasImageMovable> canvas_item = CanvasImageMovable::create();
      Glib::RefPtr<Gdk::Pixbuf> pixbuf = image->get_image_as_pixbuf();
      if(pixbuf)
        canvas_item->set_image(pixbuf);
      else
        canvas_item->set_image_empty(); //show a no-image picture.

      canvas_item->property_fill_color() = "white"; //This makes the whole area clickable, not just the outline stroke.
      child = canvas_item;
      child_item = canvas_item;
    }
    else
    {
      sharedptr<LayoutItem_Line> line = sharedptr<LayoutItem_Line>::cast_dynamic(m_layout_item);
      if(line)
      {
        double start_x = 0;
        double start_y = 0;
        double end_x  = 0;
        double end_y = 0;
        line->get_coordinates(start_x, start_y, end_x, end_y);
        
        Glib::RefPtr<CanvasLineMovable> canvas_item = CanvasLineMovable::create();
        canvas_item->property_line_width() = 1;
        canvas_item->property_stroke_color() = "black";

        Goocanvas::Points points(2);
        points.set_coordinate(0, start_x, start_y);
        points.set_coordinate(0, end_x, end_y);
        canvas_item->property_points() = points;
        child = canvas_item;
        child_item = canvas_item;
      }
      else
      {
        sharedptr<LayoutItem_Field> field = sharedptr<LayoutItem_Field>::cast_dynamic(m_layout_item);
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
         
            FieldFormatting& formatting = field->m_formatting;
            check_and_apply_formatting(canvas_item, formatting);

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
          sharedptr<LayoutItem_Portal> portal = sharedptr<LayoutItem_Portal>::cast_dynamic(m_layout_item);
          if(portal)
          {
            Glib::RefPtr<CanvasTableMovable> canvas_item = CanvasTableMovable::create();
            canvas_item->property_vert_grid_line_width() = 2;
            canvas_item->property_stroke_color() = "black";

            //Show as many rows as can fit in the height.
            const double row_height = portal->get_print_layout_row_height(); //TODO: Let the user specify the row height
            double ignore_x = 0;
            double ignore_y = 0;
            double total_width = 0;
            double total_height = 0;
            portal->get_print_layout_position(ignore_x, ignore_y, total_width, total_height);

            const double max_rows_fraction = total_height / row_height;
            double max_rows = 0;
            modf(max_rows_fraction, &max_rows);
            for(int i = 0; i < (int)max_rows; ++i)
            {
              Glib::RefPtr<CanvasRectMovable> rect_row = CanvasRectMovable::create();
              rect_row->property_fill_color() = "white"; //This makes the whole area clickable, not just the outline stroke.
              rect_row->property_line_width() = 1;
              rect_row->property_stroke_color() = "black";
              rect_row->set_width_height(total_width, row_height);

              //TODO: Add/Remove rows when resizing, instead of resizing the rows:
              canvas_item->attach(rect_row, 0 /* left_attach */, 1 /* right_attach */, i /* top_attach */, i + 1 /* right_attach */, (Gtk::AttachOptions)Gtk::FILL | Gtk::EXPAND, (Gtk::AttachOptions)Gtk::FILL | Gtk::EXPAND, 0.0, 0.0, 0.0, 0.0);
            }

            child = canvas_item;
            child_item = canvas_item;
          }
          else if(m_layout_item)
          {
            std::cerr << "CanvasLayoutItem::set_layout_item(): Unhandled LayoutItem type. part type=" << m_layout_item->get_part_type_name() << std::endl;
          }
          else
          {
            std::cerr << "CanvasLayoutItem::set_layout_item(): NULL LayoutItem type." << std::endl;
          }
        }
      }
    }
  }

  if(child && child_item)
  {
    //child_item->property_pointer_events() = 
    //  (Goocanvas::PointerEvents)(Goocanvas::CANVAS_EVENTS_VISIBLE_FILL & GOO_CANVAS_EVENTS_VISIBLE_STROKE);
      
    //Set the position and dimensions of this group to match the child:
    double x = 0;
    double y = 0;
    double width = 0;
    double height = 0;
    item->get_print_layout_position(x, y, width, height);

    set_xy(x, y);
    set_width_height(width, height);
    //std::cout << "CanvasLayoutItem::set_layout_item(): item x=" << x << std::endl;

    //Set the child (this removes the previous child):
    set_child(child);
  }

  //Scale images.
  //This can only be done after setting the size:
  Glib::RefPtr<CanvasImageMovable> canvas_image = Glib::RefPtr<CanvasImageMovable>::cast_dynamic(child);
  if(canvas_image)
  {
    canvas_image->scale_to_size();

    //It will also be rescaled when this canvas item is resized - see on_resized().
  }
}

void CanvasLayoutItem::set_db_data(const Gnome::Gda::Value& value)
{
  sharedptr<LayoutItem_Field> field = sharedptr<LayoutItem_Field>::cast_dynamic(m_layout_item);
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
      if(text_value.empty() && sharedptr<const LayoutItem_FieldSummary>::cast_dynamic(field) && (field_type == Field::TYPE_NUMERIC))
      {
        //Use get_text_for_gda_value() instead of "0" so we get the correct numerical formatting:
        Gnome::Gda::Value value = Conversions::parse_value(0);
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
      
      Glib::RefPtr<Gdk::Pixbuf> pixbuf = Conversions::get_pixbuf_for_gda_value(value);
      if(pixbuf) //TODO: Remove this if() check when goocanvas has my patch to avoid crashes when this is NULL.
        canvas_item->set_image(pixbuf);
     
      break;
    }
    default:
      std::cerr << "CanvasLayoutItem::set_db_data(): unhandled field type." << std::endl;
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

} //namespace Glom

