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
 
#include "cellrendererlist.h"
#include <gtkmm.h>


namespace Glom
{

CellRendererList::CellRendererList()
:  Glib::ObjectBase(0), //Mark this class as gtkmmproc-generated, rather than a custom class, to allow vfunc optimisations.
   //TODO: This should not be necessary - our gtkmm callbacks are somehow preventing the popup from appearing.
  m_use_second(false)
{
  m_refModel = Gtk::ListStore::create(m_model_columns);
  set_property("model", m_refModel);
  set_property("text-column", 0); //This must be a text column, in m_refModel.
  set_property("editable", true); //It would be useless if we couldn't edit it.
}

CellRendererList::~CellRendererList()
{
}

void CellRendererList::remove_all_list_items()
{
  if(m_refModel)
    m_refModel->clear();
}

void CellRendererList::append_list_item(const Glib::ustring& text, const Glib::ustring& extra)
{
  Gtk::TreeModel::Row row = *(m_refModel->append());
  row[m_model_columns.m_col_choice] = text;
  row[m_model_columns.m_col_extra] = extra;
}

void CellRendererList::set_restrict_values_to_list(bool val)
{
  set_property("has-entry", static_cast<gboolean>(!val));
}

void CellRendererList::on_editing_started(Gtk::CellEditable* cell_editable, const Glib::ustring& path)
{
  g_assert(cell_editable);

  if(m_use_second)
  {
    if(cell_editable)
    {
      //This is actually ComboBox, because GtkComboBox inherits from GtkCellEditable since GTK+ 2.6.
      //But Gtk::ComboBox does not inherit from Gtk::CellEditable because we could not break ABI.
      //So we will use the C API to get the Gtk::ComboBox:
      GtkComboBox* pCComboBox = GTK_COMBO_BOX(cell_editable->gobj());
      Gtk::ComboBox* pComboBox = Glib::wrap(pCComboBox);

      pComboBox->pack_start(m_model_columns.m_col_extra);
    }
    else
    {
      g_warning("CellRendererList::on_editing_started() cell_editable is null");
    }
  }
  
  Gtk::CellRenderer::on_editing_started(cell_editable, path);
}

void CellRendererList::set_use_second(bool use_second)
{
  m_use_second = use_second;
}

} //namespace Glom
