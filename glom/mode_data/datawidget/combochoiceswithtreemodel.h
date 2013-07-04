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

#ifndef GLOM_MODE_DATA_COMBO_CHOICES_WITH_TREE_MODEL_H
#define GLOM_MODE_DATA_COMBO_CHOICES_WITH_TREE_MODEL_H

#include <glom/mode_data/datawidget/combochoices.h>
#include <gtkmm/celllayout.h>
#include <gtkmm/treemodel.h>

namespace Glom
{

namespace DataWidgetChildren
{

class ComboChoicesWithTreeModel : public ComboChoices
{
public:
  ///You must call set_layout_item() to specify the field type and formatting of the main column.
  explicit ComboChoicesWithTreeModel();

  virtual ~ComboChoicesWithTreeModel();

  //This creates a simple ListStore, with a text cell renderer.
  virtual void set_choices_fixed(const Formatting::type_list_values& list_values, bool restricted = false);

  //This creates a db-based tree model, with appropriate cell renderers:
  virtual void set_choices_related(const Document* document, const std::shared_ptr<const LayoutItem_Field>& layout_field, const Gnome::Gda::Value& foreign_key_value);


  //Not named get_model(), to avoid clashing with ComboBox::get_model().
  Glib::RefPtr<Gtk::TreeModel> get_choices_model();

protected:
  void init();
  void create_model_non_db(guint columns_count);

  /** Get a suitable fixed height for cells, so we can display them more efficiently.
   * This caches the result to avoid repeated recalculation.
   */
  int get_fixed_cell_height(Gtk::Widget& widget);
  

  typedef Gtk::TreeModelColumn<Glib::ustring> type_model_column_string_fixed;
  typedef std::vector< type_model_column_string_fixed* > type_vec_model_columns_string_fixed;
  type_vec_model_columns_string_fixed m_vec_model_columns_string_fixed; //If set_choices_fixed() was used.

  typedef Gtk::TreeModelColumn<Gnome::Gda::Value> type_model_column_value_fixed;
  typedef std::vector< type_model_column_value_fixed* > type_vec_model_columns_value_fixed;
  type_vec_model_columns_value_fixed m_vec_model_columns_value_fixed; //If set_choices_fixed() was used.

  /** Get the index of the extra column, at the end, that is just a 
   * text representation of the first column, for use by GtkCombo with has-entry=true,
   * which accepts only a text column.
   */
  int get_fixed_model_text_column() const;


  typedef std::vector< std::shared_ptr<const LayoutItem_Field> > type_vec_const_layout_items;
  type_vec_const_layout_items m_db_layout_items; //If set_choices_related() was used.

  //This avoids us making on_cell_data() public just so that derived classes can use it,
  //though that shouldn't be necessary anyway.
  void cell_connect_cell_data_func(Gtk::CellLayout* celllayout, Gtk::CellRenderer* cell, guint model_column_index);

  /** Display the value in the cell according to the layout field's type and formatting.
   */
  void set_cell_for_field_value(Gtk::CellRenderer* cell, const std::shared_ptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value);

private:
  /// Render the model data to the cells in the view.
  void on_cell_data(const Gtk::TreeModel::iterator& iter, Gtk::CellRenderer* cell, guint model_column_index);

  Glib::RefPtr<Gtk::TreeModel> m_refModel;

  void delete_model();
  
  int m_fixed_cell_height;
};

} //namespace DataWidetChildren
} //namespace Glom

#endif // GLOM_MODE_DATA_COMBO_CHOICES_WITH_TREE_MODEL_H
