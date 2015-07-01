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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#ifndef GLOM_DATASTRUCTURE_LAYOUT_CUSTOM_TITLE_H
#define GLOM_DATASTRUCTURE_LAYOUT_CUSTOM_TITLE_H

#include <libglom/data_structure/translatable_item.h>

namespace Glom
{

class CustomTitle 
 : public TranslatableItem
{
public:

  CustomTitle();
  CustomTitle(const CustomTitle& src);
  CustomTitle(CustomTitle&& src) = delete;
  CustomTitle& operator=(const CustomTitle& src);
  CustomTitle& operator=(CustomTitle&& src) = delete;
  virtual ~CustomTitle();

  bool operator==(const CustomTitle& src) const;

  bool get_use_custom_title() const;
  void set_use_custom_title(bool use_custom_title = true);

private:

  //We need this in order to specify that an empty custom title should really be used.
  bool m_use_custom_title;
};

} //namespace Glom

#endif //GLOM_DATASTRUCTURE_LAYOUT_CUSTOM_TITLE_H



