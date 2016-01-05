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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#ifndef GLOM_DATASTRUCTURE_LAYOUTITEM_WITHFORMATTING_H
#define GLOM_DATASTRUCTURE_LAYOUTITEM_WITHFORMATTING_H

#include "layoutitem.h"
#include "formatting.h"

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
  LayoutItem_WithFormatting(LayoutItem_WithFormatting&& src) = delete;
  LayoutItem_WithFormatting& operator=(const LayoutItem_WithFormatting& src);
  LayoutItem_WithFormatting& operator=(LayoutItem_WithFormatting&& src) = delete;

  bool operator==(const LayoutItem_WithFormatting& src) const;

  Formatting m_formatting;

  /** Get the field formatting used by this layout item, which 
   * may be either custom field formatting or the default field formatting.
   */
  virtual const Formatting& get_formatting_used() const;

  /** Get the alignment for the formatting used (see get_formatting_used()),
   * choosing an appropriate alignment if it is set to HorizontalAlignment::AUTO.
   * Note that this never returns HorizontalAlignment::AUTO.
   */
  virtual Formatting::HorizontalAlignment get_formatting_used_horizontal_alignment(bool for_details_view = false) const;
};

} //namespace Glom

#endif //GLOM_DATASTRUCTURE_LAYOUTITEM_WITHFORMATTING_H



