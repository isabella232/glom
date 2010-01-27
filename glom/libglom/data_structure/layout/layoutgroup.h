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

#ifndef GLOM_DATASTRUCTURE_LAYOUTGROUP_H
#define GLOM_DATASTRUCTURE_LAYOUTGROUP_H

#include <libglom/data_structure/layout/layoutitem_field.h>
#include <list>

namespace Glom
{

class LayoutItem_Field;

class LayoutGroup : public LayoutItem
{
public:

  LayoutGroup();
  LayoutGroup(const LayoutGroup& src);
  LayoutGroup& operator=(const LayoutGroup& src);
  virtual ~LayoutGroup();

  virtual LayoutItem* clone() const;

  /** Discover whether the layout group contains the specified field.
   * @param field_name The name of the field to seach for.
   * @result True if the field is in the layout group (or its child groups).
   */
  bool has_field(const Glib::ustring& field_name) const;

  /** Discover whether the layout group contains any fields.
   * @result True if the field is in the layout group (or its child groups).
   */
  bool has_any_fields() const;

  /** Add the item to the end of the list.
   * @param item The item to add.
   */
  void add_item(const sharedptr<LayoutItem>& item);

  /** Add the item after the specified existing item.
   * @param item The item to add.
   * @param position The item after which the item should be added. 
   */
  void add_item(const sharedptr<LayoutItem>& item, const sharedptr<const LayoutItem>& position);
  
  /** Remove a layout item from the group
   * @param item The item to remove.
   */
  void remove_item (const sharedptr<LayoutItem>& item);  

  /** Remove any instance of the field (from the current table) from the layout.
   */
  virtual void remove_field(const Glib::ustring& field_name);

  /** Remove any instance of the related field from the layout.
   */
  virtual void remove_field(const Glib::ustring& table_name, const Glib::ustring& field_name);

  virtual void change_field_item_name(const Glib::ustring& table_name, const Glib::ustring& field_name, const Glib::ustring& field_name_new);
  virtual void change_related_field_item_name(const Glib::ustring& table_name, const Glib::ustring& field_name, const Glib::ustring& field_name_new);

  /** Remove any use of the relationship from the layout.
   */
  virtual void remove_relationship(const sharedptr<const Relationship>& relationship);


  void remove_all_items();

  double get_border_width() const;
  void set_border_width(double border_width);

  //virtual void debug(guint level = 0) const;

  guint get_items_count() const;

  guint get_columns_count() const;
  void set_columns_count(guint columns_count);
  
  typedef std::vector< sharedptr<LayoutItem> > type_list_items;
  type_list_items get_items();

  typedef std::vector< sharedptr<const LayoutItem> > type_list_const_items;
  type_list_const_items get_items() const;

  virtual Glib::ustring get_part_type_name() const;
  virtual Glib::ustring get_report_part_id() const;

//Allow more efficient access: protected:

  type_list_items m_list_items;

private:

  guint m_columns_count;

  double m_border_width; //For use on reports.
};

} //namespace Glom

#endif //GLOM_DATASTRUCTURE_LAYOUTGROUP_H



