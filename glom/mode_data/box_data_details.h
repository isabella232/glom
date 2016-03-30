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

#ifndef GLOM_MODE_DATA_BOX_DATA_DETAILS_H
#define GLOM_MODE_DATA_BOX_DATA_DETAILS_H

#include "config.h" // For GLOM_ENABLE_CLIENT_ONLY

#include "box_data.h"
//#include <glom/mode_data/box_data_list_related.h>
#include "flowtablewithfields.h"
#include <glom/utility_widgets/placeholder.h>
#ifndef GLOM_ENABLE_CLIENT_ONLY
#include <glom/utility_widgets/layouttoolbar.h>
#endif

namespace Glom
{

class Box_Data_Details : public Box_Data
{
public:
  explicit Box_Data_Details(bool bWithNavButtons = true);
  virtual ~Box_Data_Details();

  bool init_db_details(const FoundSet& found_set, const Glib::ustring& layout_platform, const Gnome::Gda::Value& primary_key_value);
  bool refresh_data_from_database_with_primary_key(const Gnome::Gda::Value& primary_key_value);
  bool refresh_data_from_database_blank();

  void print_layout() override;


  //Signals:
  typedef sigc::signal<void> type_signal_void;
  type_signal_void signal_nav_first();
  type_signal_void signal_nav_prev();
  type_signal_void signal_nav_next();
  type_signal_void signal_nav_last();

  typedef sigc::signal<void, const Gnome::Gda::Value&> type_signal_record_deleted; //arg is PrimaryKey.
  type_signal_record_deleted signal_record_deleted();

   /** For instance,
    * void on_requested_related_details(const Glib::ustring& table_name, Gnome::Gda::Value primary_key_value);
    */
  typedef sigc::signal<void, const Glib::ustring&, Gnome::Gda::Value> type_signal_requested_related_details;
  type_signal_requested_related_details signal_requested_related_details();

#ifndef GLOM_ENABLE_CLIENT_ONLY
  void show_layout_toolbar(bool show_toolbar = true);
#endif

  void do_new_record();
  
  void set_enable_drag_and_drop(bool enabled = true);

protected:


  //Implementations of pure virtual methods from Base_DB_Table_Data:
public:
  Gnome::Gda::Value get_primary_key_value_selected() const override; //Value in the primary key's cell.

protected:
  void set_primary_key_value(const Gtk::TreeModel::iterator& row, const Gnome::Gda::Value& value) override;
  Gnome::Gda::Value get_primary_key_value(const Gtk::TreeModel::iterator& row) const override; //Actual primary key value of this record.

  Gnome::Gda::Value get_entered_field_data(const std::shared_ptr<const LayoutItem_Field>& field) const override;
  void set_entered_field_data(const std::shared_ptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value) override;
  void set_entered_field_data(const Gtk::TreeModel::iterator& row, const std::shared_ptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value) override;


  bool fill_from_database() override;
  void create_layout() override;

  std::shared_ptr<Field> get_field_primary_key() const override;
  void set_found_set_from_primary_key_value();

private:
  //Signal handlers:
  void on_button_new();

  void on_button_del();

  void on_button_nav_first();
  void on_button_nav_prev();
  void on_button_nav_next();
  void on_button_nav_last();

protected:

  void on_userlevel_changed(AppState::userlevels user_level) override;

#ifndef GLOM_ENABLE_CLIENT_ONLY
  void on_flowtable_layout_changed();
#endif // !GLOM_ENABLE_CLIENT_ONLY

  //Signal handler: The last 2 args are bind-ed.
  void on_related_record_added(Gnome::Gda::Value key_value, Glib::ustring strFromKeyName);

  //Signal handler: The last arg is bind-ed.
  //void on_related_user_requested_details(Gnome::Gda::Value key_value, Glib::ustring table_name);

  //This is virtual so it can be overriden in Box_Data_Details_Find.
  virtual void on_flowtable_field_edited(const std::shared_ptr<const LayoutItem_Field>& layout_field, const Gnome::Gda::Value& value);

  void on_flowtable_field_choices_changed(const std::shared_ptr<const LayoutItem_Field>& layout_field);
  void on_flowtable_field_open_details_requested(const std::shared_ptr<const LayoutItem_Field>& id, const Gnome::Gda::Value& value);
  void on_flowtable_related_record_changed(const Glib::ustring& relationship_name);
  void on_flowtable_requested_related_details(const Glib::ustring& table_name, Gnome::Gda::Value primary_key_value);

  void on_flowtable_script_button_clicked(const std::shared_ptr<const LayoutItem_Button>& layout_item);

  void recalculate_fields_for_related_records(const Glib::ustring& relationship_name);

#ifndef GLOM_ENABLE_CLIENT_ONLY
  Dialog_Layout* create_layout_dialog() const override;
  void prepare_layout_dialog(Dialog_Layout* dialog) override;
#endif // !GLOM_ENABLE_CLIENT_ONLY

  std::shared_ptr<Field> m_field_primary_key;
 void show_all_vfunc() override;

  Gnome::Gda::Value m_primary_key_value;

  //Member widgets:
  Gtk::ScrolledWindow m_ScrolledWindow;
  Gtk::Box m_hbox_content;
  FlowTableWithFields m_FlowTable;

#ifndef GLOM_ENABLE_CLIENT_ONLY
  LayoutToolbar m_Dragbar;
  bool m_show_toolbar;
#endif

  Gtk::ButtonBox m_hbox_buttons;
  Gtk::Button m_Button_New;
  Gtk::Button m_Button_Del;
  Gtk::Button m_Button_Nav_First;
  Gtk::Button m_Button_Nav_Prev;
  Gtk::Button m_Button_Nav_Next;
  Gtk::Button m_Button_Nav_Last;

  bool m_do_not_refresh_related; //Stops us from refreshing related records in response to an addition of a related record.
  bool m_ignore_signals;

  type_signal_void m_signal_nav_first;
  type_signal_void m_signal_nav_prev;
  type_signal_void m_signal_nav_next;
  type_signal_void m_signal_nav_last;

  type_signal_record_deleted m_signal_record_deleted;

  type_signal_requested_related_details m_signal_requested_related_details;

#ifndef GLOM_ENABLE_CLIENT_ONLY
  bool m_design_mode; // Cache here because we need it when the layout is redrawn
#endif
};

} //namespace Glom

#endif // GLOM_MODE_DATA_BOX_DATA_DETAILS_H
