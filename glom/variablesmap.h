/* variablesmap.h
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

#ifndef GLOM_VARIABLESMAP_H
#define GLOM_VARIABLESMAP_H

#include <gtkmm/builder.h>
#include <glibmm/date.h>
#include <map>

namespace Glom
{

/** Associates named Glade widgets with member variables.
 * Use connect_widget() to link the widgets with variables that will contain their data.
 * Then use transfer_widgets_to_variables() and transfer_variables_to_widgets() to get or set all of the variables at once.
 *
 * This is meant to be a bit like MFC's "Dialog Data Exchange and Validation".
 *
 * The association of widget and member varables follow this mapping:
 *
 * Gtk::Entry --> Glib::ustring
 * Gtk::SpinBox --> Glib::ustring
 * Gtk::ComboBox --> Glib::ustring
 * Gtk::Scale --> double
 * Gtk::Calendar --> Glib::Date
 * Gtk::CheckBox --> bool
 * Gtk::RadioButton --> bool
 *
 */
class VariablesMap
{
public:
  explicit VariablesMap(const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~VariablesMap();

  ///For ToggleButton (CheckBox and RadioButton)
  void connect_widget(const Glib::ustring& widget_name, bool& variable);

  ///For Entry, ComboBox and SpinBox
  void connect_widget(const Glib::ustring& widget_name, Glib::ustring& variable);

  ///For Scale (HScale and VScale)
  void connect_widget(const Glib::ustring& widget_name, double& variable);

  ///For Calendar
  void connect_widget(const Glib::ustring& widget_name, Glib::Date& variable);

  ///Transfer data from the widget to the variable.
  void transfer_widgets_to_variables();

  ///Transfer data from the variable to the widget.
  void transfer_variables_to_widgets();

private:

  /** Override this to validate the data that the user enters into the widgets.
   * The return value indicates whether the widgets' data is valid.
   */
  /* virtual  */ bool validate_widgets();

  void transfer_one_widget(Gtk::Widget* pWidget, bool to_variable);

  typedef std::map<Gtk::Widget*, void*> type_mapWidgetsToVariables;
  type_mapWidgetsToVariables m_mapWidgetsToVariables;

  Glib::RefPtr<Gtk::Builder> m_builder;
};

} /* namespace Glom */


#endif /* GLOM_VARIABLESMAP_H */
