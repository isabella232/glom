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
 
#ifndef GLOM_DATASTRUCTURE_LAYOUTITEM_GROUPBY_H
#define GLOM_DATASTRUCTURE_LAYOUTITEM_GROUPBY_H

#include <libglom/data_structure/layout/layoutgroup.h>
#include <libglom/data_structure/layout/layoutitem_field.h>
#include <map>

namespace Glom
{

class LayoutItem_Field;

/** The child items are fields to be shown for each record in the group.
 * The records are grouped by a specified field and there may be.
 * secondary fields that are also shown for each group.
 */
class LayoutItem_GroupBy : public LayoutGroup
{
public:

  LayoutItem_GroupBy();
  LayoutItem_GroupBy(const LayoutItem_GroupBy& src);
  LayoutItem_GroupBy& operator=(const LayoutItem_GroupBy& src);
  virtual ~LayoutItem_GroupBy();

  virtual LayoutItem* clone() const;

  typedef Formatting::type_pair_sort_field type_pair_sort_field;
  typedef Formatting::type_list_sort_fields type_list_sort_fields;

  std::shared_ptr<LayoutItem_Field> get_field_group_by();
  std::shared_ptr<const LayoutItem_Field> get_field_group_by() const;
  bool get_has_field_group_by() const;

  void set_field_group_by(const std::shared_ptr<LayoutItem_Field>& field);

  //How to sort the records in this group:
  type_list_sort_fields get_fields_sort_by();
  type_list_sort_fields get_fields_sort_by() const;
  bool get_has_fields_sort_by() const;

  void set_fields_sort_by(const type_list_sort_fields& field);

  /** Get a text representation for the a layout.
   */
  virtual Glib::ustring get_layout_display_name() const;

  virtual Glib::ustring get_part_type_name() const;
  virtual Glib::ustring get_report_part_id() const;

  std::shared_ptr<LayoutGroup> get_secondary_fields();
  std::shared_ptr<const LayoutGroup> get_secondary_fields() const;

  type_list_sort_fields get_sort_by() const;
  void set_sort_by(const type_list_sort_fields& sort_by);
  
private:

  std::shared_ptr<LayoutItem_Field> m_field_group_by;
  std::shared_ptr<LayoutGroup> m_group_secondary_fields; //For instance, show a contact name as well as the contact ID that we group by.
  type_list_sort_fields m_fields_sort_by;
};

} //namespace Glom

#endif //GLOM_DATASTRUCTURE_LayoutGroupBY_H



