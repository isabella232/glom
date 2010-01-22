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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "layoutwidgetbase.h"
#include <glibmm/i18n.h>
#include <glom/application.h>
#include <glom/utility_widgets/textviewglom.h>

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

App_Glom* LayoutWidgetBase::get_application() const
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
  if(button)
    widget_to_change = button->get_child();

  Glom::TextViewGlom* textview = dynamic_cast<Glom::TextViewGlom*>(&widget);
  if(textview)
    widget_to_change = textview->get_textview();


  if(!layout_item)
    return;

  //Horizontal alignment:
  const FieldFormatting::HorizontalAlignment alignment =
    layout_item->get_formatting_used_horizontal_alignment();
  const float x_align = (alignment == FieldFormatting::HORIZONTAL_ALIGNMENT_LEFT ? 0.0 : 1.0);
  Gtk::Misc* misc = dynamic_cast<Gtk::Misc*>(widget_to_change);
  if(misc)
    misc->set_alignment(x_align);


  const FieldFormatting& formatting = layout_item->get_formatting_used();

  //Use the text formatting:
  const Glib::ustring font_desc = formatting.get_text_format_font();
  if(!font_desc.empty())
  {
    widget_to_change->modify_font( Pango::FontDescription(font_desc) );
  }

  // "text" is the text color. "fg" doesn't seem to have any effect:
  const Glib::ustring fg = formatting.get_text_format_color_foreground();
  if(!fg.empty())
    widget_to_change->modify_text(Gtk::STATE_NORMAL, Gdk::Color(fg));

  // "base" is the background color. "bg" seems to change the border:
  const Glib::ustring bg = formatting.get_text_format_color_background();
  if(!bg.empty())
    widget_to_change->modify_base(Gtk::STATE_NORMAL, Gdk::Color(bg));
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
