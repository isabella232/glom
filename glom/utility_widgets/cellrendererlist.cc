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

#include "cellrendererlist.h"


namespace Glom
{

CellRendererList::CellRendererList()
:  Glib::ObjectBase(nullptr) //Mark this class as gtkmmproc-generated, rather than a custom class, to allow vfunc optimisations.
   //TODO: This should not be necessary - our gtkmm callbacks are somehow preventing the popup from appearing.
{
  m_refModel = Gtk::ListStore::create(m_model_columns);
  property_model() = m_refModel;
  property_text_column() = 0; //This must be a text column, in m_refModel.
  property_editable() = true; //It would be useless if we couldn't edit it.
}

CellRendererList::~CellRendererList()
{
}

void CellRendererList::remove_all_list_items()
{
  if(m_refModel)
    m_refModel->clear();
}

void CellRendererList::append_list_item(const Glib::ustring& text)
{
  Gtk::TreeModel::Row row = *(m_refModel->append());
  row[m_model_columns.m_col_choice] = text;
}

void CellRendererList::set_restrict_values_to_list(bool val)
{
  property_has_entry() = static_cast<gboolean>(!val);
}


} //namespace Glom
