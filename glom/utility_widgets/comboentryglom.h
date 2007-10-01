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

#include "config.h" // For GLOM_ENABLE_CLIENT_ONLY

#include <gtkmm.h>
#include <glom/libglom/data_structure/field.h>
#include "comboglomchoicesbase.h"
#include <libglademm.h>

namespace Glom
{

class App_Glom;

class ComboEntryGlom
: public Gtk::ComboBoxEntry,
  public ComboGlomChoicesBase
{
public:
  explicit ComboEntryGlom(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);

  ///You must call set_layout_item() to specify the field type and formatting of the main column.
  explicit ComboEntryGlom();

  ///You must call set_layout_item() to specify the field type and formatting of the main column.
  explicit ComboEntryGlom(const sharedptr<LayoutItem_Field>& field_second);

  virtual ~ComboEntryGlom();

  //Override this so we can store the text to compare later.
  //This is not virtual, so you must not use it via Gtk::Entry.
  void set_text(const Glib::ustring& text); //override

  /** Set the text from a Gnome::Gda::Value.
   */
  virtual void set_value(const Gnome::Gda::Value& value);

  virtual Gnome::Gda::Value get_value() const;

  virtual void set_layout_item(const sharedptr<LayoutItem>& layout_item, const Glib::ustring& table_name);

  virtual void set_read_only(bool read_only = true);

protected:
  void init();


  //Overrides of default signal handlers:
  //TODO: Are these really default signal handlers?
  virtual void on_entry_changed(); //From Gtk::Entry.
  virtual void on_entry_activate(); //From Gtk::Entry.
  virtual bool on_entry_focus_out_event(GdkEventFocus* event); //From Gtk::Widget

  // Note that this is a normal signal handler when glibmm was complied
  // without default signal handlers
  virtual void on_changed(); //From Gtk::ComboBox

  virtual void check_for_change();

#ifndef GLOM_ENABLE_CLIENT_ONLY
  virtual bool on_entry_button_press_event(GdkEventButton *event);
#endif // !GLOM_ENABLE_CLIENT_ONLY

  virtual App_Glom* get_application();


  Glib::ustring m_old_text;
  //Gnome::Gda::Value m_value; //The last-stored value. We have this because the displayed value might be unparseable.
};

} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_COMBOENTRY_GLOM_H

