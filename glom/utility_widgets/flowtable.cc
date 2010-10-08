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

#include "flowtable.h"
#include "layoutwidgetbase.h"
#include <iostream>
#include <gdkmm/window.h>
#include <glom/utils_ui.h>

namespace Glom
{

FlowTable::FlowTableItem::FlowTableItem(Gtk::Widget* first, FlowTable* /* flowtable */)
: m_first(first),
  m_second(0),
  m_expand_first_full(false),
  m_expand_second(false)
{

}

FlowTable::FlowTableItem::FlowTableItem(Gtk::Widget* first, Gtk::Widget* second, FlowTable* /* flowtable */)
: m_first(first),
  m_second(second),
  m_expand_first_full(false),
  m_expand_second(false)
{

}


FlowTable::FlowTable()
:
  m_design_mode(false)
{
}

FlowTable::~FlowTable()
{
}

void FlowTable::set_design_mode(bool value)
{
  m_design_mode = value;

  queue_draw(); //because this changes how the widget would be drawn.
}

void FlowTable::add(Gtk::Widget& first, Gtk::Widget& second, bool expand_second)
{
  FlowTableItem item(&first, &second, this);

  Gtk::HBox* hbox = Gtk::manage(new Gtk::HBox(false, get_horizontal_spacing()));

  item.m_expand_second = expand_second; //Expand to fill the width for all of the second item.
  m_children.push_back(item);

  hbox->pack_start(first, Gtk::PACK_SHRINK);
  hbox->pack_start(second, expand_second ? Gtk::PACK_EXPAND_WIDGET : Gtk::PACK_SHRINK);
  hbox->show();

  hbox->set_halign(Gtk::ALIGN_FILL);
  append_child(*hbox);
}

void FlowTable::add(Gtk::Widget& first, bool expand)
{
  FlowTableItem item(&first, this);

  item.m_expand_first_full = expand; //Expand to fill the width for first and second.
  m_children.push_back(item);

  first.set_halign(Gtk::ALIGN_FILL);
  append_child(first); //TODO: expand
}

void FlowTable::insert_before(Gtk::Widget& first, Gtk::Widget& before, bool expand)
{
  FlowTableItem item(&first, this);
  item.m_expand_first_full = expand;
  insert_before(item, before);
}

void FlowTable::insert_before(Gtk::Widget& first, Gtk::Widget& second, Gtk::Widget& before, bool expand_second)
{
  FlowTableItem item(&first, &second, this);
  item.m_expand_second = expand_second;
  insert_before(item, before);
}

void FlowTable::insert_before(FlowTableItem& /* item */, Gtk::Widget& /* before */)
{
  std::cerr << G_STRFUNC << ": Not implemented" << std::endl;
  //TODO:
  /*
  bool found = false;
  std::vector<FlowTableItem>::iterator pos;
  for(pos = m_children.begin(); pos != m_children.end(); ++pos)
  {
    FlowTableItem* item = &(*pos);
    if(item->m_first)
    {
      if(item->m_first->gobj() == before.gobj())
      {
        found = true;
        break;
      }

      Gtk::Alignment* alignment = dynamic_cast<Gtk::Alignment*>(item->m_first);
      if(alignment && alignment->get_child()->gobj() == before.gobj())
      {
        found = true;
        break;
      }
    }

    if(item->m_second)
    {
      if(item->m_second->gobj() == before.gobj())
      {
        found = true;
        break;
      }
      Gtk::Alignment* alignment = dynamic_cast<Gtk::Alignment*>(item->m_second);
      if(alignment && alignment->get_child()->gobj() == before.gobj())
      {
        found = true;
        break;
      }
    }
  }

  gtk_widget_set_parent(GTK_WIDGET(item.m_first->gobj()), GTK_WIDGET(gobj()));
  if(item.m_second)
  {
    gtk_widget_set_parent(GTK_WIDGET(item.m_second->gobj()), GTK_WIDGET(gobj()));
  }

  if(pos == m_children.end())
    m_children.push_back(item);
  else
    m_children.insert(pos, item);
  */
}

bool FlowTable::child_is_visible(const Gtk::Widget* widget) const
{
  #if GTKMM_MINOR_VERSION >= 18
  return widget && widget->get_visible();
  #else
  return widget && widget->is_visible();
  #endif
}

void FlowTable::remove(Gtk::Widget& first)
{
  //Gtk::Container::remove() does this too. We need to do it here too:
  if(first.is_managed_())
    first.reference();

  gtk_widget_unparent(first.gobj());

  for(type_vecChildren::iterator iter = m_children.begin(); iter != m_children.end(); ++iter)
  {
    if((iter->m_first == &first) && (iter->m_second == 0))
    {
      //g_warning("FlowTable::remove(): removing %10X", (guint)&first);

      m_children.erase(iter);
      break;
    }
  }
}

void FlowTable::remove_all()
{

  for(type_vecChildren::iterator iter = m_children.begin(); iter != m_children.end(); ++iter)
  {
    if(iter->m_first)
    {
      Gtk::Widget* widget = iter->m_first;

      if(widget->is_managed_())
        widget->reference();

      gtk_widget_unparent(GTK_WIDGET(iter->m_first->gobj()));
    }

    if(iter->m_second)
    {
      Gtk::Widget* widget = iter->m_second;

      if(widget->is_managed_())
        widget->reference();

      gtk_widget_unparent(GTK_WIDGET(iter->m_second->gobj()));
    }

  }

  m_children.clear();
}


} //namespace Glom
