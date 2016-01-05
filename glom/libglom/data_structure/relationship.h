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

#ifndef GLOM_RELATIONSHIP_H
#define GLOM_RELATIONSHIP_H

#include <libglom/data_structure/translatable_item.h>
#include <libglom/data_structure/has_title_singular.h>
#include <glibmm/ustring.h>

namespace Glom
{

class Relationship
 : public TranslatableItem,
   public HasTitleSingular
{
public: 
  Relationship();
  Relationship(const Relationship& src);
  Relationship(Relationship&& src) = delete;

  Relationship& operator=(const Relationship& src);
  Relationship& operator=(Relationship&& src) = delete;

  bool operator==(const Relationship& src) const;

  Relationship* clone() const;

  Glib::ustring get_from_table() const;
  Glib::ustring get_from_field() const;
  Glib::ustring get_to_table() const;
  Glib::ustring get_to_field() const;

  void set_from_table(const Glib::ustring& strVal);
  void set_from_field(const Glib::ustring& strVal);
  void set_to_table(const Glib::ustring& strVal);
  void set_to_field(const Glib::ustring& strVal);

  ///Whether related records will be created automatically.
  bool get_auto_create() const;
  void set_auto_create(bool val = true);

  ///Whether related records may be edited through this relationship.
  bool get_allow_edit() const;
  void set_allow_edit(bool val = true);

  /** Whether the relationship specifies from and to fields.
   * If not, then it specifies all records in the to table.
   */
  bool get_has_fields() const;

  /** Whether the relationship specifies a related table.
   */
  bool get_has_to_table() const;

private:
  Glib::ustring m_strFrom_Table;
  Glib::ustring m_strFrom_Field;
  Glib::ustring m_strTo_Table;
  Glib::ustring m_strTo_Field;
  bool m_allow_edit, m_auto_create;
};

} //namespace Glom

#endif // GLOM_RELATIONSHIP_H
