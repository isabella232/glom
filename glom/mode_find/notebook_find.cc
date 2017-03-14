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

#include "notebook_find.h"
#include <glom/signal_reemitter.h>
#include <glibmm/i18n.h>

namespace Glom
{

constexpr auto PAGENAME_DETAILS = "details";
constexpr auto PAGENAME_LIST = "list";

Notebook_Find::Notebook_Find()
{
  append_page(m_Box_List, PAGENAME_LIST, _("List"));

  //Fill composite view:
  add_view(&m_Box_List);

  append_page(m_Box_Details, PAGENAME_DETAILS, _("Details"));

  set_visible_child(PAGENAME_DETAILS); //Show the details page by default. It's more obvious for a Find.
  //TODO: Show the same layout that is being edited at the time that the mode was changed.

  //Connect Signals:
  //Pass it up to the application:
  signal_connect_for_reemit_1arg(m_Box_List.signal_find_criteria, signal_find_criteria);
  signal_connect_for_reemit_1arg(m_Box_Details.signal_find_criteria, signal_find_criteria);


  add_view(&m_Box_Details);
}

Notebook_Find::~Notebook_Find()
{
  remove_view(&m_Box_List);

  remove_view(&m_Box_Details);
}

bool Notebook_Find::init_db_details(const Glib::ustring& table_name, const Glib::ustring& layout_platform)
{
  const auto result = m_Box_List.init_db_details(table_name, layout_platform);

  m_Box_Details.init_db_details(table_name, layout_platform);

  return result;
}

void Notebook_Find::set_current_view(Notebook_Data::dataview view)
{
  if(view == Notebook_Data::dataview::LIST)
    set_visible_child(PAGENAME_LIST);
  else
    set_visible_child(PAGENAME_DETAILS);
}

} //namespace Glom
