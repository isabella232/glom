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

#ifndef GLOM_UTILITY_WIDGETS_COMBO_CHOICES_WITH_TREE_MODEL_H
#define GLOM_UTILITY_WIDGETS_COMBO_CHOICES_WITH_TREE_MODEL_H

#include <glom/mode_data/datawidget/combochoices.h>

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

  virtual void set_choices(const FieldFormatting::type_list_values& list_values);

  //Not named get_model(), to avoid clashing with ComboBox::get_model().
  Glib::RefPtr<Gtk::TreeModel> get_choices_model();
  
protected:
  void init();
  void create_model(guint columns_count);
  
  /** Derived classes should implement this to present the model in their view,
   * for instance by adding Gtk::CellRenderers.
   */
  virtual void use_model() = 0;

  virtual void set_choices_with_second(const type_list_values_with_second& list_values);

  typedef Gtk::TreeModelColumn<Glib::ustring> type_model_column;
  typedef std::vector< type_model_column* > type_vec_model_columns;
  type_vec_model_columns m_vec_model_columns;

private:
  Glib::RefPtr<Gtk::TreeModel> m_refModel;
  
  void delete_model();
};

} //namespace DataWidetChildren
} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_COMBO_CHOICES_WITH_TREE_MODEL_H
