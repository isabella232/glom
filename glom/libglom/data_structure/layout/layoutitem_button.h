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

#ifndef GLOM_DATASTRUCTURE_LAYOUTITEM_BUTTON_H
#define GLOM_DATASTRUCTURE_LAYOUTITEM_BUTTON_H

#include  <libglom/data_structure/layout/layoutitem_withformatting.h>

namespace Glom
{

class LayoutItem_Button 
 : public LayoutItem_WithFormatting
{
public:

  LayoutItem_Button();
  LayoutItem_Button(const LayoutItem_Button& src);
  LayoutItem_Button(LayoutItem_Button&& src) = delete;
  LayoutItem_Button& operator=(const LayoutItem_Button& src);
  LayoutItem_Button& operator=(LayoutItem_Button&& src) = delete;
  virtual ~LayoutItem_Button();

  virtual LayoutItem* clone() const;

  bool operator==(const LayoutItem_Button& src) const;

  virtual Glib::ustring get_part_type_name() const;

  /** Set the python code that will be executed when the button is pressed.
   */
  Glib::ustring get_script() const;

  bool get_has_script() const;

  /** Get the python code that will be executed when the button is pressed.
   */
  void set_script(const Glib::ustring& script);

private:
  Glib::ustring m_script;
};

} //namespace Glom

#endif //GLOM_DATASTRUCTURE_LAYOUTITEM_BUTTON_H



