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
#include "../../python_embed/glom_python.h"
#include <glom/libglom/data_structure/glomconversions.h>

//#include <libgnome/gnome-i18n.h>
#include <glibmm/i18n.h>

namespace Glom
{

Dialog_FieldCalculation::Dialog_FieldCalculation(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::Dialog(cobject)
{
  refGlade->get_widget("textview_calculation",  m_text_view);
  refGlade->get_widget("button_test",  m_button_test);
  refGlade->get_widget("label_triggered_by", m_label_triggered_by);

  m_button_test->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_FieldCalculation::on_button_test) );
  //on_foreach_connect(*this);

  //Dialog_Properties::set_modified(false);

  show_all_children();
}

Dialog_FieldCalculation::~Dialog_FieldCalculation()
{
}

void Dialog_FieldCalculation::set_field(const sharedptr<const Field>& field, const Glib::ustring& table_name)
{
  //set_blocked();

  m_field = glom_sharedptr_clone(field); //Remember it so we save any details that are not in our UI.
  m_table_name = table_name;  //Used for lookup combo boxes.

  m_text_view->get_buffer()->set_text( field->get_calculation() );

  //set_blocked(false);


  //Dialog_Properties::set_modified(false);
}

sharedptr<Field> Dialog_FieldCalculation::get_field() const
{
  sharedptr<Field> field = glom_sharedptr_clone(m_field); //Start with the old details, to preserve anything that is not in our UI.

  field->set_calculation( m_text_view->get_buffer()->get_text() );

  return field;
}

void Dialog_FieldCalculation::on_button_test()
{
  const Glib::ustring calculation = m_text_view->get_buffer()->get_text();

  type_map_fields field_values;

  Document_Glom* document = get_document();
  if(document)
  {
    const Document_Glom::type_vecFields fields = document->get_table_fields(m_table_name);
    for(Document_Glom::type_vecFields::const_iterator iter = fields.begin(); iter != fields.end(); ++iter)
    {
      const sharedptr<const Field> field = *iter;
      const Gnome::Gda::Value example_value = GlomConversions::get_example_value(field->get_glom_type());
      field_values[field->get_name()] = example_value;
    }
  }

  Gnome::Gda::Value value = glom_evaluate_python_function_implementation(Field::TYPE_TEXT, calculation, field_values, //TODO: Maybe use the field's type here.
    document, m_table_name);

  Frame_Glom::show_ok_dialog(_("Calculation result"), _("The result of the calculation is:\n") + value.to_string(), *this);


  //Show what fields would trigger the recalculation:
  sharedptr<Field> temp = sharedptr<Field>::create();
  temp->set_calculation(calculation);
  sharedptr<LayoutItem_Field> layoutitem_temp = sharedptr<LayoutItem_Field>::create();
  layoutitem_temp->set_full_field_details(temp);
  const type_list_const_field_items triggered_fields = get_calculation_fields(m_table_name, layoutitem_temp);

  Glib::ustring field_names;
  for(type_list_const_field_items::const_iterator iter = triggered_fields.begin(); iter != triggered_fields.end(); ++iter)
  {
    field_names += ( (*iter)->get_layout_display_name() + ", " );
  }

  const Field::type_list_strings triggered_relationships = temp->get_calculation_relationships();

  for(Field::type_list_strings::const_iterator iter = triggered_relationships.begin(); iter != triggered_relationships.end(); ++iter)
  {
    field_names += ( "related(" + *iter + "), " );
  }

  m_label_triggered_by->set_text(field_names);
}

} //namespace Glom



