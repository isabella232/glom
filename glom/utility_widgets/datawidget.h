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

#ifndef GLOM_UTILITY_WIDGETS_DATAWIDGET_H
#define GLOM_UTILITY_WIDGETS_DATAWIDGET_H

#include "placeholder.h"
#include "layoutwidgetbase.h"
#include <gtkmm/label.h>
#include "../data_structure/field.h"
#include "../document/document_glom.h"
#include "../data_structure/layout/layoutitem_field.h"

class App_Glom;

class DataWidget
 : public Gtk::EventBox,
   public LayoutWidgetBase,
   public View_Composite_Glom
{
public:
  //explicit DataWidget(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  explicit DataWidget(const LayoutItem_Field& field, const Glib::ustring& table_name);
  virtual ~DataWidget();

  virtual Gtk::Label* get_label();
  virtual const Gtk::Label* get_label() const;

  //Override this so we can store the text to compare later.
  //This is not virtual, so you must not use it via Gtk::Entry.
  //virtual void set_text(const Glib::ustring& text); //override

  virtual void set_value(const Gnome::Gda::Value& value);

  virtual Gnome::Gda::Value get_value() const;

  virtual void set_editable(bool editable = true);

  typedef sigc::signal<void, const Gnome::Gda::Value&> type_signal_edited;
  type_signal_edited signal_edited();

protected:
  virtual void setup_menu();

  //Overrides of default signal handlers:
  virtual void on_widget_edited(); //From Gtk::Entry, or Gtk::CheckButton.
  virtual bool on_button_press_event(GdkEventButton* event); //override.
  virtual void on_child_user_requested_layout();
  virtual void on_menupopup_activate_layout();
  virtual bool offer_field_list(const Glib::ustring& table_name, LayoutItem_Field& field);

  App_Glom* get_application();

  int get_suitable_width(Field::glom_field_type field_type);

  type_signal_edited m_signal_edited;

  Gtk::Label m_label;

  Gtk::Menu* m_pMenuPopup;
  Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;
  Glib::RefPtr<Gtk::UIManager> m_refUIManager;
  Glib::RefPtr<Gtk::Action> m_refContextLayout;
};

#endif //GLOM_UTILITY_WIDGETS_DATAWIDGET_H

