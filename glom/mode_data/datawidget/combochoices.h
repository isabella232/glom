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

#ifndef GLOM_UTILITY_WIDGETS_COMBO_CHOICES_H
#define GLOM_UTILITY_WIDGETS_COMBO_CHOICES_H

#include <gtkmm.h>
#include <libglom/data_structure/field.h>
#include <glom/utility_widgets/layoutwidgetfield.h>

namespace Glom
{

/** A polymorphic base class for all the combo-like widgets.
 */
class ComboChoices : public LayoutWidgetField
{
public:
  ///You must call set_layout_item() to specify the field type and formatting of the main column.
  explicit ComboChoices();

  ///You must call set_layout_item() to specify the field type and formatting of the main column.
  explicit ComboChoices(const sharedptr<LayoutItem_Field>& field_second);

  virtual ~ComboChoices();

  virtual void set_choices(const FieldFormatting::type_list_values& list_values) = 0;

  typedef std::list< std::pair<Gnome::Gda::Value, Gnome::Gda::Value> > type_list_values_with_second;
  virtual void set_choices_with_second(const type_list_values_with_second& list_values) = 0;

protected:
  void init();

  bool m_with_second;
  sharedptr<const LayoutItem_Field> m_layoutitem_second;
  //Gnome::Gda::Value m_value; //The last-stored value. We have this because the displayed value might be unparseable.
};

} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_COMBO_CHOICES_H

