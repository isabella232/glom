/* Glom
 *
 * Copyright (C) 2001-2005 Murray Cumming
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

#include "layoutwidgetbase.h"
#include <glibmm/i18n.h>
#include <glom/appwindow.h>
#include <glom/mode_data/datawidget/textview.h>
#include <glom/mode_data/datawidget/label.h>
#include <iostream>

namespace Glom
{

LayoutWidgetBase::LayoutWidgetBase()
: m_pLayoutItem(0)
#ifndef GLOM_ENABLE_CLIENT_ONLY
  , m_drag_in_progress(false)
#endif // !GLOM_ENABLE_CLIENT_ONLY
{
}

LayoutWidgetBase::~LayoutWidgetBase()
{
}

void LayoutWidgetBase::set_layout_item(const sharedptr<LayoutItem>& layout_item, const Glib::ustring& table_name)
{
  m_pLayoutItem = layout_item;
  m_table_name = table_name;
}

sharedptr<const LayoutItem> LayoutWidgetBase::get_layout_item() const
{
  return m_pLayoutItem;
}

sharedptr<LayoutItem> LayoutWidgetBase::get_layout_item()
{
  return m_pLayoutItem;
}

AppWindow* LayoutWidgetBase::get_appwindow() const
{
  return 0; //override to implement.
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
LayoutWidgetBase::type_signal_layout_changed LayoutWidgetBase::signal_layout_changed()
{
  return m_signal_layout_changed;
}

LayoutWidgetBase::type_signal_layout_item_added LayoutWidgetBase::signal_layout_item_added()
{
  return m_signal_layout_item_added;
}

LayoutWidgetBase::type_signal_user_requested_layout LayoutWidgetBase::signal_user_requested_layout()
{
  return m_signal_user_requested_layout;
}

LayoutWidgetBase::type_signal_user_requested_layout_properties LayoutWidgetBase::signal_user_requested_layout_properties()
{
  return m_signal_user_requested_layout_properties;
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

void LayoutWidgetBase::set_read_only(bool /* read_only */)
{
}

void LayoutWidgetBase::apply_formatting(Gtk::Widget& widget, const sharedptr<const LayoutItem_WithFormatting>& layout_item)
{
  Gtk::Widget* widget_to_change = &widget;

  Gtk::Button* button = dynamic_cast<Gtk::Button*>(&widget);
  DataWidgetChildren::Label* labelglom = dynamic_cast<DataWidgetChildren::Label*>(&widget);
  if(button)
    widget_to_change = button->get_child();
  else
  {
    DataWidgetChildren::TextView* textview = dynamic_cast<DataWidgetChildren::TextView*>(&widget);
    if(textview)
      widget_to_change = textview->get_textview();
    else if(labelglom)
      widget_to_change = labelglom->get_label();
  }

  if(!widget_to_change)
  {
    std::cerr << G_STRFUNC << ": widget_to_change is null." << std::endl;
    return;
  }

  if(!layout_item)
    return;

  //Horizontal alignment:
  const Formatting::HorizontalAlignment alignment =
    layout_item->get_formatting_used_horizontal_alignment(true /* for details view */);
  const Gtk::Align x_align = (alignment == Formatting::HORIZONTAL_ALIGNMENT_LEFT ? Gtk::ALIGN_START : Gtk::ALIGN_END);
  widget_to_change->set_halign(x_align);
    
  //Set justification on labels:
  //Assume that people want left/right justification of multi-line text if they chose 
  //left/right alignment of the text itself.
  {
    Gtk::Label* label = dynamic_cast<Gtk::Label*>(widget_to_change);
    if(label)
    {    
      const Gtk::Justification justification = (alignment == Formatting::HORIZONTAL_ALIGNMENT_LEFT ? Gtk::JUSTIFY_LEFT : Gtk::JUSTIFY_RIGHT);
      label->set_justify(justification);
    }
  }

  const Formatting& formatting = layout_item->get_formatting_used();

  //Use the text formatting:
  const Glib::ustring font_desc = formatting.get_text_format_font();
  if(!font_desc.empty())
  {
    widget_to_change->override_font( Pango::FontDescription(font_desc) );
  }

  
  const Glib::ustring fg = formatting.get_text_format_color_foreground();
  if(!fg.empty())
  {
    // "text" is the text color. (Works for GtkEntry and GtkTextView, 
    // for which override_color() doesn't seem to have any effect.
    widget_to_change->override_color(Gdk::RGBA(fg));
    
    // This works for GtkLabel, for which override_color() does not.
    widget_to_change->override_color(Gdk::RGBA(fg));
  }


  const Glib::ustring bg = formatting.get_text_format_color_background();
  if(!bg.empty())
  {
    if(!labelglom && !button)
    {
      // "base" is the background color for GtkEntry. "bg" seems to change the border:
      widget_to_change->override_background_color(Gdk::RGBA(bg));
    }
    //According to the gtk_widget_override_background_color() documentation, 
    //a GtkLabel can only have a background color by, for instance, placing it 
    //in a GtkEventBox. Luckily Label is actually derived from EventBox.
    else if(labelglom)
    {
      //label->override_background_color(Gdk::RGBA("bg"));
      labelglom->override_background_color(Gdk::RGBA(bg));
    }
    else if(button)
    {
      //button->override_background_color(Gdk::RGBA("bg"));
      button->override_background_color(Gdk::RGBA(bg));
    }
  }
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void LayoutWidgetBase::set_dnd_in_progress(bool drag)
{
  m_drag_in_progress = drag;
}

bool LayoutWidgetBase::get_dnd_in_progress()
{
  return m_drag_in_progress; 
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

} //namespace Glom
