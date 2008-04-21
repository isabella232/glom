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

#ifndef BOX_DATA_CALENDAR_RELATED_H
#define BOX_DATA_CALENDAR_RELATED_H

#include "config.h" // GLOM_ENABLE_CLIENT_ONLY

#include "box_data_list.h"
#include <glom/libglom/data_structure/layout/layoutitem_calendarportal.h>
#include "../utility_widgets/layoutwidgetbase.h"
#include "../utility_widgets/calendar/glomcalendar.h"

namespace Glom
{

class Dialog_Layout_Calendar_Related;

class Box_Data_Calendar_Related : 
  public Box_Data_List,
  public LayoutWidgetBase
{
public: 
  Box_Data_Calendar_Related();

  /**
   * @param portal: The full portal details
   */
  virtual bool init_db_details(const sharedptr<const LayoutItem_CalendarPortal>& portal, bool show_title = true);

  /**
   * @param foreign_key_value: The value that should be found in this table.
   * @param from_table_primary_key_value The primary key of the parent record's table, used to associate new related records.
   */
  virtual bool refresh_data_from_database_with_foreign_key(const Gnome::Gda::Value& foreign_key_value);

  sharedptr<LayoutItem_CalendarPortal> get_portal() const;
  virtual sharedptr<const Field> get_key_field() const;

  sigc::signal<void, Gnome::Gda::Value> signal_record_added;

  bool get_has_suitable_record_to_view_details() const;
  void get_suitable_table_to_view_details(Glib::ustring& table_name, sharedptr<const UsesRelationship>& relationship) const;
  void get_suitable_record_to_view_details(const Gnome::Gda::Value& primary_key_value, Glib::ustring& table_name, Gnome::Gda::Value& table_primary_key_value) const;

  ///@param relationship_name
  typedef sigc::signal<void, const Glib::ustring&> type_signal_record_changed;
  type_signal_record_changed signal_record_changed();

protected:
  virtual bool fill_from_database(); //Override.
  virtual type_vecLayoutFields get_fields_to_show() const; //override

  virtual void on_adddel_user_requested_add();
  virtual void on_adddel_user_changed(const Gtk::TreeModel::iterator& row, guint col);
  virtual void on_adddel_user_added(const Gtk::TreeModel::iterator& row, guint col_with_first_value); //Override.
  virtual void on_record_added(const Gnome::Gda::Value& primary_key_value, const Gtk::TreeModel::iterator& row); //Override. Not a signal handler.
  virtual void on_record_deleted(const Gnome::Gda::Value& primary_key_value); //override.

#ifndef GLOM_ENABLE_CLIENT_ONLY
  virtual void on_dialog_layout_hide(); //override.
#endif // !GLOM_ENABLE_CLIENT_ONLY

  virtual void enable_buttons(); //override

#ifndef GLOM_ENABLE_CLIENT_ONLY
  virtual Dialog_Layout* create_layout_dialog() const; // override.
  virtual void prepare_layout_dialog(Dialog_Layout* dialog); // override.
#endif // !GLOM_ENABLE_CLIENT_ONLY

  Glib::ustring on_calendar_details(guint year, guint month, guint day);
    
  void setup_menu();
  void on_calendar_button_press_event(GdkEventButton *event);
 
  void on_MenuPopup_activate_Edit();
  void on_MenuPopup_activate_Add();
  void on_MenuPopup_activate_Delete();
    
  #ifndef GLOM_ENABLE_CLIENT_ONLY
  void on_MenuPopup_activate_layout();
  #endif
    
protected:
  virtual Document_Glom::type_list_layout_groups create_layout_get_layout(); //override.

  Gtk::Frame m_Frame;
  Gtk::Alignment m_Alignment;
  Gtk::Label m_Label;
  GlomGtk::Calendar m_calendar;

  sharedptr<LayoutItem_CalendarPortal> m_portal;
  sharedptr<Field> m_key_field;
  Gnome::Gda::Value m_key_value;
    
  //TODO: Avoid repeating these in so many widgets:
  Gtk::Menu* m_pMenuPopup;
  Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;
  Glib::RefPtr<Gtk::UIManager> m_refUIManager;
  Glib::RefPtr<Gtk::Action> m_refContextEdit, m_refContextAdd, m_refContextDelete;

#ifndef GLOM_ENABLE_CLIENT_ONLY
  Glib::RefPtr<Gtk::Action> m_refContextLayout;
#endif

  type_signal_record_changed m_signal_record_changed;
};

} //namespace Glom

#endif
