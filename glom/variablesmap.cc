/* variablesmap.cc
 *
 * Copyright (C) 2002 The libglademm Development Team
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <glom/variablesmap.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/entry.h>
#include <gtkmm/calendar.h>
#include <gtkmm/scale.h>
#include <gtkmm/combobox.h>

namespace Glom
{

VariablesMap::VariablesMap(const Glib::RefPtr<Gtk::Builder>& builder)
: m_builder(builder)
{
}

void VariablesMap::connect_widget(const Glib::ustring& widget_name, bool& variable)
{
  Gtk::ToggleButton* pToggleButton = nullptr;
  m_builder->get_widget(widget_name, pToggleButton); //Glade::Xml will complain if it is not a ToggleButton.
  if(pToggleButton)
  {
    m_mapWidgetsToVariables[pToggleButton] = (void*)(&variable);
  }
}

void VariablesMap::connect_widget(const Glib::ustring& widget_name, Glib::ustring& variable)
{
  Gtk::Widget* pWidget = nullptr;
  m_builder->get_widget(widget_name, pWidget);

  if(auto pEntry = dynamic_cast<Gtk::Entry*>(pWidget)) //it mange both Gtk::entry and Gtk::SpinButton
  {
    m_mapWidgetsToVariables[pEntry] = (void*)(&variable);
  }

  if(auto pComboBox = dynamic_cast<Gtk::ComboBox*>(pWidget))
  {
    m_mapWidgetsToVariables[pComboBox] = (void*)(&variable);
  }
}

void VariablesMap::connect_widget(const Glib::ustring& widget_name, double& variable)
{
  Gtk::Widget* pWidget = nullptr;
  m_builder->get_widget(widget_name, pWidget);

  auto pScale = dynamic_cast<Gtk::Scale*>(pWidget);
  if(!pScale)
    return;

  m_mapWidgetsToVariables[pScale] = (void*)(&variable);
}

void VariablesMap::connect_widget(const Glib::ustring& widget_name, Glib::Date& variable)
{
  Gtk::Widget* pWidget = nullptr;
  m_builder->get_widget(widget_name, pWidget);

  auto pCalendar = dynamic_cast<Gtk::Calendar*>(pWidget);
  if(!pCalendar)
    return;

  m_mapWidgetsToVariables[pCalendar] = (void*)(&variable);
}

void VariablesMap::transfer_widgets_to_variables()
{
  if(validate_widgets()) //If the widgets' data is correct. Useful to override.
  {
    for(const auto& the_pair : m_mapWidgetsToVariables)
    {
      transfer_one_widget(the_pair.first, true); //true = to_variable.
    }
  }
}

void VariablesMap::transfer_variables_to_widgets()
{
  for(const auto& the_pair : m_mapWidgetsToVariables)
  {
    transfer_one_widget(the_pair.first, false); //false = to_widget.
  }
}


void VariablesMap::transfer_one_widget(Gtk::Widget* pWidget, bool to_variable)
{
  //Find the widget in the map:
  auto iterFind = m_mapWidgetsToVariables.find(pWidget);
  if(iterFind == m_mapWidgetsToVariables.end())
    return;

  //Get the variable for the widget:
  auto pVariable = iterFind->second;
  if(!pVariable)
    return;

  //Cast the variable appropriately and set it appropriately:
  if(auto pEntry = dynamic_cast<Gtk::Entry*>(pWidget))
  {
    auto pVar = static_cast<Glib::ustring*>(pVariable);

    if(to_variable)
      (*pVar) = pEntry->get_text();
    else
      pEntry->set_text(*pVar);
  }

  if(auto pComboBox = dynamic_cast<Gtk::ComboBox*>(pWidget))
  {
    auto pVar = static_cast<Glib::ustring*>(pVariable);
    auto pIEntry = dynamic_cast<Gtk::Entry*>(pComboBox->get_child());

    if(!to_variable)
    {
      if(pIEntry)
          (*pVar) = pIEntry->get_text();
      } else {
        if(pIEntry)
          pIEntry->set_text(*pVar);
    }
  }

  if(auto pToggleButton = dynamic_cast<Gtk::ToggleButton*>(pWidget)) //CheckButtons and RadioButtons.
  {
    auto pVar = static_cast<bool*>(pVariable);

    if(to_variable)
      (*pVar) = pToggleButton->get_active();
    else
      pToggleButton->set_active(*pVar);
  }

  if(auto pScale = dynamic_cast<Gtk::Scale*>(pWidget))
  {
    auto pVar = static_cast<double*>(pVariable);

    if(to_variable)
      (*pVar) = pScale->get_value();
    else
      pScale->set_value(*pVar);
  }

  if(auto pCalendar = dynamic_cast<Gtk::Calendar*>(pWidget))
  {
    auto pVar = static_cast<Glib::Date*>(pVariable);

    if(to_variable)
    {
      guint year,month,day;
      pCalendar->get_date(year, month, day);
      (*pVar) = Glib::Date(day, (Glib::Date::Month)month, year);
    }
    else
    {
      pCalendar->select_day(pVar->get_day());
      pCalendar->select_month(pVar->get_month(), pVar->get_year());
    }
  }
}

bool VariablesMap::validate_widgets()
{
  //Override to add validation.
  //TODO: We could add some automatic data-range and text-length validation.
  return true;
}


} // namespace Glom

