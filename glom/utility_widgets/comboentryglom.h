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

#ifndef GLOM_UTILITY_WIDGETS_COMBOENTRY_GLOM_H
#define GLOM_UTILITY_WIDGETS_COMBOENTRY_GLOM_H

#include <gtkmm.h>
#include "../data_structure/field.h"
#include "layoutwidgetbase.h"
#include <libglademm.h>

class App_Glom;

class ComboEntryGlom
: public Gtk::ComboBoxEntry,
  public LayoutWidgetBase
{
public:
  explicit ComboEntryGlom(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);

  ///You must call set_layout_item() to specify the field type and formatting of the main column.
  explicit ComboEntryGlom();

  ///You must call set_layout_item() to specify the field type and formatting of the main column.
  explicit ComboEntryGlom(const LayoutItem_Field& field_second);

  virtual ~ComboEntryGlom();

  void set_choices(const LayoutItem_Field::type_list_values& list_values);

  typedef std::list< std::pair<Gnome::Gda::Value, Gnome::Gda::Value> > type_list_values_with_second;
  void set_choices_with_second(const type_list_values_with_second& list_values);

  //Override this so we can store the text to compare later.
  //This is not virtual, so you must not use it via Gtk::Entry.
  void set_text(const Glib::ustring& text); //override

  /** Set the text from a Gnome::Gda::Value.
   */
  void set_value(const Gnome::Gda::Value& value);

  Gnome::Gda::Value get_value() const;

  typedef sigc::signal<void> type_signal_edited;
  type_signal_edited signal_edited();

protected:
  void init();

  //Overrides of default signal handlers:
  virtual void on_entry_changed(); //From Gtk::Entry.
  virtual void on_entry_activate(); //From Gtk::Entry.
  virtual bool on_entry_focus_out_event(GdkEventFocus* event); //From Gtk::Widget

  virtual void on_changed(); //From Gtk::ComboBox

  virtual void check_for_change();

  virtual bool on_entry_button_press_event(GdkEventButton *event);

  virtual App_Glom* get_application();

  //Tree model columns:
  class ModelColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:

    ModelColumns()
    { add(m_col_first); add(m_col_second); }

    Gtk::TreeModelColumn<Glib::ustring> m_col_first; //The data to choose - this must be text.
    Gtk::TreeModelColumn<Glib::ustring> m_col_second;
  };

  ModelColumns m_Columns;

  Glib::RefPtr<Gtk::ListStore> m_refModel;


  type_signal_edited m_signal_edited;

  Glib::ustring m_old_text;

  bool m_with_second;
  LayoutItem_Field m_layoutitem_second;
  //Gnome::Gda::Value m_value; //The last-stored value. We have this because the displayed value might be unparseable.
};

#endif //GLOM_UTILITY_WIDGETS_COMBOENTRY_GLOM_H

