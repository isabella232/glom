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

#ifndef GLOM_DATASTRUCTURE_LAYOUT_CUSTOM_TITLE_H
#define GLOM_DATASTRUCTURE_LAYOUT_CUSTOM_TITLE_H

#include "../translatable_item.h"

class CustomTitle 
 : public TranslatableItem
{
public:

  CustomTitle();
  CustomTitle(const CustomTitle& src);
  CustomTitle& operator=(const CustomTitle& src);
  virtual ~CustomTitle();

  bool operator==(const CustomTitle& src) const;

  bool get_use_custom_title() const;
  void set_use_custom_title(bool use_custom_title = true);

protected:

  //We need this in order to specify that an empty custom title should really be used.
  bool m_use_custom_title;
};

#endif //GLOM_DATASTRUCTURE_LAYOUT_CUSTOM_TITLE_H



