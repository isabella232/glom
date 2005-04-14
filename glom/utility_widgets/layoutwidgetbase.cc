/* Glom
 *
 * Copyright (C) 2001-2005 Murray Cumming
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

#include "layoutwidgetbase.h"
#include <glibmm/i18n.h>

LayoutWidgetBase::LayoutWidgetBase()
: m_pLayoutItem(0),
  m_pMenuPopup(0)
{
  m_refActionGroup = Gtk::ActionGroup::create();

  m_refActionGroup->add(Gtk::Action::create("ContextMenu", "Context Menu") );
  m_refContextLayout =  Gtk::Action::create("ContextLayout", _("Choose Field"));
  m_refContextLayoutProperties =  Gtk::Action::create("ContextLayoutProperties", _("Field Layout Properties"));
  m_refContextAddField =  Gtk::Action::create("ContextAddField", _("Add Field"));
  m_refContextAddRelatedRecords =  Gtk::Action::create("ContextAddRelatedRecords", _("Add Related Records"));
  m_refContextAddGroup =  Gtk::Action::create("ContextAddGroup", _("Add Group"));

}

LayoutWidgetBase::~LayoutWidgetBase()
{
  if(m_pLayoutItem)
  {
    delete m_pLayoutItem;
    m_pLayoutItem = 0;
  }
}

void LayoutWidgetBase::set_layout_item(LayoutItem* layout_item, const Glib::ustring& table_name)
{
  if(m_pLayoutItem)
    delete m_pLayoutItem;

  m_pLayoutItem = layout_item;
  m_table_name = table_name;
}

const LayoutItem* LayoutWidgetBase::get_layout_item() const
{
  return m_pLayoutItem;
}

LayoutItem* LayoutWidgetBase::get_layout_item()
{
  return m_pLayoutItem;
}

LayoutWidgetBase::type_signal_layout_changed LayoutWidgetBase::signal_layout_changed()
{
  return m_signal_layout_changed;
}

LayoutWidgetBase::type_signal_layout_item_added LayoutWidgetBase::signal_layout_item_added()
{
  return m_signal_layout_item_added;
}
