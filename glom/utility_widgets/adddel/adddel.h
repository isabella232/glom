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

#ifndef ADDDEL_H
#define ADDDEL_H

#include "gtkmm.h"
#include <glom/libglom/data_structure/field.h>
#include <libgdamm.h>
#include <libglademm.h>

#include <vector>
#include <map>

namespace Glom
{

class AddDelColumnInfo
{
public:
  AddDelColumnInfo();
  AddDelColumnInfo(const AddDelColumnInfo& src);
  AddDelColumnInfo& operator=(const AddDelColumnInfo& src);

  //If we need any more complicated style (e.g. number of decimal digits) then we will need a separate AddDelStyle class.
  enum enumStyles
  {
    STYLE_Text,
    STYLE_Numerical, //TODO: Right-justify
    STYLE_Boolean,
    STYLE_Choices
  };

  enumStyles m_style;
  Glib::ustring m_name;
  Glib::ustring m_id;
  Field::glom_field_type m_field_type; //If any.

  typedef std::vector<Glib::ustring> type_vecStrings;
  type_vecStrings m_choices;

  bool m_editable;
  bool m_visible;
  bool m_prevent_duplicates;
};

//For adding/deleting/selecting multi-columned lists of items.
//This was also an abstraction layer against the strangeness of GtkSheet, though it now uses Gtk::TreeView instead.
class AddDel : public Gtk::VBox
{
public:
  friend class InnerIgnore; //declared below.

  AddDel();
  AddDel(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~AddDel();

  virtual void set_allow_user_actions(bool bVal = true);
  virtual bool get_allow_user_actions() const;

  virtual void set_allow_add(bool val = true);
  virtual void set_allow_delete(bool val = true);

  void set_allow_column_chooser(bool value = true);

  virtual Gtk::TreeModel::iterator add_item(const Glib::ustring& strKey); //Return index of new row.

  /** Get an iterator to the blank row in which the user should add data for the new row.
   * You can then add the row to your underlying data store when some data has been filled, by handling signal_user_changed.
   */
  virtual Gtk::TreeModel::iterator get_item_placeholder(); //Return index of the placeholder row.

  virtual void remove_item(const Gtk::TreeModel::iterator& iter);
  virtual void remove_item_by_key(const Glib::ustring& strKey);

  virtual void remove_all();

  virtual Glib::ustring get_value(const Gtk::TreeModel::iterator& iter, guint col);
  virtual bool get_value_as_bool(const Gtk::TreeModel::iterator& iter, guint col);

  /** Get the row's hidden key
   */
  virtual Glib::ustring get_value_key(const Gtk::TreeModel::iterator& iter);

  /** Set the row's hidden key
   */
  virtual void set_value_key(const Gtk::TreeModel::iterator& iter, const Glib::ustring& strValue);

  virtual Glib::ustring get_value_selected(guint col);
  virtual Glib::ustring get_value_key_selected();

  virtual Gtk::TreeModel::iterator get_item_selected();

  virtual bool select_item(const Gtk::TreeModel::iterator& iter, guint column, bool start_editing = false);  //bool indicates success.
  virtual bool select_item(const Gtk::TreeModel::iterator& iter);

  //Select row with this key value:
  virtual bool select_item(const Glib::ustring& strItemText, guint column, bool start_editing = false);

  virtual guint get_count() const;

  //virtual void set_value(const Gtk::TreeModel::iterator& iter, guint col, const Gnome::Gda::Value& value);
  virtual void set_value(const Gtk::TreeModel::iterator& iter, guint col, const Glib::ustring& strValue);
  virtual void set_value(const Gtk::TreeModel::iterator& iter, guint col, unsigned long ulValue);
  virtual void set_value(const Gtk::TreeModel::iterator& iter, guint col, bool bVal);

  //virtual void set_value_selected(guint col, const Gnome::Gda::Value& value);

  virtual bool get_is_first_row(const Gtk::TreeModel::iterator& iter) const;
  virtual bool get_is_last_row(const Gtk::TreeModel::iterator& iter) const;

  /** @result Whether this is a blank row where date for a new row should be entered
   */
  virtual bool get_is_placeholder_row(const Gtk::TreeModel::iterator& iter) const;


  virtual guint add_column(const AddDelColumnInfo& column_info);
  virtual guint add_column(const Glib::ustring& strTitle, AddDelColumnInfo::enumStyles style = AddDelColumnInfo::STYLE_Text, bool editable = true, bool visible = true);
  virtual guint add_column(const Glib::ustring& strTitle, const Glib::ustring& column_id, AddDelColumnInfo::enumStyles style = AddDelColumnInfo::STYLE_Text, bool editable = true, bool visible = true);

  void prevent_duplicates(guint column_number);

  ///Allow the warning message about duplicate items to be more explicit.
  void set_prevent_duplicates_warning(const Glib::ustring& warning_text);

  guint get_columns_count() const;

  virtual Glib::ustring get_column_field(guint column_index) const;

  typedef AddDelColumnInfo::type_vecStrings type_vecStrings;

  /** Retrieves the column order, even after they have been reordered by the user.
   * @result a vector of column_id. These column_ids were provided in the call to add_column().
   */
  type_vecStrings get_columns_order() const;

  virtual void remove_all_columns();
  //virtual void set_columns_count(guint count);
  //virtual void set_column_title(guint col, const Glib::ustring& strText);
  virtual void set_column_width(guint col, guint width);

  /// For popup cells.
  virtual void set_column_choices(guint col, const type_vecStrings& vecStrings);

  virtual void construct_specified_columns(); //Delay actual use of set_column_*() stuff until this method is called.

  void set_show_column_titles(bool bVal = true);

  virtual Gtk::TreeModel::iterator get_row(const Glib::ustring& key);

  virtual void finish_editing(); //Closes active edit controls and commits the data to the cell.
  //virtual void reactivate(); //Sheet doesn't seem to update unless a cell is active.
  void set_prevent_user_signals(bool bVal = true);

  /** When this is set to true, a new row will be added automatically, and the cursor will be placed in the first column of the new row.
   * Use set_auto_add(false) if you want to provide default values for columns in the new row, or if you want to place the cursor in a different column.
   * If @a value is false then signal_user_requested_add will be emitted so that you can add the row explicitly.
   */
  void set_auto_add(bool value = true);

  Glib::RefPtr<Gtk::TreeModel> get_model();
  Glib::RefPtr<const Gtk::TreeModel> get_model() const;

  void set_rules_hint(bool val = true);

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
  void init();

  /** Get an iterator to the blank row in which the user should add data for the new row.
   * You can then add the row to your underlying data store when some data has been filled, by handling signal_user_changed.
   */
  virtual Gtk::TreeModel::iterator add_item_placeholder(); //Return index of new row.

  virtual void setup_menu();
  virtual Glib::ustring treeview_get_key(const Gtk::TreeModel::iterator& row);

  ///Add a blank row, or return the existing blank row if there already is one.
  virtual Gtk::TreeModel::iterator get_next_available_row_with_add_if_necessary();
  virtual void add_blank();


  //Signal handlers:
  virtual void on_treeview_cell_edited(const Glib::ustring& path_string, const Glib::ustring& new_text, int model_column_index);
  virtual void on_treeview_cell_edited_bool(const Glib::ustring& path_string, int model_column_index);

  void on_treeview_cell_editing_started(Gtk::CellEditable* editable, const Glib::ustring& path, int model_column_index);

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
  void set_column_header_popup(Gtk::Menu& popup);

  bool row_has_duplicates(const Gtk::TreeModel::iterator& iter) const;
  void warn_about_duplicate();

  bool get_prevent_user_signals() const;

  //Sometimes the sheet sends signals when it shouldn't:
  void set_ignore_treeview_signals(bool bVal = true);
  bool get_ignore_treeview_signals() const;

  bool get_view_column_index(guint model_column_index, guint& view_column_index);
  guint get_count_hidden_system_columns();

  //The column_id is extra information that we can use later to discover what the column shows, even when columns have been reordered.
  guint treeview_append_column(const Glib::ustring& title, Gtk::CellRenderer& cellrenderer, const Gtk::TreeModelColumnBase& model_column, const Glib::ustring& column_id);

  template<class T_ModelColumnType>
  guint treeview_append_column(const Glib::ustring& title, const Gtk::TreeModelColumn<T_ModelColumnType>& column, const Glib::ustring& column_id);

  typedef Gtk::VBox type_base;

  //Member widgets:
  Gtk::ScrolledWindow m_ScrolledWindow;

  Gtk::TreeView m_TreeView;
  Gtk::TreeModel::ColumnRecord m_ColumnRecord;
  Glib::RefPtr<Gtk::ListStore> m_refListStore;
  guint m_col_key; //The index of the hidden model column.
  guint m_col_placeholder; //The index of the placeholder-marker model column.

  typedef std::vector<AddDelColumnInfo> type_ColumnTypes;
  type_ColumnTypes m_ColumnTypes;

  Gtk::Menu m_MenuPopup;

  bool m_bAllowUserActions;

  bool m_bPreventUserSignals;
  bool m_bIgnoreSheetSignals;

  type_vecStrings m_vecColumnIDs; //We give each ViewColumn a special ID, so we know where they are after a reorder.

  Glib::ustring m_strTextActiveCell; //value before the change
  Gtk::Menu* m_pColumnHeaderPopup;
  bool m_allow_column_chooser;
  bool m_auto_add;
  bool m_allow_add;
  bool m_allow_delete;
  Glib::ustring m_prevent_duplicates_warning;

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
    InnerIgnore(AddDel* pOuter);
    ~InnerIgnore();

  protected:
    AddDel* m_pOuter;
    bool m_bPreventUserSignals, m_bIgnoreSheetSignals;
  };

};

template<class T_ModelColumnType>
guint AddDel::treeview_append_column(const Glib::ustring& title, const Gtk::TreeModelColumn<T_ModelColumnType>& column, const Glib::ustring& column_id)
{
  Gtk::CellRenderer* pCellRenderer = manage( Gtk::CellRenderer_Generation::generate_cellrenderer<T_ModelColumnType>() );
  return treeview_append_column(title, *pCellRenderer, column, column_id);
}

} //namespace Glom


#endif //ADDDEL_H
