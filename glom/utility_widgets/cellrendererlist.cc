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
#include <gtk/gtkcombobox.h>


namespace Glom
{

void c_callback_CellRendererList_on_editing_started(GtkCellRenderer* /* self */, GtkCellEditable* cell_editable, const gchar* /* path */, void* data)
{
  CellRendererList* pCppSelf = (CellRendererList*)data;
 
  if(cell_editable)
  {
    //This is actually ComboBox, because GtkComboBox inherits from GtkCellEditable since GTK+ 2.6.
    //But Gtk::ComboBox does not inherit from Gtk::CellEditable because we could not break ABI.
    //So we will use the C API to get the Gtk::ComboBox:
    GtkComboBox* pCComboBox = GTK_COMBO_BOX(cell_editable);
    Gtk::ComboBox* pComboBox = Glib::wrap(pCComboBox);


    //We don't use this convenience method, because we want more control over the renderer.
    //and CellLayout gives no way to get the renderer back afterwards.
    //(well, maybe set_cell_data_func(), but that's a bit awkward.)
    //pComboBox->pack_start(pCppSelf->m_model_columns.m_col_extra);

    Gtk::CellRenderer* cell_second = Gtk::manage(new Gtk::CellRendererText);
    cell_second->set_property("xalign", 0.0);

    //Use the renderer:
    pComboBox->pack_start(*cell_second);

    //Make the renderer render the column:
#ifdef GLIBMM_PROPERTIES_ENABLED
    pComboBox->add_attribute(cell_second->_property_renderable(), pCppSelf->m_model_columns.m_col_extra);
#else
    pComboBox->add_attribute(*cell_second, cell_second->_property_renderable(), pCppSelf->m_model_columns.m_col_extra);
#endif // GLIBMM_PROPERTIES_ENABLED

  }
  else
  {
    g_warning("CellRendererList::on_editing_started() cell_editable is null");
  }
}


CellRendererList::CellRendererList()
:  Glib::ObjectBase(0), //Mark this class as gtkmmproc-generated, rather than a custom class, to allow vfunc optimisations.
   //TODO: This should not be necessary - our gtkmm callbacks are somehow preventing the popup from appearing.
  m_use_second(false)
{
  m_refModel = Gtk::ListStore::create(m_model_columns);
  set_property("model", m_refModel);
  set_property("text-column", 0); //This must be a text column, in m_refModel.
  set_property("editable", true); //It would be useless if we couldn't edit it.

  //See the comment next to the implementation:
  //signal_editing_started().connect(sigc::mem_fun(*this, &CellRendererList::on_editing_started));

  g_signal_connect (gobj(), "editing_started", G_CALLBACK(&c_callback_CellRendererList_on_editing_started), this);
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

/* This is not used because the Gtk::CellRenderer::editing_started() signal currently sends a null cell_editable.
   We use a C callback instead.
 
void CellRendererList::on_editing_started(Gtk::CellEditable* cell_editable, const Glib::ustring&  path)
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
}
*/

void CellRendererList::set_use_second(bool use_second)
{
  m_use_second = use_second;
}

} //namespace Glom
