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


#include "dialog_fieldcalculation.h"
#include "../../box_db_table.h"
#include "../../frame_glom.h"
#include "../../glom_python.h"

//#include <libgnome/gnome-i18n.h>
#include <libintl.h>

Dialog_FieldCalculation::Dialog_FieldCalculation(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::Dialog(cobject)
{
  refGlade->get_widget("textview_calculation",  m_text_view);
  refGlade->get_widget("button_test",  m_button_test);  

  m_button_test->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_FieldCalculation::on_button_test) );
  //on_foreach_connect(*this);

  //Dialog_Properties::set_modified(false);

  show_all_children();
}

Dialog_FieldCalculation::~Dialog_FieldCalculation()
{
}

void Dialog_FieldCalculation::set_field(const Field& field, const Glib::ustring& table_name)
{
  //set_blocked();

  m_field = field; //Remember it so we save any details that are not in our UI.
  m_table_name = table_name;  //Used for lookup combo boxes.

  m_text_view->get_buffer()->set_text( field.get_calculation() );

  //set_blocked(false);


  //Dialog_Properties::set_modified(false);
}

Field Dialog_FieldCalculation::get_field() const
{
  Field field = m_field; //Start with the old details, to preserve anything that is not in our UI.

  field.set_calculation( m_text_view->get_buffer()->get_text() );

  return field;
}

void Dialog_FieldCalculation::on_button_test()
{
  const Glib::ustring calculation = m_text_view->get_buffer()->get_text();
  Glib::ustring result = glom_evaluate_python_function_implementation(calculation);

  Frame_Glom::show_ok_dialog(gettext("Calculation result"), gettext("The result of the calculation is: ") + result, *this);
}




