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
 
#ifndef GLOM_DATASTRUCTURE_LAYOUTITEM_GROUPBY_H
#define GLOM_DATASTRUCTURE_LAYOUTITEM_GROUPBY_H

#include "../layoutgroup.h"
#include "../layoutitem_field.h"
#include <map>

class LayoutItem_Field;

/** The child items are fields to be shown for each record in the group.
 * field_group_by is the field by which the records are grouped.
 */
class LayoutItem_GroupBy : public LayoutGroup
{
public:

  LayoutItem_GroupBy();
  LayoutItem_GroupBy(const LayoutItem_GroupBy& src);
  LayoutItem_GroupBy& operator=(const LayoutItem_GroupBy& src);
  virtual ~LayoutItem_GroupBy();

  virtual LayoutItem* clone() const;

  typedef std::pair< sharedptr<LayoutItem_Field>, bool /* is_ascending */> type_pair_sort_field;
  typedef std::list<type_pair_sort_field> type_list_sort_fields;

  sharedptr<LayoutItem_Field> get_field_group_by();
  sharedptr<const LayoutItem_Field> get_field_group_by() const;
  bool get_has_field_group_by() const;

  void set_field_group_by(const sharedptr<LayoutItem_Field>& field);

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

  sharedptr<LayoutGroup> m_group_secondary_fields; //For instance, show a contact name as well as the contact ID that we group by.

  type_list_sort_fields m_fields_sort_by;

protected:
  sharedptr<LayoutItem_Field> m_field_group_by;

};

#endif //GLOM_DATASTRUCTURE_LayoutGroupBY_H



