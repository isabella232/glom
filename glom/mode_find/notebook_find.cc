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

#include "notebook_find.h"
#include <glibmm/i18n.h>

namespace Glom
{

Notebook_Find::Notebook_Find()
: m_iPage_Details(0),
  m_iPage_List(0)
{
  append_page(m_Box_List, _("List"));
  m_Box_List.signal_find_criteria.connect(sigc::mem_fun(*this, &Notebook_Find::on_page_find_criteria));
  m_iPage_List = 0;
  m_iPage_Details = 1;

  //Fill composite view:
  add_view(&m_Box_List);

  append_page(m_Box_Details, _("Details"));

  set_current_page(m_iPage_Details); //Show the details page by default. It's more obvious for a Find.
  //TODO: Show the same layout that is being edited at the time that the mode was changed.

  //Connect Signals:
  m_Box_Details.signal_find_criteria.connect(sigc::mem_fun(*this, &Notebook_Find::on_page_find_criteria));

  add_view(&m_Box_Details);

  show_all_children();

}

Notebook_Find::~Notebook_Find()
{
  remove_view(&m_Box_List);

  remove_view(&m_Box_Details);
}

bool Notebook_Find::init_db_details(const Glib::ustring& table_name, const Glib::ustring& layout_platform)
{
  bool result = true;

  result = m_Box_List.init_db_details(table_name, layout_platform);

  m_Box_Details.init_db_details(table_name, layout_platform);

  return result;
}

void Notebook_Find::on_page_find_criteria(const Gnome::Gda::SqlExpr& where_clause)
{
  //Pass it up to the application.
  signal_find_criteria.emit(where_clause);
}

void Notebook_Find::set_current_view(Notebook_Data::dataview view)
{
  if(view == Notebook_Data::DATA_VIEW_List)
    set_current_page(m_iPage_List);
  else
    set_current_page(m_iPage_Details);
}

} //namespace Glom
