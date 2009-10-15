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

#include "tableinfo.h"

namespace Glom
{

TableInfo::TableInfo()
: m_sequence(0),
  m_hidden(false),
  m_default(false)
{
  m_translatable_item_type = TRANSLATABLE_TYPE_TABLE;
}

TableInfo::TableInfo(const TableInfo& src)
: TranslatableItem(src),
  m_sequence(src.m_sequence),
  m_hidden(src.m_hidden),
  m_default(src.m_default),
  m_title_singular(src.m_title_singular)
{
}

TableInfo& TableInfo::operator=(const TableInfo& src)
{
  TranslatableItem::operator=(src);

  m_sequence = src.m_sequence;
  m_hidden = src.m_hidden;
  m_default = src.m_default;
  m_title_singular = src.m_title_singular;

  return *this;
}

Glib::ustring TableInfo::get_title_singular() const
{
  Glib::ustring result;
  if(m_title_singular)
    result = m_title_singular->get_title();

  return result;
}

Glib::ustring TableInfo::get_title_singular_with_fallback() const
{
  const Glib::ustring result = get_title_singular();
  if(result.empty())
    return get_title_or_name();

  return result;
}

void TableInfo::set_title_singular(const Glib::ustring& title)
{
  if(!m_title_singular)
    m_title_singular = sharedptr<TranslatableItem>::create();

  m_title_singular->set_title(title);
}



} //namespace Glom
