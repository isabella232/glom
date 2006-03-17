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

#ifndef GLOM_DATASTRUCTURE_LAYOUTITEM_TEXT_H
#define GLOM_DATASTRUCTURE_LAYOUTITEM_TEXT_H

#include "layoutitem.h"

class LayoutItem_Text 
 : public LayoutItem
{
public:

  LayoutItem_Text();
  LayoutItem_Text(const LayoutItem_Text& src);
  LayoutItem_Text& operator=(const LayoutItem_Text& src);
  virtual ~LayoutItem_Text();

  virtual LayoutItem* clone() const;

  bool operator==(const LayoutItem_Text& src) const;

  virtual Glib::ustring get_part_type_name() const;
  virtual Glib::ustring get_report_part_id() const;

  /** Set the text that will be shown on each record.
   */
  Glib::ustring get_text() const;

  /** Get the text that will be shown on each record.
   */
  void set_text(const Glib::ustring& script);

  sharedptr<TranslatableItem> m_text; //Reuse the title concept of this class to give us translatable text.
};

#endif //GLOM_DATASTRUCTURE_LAYOUTITEM_TEXT_H



