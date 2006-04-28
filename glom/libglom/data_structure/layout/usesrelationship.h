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
 
#ifndef GLOM_DATASTRUCTURE_LAYOUT_USESRELATIONSHIP_H
#define GLOM_DATASTRUCTURE_LAYOUT_USESRELATIONSHIP_H

#include "../numeric_format.h"
#include "../relationship.h"
#include <glom/libglom/sharedptr.h>
#include <libgdamm.h>

/* Base class for classes that need to store a relationship name 
* and a cache of the actual relationship information.
*/
class UsesRelationship
{
public:

  UsesRelationship();
  UsesRelationship(const UsesRelationship& src);
  UsesRelationship& operator=(const UsesRelationship& src);
  virtual ~UsesRelationship();

  bool operator==(const UsesRelationship& src) const;

  bool get_has_relationship_name() const;
  bool get_has_related_relationship_name() const;

  /** Convenience function, equivalent to get_relationship()->get_name().
   */
  Glib::ustring get_relationship_name() const;

  /** Convenience function, equivalent to get_relationship()->get_name().
   */
  Glib::ustring get_related_relationship_name() const;

  sharedptr<Relationship> get_relationship() const;
  void set_relationship(const sharedptr<Relationship>& relationship);

  sharedptr<Relationship> get_related_relationship() const;
  void set_related_relationship(const sharedptr<Relationship>& relationship);

  /** Returns either the @a parent_table, related to table, or doubly-related to-table.
   */
  Glib::ustring get_table_used(const Glib::ustring& parent_table) const;

  Glib::ustring get_sql_join_alias_name() const;
  Glib::ustring get_sql_join_alias_definition() const;

  Glib::ustring get_sql_table_or_join_alias_name(const Glib::ustring& parent_table) const;

protected:

  //This is just cached data, so we don't need to always lookup the relationship details from the document, from the name.
  sharedptr<Relationship> m_relationship;
  sharedptr<Relationship> m_related_relationship; //Rarely used. It is for showing fields from the (related) relationships of related tables.

};

#endif //GLOM_DATASTRUCTURE_LAYOUT_USESRELATIONSHIP_H



