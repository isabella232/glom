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

#ifndef GLOM_DATASTRUCTURE_TABLEINFO_H
#define GLOM_DATASTRUCTURE_TABLEINFO_H

#include <libglom/data_structure/translatable_item.h>
#include <libglom/data_structure/has_title_singular.h>

namespace Glom
{

class TableInfo
 : public TranslatableItem,
   public HasTitleSingular
{
public:
  TableInfo();
  TableInfo(const TableInfo& src);
  TableInfo(TableInfo&& src) = delete;
  TableInfo& operator=(const TableInfo& src);
  TableInfo& operator=(TableInfo&& src) = delete;

  bool operator==(const TableInfo& src) const;
  bool operator!=(const TableInfo& src) const;

  /** Returns true if this table should not be shown in the list of tables when in operator mode.
   */
  bool get_hidden() const;

  /** See get_default().
   */
  void set_hidden(bool val = true);

  /** Returns true if this table should be shown when the system is opened.
   * Only one table can be the default table.
   */
  bool get_default() const;

  /** See get_default().
   */
  void set_default(bool val = true);

private:
  bool m_hidden;
  bool m_default;
};

} //namespace Glom

#endif //GLOM_DATASTRUCTURE_TABLEINFO_H



