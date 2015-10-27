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


#ifndef GLOM_BASE_DB_TABLE_DATA_H
#define GLOM_BASE_DB_TABLE_DATA_H

#include "base_db_table_data_readonly.h"

namespace Glom
{

/** A base class some database functionality
 * for use with a specific database table, showing data from the table.
 */
class Base_DB_Table_Data : public Base_DB_Table_Data_ReadOnly
{
public:
  Base_DB_Table_Data();
  virtual ~Base_DB_Table_Data();

  /** Tell the parent widget that something has changed in one of the shown records,
   * or a record was added or deleted.
   * This is only emitted for widgets for which it would be useful.
   *
   * @param relationship_name, if any.
   */
  typedef sigc::signal<void> type_signal_record_changed;
  type_signal_record_changed signal_record_changed();

protected:

  /** Create a new record with all the entered field values from the currently-active details/row.
   * @result true if the record was added to the database.
   */
  bool record_new(bool use_entered_data = true, const Gnome::Gda::Value& primary_key_value = Gnome::Gda::Value());

  Gnome::Gda::Value get_entered_field_data_field_only(const std::shared_ptr<const Field>& field) const;
  virtual Gnome::Gda::Value get_entered_field_data(const std::shared_ptr<const LayoutItem_Field>& field) const;

  //Gets the row being edited, for derived classes that have rows.
  virtual Gtk::TreeModel::iterator get_row_selected();

  virtual void set_primary_key_value(const Gtk::TreeModel::iterator& row, const Gnome::Gda::Value& value) = 0;

  void refresh_related_fields(const LayoutFieldInRecord& field_in_record_changed, const Gtk::TreeModel::iterator& row, const Gnome::Gda::Value& field_value) override;

  /** Get the fields that are in related tables, via a relationship using @a field_name changes.
   */
  type_vecConstLayoutFields get_related_fields(const std::shared_ptr<const LayoutItem_Field>& field) const;

  /** Ask the user if he really wants to delete the record.
   */
  bool confirm_delete_record();

  /** Delete a record from the database table.
   * @param primary_key_value A primary key to indentify the record to delete.
   */
  bool record_delete(const Gnome::Gda::Value& primary_key_value);

  bool add_related_record_for_field(const std::shared_ptr<const LayoutItem_Field>& layout_item_parent, const std::shared_ptr<const Relationship>& relationship, const std::shared_ptr<const Field>& primary_key_field, const Gnome::Gda::Value& primary_key_value_provided, Gnome::Gda::Value& primary_key_value_used);

  virtual void on_record_added(const Gnome::Gda::Value& primary_key_value, const Gtk::TreeModel::iterator& row); //Overridden by derived classes.
  virtual void on_record_deleted(const Gnome::Gda::Value& primary_key_value); //Overridden by derived classes.

  type_signal_record_changed m_signal_record_changed;

private:
  bool get_related_record_exists(const std::shared_ptr<const Relationship>& relationship, const Gnome::Gda::Value& key_value);
};

} //namespace Glom

#endif // GLOM_BASE_DB_TABLE_DATA_H
