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

#include <glom/mode_design/layout/combobox_fields.h>
#include <glom/appwindow.h>
#include <glibmm/i18n.h>

#include <iostream>

namespace Glom
{

ComboBox_Fields::ComboBox_Fields(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& /* builder */)
: Gtk::ComboBox(cobject),
  m_renderer_title(0)
{
  m_model = Gtk::TreeStore::create(m_model_columns);

  set_model(m_model);

  //Add name column:
  //m_renderer_name = Gtk::manage(new Gtk::CellRendererText());
  //pack_start(*m_renderer_name);
  //set_cell_data_func(*m_renderer_name, sigc::mem_fun(*this, &ComboBox_Fields::on_cell_data_name));

  //Add title column:
  m_renderer_title = Gtk::manage(new Gtk::CellRendererText());
  pack_start(*m_renderer_title);
  set_cell_data_func(*m_renderer_title, sigc::mem_fun(*this, &ComboBox_Fields::on_cell_data_title));

  set_row_separator_func(sigc::mem_fun(*this, &ComboBox_Fields::on_row_separator));
}

ComboBox_Fields::~ComboBox_Fields()
{

}

std::shared_ptr<Field> ComboBox_Fields::get_selected_field() const
{
  Gtk::TreeModel::iterator iter = get_active();
  if(iter)
  {
    Gtk::TreeModel::Row row = *iter;
    return row[m_model_columns.m_field];
  }
  else
    return std::shared_ptr<Field>();
}

Glib::ustring ComboBox_Fields::get_selected_field_name() const
{
  std::shared_ptr<Field> field = get_selected_field();
  return glom_get_sharedptr_name(field);
}

void ComboBox_Fields::set_selected_field(const std::shared_ptr<const Field>& field)
{
  if(field)
    set_selected_field(field->get_name());
  else
    set_selected_field(Glib::ustring());
}

void ComboBox_Fields::set_selected_field(const Glib::ustring& field_name)
{
  //Look for the row with this text, and activate it:
  Glib::RefPtr<Gtk::TreeModel> model = get_model();
  if(model)
  {
    for(const auto& row : model->children())
    {
      std::shared_ptr<Field> field = row[m_model_columns.m_field];
      const auto this_name = glom_get_sharedptr_name(field);

      //(An empty name means Select the parent table item.)
      if(this_name == field_name)
      {
        set_active(row);
        return; //success
      }
    }
  }

  //Not found, so mark it as blank:
  //std::cerr << G_STRFUNC << ": field not found in list: " << field_name << std::endl;

  //Avoid calling unset_active() if nothing is selected, because it triggers the changed signal unnecessarily.
  if(get_active()) //If something is active (selected).
    unset_active();
}

void ComboBox_Fields::set_fields(Document* document, const Glib::ustring parent_table_name)
{
  if(!document)
    return;

  const auto fields = document->get_table_fields(parent_table_name);

  if(!m_model)
    return;
  
  m_model->clear();

  //Fill the model:
  for(const auto& field : fields)
  {
    Gtk::TreeModel::iterator tree_iter = m_model->append();
    Gtk::TreeModel::Row row = *tree_iter;

    std::shared_ptr<Field> rel = field;
    row[m_model_columns.m_field] = rel;
    row[m_model_columns.m_separator] = false;
  }
}

void ComboBox_Fields::set_fields(Document* document, const Glib::ustring parent_table_name, Field::glom_field_type field_type)
{
  if(!document)
    return;

  const auto fields = document->get_table_fields(parent_table_name);

  if(!m_model)
    return;
  
  m_model->clear();

  //Fill the model:
  for(const auto& rel : fields)
  {
    if(rel && (rel->get_glom_type() == field_type))
    {
      std::cout << "DEBUG: ComboBox_Fields::set_fields() 1" << std::endl;
      Gtk::TreeModel::iterator tree_iter = m_model->append();
      std::cout << "DEBUG: ComboBox_Fields::set_fields() 2" << std::endl;
      Gtk::TreeModel::Row row = *tree_iter;

      row[m_model_columns.m_field] = rel;
      row[m_model_columns.m_separator] = false;
    }
  }
}

void ComboBox_Fields::set_fields(const type_vec_fields& fields, bool with_none_item)
{
  if(!m_model)
    return;
  
  m_model->clear();

  if(with_none_item)
  {
    //Add a special "None" item, so the user can clear the GtkComboBox:
    Gtk::TreeModel::iterator tree_iter = m_model->append();
    Gtk::TreeModel::Row row = *tree_iter;

    row[m_model_columns.m_field] = std::shared_ptr<Field>(); 
    row[m_model_columns.m_separator] = false;

    //Add a separator after the "None" item:
    tree_iter = m_model->append();
    row = *tree_iter;

    row[m_model_columns.m_field] = std::shared_ptr<Field>(); 
    row[m_model_columns.m_separator] = true;
  }

  //Fill the model:
  for(const auto& field : fields)
  {
    Gtk::TreeModel::iterator tree_iter = m_model->append();
    Gtk::TreeModel::Row row = *tree_iter;

    row[m_model_columns.m_field] = field;
    row[m_model_columns.m_separator] = false;
  }
}

void ComboBox_Fields::on_cell_data_title(const Gtk::TreeModel::const_iterator& iter)
{
  Gtk::TreeModel::Row row = *iter;
  std::shared_ptr<Field> field = row[m_model_columns.m_field];
  if(field)
  {
    m_renderer_title->set_property("text", item_get_title_or_name(field));
  }
  else
  {
    // A special "None" item, allowing the user to do the equivalent of clearing the combobox,
    // which is not normally possible with the GtkComboBox UI:

    //set_property() does not work with a const gchar*, so we explicitly create a ustring.
    //otherwise we get this warning:
    //" unable to set property `text' of type `gchararray' from value of type `glibmm__CustomPointer_Pc' "
    //TODO: Add a template specialization to Glib::ObjectBase::set_property() to allow this?
    m_renderer_title->set_property("text", Glib::ustring(_("(None)")));
  }
}

bool ComboBox_Fields::on_row_separator(const Glib::RefPtr<Gtk::TreeModel>& /* model */, const Gtk::TreeModel::const_iterator& iter)
{
  Gtk::TreeModel::Row row = *iter;
  const bool separator = row[m_model_columns.m_separator];
  return separator;
}


} //namespace Glom




