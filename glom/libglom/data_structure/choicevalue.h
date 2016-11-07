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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#ifndef GLOM_DATASTRUCTURE_CHOICEVALUE_H
#define GLOM_DATASTRUCTURE_CHOICEVALUE_H

#include <libglom/data_structure/translatable_item.h>
#include <libgdamm/value.h>

namespace Glom
{


/** A value of a custom choice, for a field or a layout item.
 * This is translatable, but that only make sense for text fields.
 *
 * Text-specific methods such as get/set_title() should be ignored.
 */
class ChoiceValue : public TranslatableItem
{
public:

  ChoiceValue();
  ChoiceValue(const ChoiceValue& src) = default;
  ChoiceValue(ChoiceValue&& src) = delete;

  ChoiceValue& operator=(const ChoiceValue& src) = default;
  ChoiceValue& operator=(ChoiceValue&& src) = delete;

  bool operator==(const ChoiceValue& src) const;
  bool operator!=(const ChoiceValue& src) const;

  ChoiceValue* clone() const;

  void set_value(const Gnome::Gda::Value& value);
  Gnome::Gda::Value get_value() const;

  /** This override makes sure that we can generically use
   * ChoiceValue like any other TranslatableItem,
   * assuming that the value is a title,
   * if it is a text value.
   */
  Glib::ustring get_title_original() const noexcept override;

  /** Whether the value is of a type that can be translated.
   * This means that it must be a text type.
   */
  bool is_translatable() const;


private:
  Gnome::Gda::Value m_value;
};

} //namespace Glom

#endif //GLOM_DATASTRUCTURE_CHOICEVALUE_H

