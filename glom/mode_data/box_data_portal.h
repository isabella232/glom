/* Glom
 *
 * Copyright (C) 2001-2008 Murray Cumming
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

#ifndef GLOM_MODE_DATA_BOX_DATA_PORTAL_H
#define GLOM_MODE_DATA_BOX_DATA_PORTAL_H

#include "config.h" // GLOM_ENABLE_CLIENT_ONLY

#include "box_data_manyrecords.h"
#include <glom/utility_widgets/layoutwidgetbase.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>


namespace Glom
{

class Box_Data_Details;

/** This is a base class for data widgets that should show multiple related records.
 */
class Box_Data_Portal : 
  public Box_Data_ManyRecords,
  public LayoutWidgetBase     
{
public: 
  Box_Data_Portal();
  
  /**
   * @param portal: The full portal details
   */
  bool init_db_details(const std::shared_ptr<const LayoutItem_Portal>& portal, bool show_title = true);

  /** Use this if no portal is yet defined, so the user can use the context menu to define a portal.
   */
  virtual bool init_db_details_without_portal(const Glib::ustring& parent_table) = 0;

  /** Update a portal if a relevant value in its parent table has changed.
   *
   * @param foreign_key_value: The value that should be found in this table.
   */
  bool refresh_data_from_database_with_foreign_key(const Gnome::Gda::Value& foreign_key_value);

  std::shared_ptr<LayoutItem_Portal> get_portal();
  std::shared_ptr<const LayoutItem_Portal> get_portal() const;
  std::shared_ptr<const Field> get_key_field() const;

  sigc::signal<void, Gnome::Gda::Value> signal_record_added;

  /** Tell the parent widget that something has changed in one of the related records,
   * or a record was added or deleted.
   *
   * @param relationship_name, if any.
   */
  typedef sigc::signal<void, const Glib::ustring&> type_signal_portal_record_changed;
  type_signal_portal_record_changed signal_portal_record_changed();
    
  bool get_has_suitable_record_to_view_details() const;

  /** Discover what record to show, in what table, when clicking on a related record.
   * This record will not necessarily just be the directly related record.
   *
   * @param primary_key_value Identifies the related record that has been clicked.
   * @param table_name The table that should be shown.
   * @param table_primary_key_value Identifies the record in that table that should be shown.
   */
  void get_suitable_record_to_view_details(const Gnome::Gda::Value& primary_key_value, Glib::ustring& table_name, Gnome::Gda::Value& table_primary_key_value) const;

  /** Prevent any attempts to change actual records,
   * if the widget is just being used to enter find critera,
   * and prevents any need for data retrieval from the database, because
   * no data will be displayed.
   *
   * @param val True if find mode should be used.
   */
  virtual void set_find_mode(bool val = true);

  Glib::ustring get_parent_table() const;
  void set_parent_table(const Glib::ustring& parent_table);

protected:
  type_vecConstLayoutFields get_fields_to_show() const override;
    
  //Implementations of pure virtual methods from Base_DB_Table_Data:
  std::shared_ptr<Field> get_field_primary_key() const override;

  //Overrides of virtual methods from Base_Db_Table_Data: 
  void on_record_added(const Gnome::Gda::Value& primary_key_value, const Gtk::TreeModel::iterator& row) override; // Not a signal handler.
  void on_record_deleted(const Gnome::Gda::Value& primary_key_value) override;

#ifndef GLOM_ENABLE_CLIENT_ONLY
  void on_dialog_layout_hide() override;
#endif // !GLOM_ENABLE_CLIENT_ONLY

  virtual Document::type_list_layout_groups create_layout_get_layout();

  void make_record_related(const Gnome::Gda::Value& related_record_primary_key_value);

  virtual void show_title_in_ui(const Glib::ustring& title) = 0;

  /** Get the title of the relationship used by the portal.
   */
  Glib::ustring get_title(const Glib::ustring& locale) const;
  
  /** Get the singular title of the relationship used by the portal.
   */
  Glib::ustring get_title_singular(const Glib::ustring& locale) const;  
  
  Gtk::Frame m_Frame;
  Gtk::Label m_Label;

private:
  Glib::ustring m_parent_table; //A duplicate of the from_table in the poral, but only when get_portal() is not null.
  
  // m_key_field and m_key_value are the field and its value in this table that 
  // must match another field in the parent table.

public:
  std::shared_ptr<Field> m_key_field;
  Gnome::Gda::Value m_key_value;

private:
  bool m_find_mode;
    
  type_signal_portal_record_changed m_signal_portal_record_changed;
};

} //namespace Glom

#endif // GLOM_MODE_DATA_BOX_DATA_PORTAL_H
