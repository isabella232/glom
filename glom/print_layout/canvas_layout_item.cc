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

#include <glom/utils_ui.h>
#include "canvas_layout_item.h"
#include <glom/utility_widgets/canvas/canvas_rect_movable.h>
#include <glom/utility_widgets/canvas/canvas_text_movable.h>
#include <glom/utility_widgets/canvas/canvas_image_movable.h>
#include <glom/utility_widgets/canvas/canvas_line_movable.h>
#include <glom/utility_widgets/canvas/canvas_group_movable.h>
#include <glom/utility_widgets/canvas/canvas_table_movable.h>
#include <libglom/data_structure/layout/layoutitem_button.h>
#include <libglom/data_structure/layout/layoutitem_text.h>
#include <libglom/data_structure/layout/layoutitem_image.h>
#include <libglom/data_structure/layout/layoutitem_field.h>
#include <libglom/data_structure/layout/layoutitem_line.h>
#include <libglom/data_structure/layout/layoutitem_portal.h>
#include <libglom/data_structure/layout/report_parts/layoutitem_fieldsummary.h>
#include <libglom/data_structure/glomconversions.h>
#include <glibmm/i18n.h>
#include <math.h>
#include <algorithm> //For std::max().

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

void CanvasLayoutItem::apply_formatting(const Glib::RefPtr<CanvasTextMovable>& canvas_item, const sharedptr<const LayoutItem_WithFormatting>& layout_item)
{
  if(!canvas_item)
    return;

  if(!layout_item)
    return;

  //Horizontal alignment:
  const FieldFormatting::HorizontalAlignment alignment =
    layout_item->get_formatting_used_horizontal_alignment();
  const Pango::Alignment x_align = (alignment == FieldFormatting::HORIZONTAL_ALIGNMENT_LEFT ? Pango::ALIGN_LEFT : Pango::ALIGN_RIGHT);
  canvas_item->property_alignment() = x_align;

  const FieldFormatting& formatting = layout_item->get_formatting_used();

  Glib::ustring font = formatting.get_text_format_font();
  if(font.empty())
  {
    font = "Serif 9";

    //Set it in the input parameter,
    //so that this is the default:
    //TODO? formatting.set_text_format_font(font);
  }

  canvas_item->set_font_points(font);

  //TODO: Handle horizontal alignment.

  //TODO: Are these sensible properties? Maybe we need to use markup:
  //TODO: Use the negative color.
  const Glib::ustring fg = formatting.get_text_format_color_foreground();
  if(!fg.empty())
  {
    canvas_item->property_stroke_color() = fg;
  }

  const Glib::ustring bg = formatting.get_text_format_color_background();
  if(!bg.empty())
  {
    canvas_item->property_fill_color() = bg;
  }
}

void CanvasLayoutItem::on_resized()
{
  Glib::RefPtr<CanvasImageMovable> canvas_image = Glib::RefPtr<CanvasImageMovable>::cast_dynamic(get_child());
  if(canvas_image)
    canvas_image->scale_to_size();
}

void CanvasLayoutItem::set_layout_item(const sharedptr<LayoutItem>& layout_item)
{
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

int CanvasLayoutItem::get_rows_count_for_portal(const sharedptr<const LayoutItem_Portal>& portal, double& row_height)
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

  return max_rows;
}

Glib::RefPtr<CanvasItemMovable> CanvasLayoutItem::create_canvas_item_for_layout_item(const sharedptr<LayoutItem>& layout_item)
{
  sharedptr<LayoutItem_Line> line;

  Glib::RefPtr<CanvasItemMovable> child;
  Glib::RefPtr<Goocanvas::Item> child_item;
  sharedptr<LayoutItem_Text> text = sharedptr<LayoutItem_Text>::cast_dynamic(layout_item);
  if(text)
  {
    Glib::RefPtr<CanvasTextMovable> canvas_item = CanvasTextMovable::create();
    canvas_item->property_line_width() = 0;

    apply_formatting(canvas_item, text);

    canvas_item->set_text(text->get_text());
    child = canvas_item;
    child_item = canvas_item;
  }
  else
  {
    sharedptr<LayoutItem_Image> image = sharedptr<LayoutItem_Image>::cast_dynamic(layout_item);
    if(image)
    {
      Glib::RefPtr<CanvasImageMovable> canvas_item = CanvasImageMovable::create();
      Glib::RefPtr<Gdk::Pixbuf> pixbuf = Utils::get_pixbuf_for_gda_value(image->m_image);
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
      sharedptr<LayoutItem_Line> line = sharedptr<LayoutItem_Line>::cast_dynamic(layout_item);
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
        sharedptr<LayoutItem_Field> field = sharedptr<LayoutItem_Field>::cast_dynamic(layout_item);
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
          sharedptr<LayoutItem_Portal> portal = sharedptr<LayoutItem_Portal>::cast_dynamic(layout_item);
          if(portal)
          {
            Glib::RefPtr<CanvasTableMovable> canvas_item = CanvasTableMovable::create();
            canvas_item->property_vert_grid_line_width() = 1;
            canvas_item->property_horz_grid_line_width() = 1;
            canvas_item->property_stroke_color() = "black";

            //Show as many rows as can fit in the height.
            double row_height = 0;
            const int max_rows = get_rows_count_for_portal(portal, row_height);

            const LayoutGroup::type_list_items child_items = portal->get_items();

            for(guint row = 0; row < (guint)max_rows; ++row)
            {
              guint col = 0;
              for(LayoutGroup::type_list_items::const_iterator iter = child_items.begin(); iter != child_items.end(); ++iter)
              {
                sharedptr<LayoutItem> layout_item = *iter;

                //We use create_canvas_item_for_layout_item() instead of just
                //creating another CanvasLayoutItem, because that would be a group,
                //but goocanvas cannot yet support Groups inside Tables. murrayc.
                //TODO: Bug number.
                Glib::RefPtr<CanvasItemMovable> cell = create_canvas_item_for_layout_item(layout_item);
                if(cell)
                {
                  //Make sure that the width is sensible:
                  guint width = layout_item->get_display_width();
                  width = std::max(width, (guint)10);
                  cell->set_width_height(width, row_height);
                  std::cout << "DEBUG: width=" << width << std::endl;

                  //TODO: Add/Remove rows when resizing, instead of resizing the rows:
                  Glib::RefPtr<Goocanvas::Item> cell_as_item = CanvasItemMovable::cast_to_item(cell);
                  if(cell_as_item)
                  {
                    canvas_item->attach(cell_as_item,
                      col /* left_attach */, col+1 /* right_attach */,
                      row /* top_attach */, row + 1 /* right_attach */,
                      Gtk::FILL, (Gtk::AttachOptions)Gtk::FILL | Gtk::EXPAND);
                  }
                }

                ++col;
              }
            }

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

  if(child && child_item)
  {
    //child_item->property_pointer_events() =
    //  (Goocanvas::PointerEvents)(Goocanvas::EVENTS_VISIBLE_FILL & GOO_CANVAS_EVENTS_VISIBLE_STROKE);

    //Set the position and dimensions of this group to match the child:
    double x = 0;
    double y = 0;
    double width = 0;
    double height = 0;
    layout_item->get_print_layout_position(x, y, width, height);
    child->set_width_height(width, height);
    //std::cout << "debug: " << G_STRFUNC << ": item x=" << x << std::endl;
  }

  //Scale images.
  //This can only be done after setting the size:
  Glib::RefPtr<CanvasImageMovable> canvas_image = Glib::RefPtr<CanvasImageMovable>::cast_dynamic(child);
  if(canvas_image)
  {
    canvas_image->scale_to_size();

    //It will also be rescaled when this canvas item is resized - see on_resized().
  }

  return child;
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

      Glib::RefPtr<Gdk::Pixbuf> pixbuf = Utils::get_pixbuf_for_gda_value(value);
      if(pixbuf) //TODO: Remove this if() check when goocanvas has my patch to avoid crashes when this is NULL.
        canvas_item->set_image(pixbuf);

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

} //namespace Glom
