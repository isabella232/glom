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

#ifndef GLOM_MODE_DATA_COMBO_H
#define GLOM_MODE_DATA_COMBO_H

#include "config.h" // For GLOM_ENABLE_CLIENT_ONLY

#include <glom/mode_data/datawidget/combochoiceswithtreemodel.h>
#include <gtkmm/combobox.h>

namespace Glom
{

class AppWindow;

namespace DataWidgetChildren
{

/** A Gtk::ComboBox that can show choices of field values.
 * Use this when the user should only be allowed to enter values that are in the choices.
 */
class ComboGlom
:
  public Gtk::ComboBox,
  public ComboChoicesWithTreeModel
{
public:

  ///You must call set_layout_item() to specify the field type and formatting of the main column.
  explicit ComboGlom(bool has_entry = false);

  //This creates a simple ListStore, with a text cell renderer.
  void set_choices_fixed(const Formatting::type_list_values& list_values, bool restricted = false) override;

  //This creates a db-based tree model, with appropriate cell renderers:
  void set_choices_related(const std::shared_ptr<const Document>& document, const LayoutItem_Field& layout_field, const Gnome::Gda::Value& foreign_key_value) override;

  void set_read_only(bool read_only = true) override;

  /** Set the text from a Gnome::Gda::Value.
   */
  void set_value(const Gnome::Gda::Value& value) override;

  Gnome::Gda::Value get_value() const override;

private:

  void on_fixed_cell_data(const Gtk::TreeModel::iterator& iter, Gtk::CellRenderer* cell, guint model_column_index);

  // Note that this is a normal signal handler when glibmm was complied
  // without default signal handlers
  void on_changed() override; //From Gtk::ComboBox

  void check_for_change();

#ifndef GLOM_ENABLE_CLIENT_ONLY
  bool on_button_press_event(GdkEventButton *event) override;
#endif // !GLOM_ENABLE_CLIENT_ONLY

  AppWindow* get_appwindow() const override;


  Gnome::Gda::Value m_old_value; //TODO: Only useful for navigation, which currently has no implementation.
  //Gnome::Gda::Value m_value; //The last-stored value. We have this because the displayed value might be unparseable.

  //Prevent us from emitting signals just because set_value() was called:
  bool m_ignore_changed;
};

} //namespace DataWidetChildren

} //namespace Glom

#endif // GLOM_MODE_DATA_COMBO_H
