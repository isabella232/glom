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

#include "cellrenderer_dblist.h"
#include <glom/mode_data/datawidget/cellcreation.h>
#include <glom/mode_data/datawidget/treemodel_db_withextratext.h>
#include <libglom/data_structure/glomconversions.h>
#include <iostream>


namespace Glom
{

CellRendererDbList::CellRendererDbList()
: m_repacked_first_cell(false),
  m_document(nullptr)
{
}

void CellRendererDbList::set_choices_fixed(const Formatting::type_list_values& list_values, bool restricted)
{
  ComboChoicesWithTreeModel::set_choices_fixed(list_values, restricted);

  auto model = get_choices_model();

  //Show model in the view:
  property_model() = model;
  property_text_column() = get_fixed_model_text_column(); //This must be a text column, in m_model.
  property_editable() = true; //It would be useless if we couldn't edit it.

  //The other cells are added in on_editing_started().
}

void CellRendererDbList::set_choices_related(const std::shared_ptr<const Document>& document, const std::shared_ptr<const LayoutItem_Field>& layout_field, const Gnome::Gda::Value& foreign_key_value)
{
  ComboChoicesWithTreeModel::set_choices_related(document, layout_field, foreign_key_value);

  auto model = get_choices_model();
  if(!model)
  {
    std::cerr << G_STRFUNC << ": model is null\n";
  }

  //Show model in the view:
  property_model() = model;

  auto model_db =
    Glib::RefPtr<DbTreeModelWithExtraText>::cast_dynamic(model);
  if(model_db)
    property_text_column() = model_db->get_text_column();
  else
  {
    std::cerr << G_STRFUNC << ": The model is not a DbTreeModelWithExtraText.\n";
    return;
  }

  property_editable() = true; //It would be useless if we couldn't edit it.

  //The other cells are added in on_editing_started(),
  //which uses the document.
  m_document = document;
}


void CellRendererDbList::set_restrict_values_to_list(bool val)
{
  property_has_entry() = !val;
}

void CellRendererDbList::repack_cells_fixed(Gtk::CellLayout* combobox)
{
  //We need an actual widget, to guess the fixed cell height.
  auto widget = dynamic_cast<Gtk::Widget*>(combobox);
  if(!widget)
  {
    std::cerr << G_STRFUNC << ": widget is null.\n";
    return;
  }

  if(!m_repacked_first_cell)
  {
    //Get the default column, created by set_text_column():
    auto cell = dynamic_cast<Gtk::CellRendererText*>(combobox->get_first_cell());
    if (cell)
    {
      //Unpack and repack it with expand=false instead of expand=true:
      //We don't expand the first column, so we can align the other columns.
      cell->reference();
      combobox->clear();
      combobox->pack_start(*cell, false);
      cell->unreference();

      //Make the renderer render the column:
      combobox->add_attribute(*cell, "text", get_fixed_model_text_column());

      cell->property_xalign() = 0.0f;

      m_repacked_first_cell = true; //Avoid doing this again.
    }
  }

  //Add extra cells:
  Glib::ListHandle<Gtk::CellRenderer*> cells = combobox->get_cells();
  if(cells.size() < m_vec_model_columns_value_fixed.size())
  {
    for(guint col = cells.size(); col != m_vec_model_columns_value_fixed.size(); ++col)
    {
      Gtk::CellRenderer* cell = nullptr;
      if(m_db_layout_items.empty())
        cell = Gtk::manage(new Gtk::CellRendererText);
      else if(col < m_db_layout_items.size())
      {
        auto layout_item = m_db_layout_items[col];
        cell = create_cell(layout_item, m_table_name, m_document, get_fixed_cell_height(*widget));
      }

      if(!cell)
        continue;

      //Use the renderer:
      combobox->pack_start(*cell, true);

      //Make the renderer render the column:
      combobox->add_attribute(*cell, "text", col);

      cell->property_xalign() = 0.0f;
    }
  }
}

void CellRendererDbList::repack_cells_related(Gtk::CellLayout* combobox)
{
  //We need an actual widget, to guess the fixed cell height.
  auto widget = dynamic_cast<Gtk::Widget*>(combobox);
  if(!widget)
  {
    std::cerr << G_STRFUNC << ": widget is null.\n";
    return;
  }

  const auto cells = combobox->get_cells();
  const auto initial_cells_count = cells.size();

  guint i = 0;
  for(const auto& layout_item : m_db_layout_items)
  {
    Gtk::CellRenderer* cell = nullptr;

    if(i == 0 && !m_repacked_first_cell)
    {
      //Get the default column, created by set_text_column():
      cell = combobox->get_first_cell();
      if(!cell)
      {
        //This is normal, for instance if the item is meant to be hidden.
        //std::cerr << G_STRFUNC << ": get_first_cell() returned null.\n";
      }
      else
      {
        //Unpack and repack it with expand=false instead of expand=true:
        //We don't expand the first column, so we can align the other columns.
        cell->reference();
        combobox->clear();
        combobox->pack_start(*cell, false);
        cell->unreference();
        cell_connect_cell_data_func(combobox, cell, i);

         m_repacked_first_cell = true;
      }
    }
    else if(i >= initial_cells_count)
    {
      //Create the cell:
      cell = create_cell(layout_item, m_table_name, m_document, get_fixed_cell_height(*widget));
      if(!cell)
      {
        std::cerr << G_STRFUNC << ": create_cell() returned 0.\n";
      }
      else
      {
        combobox->pack_start(*cell, true);

        cell_connect_cell_data_func(combobox, cell, i);
      }
    }

    ++i;
  }
}

void CellRendererDbList::on_editing_started(Gtk::CellEditable* cell_editable, const Glib::ustring& path)
{
  //This can happen if no text-column has been set yet,
  //though that shouldn't really happen.
  if(!cell_editable)
  {
    std::cerr << G_STRFUNC << ": cell_editable was null\n";
    return;
  }

  auto combobox = dynamic_cast<Gtk::CellLayout*>(cell_editable);
  if(!combobox)
    return;

  //The DB model has a special virtual text column,
  //and the simple model just has text in all columns:
  auto model_db =
    Glib::RefPtr<DbTreeModelWithExtraText>::cast_dynamic(get_choices_model());
  if(model_db)
    repack_cells_related(combobox);
  else
    repack_cells_fixed(combobox);

  Gtk::CellRenderer::on_editing_started(cell_editable, path);
}

void CellRendererDbList::set_value(const Gnome::Gda::Value& value)
{
  auto layout_item = std::dynamic_pointer_cast<const LayoutItem_Field>(get_layout_item());
  if(!layout_item)
    return;

  set_text(Conversions::get_text_for_gda_value(layout_item->get_glom_type(), value, layout_item->get_formatting_used().m_numeric_format));

  //Show a different color if the value is numeric, if that's specified:
  /* TODO:
  if(layout_item->get_glom_type() == Field::glom_field_type::NUMERIC)
  {
    std::vector<Gtk::CellRenderer*> cells = get_cells();
    if(cells.empty())
      return;

    auto cell = dynamic_cast<Gtk::CellRendererText*>(cells[0]);
    if(!cell)
      return;

    const Glib::ustring fg_color =
      layout_item->get_formatting_used().get_text_format_color_foreground_to_use(value);
    if(fg_color.empty())
    {
      //GtkComboBox doesn't interpret "" as an unset. TODO: Fix that?
      cell->property_foreground_set() = false;
    }
    else
      cell->property_foreground() = fg_color;
  }
  */
}

Gnome::Gda::Value CellRendererDbList::get_value() const
{
  auto layout_item = std::dynamic_pointer_cast<const LayoutItem_Field>(get_layout_item());
  bool success = false;

  const auto text = get_text();
  return Conversions::parse_value(layout_item->get_glom_type(), text, layout_item->get_formatting_used().m_numeric_format, success);
}

void CellRendererDbList::set_text(const Glib::ustring& text)
{
  property_text() = text;
}

Glib::ustring CellRendererDbList::get_text() const
{
  return property_text();
}



} //namespace Glom
