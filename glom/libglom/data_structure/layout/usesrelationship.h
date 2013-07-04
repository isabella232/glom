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
 
#ifndef GLOM_DATASTRUCTURE_LAYOUT_USESRELATIONSHIP_H
#define GLOM_DATASTRUCTURE_LAYOUT_USESRELATIONSHIP_H

#include <libglom/data_structure/numeric_format.h>
#include <libglom/data_structure/relationship.h>
#include <libglom/sharedptr.h>

namespace Glom
{

class Field;

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

  /** Return the relationship used by this item, if any, or a null std::shared_ptr.
   * See also get_has_relationship_name() which can prevent the need for your  
   * own null std::shared_ptr check.
   */ 
  std::shared_ptr<const Relationship> get_relationship() const;

  void set_relationship(const std::shared_ptr<const Relationship>& relationship);

  /** Return the related relationship used by this item, if any, or a null std::shared_ptr.
   * See also get_has_related_relationship_name() which can prevent the need for your  
   * own null std::shared_ptr check.
   */ 
  std::shared_ptr<const Relationship> get_related_relationship() const;

  void set_related_relationship(const std::shared_ptr<const Relationship>& relationship);

  /** Returns either the @a parent_table, related to table, or doubly-related to-table.
   */
  Glib::ustring get_table_used(const Glib::ustring& parent_table) const;

  /** Get the title of the relationship that is actually used,
   * falling back to the relationship's name.
   * @param parent_table_title The title of table to which the item (or its relatinoships) belong.
   */
  Glib::ustring get_title_used(const Glib::ustring& parent_table_title, const Glib::ustring& locale) const;
  
  /** Get the singular title of the relationship that is actually used,
   * falling back to the regular (plural) title, and then to the relationship's name.
   * @param parent_table_title The title of table to which the item (or its relatinoships) belong.
   */
  Glib::ustring get_title_singular_used(const Glib::ustring& parent_table_title, const Glib::ustring& locale) const;

  Glib::ustring get_to_field_used() const;

  /** Get the name of the related relationship used, if any, or the relationship 
   * if there is no related relationship, or an empty string if neither are 
   * used by this item.
   */ 
  Glib::ustring get_relationship_name_used() const;

  /** Discover whether the relationship used allows the user to edit values 
   * in its to table.
   */
  bool get_relationship_used_allows_edit() const;

  /** Get a name to use as an alias in SQL statements.
   * This will always be the same string for items that have the same definition.
   */ 
  Glib::ustring get_sql_join_alias_name() const;
  
  /** Get the item's alias name, if it uses a relationship, or just get its table name.
   * @param parent_table The table to which the item (or its relatinoships) belong.
   */ 
  Glib::ustring get_sql_table_or_join_alias_name(const Glib::ustring& parent_table) const;
  
  
  /** Get a human-readable representation of th relationship.
   * This just concatenates the chain of relationships, separating them by ":".
   */
  Glib::ustring get_relationship_display_name() const;

private:

  //This is just cached data, so we don't need to always lookup the relationship details from the document, from the name.
  std::shared_ptr<const Relationship> m_relationship;
  std::shared_ptr<const Relationship> m_related_relationship; //Rarely used. It is for showing fields from the (related) relationships of related tables.
};

} //namespace Glom

#endif //GLOM_DATASTRUCTURE_LAYOUT_USESRELATIONSHIP_H



