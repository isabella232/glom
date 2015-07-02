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

#include "textview.h"
#include <libglom/data_structure/glomconversions.h>
#include <gtkmm/messagedialog.h>
#include <glom/dialog_invalid_data.h>
#include <libglom/data_structure/glomconversions.h>
#include <glom/appwindow.h>
#include <glibmm/i18n.h>
//#include <sstream> //For stringstream

#include <locale>     // for locale, time_put
#include <ctime>     // for struct tm
#include <iostream>   // for cout, endl

namespace Glom
{

namespace DataWidgetChildren
{

TextView::TextView(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& /* builder */)
: Gtk::ScrolledWindow(cobject),
  m_glom_type(Field::TYPE_TEXT)
{
  init();
}

TextView::TextView(Field::glom_field_type glom_type)
: m_glom_type(glom_type)
{
  init();
}

void TextView::init()
{
#ifndef GLOM_ENABLE_CLIENT_ONLY
  setup_menu(this);
#endif // !GLOM_ENABLE_CLIENT_ONLY

  set_shadow_type(Gtk::SHADOW_IN);

  m_TextView.show();
  add(m_TextView);

  //Wrap text, and allow vertical scrolling:
  set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC); 
  set_shadow_type(Gtk::SHADOW_IN);
  m_TextView.set_wrap_mode(Gtk::WRAP_WORD);

  //We use connect(slot, false) to connect before the default signal handler, because the default signal handler prevents _further_ handling.
  m_TextView.signal_focus_out_event().connect(sigc::mem_fun(*this, &TextView::on_textview_focus_out_event), false);
  // m_TextView.get_buffer()->signal_end_user_action().connect(sigc::mem_fun(*this, &TextView::on_buffer_changed));
}

TextView::~TextView()
{
}

void TextView::set_glom_type(Field::glom_field_type glom_type)
{
  m_glom_type = glom_type;
}

void TextView::check_for_change()
{
  const auto new_text = m_TextView.get_buffer()->get_text();
  if(new_text != m_old_text)
  {
    //Validate the input:
    bool success = false;

    std::shared_ptr<const LayoutItem_Field>layout_item = std::dynamic_pointer_cast<const LayoutItem_Field>(get_layout_item());
    Gnome::Gda::Value value = Conversions::parse_value(m_glom_type, new_text, layout_item->get_formatting_used().m_numeric_format, success);

    if(success)
    {
      //Actually show the canonical text:
      set_value(value);
      m_signal_edited.emit(); //The text was edited, so tell the client code.
    }
    else
    {
      //Tell the user and offer to revert or try again:
      bool revert = glom_show_dialog_invalid_data(m_glom_type);
      if(revert)
      {
        set_text(m_old_text);
      }
      else
        grab_focus(); //Force the user back into the same field, so that the field can be checked again and eventually corrected or reverted.
    }
  }
}

bool TextView::on_textview_focus_out_event(GdkEventFocus* event)
{
  //Call base class:
  bool result = Gtk::ScrolledWindow::on_focus_out_event(event);

  //The user has finished editing.
  check_for_change();

  return result;
}


/*

void TextView::on_activate()
{ 
  //Call base class:
  Gtk::TextView::on_activate();

  //The user has finished editing.
  check_for_change();
}
*/

//TODO: Remove this? It doesn't seem to be used.
void TextView::on_buffer_changed()
{
  check_for_change();
}

/*
void TextView::on_insert_text(const Glib::ustring& text, int* position)
{
  Gtk::TextView::on_insert_text(text, position);
}

*/

void TextView::set_value(const Gnome::Gda::Value& value)
{
  std::shared_ptr<const LayoutItem_Field>layout_item = std::dynamic_pointer_cast<const LayoutItem_Field>(get_layout_item());
  if(layout_item)
    set_text(Conversions::get_text_for_gda_value(m_glom_type, value, layout_item->get_formatting_used().m_numeric_format));
}

void TextView::set_text(const Glib::ustring& text)
{
  m_old_text = text;

  //Call base class:
  m_TextView.get_buffer()->set_text(text);
}

Gnome::Gda::Value TextView::get_value() const
{
  bool success = false;

  std::shared_ptr<const LayoutItem_Field>layout_item = std::dynamic_pointer_cast<const LayoutItem_Field>(get_layout_item());

  TextView* pNonConstThis = const_cast<TextView*>(this); //Gtk::TextBuffer::get_text() is non-const in gtkmm <=2.6.
  return Conversions::parse_value(m_glom_type, pNonConstThis->m_TextView.get_buffer()->get_text(true), layout_item->get_formatting_used().m_numeric_format, success);
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
bool TextView::on_button_press_event(GdkEventButton *event)
{
  //Enable/Disable items.
  //We did this earlier, but get_appwindow is more likely to work now:
  AppWindow* pApp = get_appwindow();
  if(pApp)
  {
    pApp->add_developer_action(m_refContextLayout); //So that it can be disabled when not in developer mode.
    pApp->add_developer_action(m_refContextAddField);
    pApp->add_developer_action(m_refContextAddRelatedRecords);
    pApp->add_developer_action(m_refContextAddGroup);

    pApp->update_userlevel_ui(); //Update our action's sensitivity. 

    //Only show this popup in developer mode, so operators still see the default GtkEntry context menu.
    //TODO: It would be better to add it somehow to the standard context menu.
    if(pApp->get_userlevel() == AppState::USERLEVEL_DEVELOPER)
    {
      GdkModifierType mods;
      gdk_window_get_device_position( gtk_widget_get_window (Gtk::Widget::gobj()), event->device, 0, 0, &mods );
      if(mods & GDK_BUTTON3_MASK)
      {
        //Give user choices of actions on this item:
        m_pMenuPopup->popup(event->button, event->time);
        return true; //We handled this event.
      }
    }

  }

  return Gtk::ScrolledWindow::on_button_press_event(event);
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

AppWindow* TextView::get_appwindow() const
{
  Gtk::Container* pWindow = const_cast<Gtk::Container*>(get_toplevel());
  //TODO: This only works when the child widget is already in its parent.

  return dynamic_cast<AppWindow*>(pWindow);
}

TextView::type_text_view* TextView::get_textview()
{
  return &m_TextView;
}

void TextView::set_read_only(bool read_only)
{
  m_TextView.set_editable(!read_only);
}

} //namespace DataWidetChildren
} //namespace Glom
