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

#ifndef BOX_DATA_H
#define BOX_DATA_H

#include "config.h" // GLOM_ENABLE_CLIENT_ONLY

#include "../box_db_table.h"
#include "dialog_layout.h"

namespace Glom
{

/** Call init_db_details() to create the layout and fill it with data from the database.
 * Call refresh_data_from_database() to fill the existing layout with up-to-date data from the database.
 *
 * Derived classes should implement create_layout() to create/arrange the widgets for the groups, fields, portals, etc.
 * Derived classes should implement fill_from_database() to get the data from the database and fill the widgets created by create_layout().
 */
class Box_Data : public Box_DB_Table
{
public: 
  Box_Data();
  virtual ~Box_Data();

  ///Create the layout for the database structure, and fill it with data from the database.
  virtual bool init_db_details(const FoundSet& found_set);

  //Fill the existing layout with data from the database:
  virtual bool refresh_data_from_database_with_where_clause(const FoundSet& found_set);

  virtual void print_layout(); //A test, for now.

  ///Get the existing where clause, previously supplied to init_db_details().
  FoundSet get_found_set() const;

  virtual Glib::ustring get_find_where_clause() const;

  virtual void set_unstored_data(bool bVal);
  virtual bool get_unstored_data() const;

  virtual bool confirm_discard_unstored_data() const;

#ifndef GLOM_ENABLE_CLIENT_ONLY
  virtual void show_layout_dialog();
#endif // !GLOM_ENABLE_CLIENT_ONLY
  Glib::ustring get_layout_name() const;

  //Signals:

  /** Emitted when the user has entered a find critera that
   * should be used to find and display records.
   * Used by _Find sub-classes.
   * @param find_criteria The SQL where clause.
   */
  //Should be a MI class, derived by those sub-classes. TODO.
  //where_clause.
  sigc::signal<void, Glib::ustring> signal_find_criteria;

#ifndef GLOM_ENABLE_CLIENT_ONLY
  //g++ 3.4 needs this to be public when used from Box_Data_Details. I'm not sure why. murrayc.
  virtual void on_dialog_layout_hide();
#endif // !GLOM_ENABLE_CLIENT_ONLY

protected:

  /* Create the layout based on the database structure and saved layout,
   * so that fill_from_database() can fill it with data.
   */
  virtual void create_layout();

  ///Fill the existing layout with data from the database.
  virtual bool fill_from_database(); //override.

  virtual void refresh_related_fields(const LayoutFieldInRecord& field_in_record_changed, const Gtk::TreeModel::iterator& row, const Gnome::Gda::Value& field_value);

  virtual type_vecLayoutFields get_fields_to_show() const;

  virtual bool get_related_record_exists(const sharedptr<const Relationship>& relationship, const sharedptr<const Field>& key_field, const Gnome::Gda::Value& key_value);
  virtual bool add_related_record_for_field(const sharedptr<const LayoutItem_Field>& layout_item_parent, const sharedptr<const Relationship>& relationship, const sharedptr<const Field>& primary_key_field, const Gnome::Gda::Value& primary_key_value_provided);

  type_vecLayoutFields get_table_fields_to_show(const Glib::ustring& table_name) const;

  /** Get the layout groups, with the Field information filled in.
   */
  Document_Glom::type_mapLayoutGroupSequence get_data_layout_groups(const Glib::ustring& layout);
  void fill_layout_group_field_info(const sharedptr<LayoutGroup>& group, const Privileges& table_privs);


  /** Get the fields that are in related tables, via a relationship using @a field_name changes.
  */
  type_vecLayoutFields get_related_fields(const sharedptr<const LayoutItem_Field>& field) const;

  bool record_delete(const Gnome::Gda::Value& primary_key_value);

  ///This allows record_new() to set the generated/entered primary key value, needed by Box_Data_List:
  virtual void set_primary_key_value(const Gtk::TreeModel::iterator& row, const Gnome::Gda::Value& value);

  ///New record with all entered field values.
  Glib::RefPtr<Gnome::Gda::DataModel> record_new(bool use_entered_data = true, const Gnome::Gda::Value& primary_key_value = Gnome::Gda::Value()); 
  Glib::RefPtr<Gnome::Gda::DataModel> record_new(bool use_entered_data, const Gnome::Gda::Value& primary_key_value, const Gtk::TreeModel::iterator& row);

  Gnome::Gda::Value generate_next_auto_increment(const Glib::ustring& table_name, const Glib::ustring field_name);

  virtual sharedptr<Field> get_field_primary_key() const = 0;
  virtual Gnome::Gda::Value get_primary_key_value_selected() = 0;
  //virtual bool get_field(const Glib::ustring& name, sharedptr<Field>& field) const;

  bool confirm_delete_record();

  void execute_button_script(const sharedptr<const LayoutItem_Button>& layout_item, const Gnome::Gda::Value& primary_key_value);

  //Signal handlers:
  virtual void on_Button_Find(); //only used by _Find sub-classes. Should be MI.

  static Glib::ustring xslt_process(const xmlpp::Document& xml_document, const std::string& filepath_xslt);

  Gtk::Button m_Button_Find; //only used by _Find sub-classes. Should be MI.
  Gtk::Label m_Label_FindStatus;

  bool m_bUnstoredData;

#ifndef GLOM_ENABLE_CLIENT_ONLY
  Dialog_Layout* m_pDialogLayout;
#endif // !GLOM_ENABLE_CLIENT_ONLY
  Glib::ustring m_layout_name;

  FoundSet m_found_set;

  type_vecFields m_TableFields; //A cache, so we don't have to repeatedly get them from the Document.
  type_vecLayoutFields m_FieldsShown; //And any extra keys needed by shown fields.
};

} //namespace Glom

#endif
