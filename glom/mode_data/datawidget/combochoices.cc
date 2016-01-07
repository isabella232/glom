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

#include "combochoices.h"
#include <libglom/data_structure/glomconversions.h>
#include <libglom/document/document.h>
#include <libglom/connectionpool.h>
#include <libglom/utils.h>
#include <glibmm/i18n.h>
//#include <sstream> //For stringstream

#include <locale>     // for locale, time_put
#include <ctime>     // for struct tm
#include <iostream>   // for cout, endl


namespace Glom
{

namespace DataWidgetChildren
{

ComboChoices::ComboChoices()
{
}

bool ComboChoices::refresh_data_from_database_with_foreign_key(const Document* /* document */, const Gnome::Gda::Value& /* foreign_key_value */)
{
  /** TODO:
  auto layout_item =
    std::dynamic_pointer_cast<LayoutItem_Field>(get_layout_item());

  if(!layout_item || Conversions::value_is_empty(foreign_key_value))
  {
    //Clear the choices list:
    type_list_values_with_second list_values;
    set_choices_with_second(list_values);
    return true;
  }

  const auto list_values = Utils::get_choice_values(document, layout_item, foreign_key_value);
  const auto old_value = get_value();
  set_choices_with_second(list_values);
  set_value(old_value); //Try to preserve the value, even in iter-based ComboBoxes.

  */
  return true;
}

void ComboChoices::set_choices_related(const Document* /* document */, const std::shared_ptr<const LayoutItem_Field>& /* layout_field */, const Gnome::Gda::Value& /* foreign_key_value */)
{
  /* TODO:
  type_list_values_with_second list_values;

  auto layout_item =
    std::dynamic_pointer_cast<LayoutItem_Field>(get_layout_item());
  if(layout_item)
  {
    bool choice_show_all = false;
    const auto choice_relationship =
      layout_item->get_formatting_used().get_choices_related_relationship(choice_show_all);

    //Set the values now because if it will be the same regardless of the foreign key value.
    //Otherwise show them when refresh_data_from_database_with_foreign_key() is called.
    if(choice_relationship && choice_show_all)
    {
      list_values = Utils::get_choice_values_all(document, layout_item);
    }
  }

  const auto old_value = get_value();
  set_choices_with_second(list_values);
  set_value(old_value); //Try to preserve the value, even in iter-based ComboBoxes.
  */
}

} //namespace DataWidgetChildren
} //namespace Glom
