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

#include "config.h" // For GLOM_ENABLE_CLIENT_ONLY

#include "placeholder.h"
#include "layoutwidgetbase.h"
#include <gtkmm/label.h>
#include <glom/libglom/data_structure/field.h>
#include <glom/libglom/document/document_glom.h>
#include <glom/libglom/data_structure/layout/layoutitem_field.h>
#include "../mode_data/treestore_layout.h" //Forthe enum.

namespace Glom
{

class App_Glom;

class DataWidget
 : public Gtk::EventBox,
   public LayoutWidgetBase,
   public View_Composite_Glom
{
public:
  //explicit DataWidget(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  explicit DataWidget(const sharedptr<LayoutItem_Field>& field, const Glib::ustring& table_name, const Document_Glom* document);
  virtual ~DataWidget();

  virtual Gtk::Label* get_label();
  virtual const Gtk::Label* get_label() const;

  //Override this so we can store the text to compare later.
  //This is not virtual, so you must not use it via Gtk::Entry.
  //virtual void set_text(const Glib::ustring& text); //override

  virtual void set_value(const Gnome::Gda::Value& value);

  virtual Gnome::Gda::Value get_value() const;

  virtual void set_editable(bool editable = true);
  virtual void set_viewable(bool viewable = true);

  static sharedptr<LayoutItem_Field> offer_field_list(const Glib::ustring& table_name, const sharedptr<const LayoutItem_Field>& start_field, Document_Glom* document, App_Glom* app);
  sharedptr<LayoutItem_Field> offer_field_list(const Glib::ustring& table_name);
  sharedptr<LayoutItem_Field> offer_field_list(const Glib::ustring& table_name, const sharedptr<const LayoutItem_Field>& start_field);
#ifndef GLOM_ENABLE_CLIENT_ONLY
  sharedptr<LayoutItem_Field> offer_field_layout(const sharedptr<const LayoutItem_Field>& start_field);
#endif // !GLOM_ENABLE_CLIENT_ONLY

  /// Get the actual child widget used to show the data:
  Gtk::Widget* get_data_child_widget();
  const Gtk::Widget* get_data_child_widget() const;

  typedef sigc::signal<void, const Gnome::Gda::Value&> type_signal_edited;
  type_signal_edited signal_edited();

  typedef sigc::signal<void, const Gnome::Gda::Value&> type_signal_open_details_requested;
  type_signal_open_details_requested signal_open_details_requested();

protected:
  //virtual void setup_menu();

  //Overrides of default signal handlers:
  void on_widget_edited(); //From Gtk::Entry, or Gtk::CheckButton.
#ifndef GLOM_ENABLE_CLIENT_ONLY
  virtual bool on_button_press_event(GdkEventButton* event); //override.
  virtual void on_child_user_requested_layout();
  virtual void on_child_user_requested_layout_properties();
  virtual void on_child_layout_item_added(LayoutWidgetBase::enumType item_type);
#endif // !GLOM_ENABLE_CLIENT_ONLY
  void on_button_open_details();
  void on_button_select_id();
  void on_button_choose_date();

  // Don't call it on_style_changed, otherwise we would override a virtual
  // function from Gtk::Widget. We could indeed do that, but we do it with
  // a normal signal handler, because we have to do it this way anyway in
  // case default signal handlers have been disabled in glibmm.
  void on_self_style_changed(const Glib::RefPtr<Gtk::Style>& style);

#ifndef GLOM_ENABLE_CLIENT_ONLY
  virtual void on_menupopup_activate_layout(); //override
  virtual void on_menupopup_activate_layout_properties(); //override
  //virtual void on_menupopup_add_item(LayoutWidgetBase::enumType item);
#endif // !GLOM_ENABLE_CLIENT_ONLY

  App_Glom* get_application();

  void set_child_size_by_field(const sharedptr<const LayoutItem_Field>& field);
  int get_suitable_width(const sharedptr<const LayoutItem_Field>& field_layout);

  /** Show a dialog with a Find so that the user can choose an ID value to indicate the related record.
   */
  bool offer_related_record_id_find(Gnome::Gda::Value& chosen_id);

  type_signal_edited m_signal_edited;
  type_signal_open_details_requested m_signal_open_details_requested;

  Gtk::Label m_label;
  Gtk::Widget* m_child;
};

} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_DATAWIDGET_H

