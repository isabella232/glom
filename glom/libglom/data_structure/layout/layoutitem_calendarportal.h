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

#ifndef GLOM_DATASTRUCTURE_LAYOUTITEM_CALENDAR_PORTAL_H
#define GLOM_DATASTRUCTURE_LAYOUTITEM_CALENDAR_PORTAL_H

#include "layoutitem_portal.h"

namespace Glom
{

class LayoutItem_CalendarPortal : public LayoutItem_Portal
{
public:

  LayoutItem_CalendarPortal();
  LayoutItem_CalendarPortal(const LayoutItem_CalendarPortal& src);
  LayoutItem_CalendarPortal& operator=(const LayoutItem_CalendarPortal& src);
  virtual ~LayoutItem_CalendarPortal();

  virtual LayoutItem* clone() const;

  virtual Glib::ustring get_part_type_name() const;

  sharedptr<Field> get_date_field();
  sharedptr<const Field> get_date_field() const;
    
  void set_date_field(const sharedptr<Field>& field);
    
  virtual void change_field_item_name(const Glib::ustring& table_name, const Glib::ustring& field_name, const Glib::ustring& field_name_new);
  virtual void change_related_field_item_name(const Glib::ustring& table_name, const Glib::ustring& field_name, const Glib::ustring& field_name_new);


private:
  sharedptr<Field> m_date_field;
};

} //namespace Glom

#endif //GLOM_DATASTRUCTURE_LAYOUTITEM_CALENDAR_PORTAL_H



