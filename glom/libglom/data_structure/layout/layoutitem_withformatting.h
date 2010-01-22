/* Glom
 *
 * Copyright (C) 2009 Murray Cumming
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

#ifndef GLOM_DATASTRUCTURE_LAYOUTITEM_WITHFORMATTING_H
#define GLOM_DATASTRUCTURE_LAYOUTITEM_WITHFORMATTING_H

#include "layoutitem.h"
#include "fieldformatting.h"

namespace Glom
{

/** A base class for all layout items that may have formatting options.
 * See get_formatting_used().
 */
class LayoutItem_WithFormatting 
 : public LayoutItem
{
public:

  LayoutItem_WithFormatting();
  LayoutItem_WithFormatting(const LayoutItem_WithFormatting& src);
  LayoutItem_WithFormatting& operator=(const LayoutItem_WithFormatting& src);
  virtual ~LayoutItem_WithFormatting();

  bool operator==(const LayoutItem_WithFormatting& src) const;

  FieldFormatting m_formatting;

  /** Get the field formatting used by this layout item, which 
   * may be either custom field formatting or the default field formatting.
   */
  virtual const FieldFormatting& get_formatting_used() const;

  /** Get the alignment for the formatting used (see get_formatting_used()),
   * choosing an appropriate alignment if it is set to HORIZONTAL_ALIGNMENT_AUTO.
   * Note that this never returns HORIZONTAL_ALIGNMENT_AUTO.
   */
  virtual FieldFormatting::HorizontalAlignment get_formatting_used_horizontal_alignment() const;
};

} //namespace Glom

#endif //GLOM_DATASTRUCTURE_LAYOUTITEM_WITHFORMATTING_H



