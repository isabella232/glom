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

#ifndef GLOM_MODE_DATA_ENTRY_H
#define GLOM_MODE_DATA_ENTRY_H

#include "config.h" // For GLOM_ENABLE_CLIENT_ONLY

#include <gtkmm/entry.h>
#include <libglom/data_structure/field.h>
#include <glom/utility_widgets/layoutwidgetfield.h>
#include <gtkmm/builder.h>

namespace Glom
{

class AppWindow;

namespace DataWidgetChildren
{

class Entry
:
  public Gtk::Entry,
  public LayoutWidgetField
{
public:
  explicit Entry(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  explicit Entry(Field::glom_field_type glom_type = Field::TYPE_TEXT);
  virtual ~Entry();

  virtual void set_layout_item(const std::shared_ptr<LayoutItem>& layout_item, const Glib::ustring& table_name) override;

  void set_glom_type(Field::glom_field_type glom_type);

  //Override this so we can store the text to compare later.
  //This is not virtual, so you must not use it via Gtk::Entry.
  void set_text(const Glib::ustring& text);

  /** Set the text from a Gnome::Gda::Value.
   */
  virtual void set_value(const Gnome::Gda::Value& value) override;

  virtual Gnome::Gda::Value get_value() const override;

  virtual void set_read_only(bool read_only = true) override;

private:
  void init();

  //Overrides of default signal handlers:
  virtual void on_changed() override; //From Gtk::Entry.
  virtual void on_activate() override; //From Gtk::Entry.
  virtual bool on_focus_out_event(GdkEventFocus* focus_event) override; //From Gtk::Widget

  void check_for_change();

#ifndef GLOM_ENABLE_CLIENT_ONLY
  virtual bool on_button_press_event(GdkEventButton *event) override;
#endif // !GLOM_ENABLE_CLIENT_ONLY

  virtual AppWindow* get_appwindow() const override;

  Glib::ustring m_old_text;
  Field::glom_field_type m_glom_type; //Store the type so we can validate the text accordingly.

  //Gnome::Gda::Value m_value; //The last-stored value. We have this because the displayed value might be unparseable.
};

} //namespace DataWidetChildren
} //namespace Glom

#endif // GLOM_MODE_DATA_ENTRY_H
