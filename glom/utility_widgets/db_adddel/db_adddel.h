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

#ifndef GLOM_DB_ADDDEL_H
#define GLOM_DB_ADDDEL_H

#include "gtkmm.h"
#include "../../data_structure/field.h"
#include <libgdamm.h>

#include <vector>
#include <map>


class DbAddDelColumnInfo
{
public:
  DbAddDelColumnInfo();
  DbAddDelColumnInfo(const DbAddDelColumnInfo& src);
  DbAddDelColumnInfo& operator=(const DbAddDelColumnInfo& src);

  Field m_field;

  typedef std::vector<Glib::ustring> type_vecStrings;
  type_vecStrings m_choices;

  bool m_editable;
  bool m_visible;
};

class DbTreeViewColumnGlom;

//For adding/deleting/selecting multi-columned lists of items.
//This was also an abstraction layer against the strangeness of GtkSheet, though it now uses Gtk::TreeView instead.
class DbAddDel : public Gtk::VBox
{
public:
  friend class InnerIgnore; //declared below.

  DbAddDel();
  virtual ~DbAddDel();

  virtual void set_allow_user_actions(bool bVal = true);
  virtual bool get_allow_user_actions() const;

  virtual void set_allow_add(bool val = true);
  virtual void set_allow_delete(bool val = true);
    
  virtual void set_allow_column_chooser(bool value = true);
  
  virtual Gtk::TreeModel::iterator add_item(const Gnome::Gda::Value& valKey); //Return index of new row.

  /** Get an iterator to the blank row in which the user should add data for the new row.
   * You can then add the row to your underlying data store when some data has been filled, by handling signal_user_changed.
   */
  virtual Gtk::TreeModel::iterator get_item_placeholder(); //Return index of the placeholder row.
  
  virtual void remove_item(const Gtk::TreeModel::iterator& iter);

  virtual void remove_all();

  virtual Gnome::Gda::Value get_value_as_value(const Gtk::TreeModel::iterator& iter, guint col );

  /** Get the row's hidden key
   */
  virtual Gnome::Gda::Value get_value_key_as_value(const Gtk::TreeModel::iterator& iter);

  /** Set the row's hidden key
   */
  virtual void set_value_key(const Gtk::TreeModel::iterator& iter, const Gnome::Gda::Value& value);

  /** @param col A value returned from add_column().
   * @result The value on the selected row.
   */
  virtual Gnome::Gda::Value get_value_selected_as_value(guint col);
  virtual Gnome::Gda::Value get_value_key_selected_as_value();
  
  virtual Gtk::TreeModel::iterator get_item_selected();

  /** 
   * @param iter The row to be selected. 
   * @param column A value returned from add_column().
   * @param start_editing Whether editing should start in the cell.
   * @result Whether the row was successfully selected.
   */
  virtual bool select_item(const Gtk::TreeModel::iterator& iter, guint column, bool start_editing = false);  //bool indicates success.
  virtual bool select_item(const Gtk::TreeModel::iterator& iter);
  
  virtual guint get_count() const;

  /** 
   * @param iter The row to be changed. 
   * @param col A value returned from add_column().
   * @param value The new value.
   */
  virtual void set_value(const Gtk::TreeModel::iterator& iter, guint col, const Gnome::Gda::Value& value);

  /** 
   * @param col A value returned from add_column().
   * @param value The new value.
   */
  virtual void set_value_selected(guint col, const Gnome::Gda::Value& value);

  virtual bool get_is_first_row(const Gtk::TreeModel::iterator& iter) const;
  virtual bool get_is_last_row(const Gtk::TreeModel::iterator& iter) const;

  /** @result Whether this is a blank row where date for a new row should be entered
   */
  virtual bool get_is_placeholder_row(const Gtk::TreeModel::iterator& iter) const;
  
  virtual void set_select_text(const Glib::ustring& strVal);
  virtual Glib::ustring get_select_text() const;

  //Use this in order to use get_value_key_as_value().
  virtual void set_key_type(const Field& field);
 
  /** @result The index of the new column.
   */
  virtual guint add_column(const Field& field, bool editable = true, bool visible = true);

  virtual guint get_columns_count() const;

  virtual Glib::ustring get_column_field(guint column_index) const;
  
  typedef DbAddDelColumnInfo::type_vecStrings type_vecStrings;

  /** Retrieves the column order, even after they have been reordered by the user.
   * @result a vector of column_id. These column_ids were provided in the call to add_column().
   */
  virtual type_vecStrings get_columns_order() const;
  
  virtual void remove_all_columns();
  //virtual void set_columns_count(guint count);
  //virtual void set_column_title(guint col, const Glib::ustring& strText);
  virtual void set_column_width(guint col, guint width);

  /// For popup cells.
  virtual void set_column_choices(guint col, const type_vecStrings& vecStrings);
   
  virtual void construct_specified_columns(); //Delay actual use of set_column_*() stuff until this method is called.

  virtual void set_show_column_titles(bool bVal = true);

  virtual Gtk::TreeModel::iterator get_row(const Gnome::Gda::Value& key);

  virtual void finish_editing(); //Closes active edit controls and commits the data to the cell.
  //virtual void reactivate(); //Sheet doesn't seem to update unless a cell is active.
  void set_prevent_user_signals(bool bVal = true);

  /** When this is set to true, a new row will be added automatically, and the cursor will be placed in the first column of the new row.
   * Use set_auto_add(false) if you want to provide default values for columns in the new row, or if you want to place the cursor in a different column.
   * If @a value is false then signal_user_requested_add will be emitted so that you can add the row explicitly.
   */
  virtual void set_auto_add(bool value = true);

  Glib::RefPtr<Gtk::TreeModel> get_model();
  Glib::RefPtr<const Gtk::TreeModel> get_model() const;

  virtual void set_rules_hint(bool val = true);
      
  //Signals:
  //row number.
  typedef sigc::signal<void, const Gtk::TreeModel::iterator&> type_signal_user_added;
  type_signal_user_added signal_user_added();

  //row number, col number.
  typedef sigc::signal<void, const Gtk::TreeModel::iterator&, guint> type_signal_user_changed;
  type_signal_user_changed signal_user_changed();

  //start row, end row
  typedef sigc::signal<void, const Gtk::TreeModel::iterator&, const Gtk::TreeModel::iterator&> type_signal_user_requested_delete;
  type_signal_user_requested_delete signal_user_requested_delete();

  //row number.
  typedef sigc::signal<void, const Gtk::TreeModel::iterator&> type_signal_user_requested_edit;
  type_signal_user_requested_edit signal_user_requested_edit();

  typedef sigc::signal<void> type_signal_user_requested_add;
  type_signal_user_requested_add signal_user_requested_add();

  //row number, col number.
  typedef sigc::signal<void, const Gtk::TreeModel::iterator&, guint> type_signal_user_activated;
  type_signal_user_activated signal_user_activated();

  typedef sigc::signal<void> type_signal_user_reordered_columns;
  type_signal_user_reordered_columns signal_user_reordered_columns();

  bool get_model_column_index(guint view_column_index, guint& model_column_index);

  virtual Gtk::TreeModel::iterator get_last_row();
  virtual Gtk::TreeModel::iterator get_last_row() const;
  
protected:

  /** Get an iterator to the blank row in which the user should add data for the new row.
   * You can then add the row to your underlying data store when some data has been filled, by handling signal_user_changed.
   */
  virtual Gtk::TreeModel::iterator add_item_placeholder(); //Return index of new row.
  
  virtual void setup_menu();
  virtual Gnome::Gda::Value treeview_get_key(const Gtk::TreeModel::iterator& row);

  ///Add a blank row, or return the existing blank row if there already is one.
  virtual Gtk::TreeModel::iterator get_next_available_row_with_add_if_necessary();
  virtual void add_blank();

  
  //Signal handlers:
  void treeviewcolumn_on_cell_data(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter, int model_column_index);

  virtual void on_treeview_cell_edited(const Glib::ustring& path_string, const Glib::ustring& new_text, int model_column_index);
  virtual void on_treeview_cell_edited_bool(const Glib::ustring& path_string, int model_column_index);
  
  virtual bool on_treeview_column_drop(Gtk::TreeView* treeview, Gtk::TreeViewColumn* column, Gtk::TreeViewColumn* prev_column, Gtk::TreeViewColumn* next_column);
  virtual void on_treeview_columns_changed();
  
  virtual bool on_button_press_event_Popup(GdkEventButton* event);

  virtual void on_MenuPopup_activate_Edit();
  virtual void on_MenuPopup_activate_Delete();
  virtual void on_MenuPopup_activate_ChooseColumns();

  virtual void on_treeview_button_press_event(GdkEventButton* event);

  virtual bool on_treeview_columnheader_button_press_event(GdkEventButton* event);

  /** Set the menu to popup when the user right-clicks on the column titles.
   * This method does not take ownership of the Gtk::Menu.
   */
  virtual void set_column_header_popup(Gtk::Menu& popup);



  bool get_prevent_user_signals() const;

  //Sometimes the sheet sends signals when it shouldn't:
  void set_ignore_treeview_signals(bool bVal = true);
  bool get_ignore_treeview_signals() const;

  /** @param model_column_index A value returned from add_column().
   * @param view_column_index The index of the corresponding view column.
   */
  bool get_view_column_index(guint model_column_index, guint& view_column_index);
  
  guint get_count_hidden_system_columns();

  //The column_id is extra information that we can use later to discover what the column shows, even when columns have been reordered.
  guint treeview_append_column(const Glib::ustring& title, Gtk::CellRenderer& cellrenderer, int model_column_index);

  static Glib::ustring string_escape_underscores(const Glib::ustring& text);
  
  typedef Gtk::VBox type_base;

  //Member widgets:
  Gtk::ScrolledWindow m_ScrolledWindow;

  Gtk::TreeView m_TreeView;
  Gtk::TreeModel::ColumnRecord m_ColumnRecord;
  Glib::RefPtr<Gtk::ListStore> m_refListStore;
  
  //Hidden internal columns:
  Gtk::TreeModelColumn<Gnome::Gda::Value>* m_modelcolumn_key;
  Gtk::TreeModelColumn<bool>* m_modelcolumn_placeholder; //placeholder-marker model column.
    
  
  //Columns, not including the hidden internal columns:
  typedef std::vector<DbAddDelColumnInfo> type_ColumnTypes;
  type_ColumnTypes m_ColumnTypes;

  Gtk::Menu m_MenuPopup;

  Glib::ustring m_strSelectText; //e.g. 'Edit', 'Use'.

  bool m_bAllowUserActions;

  bool m_bPreventUserSignals;
  bool m_bIgnoreSheetSignals;
  
  type_vecStrings m_vecColumnIDs; //We give each ViewColumn a special ID, so we know where they are after a reorder.
  
  Gtk::Menu* m_pColumnHeaderPopup;
  bool m_allow_column_chooser;
  bool m_auto_add;
  bool m_allow_add;
  bool m_allow_delete;

  Field m_key_field;
  
  //signals:
  type_signal_user_added m_signal_user_added;
  type_signal_user_changed m_signal_user_changed;
  type_signal_user_requested_delete m_signal_user_requested_delete;
  type_signal_user_requested_edit m_signal_user_requested_edit;
  type_signal_user_requested_add m_signal_user_requested_add;
  type_signal_user_activated m_signal_user_activated;
  type_signal_user_reordered_columns m_signal_user_reordered_columns;

  //An instance of InnerIgnore remembers the ignore settings,
  //then restores them when it goes out of scope and is destroyed.
  class InnerIgnore
  {
  public:
    InnerIgnore(DbAddDel* pOuter);
    ~InnerIgnore();

  protected:
    DbAddDel* m_pOuter;
    bool m_bPreventUserSignals, m_bIgnoreSheetSignals;
  };

/*
  class DynamicColumnRecord : public Gtk::TreeModel::ColumnRecord
  {
    typedef std::vector<Gtk::TreeModelColumnBase> type_vecColumns;
    type_vecColumns m_vecColumns;
  };
*/
};
 



#endif //GLOM_DB_ADDDEL_H
