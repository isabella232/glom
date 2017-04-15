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

#include "dialog_formatting.h"
#include <glom/glade_utils.h>
#include <glibmm/i18n.h>

namespace Glom
{

Dialog_Formatting::Dialog_Formatting()
: m_box_formatting(nullptr)
{
  set_title(_("Formatting"));
  set_margin(6);

  //Get the formatting stuff:
  Utils::box_pack_start_glade_child_widget_derived_with_warning(get_content_area(), m_box_formatting);
  add_view(m_box_formatting);

  add_button(_("_Cancel"), Gtk::ResponseType::CANCEL);
  add_button(_("_Save"), Gtk::ResponseType::OK);
}

Dialog_Formatting::~Dialog_Formatting()
{
  remove_view(m_box_formatting);
}

void Dialog_Formatting::set_item(const std::shared_ptr<const LayoutItem_WithFormatting>& layout_item, bool show_numeric)
{
  m_box_formatting->set_formatting_for_non_field(layout_item->m_formatting, show_numeric);

  enforce_constraints();
}

void Dialog_Formatting::use_item_chosen(const std::shared_ptr<LayoutItem_WithFormatting>& layout_item)
{
  if(!layout_item)
    return;

  m_box_formatting->get_formatting(layout_item->m_formatting);
}

void Dialog_Formatting::enforce_constraints()
{
}

} //namespace Glom
