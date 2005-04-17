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

#include <glibmm/ustring.h>

class LayoutItem
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

  virtual void set_name(const Glib::ustring& name);
  virtual Glib::ustring get_name() const; //For use with our std::find_if() predicate.

  virtual bool get_editable() const;
  virtual void set_editable(bool val);

  guint m_sequence;
  //bool m_hidden;

protected:
  Glib::ustring m_name;
  bool m_editable;
};

#endif //GLOM_DATASTRUCTURE_LAYOUTITEM_H



