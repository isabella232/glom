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

#ifndef GLOM_UTILITY_WIDGETS_ENTRY_GLOM_H
#define GLOM_UTILITY_WIDGETS_ENTRY_GLOM_H

#include <gtkmm/entry.h>
#include "../data_structure/field.h"

class EntryGlom : public Gtk::Entry
{
public:
  EntryGlom(Field::glom_field_type glom_type);
  virtual ~EntryGlom();


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

  ///Check whether the entered text is appropriate for the field type.
  virtual bool validate_text() const;
  
  //Overrides of default signal handlers:
  virtual void on_changed(); //From Gtk::Entry.
  virtual void on_activate(); //From Gtk::Entry.
  virtual bool on_focus_out_event(GdkEventFocus* event); //From Gtk::Widget
  virtual void on_insert_text(const Glib::ustring& text, int* position); //From Gtk::Editable

  virtual void check_for_change();
    
  type_signal_edited m_signal_edited;

  Glib::ustring m_old_text;
  Field::glom_field_type m_glom_type; //Store the type so we can validate the text accordingly.

};

#endif //GLOM_UTILITY_WIDGETS_ENTRY_GLOM_H

