/* Glom
 *
 * Copyright (C) 2001-2006 Murray Cumming
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

#ifndef DB_POSTGRES_H
#define DB_POSTGRES_H

#include <gtkmm.h>

#include <glom/base_db.h>
#include <glom/libglom/document/document_glom.h>
#include <glom/libglom/connectionpool.h>

namespace Glom
{

class GlomPostgres : public Base_DB
{
public:

  /** @param not_extras If this is true, then do not set extra details, such as NOT NULL. You should do that later, when you are ready.
   */
  static bool postgres_add_column(const Glib::ustring& table_name, const sharedptr<const Field>& field, bool not_extras = false);

  /**
   * @param table_name The name of the table that will be affected.
   * @param field_old The definition of the field that will be changed.
   * @param field The new definition to give the field.
   * @param set_anyway If this is true, then set the extra details even if @field_old has the same properties.
   * @result The new field definition, with any necessary changes.
   */
  static sharedptr<Field> postgres_change_column_extras(const Glib::ustring& table_name, const sharedptr<const Field>& field_old, const sharedptr<const Field>& field, bool set_anyway = false);

protected:
  //Utility functions to help with the odd formats of postgres internal catalog fields:
  static type_vecStrings pg_list_separate(const Glib::ustring& str);

};

} //namespace Glom

#endif //DB_POSTGRES_H

