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

#ifndef GLOM_DB_ADDDEL_H
#define GLOM_DB_ADDDEL_H

#include <gtkmm/box.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <libglom/data_structure/layout/layoutitem_field.h>
#include <glom/mode_data/datawidget/treemodel_db.h>
#include <libglom/document/document.h>
#include <glom/base_db_table_data.h>
#include <giomm/simpleactiongroup.h>

#include <vector>
#include <map>

#include "config.h" // For GLOM_ENABLE_CLIENT_ONLY

namespace Glom
{

class AppWindow;

class DbTreeViewColumnGlom;

/** For adding/deleting/selecting record rows.
 */
class DbAddDel
 : public Gtk::Box,
   public Base_DB_Table_Data
{
public:
  friend class InnerIgnore; //declared below.

  DbAddDel();
  virtual ~DbAddDel();

  virtual void set_allow_user_actions(bool value = true);
  bool get_allow_user_actions() const;

  virtual void set_allow_add(bool val = true);
  virtual void set_allow_delete(bool val = true);

  /** Prevent any attempts by this class to change actual records,
   * if the widget is just being used to enter find critera,
   * and prevents any need for data retrieval from the database, because
   * no data will be displayed.
   *
   * @param val True if find mode should be used.
   */
  void set_find_mode(bool val = true);

  /** Prevent more than one record from being added,
   * Use this if the portal is showing related records,
   * and if the relationship's to-field is unique or a primary key.
   * In this case, adding a new record would require a duplicate value in that
   * unique field.
   * When the user tries to do this, he will see an explanatory dialog from this
   * widget.
   *
   * @param val True if multiple records  should be presented.
   */
  void set_allow_only_one_related_record(bool val = true);


  //Gtk::TreeModel::iterator add_item(const Gnome::Gda::Value& valKey); //Return index of new row.

  void remove_item(const Gtk::TreeModel::iterator& iter);

  Gnome::Gda::Value get_value(const Gtk::TreeModel::iterator& iter, const std::shared_ptr<const LayoutItem_Field>& layout_item) const;

  /** Get the row's hidden key
   */
  Gnome::Gda::Value get_value_key(const Gtk::TreeModel::iterator& iter) const;

  /** Set the row's hidden key
   */
  void set_value_key(const Gtk::TreeModel::iterator& iter, const Gnome::Gda::Value& value);

  /** @param col A value returned from add_column().
   * @result The value on the selected row.
   */
  Gnome::Gda::Value get_value_selected(const std::shared_ptr<const LayoutItem_Field>& layout_item) const;
  Gnome::Gda::Value get_value_key_selected() const;

  Gtk::TreeModel::iterator get_item_selected();
  Gtk::TreeModel::iterator get_item_selected() const; //There is no TreeModel::const_iterator

  /**
   * @param iter The row to be selected.
   * @param column A value returned from add_column().
   * @param start_editing Whether editing should start in the cell.
   * @result Whether the row was successfully selected.
   */
  bool select_item(const Gtk::TreeModel::iterator& iter, const std::shared_ptr<const LayoutItem>& layout_item, bool start_editing = false);  //bool indicates success.
  bool select_item(const Gtk::TreeModel::iterator& iter, bool start_editing = false);

  guint get_count() const;

  /**
   * @param iter The row to be changed.
   * @param layout_item Describes the column(s) whose values should be changed.
   * @param value The new value.
   */
  virtual void set_value(const Gtk::TreeModel::iterator& iter, const std::shared_ptr<const LayoutItem_Field>& layout_item, const Gnome::Gda::Value& value);

  /**
   * @param col A value returned from add_column().
   * @param value The new value.
   */
  virtual void set_value_selected(const std::shared_ptr<const LayoutItem_Field>& layout_item, const Gnome::Gda::Value& value);

  bool get_is_first_row(const Gtk::TreeModel::iterator& iter) const;

  /** Check whether the row is the last (non-placeholder) row.
   * This will also return true if @a iter is the placeholder row,
   * but this should be used to identify the last real row.
   * @see get_is_placeholder_row()
   */
  bool get_is_last_row(const Gtk::TreeModel::iterator& iter) const;

  /** @result Whether this is a blank row where date for a new row should be entered
   */
  bool get_is_placeholder_row(const Gtk::TreeModel::iterator& iter) const;

  std::shared_ptr<Field> get_key_field() const;
  void set_key_field(const std::shared_ptr<Field>& field);

  void set_table_name(const Glib::ustring& table_name);

  ///For instance, if the user cannot view the table then don't try to get the records.
  void set_allow_view(bool val = true);

  ///Whether each row should have a button, to request edit.
  virtual void set_allow_view_details(bool val = true);
  bool get_allow_view_details() const;

  //TODO: Just add this as a parameter to a different method?
  /** Specify which records to show.
   * This does not actually request the data from the database - it just
   * sets the found set to use when that happens later.
   */
  void set_found_set(const FoundSet& found_set);

  /** Set the items to show, and actually get and show the data from the database.
   * The items are not const, so that their display widths can be changed in the UI.
   */
  void set_columns(const LayoutGroup::type_list_items& layout_items);


  FoundSet get_found_set() const;

  guint get_columns_count() const;

  std::shared_ptr<const LayoutItem_Field> get_column_field(guint column_index) const;

  void remove_all_columns();

  /// For popup cells.
  //void set_column_choices(guint col, const type_vec_strings& vecStrings);

  void construct_specified_columns(); //Delay actual use of set_column_*() stuff until this method is called.

  bool refresh_from_database();
  bool refresh_from_database_blank();

  Gtk::TreeModel::iterator get_row(const Gnome::Gda::Value& key);

  void finish_editing(); //Closes active edit controls and commits the data to the cell.
  //void reactivate(); //Sheet doesn't seem to update unless a cell is active.
  void set_prevent_user_signals(bool value = true);

  //TODO_refactor: make private.

private:
  void user_added(const Gtk::TreeModel::iterator& row);

public:
  Glib::RefPtr<Gtk::TreeModel> get_model();
  Glib::RefPtr<const Gtk::TreeModel> get_model() const;

  //Signals:

#ifndef GLOM_ENABLE_CLIENT_ONLY
  /** Emitted when the user wants to edit the layout of the items in this widget.
   */
  typedef sigc::signal<void> type_signal_user_requested_layout;
  type_signal_user_requested_layout signal_user_requested_layout();
#endif // !GLOM_ENABLE_CLIENT_ONLY

  /** Emitted when the user request a view/edit of the details of the record.
   * @param row
   */
  typedef sigc::signal<void, const Gtk::TreeModel::iterator&> type_signal_user_requested_edit;
  type_signal_user_requested_edit signal_user_requested_edit();

  /** Emitted when the user clicks on a script button.
   * @param layout_button The layout item for the script button that was clicked.
   * @param row
   */
  typedef sigc::signal<void, const std::shared_ptr<const LayoutItem_Button>&, const Gtk::TreeModel::iterator&> type_signal_script_button_clicked;
  type_signal_script_button_clicked signal_script_button_clicked();

  /** Allow a parent widget to set the foreign key when a record is added,
   * to make the new record a related record.
   *
   * @param row Row number
   * @param primary_key_value The value of the primary key of the new related record.
   */
  typedef sigc::signal<void, const Gtk::TreeModel::iterator&, const Gnome::Gda::Value&> type_signal_record_added;
  type_signal_record_added signal_record_added();

  /** Emitted when the user changed the sort order,
   * for instance by clicking on a column header.
   */
  typedef sigc::signal<void> type_signal_sort_clause_changed;
  type_signal_sort_clause_changed signal_sort_clause_changed();
  
  /** Emitted when the user selected (or deselected) a record.
   */
  typedef sigc::signal<void> type_signal_record_selection_changed;
  type_signal_record_selection_changed signal_record_selection_changed();

  /** Get the last row.
   * This will never return the placeholder row. 
   */
  Gtk::TreeModel::iterator get_last_row();
  
  /** Get the last row.
   * This will never return the placeholder row. 
   */
  Gtk::TreeModel::iterator get_last_row() const;

  void set_open_button_title(const Glib::ustring& title);


  /** Add a new row to the list, for the user to enter record details,
   * adding the generated primary key if necessary.
   */
  bool start_new_record();
  
  /** Request a height for this widget, based on the number of rows to show.
   * The widget will change its requested height if it is filled with enough 
   * data to need more than the @a rows_count_min, if @a rows_count_max allows that.
   */
  void set_height_rows(gulong rows_count_min, gulong rows_count_max);

private:

  void set_value(const Gtk::TreeModel::iterator& iter, const std::shared_ptr<const LayoutItem_Field>& layout_item, const Gnome::Gda::Value& value, bool set_specified_field_layout);

  //Overrides of Base_DB/Base_DB_Table methods:
  void set_entered_field_data(const std::shared_ptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value) override;
  void set_entered_field_data(const Gtk::TreeModel::iterator& row, const std::shared_ptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value) override;
  Gnome::Gda::Value get_entered_field_data(const std::shared_ptr<const LayoutItem_Field>& field) const override;
  Gtk::TreeModel::iterator get_row_selected() override;

  //Implementations of pure virtual methods from Base_DB_Table_Data:
  std::shared_ptr<Field> get_field_primary_key() const override;
  Gnome::Gda::Value get_primary_key_value_selected() const override;
  void set_primary_key_value(const Gtk::TreeModel::iterator& row, const Gnome::Gda::Value& value) override;
  Gnome::Gda::Value get_primary_key_value(const Gtk::TreeModel::iterator& row) const override;

  Gtk::CellRenderer* construct_specified_columns_cellrenderer(const std::shared_ptr<LayoutItem>& layout_item, int model_column_index, int data_model_column_index);

  bool get_model_column_index(guint view_column_index, guint& model_column_index);


  typedef std::list<guint> type_list_indexes;
  ///Return the column indexes of any columns that display this field.
  type_list_indexes get_column_index(const std::shared_ptr<const LayoutItem>& layout_item) const;

  /// Get indexes of any columns with choices with !show_all relationships that have @a from_key as the from_key.
  type_list_indexes get_choice_index(const std::shared_ptr<const LayoutItem_Field>& from_key);

  /** Return the query column index of any columns that display this field:
   * @param including_specified_field_layout If false, then don't return the actual layout item itself.
   */
  type_list_indexes get_data_model_column_index(const std::shared_ptr<const LayoutItem_Field>& layout_item_field, bool including_specified_field_layout = true) const;

protected:
  void setup_menu(Gtk::Widget* widget);

  /// A common handler for the edit button, the context menu, etc.
  void do_user_requested_edit();

  virtual void on_selection_changed(bool selection);

private:
  Gnome::Gda::Value treeview_get_key(const Gtk::TreeModel::iterator& row) const;

  ///Add a blank row, or return the existing blank row if there already is one.
  //Gtk::TreeModel::iterator get_next_available_row_with_add_if_necessary();

  //Signal handlers:
  void treeviewcolumn_on_cell_data(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter, int model_column_index, int data_model_column_index);
  void on_treeview_cell_editing_started(Gtk::CellEditable* cell_editable, const Glib::ustring& path);


  void on_treeview_cell_edited(const Glib::ustring& path_string, const Glib::ustring& new_text, int model_column_index, int data_model_column_index);
  void on_treeview_cell_edited_bool(const Glib::ustring& path_string, int model_column_index, int data_model_column_index);
  void on_idle_treeview_cell_edited_revert(const Gtk::TreeModel::Row& row, guint model_column_index);

  void on_treeview_columns_changed();

  bool on_button_press_event_Popup(GdkEventButton* button_event);
  void on_treeview_button_press_event(GdkEventButton* button_event);
  void on_treeview_selection_changed();

protected:
  void on_MenuPopup_activate_Edit();
  void on_MenuPopup_activate_Add();
  void on_MenuPopup_activate_Delete();

private:


#ifndef GLOM_ENABLE_CLIENT_ONLY
  void on_MenuPopup_activate_layout();
#endif

  void on_treeview_column_clicked(int model_column_index);
  //void on_treeview_column_resized(int model_column_index, DbTreeViewColumnGlom* view_column);
  void on_idle_row_edit();
  void on_cell_button_clicked(const Gtk::TreeModel::Path& path);
  void on_cell_layout_button_clicked(const Gtk::TreeModel::Path& path, int model_column_index);

#ifdef GLOM_ENABLE_CLIENT_ONLY
  // Don't name it on_style_changed, otherwise we would override a virtual
  // function from Gtk::Widget. We could indeed do that, but we do it with
  // a normal signal handler, because we have to do it this way anyway in
  // case default signal handlers have been disabled in glibmm.
  void on_self_style_changed(const Glib::RefPtr<Gtk::Style>& style);
#endif //GLOM_ENABLE_CLIENT_ONLY

  bool get_prevent_user_signals() const;

  /** @param model_column_index A value returned from add_column().
   * @param view_column_index The index of the corresponding view column.
   */
  bool get_view_column_index(guint model_column_index, guint& view_column_index) const;

  guint get_count_hidden_system_columns() const;

  //The column_id is extra information that we can use later to discover what the column shows, even when columns have been reordered.
  guint treeview_append_column(const Glib::ustring& title, Gtk::CellRenderer& cellrenderer, int model_column_index, int data_model_column_index, bool expand);

  /** Show a model that gives a visual hint to the developer,
   * when he has not yet specified fields to show.
   * TODO: Make this more obvious in a less strange way.
   */
  void show_hint_model();

  guint get_fixed_cell_height();

  //TODO: Remove this and use AppGlom::get_appwindow() instead?
  AppWindow* get_appwindow();

  void refresh_cell_choices_data_from_database_with_foreign_key(guint model_index, const Gnome::Gda::Value& foreign_key_value);

  typedef Gtk::Box type_base;

  //Member widgets:
  Gtk::ScrolledWindow m_ScrolledWindow;
  Gtk::TreeView m_TreeView;

  Glib::RefPtr<DbTreeModel> m_refListStore;

  //Columns, not including the hidden internal columns:
  typedef LayoutGroup::type_list_items type_column_items;
  type_column_items m_column_items;
  FoundSet m_found_set; //table, where_clause, sort_clause.

  bool m_column_is_sorted; //If empty, then m_column_sorted and m_column_sorted_direction should not be used.
  bool m_column_sorted_direction; //true means ascending.
  guint m_column_sorted; //Previously-clicked (on the treeview header) column. Remember it so we can reverse the sort order on a second click.

protected:
  Glib::ustring m_open_button_title; //Allow us to change "Open" to "Select".

private:
  //TODO: Avoid repeating these in so many widgets:
  std::unique_ptr<Gtk::Menu> m_pMenuPopup;
  Glib::RefPtr<Gio::SimpleActionGroup> m_refActionGroup;
  Glib::RefPtr<Gio::SimpleAction> m_refContextEdit, m_refContextAdd, m_refContextDelete;

#ifndef GLOM_ENABLE_CLIENT_ONLY
  Glib::RefPtr<Gio::SimpleAction> m_refContextLayout;
#endif

  bool m_bAllowUserActions;

  bool m_bPreventUserSignals;
  bool m_bIgnoreTreeViewSignals;

  type_vec_strings m_vecColumnIDs; //We give each ViewColumn a special ID, so we know where they are after a reorder.

protected:
  bool m_allow_add;
  bool m_allow_delete;

private:

  void treeview_delete_all_columns();

  Gtk::Window* get_app_window();
  const Gtk::Window* get_app_window() const;

  bool m_find_mode;
  bool m_allow_only_one_related_record;

  //Used to revert the currently-edited cell.
  bool m_validation_retry;
  Glib::ustring m_validation_invalid_text_for_retry;

  /// The primary key for the table:
  std::shared_ptr<Field> m_key_field;

  bool m_allow_view;
  bool m_allow_view_details;

  Gtk::TreeViewColumn* m_treeviewcolumn_button;

  //Signals:
  type_signal_user_requested_edit m_signal_user_requested_edit;
  type_signal_script_button_clicked m_signal_script_button_clicked;
  type_signal_record_added m_signal_record_added;
  type_signal_sort_clause_changed m_signal_sort_clause_changed;
  type_signal_record_selection_changed m_signal_record_selection_changed;

#ifndef GLOM_ENABLE_CLIENT_ONLY
  type_signal_user_requested_layout m_signal_user_requested_layout;
#endif // !GLOM_ENABLE_CLIENT_ONLY

  //TODO: Do this properly:
  //type_signal_user_added m_signal_record_count_changed;

  bool get_ignore_treeview_signals() const;
  void set_ignore_treeview_signals(bool ignore = true);

  //An instance of InnerIgnore remembers the ignore settings,
  //then restores them when it goes out of scope and is destroyed.
  class InnerIgnore
  {
  public:
    explicit InnerIgnore(DbAddDel* pOuter);
    ~InnerIgnore();

  private:
    DbAddDel* m_pOuter;
    bool m_bPreventUserSignals, m_bIgnoreTreeViewSignals;
  };

  //When no columns have been chosen in the layout editor,
  //show this model to give the user a hint about what to do:
  class ModelColumnsEmptyHint : public Gtk::TreeModel::ColumnRecord
  {
  public:

    ModelColumnsEmptyHint()
    { add(m_col_hint); }

    Gtk::TreeModelColumn<Glib::ustring> m_col_hint;
  };

  ModelColumnsEmptyHint m_columns_hint;
  Glib::RefPtr<Gtk::ListStore> m_model_hint;

  guint m_fixed_cell_height;

  /// Discover the right-most text column, so we can make it expand.
  bool get_column_to_expand(guint& column_to_expand) const;

  //TODO_refactor: Give these better names, and document them:
  void user_changed(const Gtk::TreeModel::iterator& row, guint col);
  void user_requested_delete(const Gtk::TreeModel::iterator& rowStart, const Gtk::TreeModel::iterator&  /* rowEnd TODO */);

  void set_height_rows_actual(gulong rows_count);

  //TODO_refactor: Make some other methods private too.
  /** Get an iterator to the blank row in which the user should add data for the new row.
   * You can then add the row to your underlying data store when some data has been filled, by handling signal_user_changed.
   */
  Gtk::TreeModel::iterator get_item_placeholder(); //Return index of the placeholder row.

  gulong m_rows_count_min;
  gulong m_rows_count_max;
};

} //namespace Glom



#endif //GLOM_DB_ADDDEL_H
