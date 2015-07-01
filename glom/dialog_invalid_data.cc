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
 
#include "dialog_invalid_data.h"
#include <libglom/data_structure/glomconversions.h>
#include <glom/utils_ui.h>
#include <glom/glade_utils.h>

namespace Glom
{

const char Dialog_InvalidData::glade_id[] = "dialog_data_invalid_format";
const bool Dialog_InvalidData::glade_developer(false);

/** Show the dialog.
 * @result true if the data in the field should be reverted.
 */
bool glom_show_dialog_invalid_data(Field::glom_field_type glom_type)
{
  Dialog_InvalidData* dialog = 0;
  Utils::get_glade_widget_derived_with_warning(dialog);
  if(!dialog) //Unlikely and it already warns on stderr.
    return false;
    
  dialog->set_example_data(glom_type);
  //dialog->set_transient_for(*this);
  const auto response = Glom::UiUtils::dialog_run_with_help(dialog);
  
  delete dialog;
  return (response == 2); //The glade file has a response of 2 for the Revert button.
}

Dialog_InvalidData::Dialog_InvalidData(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Dialog(cobject),
  m_label(0)
{
  builder->get_widget("label_example_data", m_label);
}

Dialog_InvalidData::~Dialog_InvalidData()
{
}

void Dialog_InvalidData::set_example_data(Field::glom_field_type glom_type)
{
  Glib::ustring example_text;
  switch(glom_type)
  {
    case(Field::TYPE_DATE):
    {
      Glib::Date date(31, Glib::Date::JANUARY, 2005);
      example_text = Conversions::get_text_for_gda_value(glom_type, Gnome::Gda::Value(date));
      break;
    }
    case(Field::TYPE_TIME):
    {
      Gnome::Gda::Time time = {0, 0, 0, 0, 0};
      time.hour = 13;
      time.minute = 02;
      time.second = 03;
      example_text = Conversions::get_text_for_gda_value(glom_type, Gnome::Gda::Value(time));
      break;
    }
    case(Field::TYPE_NUMERIC):
    {
      Gnome::Gda::Value gda_value;
      gda_value.set_double(12345678.91L);
      example_text = Conversions::get_text_for_gda_value(glom_type, gda_value);
      break;
    }
    case(Field::TYPE_TEXT):
    default:
    {
      example_text = "Abcdefghi jklmnopq lmnopqr";
      break;
    }
  }

  if(m_label)
    m_label->set_text(example_text);
}

} //namespace Glom
