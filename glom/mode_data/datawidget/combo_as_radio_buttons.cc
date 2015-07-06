/* Glom
 *
 * Copyright (C) 2010 Murray Cumming
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

#include "combo_as_radio_buttons.h"
#include <libglom/data_structure/glomconversions.h>
#include <gtkmm/messagedialog.h>
#include <glom/dialog_invalid_data.h>
#include <libglom/data_structure/glomconversions.h>
#include <glom/appwindow.h>
#include <glibmm/i18n.h>
//#include <sstream> //For stringstream

#include <iostream>   // for cout, endl

namespace Glom
{

namespace DataWidgetChildren
{

ComboAsRadioButtons::ComboAsRadioButtons()
: Gtk::Box(Gtk::ORIENTATION_VERTICAL),
  ComboChoices()
{
#ifndef GLOM_ENABLE_CLIENT_ONLY
  setup_menu(this);
#endif // !GLOM_ENABLE_CLIENT_ONLY

  init();
}

void ComboAsRadioButtons::init()
{
  //if(m_glom_type == Field::TYPE_NUMERIC)
   // get_entry()->set_alignment(1.0); //Align numbers to the right.
}

void ComboAsRadioButtons::set_choices_with_second(const type_list_values_with_second& list_values)
{
  //Clear existing buttons:
  for(const auto& the_pair : m_map_buttons)
  {
     Gtk::RadioButton* button = the_pair.second;
     delete button;
  }
  m_map_buttons.clear();

  std::shared_ptr<LayoutItem_Field> layout_item =
    std::dynamic_pointer_cast<LayoutItem_Field>(get_layout_item());
  const auto format = layout_item->get_formatting_used();
  std::shared_ptr<const Relationship> choice_relationship;
  std::shared_ptr<const LayoutItem_Field> layout_choice_first;
  std::shared_ptr<const LayoutGroup> layout_choice_extra;
  Formatting::type_list_sort_fields choice_sort_fields; //Ignored. TODO?
  bool choice_show_all = false;
  format.get_choices_related(choice_relationship, layout_choice_first, layout_choice_extra, choice_sort_fields, choice_show_all);

  LayoutGroup::type_list_const_items extra_fields;
  if(layout_choice_extra)
    extra_fields = layout_choice_extra->get_items_recursive();

  //Add new buttons:
  Gtk::RadioButton::Group group;
  for(const auto& the_pair : list_values)
  {
    if(layout_choice_first)
    {
      const auto value_first = Conversions::get_text_for_gda_value(layout_choice_first->get_glom_type(), the_pair.first, layout_choice_first->get_formatting_used().m_numeric_format);
      Glib::ustring title = value_first;

      const type_list_values extra_values = the_pair.second;
      if(layout_choice_extra && !extra_values.empty())
      {
        type_list_values::const_iterator iterValues = extra_values.begin();
        for(const auto& item : extra_fields)
        {
          if(iterValues != extra_values.end()) {
            const std::shared_ptr<const LayoutItem_Field> item_field = std::dynamic_pointer_cast<const LayoutItem_Field>(item);
            if(item_field)
            {
              const Gnome::Gda::Value value = *iterValues; //TODO: Use a vector instead?
              const auto value_second = Conversions::get_text_for_gda_value(item_field->get_glom_type(), value, item_field->get_formatting_used().m_numeric_format);

              title += " - " + value_second; //TODO: Find a better way to join them?
            }

            ++iterValues;
          }
        }
      }

      Gtk::RadioButton* button = new Gtk::RadioButton(group, title);
      m_map_buttons[value_first] = button;
      pack_start(*button);
      button->show();
      button->signal_toggled().connect(
        sigc::mem_fun(*this, &ComboAsRadioButtons::on_radiobutton_toggled));

      //TODO: This doesn't seem be be emitted:
      button->signal_button_press_event().connect(
        sigc::mem_fun(*this, &ComboAsRadioButtons::on_radiobutton_button_press_event), false);
    }
  }
}

void ComboAsRadioButtons::set_choices_fixed(const Formatting::type_list_values& list_values, bool /* restricted */)
{
  //Clear existing buttons:
  for(const auto& the_pair : m_map_buttons)
  {
     Gtk::RadioButton* button = the_pair.second;
     delete button;
  }
  m_map_buttons.clear();

  //Add new buttons:
  Gtk::RadioButton::Group group;
  for(const auto& choicevalue : list_values)
  {
    std::shared_ptr<const LayoutItem_Field> layout_item = std::dynamic_pointer_cast<LayoutItem_Field>(get_layout_item());
    if(layout_item)
    {
      Gnome::Gda::Value value;
      if(choicevalue)
        value = choicevalue->get_value();

      const auto value_first = Conversions::get_text_for_gda_value(layout_item->get_glom_type(), value, layout_item->get_formatting_used().m_numeric_format);

      Gtk::RadioButton* button = new Gtk::RadioButton(group, value_first);
      m_map_buttons[value_first] = button;
      pack_start(*button);
      button->show();
      button->signal_toggled().connect(
        sigc::mem_fun(*this, &ComboAsRadioButtons::on_radiobutton_toggled));
    }
  }
}

void ComboAsRadioButtons::set_choices_related(const Document* document, const std::shared_ptr<const LayoutItem_Field>& layout_field, const Gnome::Gda::Value& foreign_key_value)
{
  const Utils::type_list_values_with_second list_values =
    Utils::get_choice_values(document, layout_field, foreign_key_value);
  set_choices_with_second(list_values);
}

ComboAsRadioButtons::~ComboAsRadioButtons()
{
  for(const auto& the_pair : m_map_buttons)
  {
    Gtk::RadioButton* button = the_pair.second;
    delete button;
  }
  m_map_buttons.clear();
}

void ComboAsRadioButtons::check_for_change()
{
  Glib::ustring new_text = get_text();
  if(new_text == m_old_text)
    return;

  //Validate the input:
  bool success = false;

  std::shared_ptr<const LayoutItem_Field> layout_item = std::dynamic_pointer_cast<const LayoutItem_Field>(get_layout_item());
  const auto value = Conversions::parse_value(layout_item->get_glom_type(), new_text, layout_item->get_formatting_used().m_numeric_format, success);

  if(success)
  {
    //Actually show the canonical text:
    set_value(value);
    m_signal_edited.emit(); //The text was edited, so tell the client code.
  }
  else
  {
    //Tell the user and offer to revert or try again:
    const auto revert = glom_show_dialog_invalid_data(layout_item->get_glom_type());
    if(revert)
    {
      set_text(m_old_text);
    }
    else
      grab_focus(); //Force the user back into the same field, so that the field can be checked again and eventually corrected or reverted.
  }
}

void ComboAsRadioButtons::set_value(const Gnome::Gda::Value& value)
{
  std::shared_ptr<const LayoutItem_Field> layout_item = std::dynamic_pointer_cast<const LayoutItem_Field>(get_layout_item());
  if(!layout_item)
    return;

  set_text(Conversions::get_text_for_gda_value(layout_item->get_glom_type(), value, layout_item->get_formatting_used().m_numeric_format));

  //Show a different color if the value is numeric, if that's specified:
  if(layout_item->get_glom_type() == Field::TYPE_NUMERIC)
  {
    //TODO
  }
}

void ComboAsRadioButtons::set_text(const Glib::ustring& text)
{
  m_old_text = text;

  auto iter = m_map_buttons.find(text);
  if(iter != m_map_buttons.end())
  {
    Gtk::RadioButton* button = iter->second;
    if(button)
    {
      button->set_active();
      return;
    }
  }

  //std::cerr << G_STRFUNC << ": no item found for: " << text << std::endl;
}

Gnome::Gda::Value ComboAsRadioButtons::get_value() const
{
  std::shared_ptr<const LayoutItem_Field> layout_item = std::dynamic_pointer_cast<const LayoutItem_Field>(get_layout_item());
  bool success = false;

  const auto text = get_text();
  return Conversions::parse_value(layout_item->get_glom_type(), text, layout_item->get_formatting_used().m_numeric_format, success);
}

Glib::ustring ComboAsRadioButtons::get_text() const
{
  //Get the active row:
  for(const auto& the_pair : m_map_buttons)
  {
    Gtk::CheckButton* button = the_pair.second;
    if(button && button->get_active())
    {
      return the_pair.first;
    }
  }

  return Glib::ustring();
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void ComboAsRadioButtons::show_context_menu(GdkEventButton *button_event)
{
  std::cout << "ComboAsRadioButtons::show_context_menu()" << std::endl;
  AppWindow* pApp = get_appwindow();
  if(pApp)
  {
    //Enable/Disable items.
    //We did this earlier, but get_appwindow is more likely to work now:
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
      gdk_window_get_device_position( gtk_widget_get_window (Gtk::Widget::gobj()), button_event->device, 0, 0, &mods );
      if(mods & GDK_BUTTON3_MASK)
      {
        //Give user choices of actions on this item:
        m_pMenuPopup->popup(button_event->button, button_event->time);
      }
    }
  }
}

bool ComboAsRadioButtons::on_radiobutton_button_press_event(GdkEventButton *button_event)
{
  show_context_menu(button_event);
  return false; //Let other signal handlers handle it too.
}

bool ComboAsRadioButtons::on_button_press_event(GdkEventButton *button_event)
{
  show_context_menu(button_event);

  return Gtk::Box::on_button_press_event(button_event);
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

AppWindow* ComboAsRadioButtons::get_appwindow() const
{
  Gtk::Container* pWindow = const_cast<Gtk::Container*>(get_toplevel());
  //TODO: This only works when the child widget is already in its parent.

  return dynamic_cast<AppWindow*>(pWindow);
}


void ComboAsRadioButtons::on_radiobutton_toggled()
{
  check_for_change();
}

void ComboAsRadioButtons::set_read_only(bool /* read_only */)
{
  //TODO
}

} //namespace DataWidetChildren
} //namespace Glom
