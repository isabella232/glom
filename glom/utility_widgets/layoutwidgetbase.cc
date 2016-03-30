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
#include <glom/appwindow.h>
#include <glom/mode_data/datawidget/textview.h>
#include <glom/mode_data/datawidget/label.h>
#include <glom/utils_ui.h>
#include <iostream>

namespace Glom
{

LayoutWidgetBase::LayoutWidgetBase()
: m_layout_item(nullptr)
{
}

void LayoutWidgetBase::set_layout_item(const std::shared_ptr<LayoutItem>& layout_item, const Glib::ustring& table_name)
{
  m_layout_item = layout_item;
  m_table_name = table_name;
}

std::shared_ptr<const LayoutItem> LayoutWidgetBase::get_layout_item() const
{
  return m_layout_item;
}

std::shared_ptr<LayoutItem> LayoutWidgetBase::get_layout_item()
{
  return m_layout_item;
}

AppWindow* LayoutWidgetBase::get_appwindow() const
{
  return nullptr; //override to implement.
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

void LayoutWidgetBase::apply_formatting(Gtk::Widget& widget, const std::shared_ptr<const LayoutItem_WithFormatting>& layout_item)
{
  auto widget_to_change = &widget;

  auto button = dynamic_cast<Gtk::Button*>(&widget);
  auto labelglom = dynamic_cast<DataWidgetChildren::Label*>(&widget);
  if(button)
    widget_to_change = button->get_child();
  else
  {
    auto textview = dynamic_cast<DataWidgetChildren::TextView*>(&widget);
    if(textview)
      widget_to_change = textview->get_textview();
    else if(labelglom)
      widget_to_change = labelglom->get_label();
  }

  if(!widget_to_change)
  {
    std::cerr << G_STRFUNC << ": widget_to_change is null.\n";
    return;
  }

  if(!layout_item)
    return;

  //Set justification on labels and text views:
  //Assume that people want left/right justification of multi-line text if they chose 
  //left/right alignment of the text itself.
  {
    const Formatting::HorizontalAlignment alignment =
     layout_item->get_formatting_used_horizontal_alignment(true /* for details view */);
    const Gtk::Justification justification =
      (alignment == Formatting::HorizontalAlignment::LEFT ? Gtk::JUSTIFY_LEFT : Gtk::JUSTIFY_RIGHT);
    const Gtk::Align x_align =
      (alignment == Formatting::HorizontalAlignment::LEFT ? Gtk::ALIGN_START : Gtk::ALIGN_END);

    auto label = dynamic_cast<Gtk::Label*>(widget_to_change);
    if(label)
    {
      //Note that set_justify() does nothing for single lines of text,
      //and doesn't align the block of text itself,
      //so we use set_xalign() to get the effect even for single lines of text.
      //See http://www.murrayc.com/permalink/2015/03/02/gtk-aligning-justification-in-text-widgets/
      label->set_justify(justification);
      label->set_xalign(x_align);
    } else {
      auto textview = dynamic_cast<Gtk::TextView*>(widget_to_change);
      if(textview)
      {
        //Note that, unlike Gtk::Label::set_justify(), this does have an effect
        //even for single lines of text.
        //See http://www.murrayc.com/permalink/2015/03/02/gtk-aligning-justification-in-text-widgets/
        textview->set_justification(justification);
      } else {
        auto entry = dynamic_cast<Gtk::Entry*>(widget_to_change);
        if(entry)
        {
          //See http://www.murrayc.com/permalink/2015/03/02/gtk-aligning-justification-in-text-widgets/
          entry->set_alignment(x_align);
        }
      }
    }
  }

  const auto formatting = layout_item->get_formatting_used();

  //Use the text formatting:
  const auto font_desc = formatting.get_text_format_font();
  if(!font_desc.empty())
  {
    UiUtils::load_font_into_css_provider(*widget_to_change, font_desc);
  }

  
  const auto fg = formatting.get_text_format_color_foreground();
  if(!fg.empty())
  {
    UiUtils::load_color_into_css_provider(*widget_to_change, fg);
  }

  const auto bg = formatting.get_text_format_color_background();
  if(!bg.empty())
  {
    if(!labelglom && !button)
    {
      UiUtils::load_background_color_into_css_provider(*widget_to_change, bg);
    }
    //According to the gtk_widget_override_background_color() documentation, 
    //a GtkLabel can only have a background color by, for instance, placing it 
    //in a GtkEventBox. Luckily Label is actually derived from EventBox.
    else if(labelglom)
    {
      UiUtils::load_background_color_into_css_provider(*labelglom, bg);
    }
    else if(button)
    {
      UiUtils::load_background_color_into_css_provider(*button, bg);
    }
  }
}

} //namespace Glom
