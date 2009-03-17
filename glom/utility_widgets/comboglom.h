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

#ifndef GLOM_UTILITY_WIDGETS_COMBO_GLOM_H
#define GLOM_UTILITY_WIDGETS_COMBO_GLOM_H

#include <libglom/libglom_config.h> // For GLOM_ENABLE_CLIENT_ONLY

#include <gtkmm.h>
#include <libglom/data_structure/field.h>
#include "comboglomchoicesbase.h"
#include <gtkmm/builder.h>

namespace Glom
{

class App_Glom;

class ComboGlom
: public Gtk::ComboBox,
  public ComboGlomChoicesBase
{
public:
  explicit ComboGlom(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

  ///You must call set_layout_item() to specify the field type and formatting of the main column.
  explicit ComboGlom();

  ///You must call set_layout_item() to specify the field type and formatting of the main column.
  explicit ComboGlom(const sharedptr<LayoutItem_Field>& field_second);

  virtual ~ComboGlom();

  virtual void set_read_only(bool read_only = true);


  //Override this so we can store the text to compare later.
  //This is not virtual, so you must not use it via Gtk::Entry.
  void set_text(const Glib::ustring& text); //override

  Glib::ustring get_text() const;

  /** Set the text from a Gnome::Gda::Value.
   */
  virtual void set_value(const Gnome::Gda::Value& value);

  virtual Gnome::Gda::Value get_value() const;

private:
  void init();

  // Note this is a normal signal handlers when compiled without default
  // signal handlers
  virtual void on_changed(); //From Gtk::ComboBox

  virtual void check_for_change();

#ifndef GLOM_ENABLE_CLIENT_ONLY
  virtual bool on_button_press_event(GdkEventButton *event);
#endif // !GLOM_ENABLE_CLIENT_ONLY

  virtual App_Glom* get_application();


  Glib::ustring m_old_text;
  //Gnome::Gda::Value m_value; //The last-stored value. We have this because the displayed value might be unparseable.
};

} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_COMBOENTRY_GLOM_H

