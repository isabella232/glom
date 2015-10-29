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

#ifndef GLOM_UTILITY_WIDGETS_DATAWIDGET_H
#define GLOM_UTILITY_WIDGETS_DATAWIDGET_H

#include "config.h" // For GLOM_ENABLE_CLIENT_ONLY

#include <glom/utility_widgets/placeholder.h>
#include <glom/utility_widgets/layoutwidgetmenu.h>
#include <gtkmm/label.h>
#include <libglom/data_structure/field.h>
#include <libglom/document/view.h>
#include <libglom/data_structure/layout/layoutitem_field.h>
#include <glom/mode_design/layout/treestore_layout.h> //Forthe enum.
#include <gtkmm/eventbox.h>
#include <gtkmm/button.h>

namespace Glom
{

class AppWindow;

class DataWidget
 : public Gtk::EventBox,
   public LayoutWidgetMenu,
   public View_Composite_Glom
{
public:
  explicit DataWidget(const std::shared_ptr<LayoutItem_Field>& field, const Glib::ustring& table_name, const Document* document);
  virtual ~DataWidget();

  virtual Gtk::Label* get_label();
  virtual const Gtk::Label* get_label() const;

  //Override this so we can store the text to compare later.
  //This is not virtual, so you must not use it via Gtk::Entry.
  //void set_text(const Glib::ustring& text) override;

  virtual void set_value(const Gnome::Gda::Value& value);

  virtual Gnome::Gda::Value get_value() const;

  virtual void set_editable(bool editable = true);
  virtual void set_viewable(bool viewable = true);

#ifndef GLOM_ENABLE_CLIENT_ONLY
  static std::shared_ptr<LayoutItem_Field> offer_field_list(const Glib::ustring& table_name, const std::shared_ptr<const LayoutItem_Field>& start_field, Document* document, AppWindow* app);
  std::shared_ptr<LayoutItem_Field> offer_field_list(const Glib::ustring& table_name);
  std::shared_ptr<LayoutItem_Field> offer_field_list(const Glib::ustring& table_name, const std::shared_ptr<const LayoutItem_Field>& start_field);

  std::shared_ptr<LayoutItem_Field> offer_field_layout(const std::shared_ptr<const LayoutItem_Field>& start_field);
#endif // !GLOM_ENABLE_CLIENT_ONLY

  /// Get the actual child widget used to show the data:
  Gtk::Widget* get_data_child_widget();
  const Gtk::Widget* get_data_child_widget() const;

  typedef sigc::signal<void, const Gnome::Gda::Value&> type_signal_edited;
  type_signal_edited signal_edited();

  typedef sigc::signal<void, const Gnome::Gda::Value&> type_signal_open_details_requested;
  type_signal_open_details_requested signal_open_details_requested();

  /** For instance,
   * void on_choices_changed();
   */
  typedef sigc::signal<void> type_signal_choices_changed;

  /** This is emitted when the related records, used by a choices combobox,
   * have been changed. For instance, when the user adds a new choice via the "New" button.
   */
  type_signal_choices_changed signal_choices_changed();

private:
  //virtual void setup_menu();

  //Overrides of default signal handlers:
  void on_widget_edited(); //From Gtk::Entry, or Gtk::CheckButton.
#ifndef GLOM_ENABLE_CLIENT_ONLY
  bool on_button_press_event(GdkEventButton* button_event) override;
  virtual void on_child_user_requested_layout();
  virtual void on_child_user_requested_layout_properties();
  virtual void on_child_layout_item_added(LayoutWidgetBase::enumType item_type);
#endif // !GLOM_ENABLE_CLIENT_ONLY
  void on_button_open_details();
  void on_button_select_id();
  void on_button_new_id();
  void on_button_choose_date();

  // Don't call it on_style_changed, otherwise we would override a virtual
  // function from Gtk::Widget. We could indeed do that, but we do it with
  // a normal signal handler, because we have to do it this way anyway in
  // case default signal handlers have been disabled in glibmm.
  void on_self_style_changed(const Glib::RefPtr<Gtk::Style>& style);

#ifndef GLOM_ENABLE_CLIENT_ONLY
  void on_menupopup_activate_layout() override;
  void on_menupopup_activate_layout_properties() override;
  //virtual void on_menupopup_add_item(LayoutWidgetBase::enumType item);
#endif // !GLOM_ENABLE_CLIENT_ONLY

  AppWindow* get_appwindow() const override;

  void set_child_size_by_field(const std::shared_ptr<const LayoutItem_Field>& field);
  int get_suitable_width(const std::shared_ptr<const LayoutItem_Field>& field_layout);


  /** Show a dialog with a Find so that the user can choose an ID value to indicate the related record.
   */
  bool offer_related_record_id_find(Gnome::Gda::Value& chosen_id);

  /** Show a dialog with Details so that the user can add a new record and then use that ID value to indicate that related record.
   */
  bool offer_related_record_id_new(Gnome::Gda::Value& chosen_id);

private:
  void update_go_to_details_button_sensitivity();

protected:
  type_signal_edited m_signal_edited;
  type_signal_open_details_requested m_signal_open_details_requested;
  type_signal_choices_changed m_signal_choices_changed;

  Gtk::Label m_label;
  Gtk::Widget* m_child;
  Gtk::Button* m_button_go_to_details;
};

} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_DATAWIDGET_H

