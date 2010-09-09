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

#include "cellrenderer_dblist.h"
#include <gtkmm.h>
#include <libglom/data_structure/glomconversions.h>


namespace Glom
{

CellRendererDbList::CellRendererDbList()
:  Glib::ObjectBase(0) //Mark this class as gtkmmproc-generated, rather than a custom class, to allow vfunc optimisations.
   //TODO: This should not be necessary - our gtkmm callbacks are somehow preventing the popup from
{
}

CellRendererDbList::~CellRendererDbList()
{
}


void CellRendererDbList::create_model(guint columns_count)
{
  //Create the model itself:
  DataWidgetChildren::ComboChoicesWithTreeModel::create_model(columns_count);

  //Show model in the view:
  property_model() = m_refModel;
  property_text_column() = 0; //This must be a text column, in m_refModel.
  property_editable() = true; //It would be useless if we couldn't edit it.

  //The other cells are added in on_editing_started().
}

void CellRendererDbList::set_restrict_values_to_list(bool val)
{
  property_has_entry() = !val;
}

void CellRendererDbList::on_editing_started(Gtk::CellEditable* cell_editable, const Glib::ustring& path)
{
  g_assert(cell_editable);

  Gtk::CellLayout* combobox = dynamic_cast<Gtk::CellLayout*>(cell_editable);
  if(!combobox)
    return;

  Glib::ListHandle<Gtk::CellRenderer*> cells = combobox->get_cells();
  if(cells.size() < m_vec_model_columns.size())
  {
    for(guint col = cells.size(); col != m_vec_model_columns.size(); ++col)
    {
      Gtk::CellRendererText* cell = 0;
      if(col == 0)
      {
        //Get the default column, created by set_text_column()?
        cell = dynamic_cast<Gtk::CellRendererText*>(combobox->get_first_cell());
      }

      if(!cell)
      {
        //Create the cell:
        cell = Gtk::manage(new Gtk::CellRendererText);

        //Use the renderer:
        //We don't expand the first column, so we can align the other columns.
        //Otherwise the other columns appear center-aligned.
        //This bug is relevant: https://bugzilla.gnome.org/show_bug.cgi?id=629133
        if(col == 0) //Impossible anyway, because we use the text-column property.
          combobox->pack_start(*cell, false); //Unfortunately gtk_combo_box_entry_set_text_column() has already used true, making our xalign=0.0 useless.
        else
          combobox->pack_start(*cell, true);
      }

      //Make the renderer render the column:
      combobox->add_attribute(*cell, "text", col);
    }
  }

  Gtk::CellRenderer::on_editing_started(cell_editable, path);
}

void CellRendererDbList::set_value(const Gnome::Gda::Value& value)
{
  sharedptr<const LayoutItem_Field> layout_item = sharedptr<const LayoutItem_Field>::cast_dynamic(get_layout_item());
  if(!layout_item)
    return;

  set_text(Conversions::get_text_for_gda_value(layout_item->get_glom_type(), value, layout_item->get_formatting_used().m_numeric_format));

  //Show a different color if the value is numeric, if that's specified:
  /* TODO:
  if(layout_item->get_glom_type() == Field::TYPE_NUMERIC)
  {
    std::vector<Gtk::CellRenderer*> cells = get_cells();
    if(cells.empty())
      return;

    Gtk::CellRendererText* cell = dynamic_cast<Gtk::CellRendererText*>(cells[0]);
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
  sharedptr<const LayoutItem_Field> layout_item = sharedptr<const LayoutItem_Field>::cast_dynamic(get_layout_item());
  bool success = false;

  const Glib::ustring text = get_text();
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

void CellRendererDbList::set_choices_with_second(const type_list_values_with_second& list_values)
{
  DataWidgetChildren::ComboChoicesWithTreeModel::set_choices_with_second(list_values);
}



} //namespace Glom
