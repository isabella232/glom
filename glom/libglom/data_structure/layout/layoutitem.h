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

#ifndef GLOM_DATASTRUCTURE_LAYOUTITEM_H
#define GLOM_DATASTRUCTURE_LAYOUTITEM_H

#include "../translatable_item.h"
#include <glibmm/ustring.h>

namespace Glom
{

class LayoutItem : public TranslatableItem
{
public:

  LayoutItem();
  LayoutItem(const LayoutItem& src);
  LayoutItem& operator=(const LayoutItem& src);
  virtual ~LayoutItem();

  /** Create a new copied instance.
   * This allows us to deep-copy a list of LayoutItems.
   */
  virtual LayoutItem* clone() const = 0;

  bool operator==(const LayoutItem& src) const;

  virtual bool get_editable() const;
  virtual void set_editable(bool val);

  virtual Glib::ustring get_layout_display_name() const;
  virtual Glib::ustring get_part_type_name() const = 0;

  /** Gets the node name to use for the intermediate XML,
   * (and usually, the CSS style class to use for the resulting HTML).
   */
  virtual Glib::ustring get_report_part_id() const;

  guint m_sequence;
  //bool m_hidden;

protected:
  Glib::ustring m_name;
  bool m_editable;
};

} //namespace Glom

#endif //GLOM_DATASTRUCTURE_LAYOUTITEM_H



