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

#ifndef GLOM_UTLITY_WIDGETS_ADDDEL_H
#define GLOM_UTLITY_WIDGETS_ADDDEL_H

#include <libglom/data_structure/field.h>
#include <gtkmm/builder.h>
#include <gtkmm/treeview.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/box.h>
#include <gtkmm/liststore.h>
#include <gtkmm/uimanager.h>
#include <giomm/simpleactiongroup.h>

#include <vector>
#include <map>

namespace Glom
{

class AddDelColumnInfo
{
public:
  AddDelColumnInfo();
  AddDelColumnInfo(const AddDelColumnInfo& src);
  AddDelColumnInfo(AddDelColumnInfo&& src) = delete;
  AddDelColumnInfo& operator=(const AddDelColumnInfo& src);
  AddDelColumnInfo& operator=(AddDelColumnInfo&& src) = delete;

  //If we need any more complicated style (e.g. number of decimal digits) then we will need a separate AddDelStyle class.
  enum class enumStyles
  {
    Text,
    Numerical, //TODO: Right-justify
    Boolean,
    Choices
  };

  enumStyles m_style;
  Glib::ustring m_name;
  Glib::ustring m_id;
  Field::glom_field_type m_field_type; //If any.

  typedef std::vector<Glib::ustring> type_vec_strings;
  type_vec_strings m_choices;

  bool m_editable;
  bool m_visible;
  bool m_prevent_duplicates;
};

/** For adding/deleting/selecting multi-columned lists of items,
 * allowing the user to add a new item by just entering data in an empty row at the end.
 */
class AddDel : public Gtk::Box
{
public:
  friend class InnerIgnore; //declared below.

  AddDel();
  AddDel(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~AddDel();

  // Set the accessible name for the TreeView widget
  void set_treeview_accessible_name(const Glib::ustring& name);

  virtual void set_allow_user_actions(bool bVal = true);
  bool get_allow_user_actions() const;

  virtual void set_allow_add(bool val = true);
  virtual void set_allow_delete(bool val = true);

  Gtk::TreeModel::iterator add_item(const Glib::ustring& strKey); //Return index of new row.

  /** Get an iterator to the blank row in which the user should add data for the new row.
   * You can then add the row to your underlying data store when some data has been filled, by handling signal_user_changed.
   */
  Gtk::TreeModel::iterator get_item_placeholder(); //Return index of the placeholder row.

  void remove_item(const Gtk::TreeModel::iterator& iter);
  void remove_item_by_key(const Glib::ustring& strKey);

  void remove_all();

  Glib::ustring get_value(const Gtk::TreeModel::iterator& iter, guint col);
  bool get_value_as_bool(const Gtk::TreeModel::iterator& iter, guint col);

  /** Get the row's hidden key
   */
  Glib::ustring get_value_key(const Gtk::TreeModel::iterator& iter);

  /** Set the row's hidden key
   */
  void set_value_key(const Gtk::TreeModel::iterator& iter, const Glib::ustring& strValue);

  Glib::ustring get_value_selected(guint col);
  Glib::ustring get_value_key_selected();

  Gtk::TreeModel::iterator get_item_selected();

  bool select_item(const Gtk::TreeModel::iterator& iter, guint column, bool start_editing = false);  //bool indicates success.
  bool select_item(const Gtk::TreeModel::iterator& iter);

  //Select row with this key value:
  bool select_item(const Glib::ustring& strItemText, guint column, bool start_editing = false);

  guint get_count() const;

  //void set_value(const Gtk::TreeModel::iterator& iter, guint col, const Gnome::Gda::Value& value);
  void set_value(const Gtk::TreeModel::iterator& iter, guint col, const Glib::ustring& strValue);
  void set_value(const Gtk::TreeModel::iterator& iter, guint col, unsigned long ulValue);
  void set_value(const Gtk::TreeModel::iterator& iter, guint col, bool bVal);

  //void set_value_selected(guint col, const Gnome::Gda::Value& value);

  bool get_is_first_row(const Gtk::TreeModel::iterator& iter) const;
  bool get_is_last_row(const Gtk::TreeModel::iterator& iter) const;

  /** @result Whether this is a blank row where date for a new row should be entered
   */
  bool get_is_placeholder_row(const Gtk::TreeModel::iterator& iter) const;


  guint add_column(const AddDelColumnInfo& column_info);
  guint add_column(const Glib::ustring& strTitle, AddDelColumnInfo::enumStyles style = AddDelColumnInfo::enumStyles::Text, bool editable = true, bool visible = true);
  guint add_column(const Glib::ustring& strTitle, const Glib::ustring& column_id, AddDelColumnInfo::enumStyles style = AddDelColumnInfo::enumStyles::Text, bool editable = true, bool visible = true);

  void prevent_duplicates(guint column_number);

  ///Allow the warning message about duplicate items to be more explicit.
  void set_prevent_duplicates_warning(const Glib::ustring& warning_text);

  guint get_columns_count() const;

  Glib::ustring get_column_field(guint column_index) const;

  typedef AddDelColumnInfo::type_vec_strings type_vec_strings;

  /** Retrieves the column order, even after they have been reordered by the user.
   * @result a vector of column_id. These column_ids were provided in the call to add_column().
   */
  type_vec_strings get_columns_order() const;

  void remove_all_columns();
  //void set_columns_count(guint count);
  //void set_column_title(guint col, const Glib::ustring& strText);
  void set_column_width(guint col, guint width);

  /// For popup cells.
  void set_column_choices(guint col, const type_vec_strings& vecStrings);

  void construct_specified_columns(); //Delay actual use of set_column_*() stuff until this method is called.

  Gtk::TreeModel::iterator get_row(const Glib::ustring& key);

  void finish_editing(); //Closes active edit controls and commits the data to the cell.
  //void reactivate(); //Sheet doesn't seem to update unless a cell is active.
  void set_prevent_user_signals(bool bVal = true);

  /** When this is set to true, a new row will be added automatically, and the cursor will be placed in the first column of the new row.
   * Use set_auto_add(false) if you want to provide default values for columns in the new row, or if you want to place the cursor in a different column.
   * If @a value is false then signal_user_requested_add will be emitted so that you can add the row explicitly.
   */
  void set_auto_add(bool value = true);

  Glib::RefPtr<Gtk::TreeModel> get_model();
  Glib::RefPtr<const Gtk::TreeModel> get_model() const;

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

  //TODO: Add this to the menu?
  /** This signal is emitted when the user presses the "extra" button.
   * This sends the row number.
   */
  typedef sigc::signal<void, const Gtk::TreeModel::iterator&> type_signal_user_requested_extra;
  type_signal_user_requested_extra signal_user_requested_extra();

  typedef sigc::signal<void> type_signal_user_requested_add;
  type_signal_user_requested_add signal_user_requested_add();

  //row number, col number.
  typedef sigc::signal<void, const Gtk::TreeModel::iterator&, guint> type_signal_user_activated;
  type_signal_user_activated signal_user_activated();

  typedef sigc::signal<void> type_signal_user_reordered_columns;
  type_signal_user_reordered_columns signal_user_reordered_columns();

  bool get_model_column_index(guint view_column_index, guint& model_column_index);

  /** Get the last row. This will generally be the placeholder row.
   */
  Gtk::TreeModel::iterator get_last_row();

  /** Get the last row. This will generally be the placeholder row.
   */
  Gtk::TreeModel::iterator get_last_row() const;


protected:
  void setup_menu(Gtk::Widget* widget);

  guint get_count_hidden_system_columns();

  void on_MenuPopup_activate_Edit();
  void on_MenuPopup_activate_Delete();

private:

private:
  void init();

  /** Get an iterator to the blank row in which the user should add data for the new row.
   * You can then add the row to your underlying data store when some data has been filled, by handling signal_user_changed.
   */
  Gtk::TreeModel::iterator add_item_placeholder(); //Return index of new row.

  Glib::ustring treeview_get_key(const Gtk::TreeModel::iterator& row);

  ///Add a blank row, or return the existing blank row if there already is one.
  Gtk::TreeModel::iterator get_next_available_row_with_add_if_necessary();
  void add_blank();


  //Signal handlers:
  void on_treeview_cell_edited(const Glib::ustring& path_string, const Glib::ustring& new_text, int model_column_index);
  void on_treeview_cell_edited_bool(const Glib::ustring& path_string, int model_column_index);

  void on_treeview_cell_editing_started(Gtk::CellEditable* editable, const Glib::ustring& path, int model_column_index);

  bool on_treeview_column_drop(Gtk::TreeView* treeview, Gtk::TreeViewColumn* column, Gtk::TreeViewColumn* prev_column, Gtk::TreeViewColumn* next_column);
  void on_treeview_columns_changed();

  bool on_button_press_event_Popup(GdkEventButton* button_event);

  void on_treeview_button_press_event(GdkEventButton* button_event);

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

  //The column_id is extra information that we can use later to discover what the column shows, even when columns have been reordered.
  guint treeview_append_column(const Glib::ustring& title, Gtk::CellRenderer& cellrenderer, const Gtk::TreeModelColumnBase& model_column, const Glib::ustring& column_id);

  template<class T_ModelColumnType>
  guint treeview_append_column(const Glib::ustring& title, const Gtk::TreeModelColumn<T_ModelColumnType>& column, const Glib::ustring& column_id);

  typedef Gtk::Box type_base;

  //Member widgets:
  Gtk::ScrolledWindow m_ScrolledWindow;

protected:
  Gtk::TreeView m_TreeView;

private:
  Gtk::TreeModel::ColumnRecord m_ColumnRecord;
  Glib::RefPtr<Gtk::ListStore> m_refListStore;
  guint m_col_key; //The index of the hidden model column.
  guint m_col_placeholder; //The index of the placeholder-marker model column.

  typedef std::vector<AddDelColumnInfo> type_ColumnTypes;
  type_ColumnTypes m_ColumnTypes;

  bool m_bAllowUserActions;

  bool m_bPreventUserSignals;
  bool m_bIgnoreSheetSignals;

  type_vec_strings m_vecColumnIDs; //We give each ViewColumn a special ID, so we know where they are after a reorder.

  Glib::ustring m_strTextActiveCell; //value before the change
  Gtk::Menu* m_pMenuPopup;
  Glib::RefPtr<Gio::SimpleActionGroup> m_refActionGroup;
  Glib::RefPtr<Gio::SimpleAction> m_refContextEdit, m_refContextDelete;

protected:
  bool m_auto_add;

private:
  bool m_allow_add;
  bool m_allow_delete;
  Glib::ustring m_prevent_duplicates_warning;

  //signals:
  type_signal_user_added m_signal_user_added;
  type_signal_user_changed m_signal_user_changed;
  type_signal_user_requested_delete m_signal_user_requested_delete;
  type_signal_user_requested_edit m_signal_user_requested_edit;
  type_signal_user_requested_extra m_signal_user_requested_extra;
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
  auto pCellRenderer = manage( Gtk::CellRenderer_Generation::generate_cellrenderer<T_ModelColumnType>() );
  return treeview_append_column(title, *pCellRenderer, column, column_id);
}

} //namespace Glom


#endif // GLOM_UTLITY_WIDGETS_ADDDEL_H
