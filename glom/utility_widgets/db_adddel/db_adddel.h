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

#include <gtkmm.h>
#include <glom/libglom/data_structure/layout/layoutitem_field.h>
#include <libgdamm.h>
#include <glom/utility_widgets/db_adddel/treemodel_with_addrow.h>
#include <glom/libglom/document/document_glom.h>
#include <glom/base_db_table_data.h>

#include <vector>
#include <map>

#include "config.h" // For GLOM_ENABLE_CLIENT_ONLY

namespace Glom
{

class App_Glom;

class DbAddDelColumnInfo
{
public:
  DbAddDelColumnInfo();
  DbAddDelColumnInfo(const DbAddDelColumnInfo& src);
  DbAddDelColumnInfo& operator=(const DbAddDelColumnInfo& src);

  sharedptr<LayoutItem> m_item;

  //For fields with choices:
  typedef std::vector<Glib::ustring> type_vecStrings;
  type_vecStrings m_choices;

  bool m_editable;
  bool m_visible;
};

class DbTreeViewColumnGlom;

/** For adding/deleting/selecting record rows.
 */
class DbAddDel
 : public Gtk::VBox,
   public Base_DB_Table_Data
{
public:
  friend class InnerIgnore; //declared below.

  DbAddDel();
  virtual ~DbAddDel();

  virtual void set_allow_user_actions(bool bVal = true);
  virtual bool get_allow_user_actions() const;

  virtual void set_allow_add(bool val = true);
  virtual void set_allow_delete(bool val = true);
    
  /** Prevent any attempts by this class to change actual records,
   * if the widget is just being used to enter find critera.
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

  Gnome::Gda::Value get_value(const Gtk::TreeModel::iterator& iter, const sharedptr<const LayoutItem_Field>& layout_item) const;

  /** Get the row's hidden key
   */
  Gnome::Gda::Value get_value_key(const Gtk::TreeModel::iterator& iter) const;

  /** Set the row's hidden key
   */
  void set_value_key(const Gtk::TreeModel::iterator& iter, const Gnome::Gda::Value& value);

  /** @param col A value returned from add_column().
   * @result The value on the selected row.
   */
  Gnome::Gda::Value get_value_selected(const sharedptr<const LayoutItem_Field>& layout_item) const;
  Gnome::Gda::Value get_value_key_selected() const;

  Gtk::TreeModel::iterator get_item_selected();
  Gtk::TreeModel::iterator get_item_selected() const; //There is no TreeModel::const_iterator

  /** 
   * @param iter The row to be selected. 
   * @param column A value returned from add_column().
   * @param start_editing Whether editing should start in the cell.
   * @result Whether the row was successfully selected.
   */
  bool select_item(const Gtk::TreeModel::iterator& iter, const sharedptr<const LayoutItem>& layout_item, bool start_editing = false);  //bool indicates success.
  bool select_item(const Gtk::TreeModel::iterator& iter, bool start_editing = false);

  guint get_count() const;

  /** 
   * @param iter The row to be changed. 
   * @param layout_item Describes the column(s) whose values should be changed.
   * @param value The new value.
   */
  virtual void set_value(const Gtk::TreeModel::iterator& iter, const sharedptr<const LayoutItem_Field>& layout_item, const Gnome::Gda::Value& value);

  /** 
   * @param col A value returned from add_column().
   * @param value The new value.
   */
  virtual void set_value_selected(const sharedptr<const LayoutItem_Field>& layout_item, const Gnome::Gda::Value& value);

  bool get_is_first_row(const Gtk::TreeModel::iterator& iter) const;
  bool get_is_last_row(const Gtk::TreeModel::iterator& iter) const;

  /** @result Whether this is a blank row where date for a new row should be entered
   */
  bool get_is_placeholder_row(const Gtk::TreeModel::iterator& iter) const;

  sharedptr<Field> get_key_field() const;
  void set_key_field(const sharedptr<Field>& field);

  void set_table_name(const Glib::ustring& table_name);

  ///For instance, if the user cannot view the table then don't try to get the records.
  void set_allow_view(bool val = true);

  ///Whether each row should have a button, to request edit.
  virtual void set_allow_view_details(bool val = true);
  bool get_allow_view_details() const;


  /** @result The index of the new column.
   */
  guint add_column(const sharedptr<LayoutItem>& layout_item);

  /// Specify which records to show:
  void set_found_set(const FoundSet& found_set);

  FoundSet get_found_set() const;

  /// Start using the added columns.
  void set_columns_ready();

  guint get_columns_count() const;

  sharedptr<const LayoutItem_Field> get_column_field(guint column_index) const;

  typedef DbAddDelColumnInfo::type_vecStrings type_vecStrings;

  /** Retrieves the column order, even after they have been reordered by the user.
   * @result a vector of column_id. These column_ids were provided in the call to add_column().
   */
  type_vecStrings get_columns_order() const;

  void remove_all_columns();
  //virtual void set_columns_count(guint count);
  //virtual void set_column_title(guint col, const Glib::ustring& strText);
  void set_column_width(guint col, guint width);

  /// For popup cells.
  void set_column_choices(guint col, const type_vecStrings& vecStrings);

  void construct_specified_columns(); //Delay actual use of set_column_*() stuff until this method is called.

  bool refresh_from_database();
  bool refresh_from_database_blank();

  void set_show_column_titles(bool bVal = true);

  Gtk::TreeModel::iterator get_row(const Gnome::Gda::Value& key);

  void finish_editing(); //Closes active edit controls and commits the data to the cell.
  //virtual void reactivate(); //Sheet doesn't seem to update unless a cell is active.
  void set_prevent_user_signals(bool bVal = true);
    
  //TODO_refactor: make private.

  void user_added(const Gtk::TreeModel::iterator& row);

  Glib::RefPtr<Gtk::TreeModel> get_model();
  Glib::RefPtr<const Gtk::TreeModel> get_model() const;

  void set_rules_hint(bool val = true);

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
  typedef sigc::signal<void, const sharedptr<const LayoutItem_Button>&, const Gtk::TreeModel::iterator&> type_signal_script_button_clicked;
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

 
  virtual Gtk::TreeModel::iterator get_last_row();
  virtual Gtk::TreeModel::iterator get_last_row() const;

  virtual void set_open_button_title(const Glib::ustring& title);

protected:
  
  
  //Overrides of Base_DB/Base_DB_Table methods:
  virtual void set_entered_field_data(const sharedptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value);
  virtual void set_entered_field_data(const Gtk::TreeModel::iterator& row, const sharedptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value);
  virtual Gnome::Gda::Value get_entered_field_data(const sharedptr<const LayoutItem_Field>& field) const;
  virtual Gtk::TreeModel::iterator get_row_selected();
 
  //Implementations of pure virtual methods from Base_DB_Table_Data:
  virtual sharedptr<Field> get_field_primary_key() const;
  virtual Gnome::Gda::Value get_primary_key_value_selected() const;
  virtual void set_primary_key_value(const Gtk::TreeModel::iterator& row, const Gnome::Gda::Value& value);
  virtual Gnome::Gda::Value get_primary_key_value(const Gtk::TreeModel::iterator& row) const;
      
  Gtk::CellRenderer* construct_specified_columns_cellrenderer(const sharedptr<LayoutItem>& layout_item, int model_column_index, int data_model_column_index);

  bool get_model_column_index(guint view_column_index, guint& model_column_index);


  typedef std::list<guint> type_list_indexes;
  ///Return the column indexes of any columns that display this field.
  virtual type_list_indexes get_column_index(const sharedptr<const LayoutItem>& layout_item) const;

  ///Return the query column index of any columns that display this field:
  type_list_indexes get_data_model_column_index(const sharedptr<const LayoutItem_Field>& layout_item_field) const;

  virtual void setup_menu();
  virtual Gnome::Gda::Value treeview_get_key(const Gtk::TreeModel::iterator& row) const;

  ///Add a blank row, or return the existing blank row if there already is one.
  //virtual Gtk::TreeModel::iterator get_next_available_row_with_add_if_necessary();

  //Signal handlers:
  void treeviewcolumn_on_cell_data(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter, int model_column_index, int data_model_column_index);

  //TODO: Remove virtuals after checking that there are no method overrides:
  virtual void on_treeview_cell_edited(const Glib::ustring& path_string, const Glib::ustring& new_text, int model_column_index, int data_model_column_index);
  virtual void on_treeview_cell_edited_bool(const Glib::ustring& path_string, int model_column_index, int data_model_column_index);

  virtual bool on_treeview_column_drop(Gtk::TreeView* treeview, Gtk::TreeViewColumn* column, Gtk::TreeViewColumn* prev_column, Gtk::TreeViewColumn* next_column);
  virtual void on_treeview_columns_changed();

  virtual bool on_button_press_event_Popup(GdkEventButton* event);

  void on_MenuPopup_activate_Edit();
  void on_MenuPopup_activate_Add();
  void on_MenuPopup_activate_Delete();

#ifndef GLOM_ENABLE_CLIENT_ONLY
  void on_MenuPopup_activate_layout();
#endif

  virtual void on_treeview_button_press_event(GdkEventButton* event);

  virtual bool on_treeview_columnheader_button_press_event(GdkEventButton* event);
  virtual void on_treeview_column_clicked(int model_column_index);
  void on_treeview_column_resized(int model_column_index, DbTreeViewColumnGlom* view_column);
  virtual void on_cell_button_clicked(const Gtk::TreeModel::Path& path);
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
  guint treeview_append_column(const Glib::ustring& title, Gtk::CellRenderer& cellrenderer, int model_column_index, int data_model_column_index);

  /** Show a model that gives a visual hint to the developer,
   * when he has not yet specified fields to show.
   * TODO: Make this more obvious in a less strange way.
   */
  void show_hint_model();

  int get_fixed_cell_height();

  //TODO: Remove this and use AppGlom::get_application() instead?
  App_Glom* get_application();

  static void apply_formatting(Gtk::CellRenderer* renderer, const FieldFormatting& formatting);

  typedef Gtk::VBox type_base;

  //Member widgets:
  Gtk::ScrolledWindow m_ScrolledWindow;

  Gtk::TreeView m_TreeView;
  Gtk::TreeModel::ColumnRecord m_ColumnRecord;

  //Either a db-backed model, or a liststore-derived find-criteria model:
  TreeModelWithAddRow* m_refListStore; //We use this without a Glib::RefPtr to avoid the complication of ObjectBase as an MI base.
  Glib::RefPtr<Gtk::TreeModel> m_refListStore_as_model; //To avoid repeated dynamic_casts.

  //Columns, not including the hidden internal columns:
  typedef std::vector<DbAddDelColumnInfo> type_ColumnTypes;
  type_ColumnTypes m_ColumnTypes;
  FoundSet m_found_set; //table, where_clause, sort_clause.

  bool m_column_is_sorted; //If empty, then m_column_sorted and m_column_sorted_direction should not be used.
  bool m_column_sorted_direction; //true means ascending. 
  guint m_column_sorted; //Previously-clicked (on the treeview header) column. Remember it so we can reverse the sort order on a second click.

  Glib::ustring m_open_button_title; //Allow us to change "Open" to "Select".

  //TODO: Avoid repeating these in so many widgets:
  Gtk::Menu* m_pMenuPopup;
  Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;
  Glib::RefPtr<Gtk::UIManager> m_refUIManager;
  Glib::RefPtr<Gtk::Action> m_refContextEdit, m_refContextAdd, m_refContextDelete;

#ifndef GLOM_ENABLE_CLIENT_ONLY
  Glib::RefPtr<Gtk::Action> m_refContextLayout;
#endif

  bool m_bAllowUserActions;

  bool m_bPreventUserSignals;
  bool m_bIgnoreTreeViewSignals;

  type_vecStrings m_vecColumnIDs; //We give each ViewColumn a special ID, so we know where they are after a reorder.

  bool m_allow_add;
  bool m_allow_delete;
    
  bool m_find_mode;
  bool m_allow_only_one_related_record;

  /// The primary key for the table:
  sharedptr<Field> m_key_field;

  bool m_columns_ready;
  bool m_allow_view;
  bool m_allow_view_details;
  Gtk::TreeViewColumn* m_treeviewcolumn_button;

  //Signals:
  type_signal_user_requested_edit m_signal_user_requested_edit;
  type_signal_script_button_clicked m_signal_script_button_clicked;
  type_signal_record_added m_signal_record_added;
  type_signal_sort_clause_changed m_signal_sort_clause_changed;

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
    InnerIgnore(DbAddDel* pOuter);
    ~InnerIgnore();

  protected:
    DbAddDel* m_pOuter;
    bool m_bPreventUserSignals, m_bIgnoreTreeViewSignals;
  };

/*
  class DynamicColumnRecord : public Gtk::TreeModel::ColumnRecord
  {
    typedef std::vector<Gtk::TreeModelColumnBase> type_vecColumns;
    type_vecColumns m_vecColumns;
  };
*/

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

  int m_fixed_cell_height;
    
    
private:
  
  //TODO_refactor: Give these better names, and document them:
  bool start_new_record();
  void user_changed(const Gtk::TreeModel::iterator& row, guint col);
  void user_requested_delete(const Gtk::TreeModel::iterator& rowStart, const Gtk::TreeModel::iterator&  /* rowEnd TODO */);

  //TODO_refactor: Make some other methods private too.
  /** Get an iterator to the blank row in which the user should add data for the new row.
   * You can then add the row to your underlying data store when some data has been filled, by handling signal_user_changed.
   */
  Gtk::TreeModel::iterator get_item_placeholder(); //Return index of the placeholder row.
};

} //namespace Glom



#endif //GLOM_DB_ADDDEL_H
