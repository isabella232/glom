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

#ifndef GLOM_DATASTRUCTURE_LAYOUTITEM_PORTAL_H
#define GLOM_DATASTRUCTURE_LAYOUTITEM_PORTAL_H

#include "layoutgroup.h"
#include <libglom/data_structure/field.h>
#include <libglom/data_structure/relationship.h>

namespace Glom
{

class LayoutItem_Portal
: public LayoutGroup,
  public UsesRelationship
{
public:

  LayoutItem_Portal();
  LayoutItem_Portal(const LayoutItem_Portal& src);
  LayoutItem_Portal& operator=(const LayoutItem_Portal& src);
  virtual ~LayoutItem_Portal();

  virtual LayoutItem* clone() const;

  virtual Glib::ustring get_part_type_name() const;

  virtual void change_field_item_name(const Glib::ustring& table_name, const Glib::ustring& field_name, const Glib::ustring& field_name_new);
  virtual void change_related_field_item_name(const Glib::ustring& table_name, const Glib::ustring& field_name, const Glib::ustring& field_name_new);

  ///A helper method to avoid extra ifs to avoid null dereferencing.
  Glib::ustring get_from_table() const;

  //virtual void debug(guint level = 0) const;

  /** Gets the relationship to use for navigation if get_navigation_type() is 
   * NAVIGATION_NONE.
   */
  sharedptr<UsesRelationship> get_navigation_relationship_specific();

  /** Get the @a relationship to use for navigation if get_navigation_type() is 
   * NAVIGATION_NONE.
   */
  sharedptr<const UsesRelationship> get_navigation_relationship_specific() const;

  /** Set the @a relationship to use for navigation if get_navigation_type() is 
   * NAVIGATION_NONE.
   */
  void set_navigation_relationship_specific(const sharedptr<UsesRelationship>& relationship);

  void reset_navigation_relationship();

  /** The navigation (if any) that should be used when the user 
   * activates a related record row.
   */
  enum navigation_type
  {
    NAVIGATION_NONE, /**< No navigation will be offered. */
    NAVIGATION_AUTOMATIC, /**< The destination related table will be chosen automatically based on the relationship and the visible fields. */
    NAVIGATION_SPECIFIC /**< The destination related table will be determined by a specified relationship. */
  };

  /** Discover what @a type (if any) navigation should be used when the user 
   * activates a related record row.
   */
  navigation_type get_navigation_type() const;

  /** Set what @a type (if any) navigation should be used when the user 
   * activates a related record row.
   */
  void set_navigation_type(navigation_type type);


  /// This is used only for the print layouts.
  double get_print_layout_row_height() const;

  /// This is used only for the print layouts.
  void set_print_layout_row_height(double row_height);


private:

  sharedptr<UsesRelationship> m_navigation_relationship_specific;

  // This is used only for the print layouts.
  double m_print_layout_row_height;

  //If no navigation relationship has been specified then it will be automatically chosen or navigation will be disabled:
  navigation_type m_navigation_type;
};

} //namespace Glom

#endif //GLOM_DATASTRUCTURE_LAYOUTITEM_PORTAL_H

