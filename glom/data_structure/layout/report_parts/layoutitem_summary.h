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
 
#ifndef GLOM_DATASTRUCTURE_LAYOUTITEM_SUMMARY_H
#define GLOM_DATASTRUCTURE_LAYOUTITEM_SUMMARY_H

#include "../layoutgroup.h"

class LayoutItem_Field;

class LayoutItem_Summary : public LayoutGroup
{
public:

  LayoutItem_Summary();
  LayoutItem_Summary(const LayoutItem_Summary& src);
  LayoutItem_Summary& operator=(const LayoutItem_Summary& src);
  virtual ~LayoutItem_Summary();

  virtual LayoutItem* clone() const;

  virtual Glib::ustring get_part_type_name() const;
  virtual Glib::ustring get_report_part_id() const;
};

#endif //GLOM_DATASTRUCTURE_LAYOUTITEM_SUMMARY_H



