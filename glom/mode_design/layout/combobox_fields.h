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

#ifndef GLOM_MODE_DESIGN_COMBOBOX_FIELDS_H
#define GLOM_MODE_DESIGN_COMBOBOX_FIELDS_H

#include <gtkmm/combobox.h>
#include <gtkmm/builder.h>
#include <libglom/data_structure/field.h>
#include <libglom/document/document.h>
#include <libglom/sharedptr.h>

#include <gtkmm/treestore.h>

namespace Glom
{

class ComboBox_Fields : public Gtk::ComboBox
{
public:
  ComboBox_Fields(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

  typedef std::vector< std::shared_ptr<Field> > type_vec_fields;
    
  /** Fill the combo box with fields.
   * @param fields The fields to show in the combo box.
   * @param with_none_type Whether to show an extra None item.
   */
  void set_fields(const type_vec_fields& fields, bool with_none_item = false);

  /** Fill the combo box with fields.
   * @param document The Document, used to get the list of fields.
   * @param parent_table_name The table whose fields should be shown.
   * @param field_type Show only fields of this type.
   */
  void set_fields(const std::shared_ptr<Document>& document, const Glib::ustring parent_table_name);
    
  /** Fill the combo box with fields, but only fields of a certain type.
   * @param document The Document, used to get the list of fields.
   * @param parent_table_name The table whose fields should be shown.
   * @param field_type Show only fields of this type.
   */
  void set_fields(const std::shared_ptr<Document>& document, const Glib::ustring parent_table_name, Field::glom_field_type field_type);

  void set_selected_field(const std::shared_ptr<const Field>& field);
  void set_selected_field(const Glib::ustring& field_name);
 
  std::shared_ptr<Field> get_selected_field() const;
  Glib::ustring get_selected_field_name() const;


private:

  //void on_cell_data_name(const Gtk::TreeModel::const_iterator& iter);
  void on_cell_data_title(const Gtk::TreeModel::const_iterator& iter);
  bool on_row_separator(const Glib::RefPtr<Gtk::TreeModel>& model, const Gtk::TreeModel::const_iterator& iter);

 
  //Tree model columns:
  //These columns are used by the model that is created by the default constructor
  class ModelColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:
    ModelColumns()
    { add(m_field); add(m_separator); }

    Gtk::TreeModelColumn< std::shared_ptr<Field> > m_field;
    Gtk::TreeModelColumn<bool> m_separator;
  };

  ModelColumns m_model_columns;
  Glib::RefPtr<Gtk::TreeStore> m_model;

  //Gtk::CellRendererText* m_renderer_name;
  Gtk::CellRendererText* m_renderer_title;
};

} //namespace Glom

#endif // GLOM_MODE_DESIGN_COMBOBOX_FIELDS_H
