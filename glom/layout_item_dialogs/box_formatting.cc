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

#include "box_formatting.h"
#include <glom/libglom/data_structure/glomconversions.h>
#include <glibmm/i18n.h>

namespace Glom
{

Box_Formatting::Box_Formatting(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::VBox(cobject),
  m_frame_numeric_format(0),
  m_frame_text_format(0),
  m_checkbox_format_text_multiline(0),
  m_spinbutton_format_text_multiline_height(0),
  m_adddel_choices_custom(0),
  m_col_index_custom_choices(0)
{
  //Numeric formatting:
  refGlade->get_widget("frame_numeric_format", m_frame_numeric_format);
  refGlade->get_widget("checkbutton_format_thousands", m_checkbox_format_use_thousands);
  refGlade->get_widget("checkbutton_format_use_decimal_places", m_checkbox_format_use_decimal_places);
  refGlade->get_widget("entry_format_decimal_places", m_entry_format_decimal_places);
  refGlade->get_widget_derived("entry_currency_symbol", m_entry_currency_symbol);

  //Text formatting:
  refGlade->get_widget("frame_text_format", m_frame_text_format);
  refGlade->get_widget("checkbutton_format_text_multiline", m_checkbox_format_text_multiline);

  refGlade->get_widget("spinbutton_format_text_multiline_height", m_spinbutton_format_text_multiline_height);

  refGlade->get_widget_derived("adddel_choices", m_adddel_choices_custom);
  m_col_index_custom_choices = m_adddel_choices_custom->add_column("Choices");
  m_adddel_choices_custom->set_allow_add();
  m_adddel_choices_custom->set_allow_delete();
  m_adddel_choices_custom->set_auto_add();

  refGlade->get_widget("checkbutton_choices_restrict", m_checkbutton_choices_restricted);

  refGlade->get_widget_derived("combobox_choices_related_relationship", m_combo_choices_relationship);
  refGlade->get_widget_derived("combobox_choices_related_field", m_combo_choices_field);
  refGlade->get_widget_derived("combobox_choices_related_field_second", m_combo_choices_field_second);
  refGlade->get_widget("radiobutton_choices_custom", m_radiobutton_choices_custom);
  refGlade->get_widget("radiobutton_choices_related", m_radiobutton_choices_related);

  m_combo_choices_relationship->signal_changed().connect(sigc::mem_fun(*this, &Box_Formatting::on_combo_choices_relationship_changed));

  show_all_children();
}

Box_Formatting::~Box_Formatting()
{
}

void Box_Formatting::set_formatting(const FieldFormatting& format, const Glib::ustring& table_name, const sharedptr<const Field>& field)
{
  m_format = format;
  m_table_name = table_name;
  m_field = field;

  m_checkbox_format_use_thousands->set_active( format.m_numeric_format.m_use_thousands_separator );
  m_checkbox_format_use_decimal_places->set_active( format.m_numeric_format.m_decimal_places_restricted );

  char pchText[10] = {0};
  sprintf(pchText, "%d", format.m_numeric_format.m_decimal_places);
  m_entry_format_decimal_places->set_text(Glib::ustring(pchText));

  m_entry_currency_symbol->get_entry()->set_text(format.m_numeric_format.m_currency_symbol);

  m_checkbox_format_text_multiline->set_active(format.get_text_format_multiline());
  m_checkbox_format_text_multiline->signal_toggled().connect( sigc::mem_fun(*this, &Box_Formatting::on_checkbox_text_multiline) );

  m_spinbutton_format_text_multiline_height->set_value(format.get_text_format_multiline_height_lines());

  //Choices:
  m_checkbutton_choices_restricted->set_active(format.get_choices_restricted());

  const Document_Glom* document = get_document();

  //Fill the list of relationships:
  const Document_Glom::type_vecRelationships vecRelationships = document->get_relationships(m_table_name);
  m_combo_choices_relationship->set_relationships(vecRelationships);

  sharedptr<Relationship> choices_relationship;
  Glib::ustring choices_field, choices_field_second;
  format.get_choices(choices_relationship, choices_field, choices_field_second);

  m_combo_choices_relationship->set_selected_relationship(choices_relationship);
  on_combo_choices_relationship_changed(); //Fill the combos so we can set their active items.
  m_combo_choices_field->set_selected_field(choices_field);
  m_combo_choices_field_second->set_selected_field(choices_field_second);

  //Custom choices:
  m_adddel_choices_custom->remove_all();
  FieldFormatting::type_list_values list_choice_values = format.get_choices_custom();
  for(FieldFormatting::type_list_values::const_iterator iter = list_choice_values.begin(); iter != list_choice_values.end(); ++iter)
  {
    //Display the value in the choices list as it would be displayed in the format:
    const Glib::ustring value_text = Conversions::get_text_for_gda_value(field->get_glom_type(), *iter, format.m_numeric_format);
    Gtk::TreeModel::iterator iter = m_adddel_choices_custom->add_item(value_text);
    m_adddel_choices_custom->set_value(iter, m_col_index_custom_choices, value_text);
  }

  m_radiobutton_choices_custom->set_active(format.get_has_custom_choices());
  m_radiobutton_choices_related->set_active(format.get_has_related_choices());

  enforce_constraints();
}

bool Box_Formatting::get_formatting(FieldFormatting& format) const
{
  //Numeric Formatting:
  m_format.m_numeric_format.m_use_thousands_separator = m_checkbox_format_use_thousands->get_active();
  m_format.m_numeric_format.m_decimal_places_restricted = m_checkbox_format_use_decimal_places->get_active();

  const Glib::ustring strDecPlaces = m_entry_format_decimal_places->get_text();
  m_format.m_numeric_format.m_decimal_places = atoi(strDecPlaces.c_str());

  m_format.m_numeric_format.m_currency_symbol = m_entry_currency_symbol->get_entry()->get_text();

  //Text formatting:
  m_format.set_text_format_multiline(m_checkbox_format_text_multiline->get_active());
  m_format.set_text_format_multiline_height_lines( m_spinbutton_format_text_multiline_height->get_value_as_int() );

  //Choices:
  m_format.set_choices_restricted(m_checkbutton_choices_restricted->get_active());

  sharedptr<Relationship> choices_relationship = m_combo_choices_relationship->get_selected_relationship();
  m_format.set_choices(choices_relationship,
    m_combo_choices_field->get_selected_field_name(),
    m_combo_choices_field_second->get_selected_field_name() );

  //Custom choices:
  FieldFormatting::type_list_values list_choice_values;
  Glib::RefPtr<Gtk::TreeModel> choices_model = m_adddel_choices_custom->get_model();
  if(choices_model)
  {
    for(Gtk::TreeModel::iterator iter = choices_model->children().begin(); iter != choices_model->children().end(); ++iter)
    {
      const Glib::ustring text = m_adddel_choices_custom->get_value(iter, m_col_index_custom_choices);
      if(!text.empty())
      {
        bool success = false;
        Gnome::Gda::Value value = Conversions::parse_value(m_field->get_glom_type(), text, m_format.m_numeric_format, success);

        if(success)
          list_choice_values.push_back(value);
      }
    }
  }

  m_format.set_choices_custom(list_choice_values);
  m_format.set_has_custom_choices(m_radiobutton_choices_custom->get_active());
  m_format.set_has_related_choices(m_radiobutton_choices_related->get_active());

  format = m_format;
  return true;
}

void Box_Formatting::on_combo_choices_relationship_changed()
{
  sharedptr<Relationship> relationship = m_combo_choices_relationship->get_selected_relationship();

  Document_Glom* pDocument = get_document();
  if(pDocument)
  {
    //Show the list of formats from this relationship:
    if(relationship)
    {
      Document_Glom::type_vecFields vecFields = pDocument->get_table_fields(relationship->get_to_table());

      m_combo_choices_field->set_fields(vecFields);
      m_combo_choices_field_second->set_fields(vecFields, true /* with_none_item */); //We add a "None" item so this GtkComboBox can be cleared by the user.
    }
  }
}

void Box_Formatting::enforce_constraints()
{
  //Hide inappropriate UI:
  const bool is_numeric = (m_field->get_glom_type() == Field::TYPE_NUMERIC);
  if(is_numeric)
    m_frame_numeric_format->show();
  else
  {
    m_frame_numeric_format->hide();
  }

  const bool is_text = (m_field->get_glom_type() == Field::TYPE_TEXT);
  if(is_text)
  {
    m_frame_text_format->show();

    //Do not allow the user to specify a text height if it is not going to be multiline:
    const bool text_height_sensitive = m_checkbox_format_text_multiline->get_active();
    m_spinbutton_format_text_multiline_height->set_sensitive(text_height_sensitive);
  }
  else
    m_frame_text_format->hide();
}

void Box_Formatting::on_checkbox_text_multiline()
{
  enforce_constraints();
}

} //namespace Glom
