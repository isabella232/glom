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
#include <glom/libglom/data_structure/layout/layoutitem_button.h>
#include <glom/libglom/data_structure/layout/layoutitem_text.h>
#include <glom/libglom/data_structure/layout/layoutitem_image.h>
#include <glom/libglom/data_structure/layout/layoutitem_field.h>
#include <glom/libglom/data_structure/layout/report_parts/layoutitem_fieldsummary.h>
#include <glom/libglom/data_structure/glomconversions.h>
#include <glom/utility_widgets/imageglom.h> //For ImageGlom::scale_keeping_ratio().
#include <glibmm/i18n.h>

namespace Glom
{

CanvasLayoutItem::CanvasLayoutItem(const sharedptr<LayoutItem>& layout_item)
: CanvasGroupResizable()
{
  set_layout_item(layout_item);
}

CanvasLayoutItem::~CanvasLayoutItem()
{
}

Glib::RefPtr<CanvasLayoutItem> CanvasLayoutItem::create(const sharedptr<LayoutItem>& layout_item)
{
  return Glib::RefPtr<CanvasLayoutItem>(new CanvasLayoutItem(layout_item));
}

sharedptr<LayoutItem> CanvasLayoutItem::get_layout_item()
{
  return m_layout_item;
}

void CanvasLayoutItem::set_layout_item(const sharedptr<LayoutItem>& item)
{
  //Remove the current child:
  if(get_n_children())
    remove_child(0);

  //Add the new child:
  m_layout_item = item;

  Glib::RefPtr<CanvasItemMovable> child;
  sharedptr<LayoutItem_Text> text = sharedptr<LayoutItem_Text>::cast_dynamic(m_layout_item);
  if(text)
  {
    Glib::RefPtr<CanvasTextMovable> canvas_item = CanvasTextMovable::create();
    canvas_item->property_text() = text->get_text();
    canvas_item->set_text_size( text->get_print_layout_text_size() );
    child = canvas_item;
  }
  else
  {
    sharedptr<LayoutItem_Image> image = sharedptr<LayoutItem_Image>::cast_dynamic(m_layout_item);
    if(image)
    {
      Glib::RefPtr<CanvasRectMovable> canvas_item = CanvasRectMovable::create();
      canvas_item->property_fill_color() = "white"; //This makes the whole area clickable, not just the outline stroke:
      child = canvas_item;
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

          //Glib::ustring name = field->get_name();
          //  if(name.empty())
          //    name = _("Choose Field");
          
          //canvas_item->property_text() = name;

          child = canvas_item;
        }
        else //text, numbers, date, time, boolean:
        {
          Glib::RefPtr<CanvasTextMovable> canvas_item = CanvasTextMovable::create();
          canvas_item->set_text_size( field->get_print_layout_text_size() );

          Glib::ustring name = field->get_name();
            if(name.empty())
              name = _("Choose Field");
          
          canvas_item->property_text() = name;

          child = canvas_item;
        }
      }
      else
      {
        std::cerr << "CanvasLayoutItem::set_layout_item(): Unhandled LayoutItem type." << std::endl;
      }
    }
  }

  if(child)
  {
    //Set the position and dimensions:
    double x = 0;
    double y = 0;
    double width = 0;
    double height = 0;
    item->get_print_layout_position(x, y, width, height);

    child->set_xy(x, y);
    child->set_width_height(width, height);

    //We do this after setting the child dimensions, 
    //so that the manipulators are correct, 
    //though it would be nice if the manipulators moved when we repositioned the child explicitly.
    set_child(child);
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
    
      canvas_item->property_text() = text_value;
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
      
      //Scale the image down to fit the item:
       //(Just resetting the height and width of the canvas item would crop the image)
      if(pixbuf)
        pixbuf = ImageGlom::scale_keeping_ratio(pixbuf, height, width);
      
      canvas_item->property_pixbuf() = pixbuf;
      
     
     
      break;
    }
    default:
      std::cerr << "CanvasLayoutItem::set_db_data(): unhandled field type." << std::endl;
  }
        
}

} //namespace Glom

