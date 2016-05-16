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

#ifndef GLOM_DATASTRUCTURE_LAYOUTGROUP_H
#define GLOM_DATASTRUCTURE_LAYOUTGROUP_H

#include <libglom/data_structure/layout/layoutitem_field.h>
#include <vector>

namespace Glom
{

class LayoutItem_Field;

class LayoutGroup : public LayoutItem
{
public:

  LayoutGroup();
  LayoutGroup(const LayoutGroup& src);
  LayoutGroup(LayoutGroup&& src) = delete;
  LayoutGroup& operator=(const LayoutGroup& src);
  LayoutGroup& operator=(LayoutGroup&& src) = delete;
  ~LayoutGroup() override;

  LayoutItem* clone() const override;

  /** Discover whether the layout group contains the specified field (from the current table).
   * @param parent_table_name The table to which this layout belongs.
   * @param table_name The table to which the field, specified by @a field_name, belongs.
   * @param field_name The name of the field to search for.
   * @result True if the field is in the layout group (or its child groups).
   */
  bool has_field(const Glib::ustring& parent_table_name, const Glib::ustring& table_name, const Glib::ustring& field_name) const;


  /** Discover whether the layout group contains any fields.
   * @result True if the field is in the layout group (or its child groups).
   */
  bool has_any_fields() const;

  /** Add the item to the end of the list.
   * @param item The item to add.
   */
  void add_item(const std::shared_ptr<LayoutItem>& item);

  /** Add the item after the specified existing item.
   * @param item The item to add.
   * @param position The item after which the item should be added.
   */
  void add_item(const std::shared_ptr<LayoutItem>& item, const std::shared_ptr<const LayoutItem>& position);

  /** Remove a layout item from the group
   * @param item The item to remove.
   */
  void remove_item(const std::shared_ptr<LayoutItem>& item);

  /** Remove any instance of the field from the layout.
   *
   * @param parent_table_name The table to which this layout belongs.
   * @param table_name The table to which the field, specified by @a field_name, belongs.
   * @param field_name The name of the field to remove.
   */
  void remove_field(const Glib::ustring& parent_table_name, const Glib::ustring& table_name, const Glib::ustring& field_name);

  //These are virtual so that the portal item can do more.
  virtual void change_field_item_name(const Glib::ustring& table_name, const Glib::ustring& field_name, const Glib::ustring& field_name_new);
  virtual void change_related_field_item_name(const Glib::ustring& table_name, const Glib::ustring& field_name, const Glib::ustring& field_name_new);

  /** Remove any use of the relationship from the layout.
   */
  virtual void remove_relationship(const std::shared_ptr<const Relationship>& relationship);


  void remove_all_items();

  double get_border_width() const;
  void set_border_width(double border_width);

  //virtual void debug(guint level = 0) const;

  guint get_items_count() const;

  guint get_columns_count() const;
  void set_columns_count(guint columns_count);

  typedef std::vector< std::shared_ptr<LayoutItem> > type_list_items;
  type_list_items get_items();

  typedef std::vector< std::shared_ptr<const LayoutItem> > type_list_const_items;
  type_list_const_items get_items() const;

  /** Get the items recursively, depth-first, not returning any groups.
   */
  type_list_const_items get_items_recursive() const;

  /** Get the items recursively, depth-first, not returning any groups.
   */
  type_list_items get_items_recursive();

  /** Get the items recursively, depth-first, also returning the groups.
   * This is only used by the tests so far.
   */
  type_list_const_items get_items_recursive_with_groups() const;

  Glib::ustring get_part_type_name() const override;
  Glib::ustring get_report_part_id() const override;

//Allow more efficient access: protected:

  type_list_items m_list_items;

private:

  guint m_columns_count;

  double m_border_width; //For use on reports.
};

} //namespace Glom

#endif //GLOM_DATASTRUCTURE_LAYOUTGROUP_H
