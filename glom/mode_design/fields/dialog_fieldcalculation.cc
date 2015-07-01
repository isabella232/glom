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


#include "dialog_fieldcalculation.h"
#include "../../box_db_table.h"
#include <glom/frame_glom.h>
#include <glom/python_embed/glom_python.h>
#include <glom/utils_ui.h>
#include <glom/appwindow.h>
#include <libglom/data_structure/glomconversions.h>
#include <gtksourceviewmm/languagemanager.h>

//#include <libgnome/gnome-i18n.h>
#include <glibmm/i18n.h>

namespace Glom
{

const char* Dialog_FieldCalculation::glade_id("window_field_calculation");
const bool Dialog_FieldCalculation::glade_developer(true);

Dialog_FieldCalculation::Dialog_FieldCalculation(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Dialog(cobject)
{
  builder->get_widget("textview_calculation",  m_text_view);
  builder->get_widget("button_test",  m_button_test);
  builder->get_widget("label_triggered_by", m_label_triggered_by);

  m_button_test->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_FieldCalculation::on_button_test) );
  //connect_each_widget(this);

  if(m_text_view)
  {
    m_text_view->set_highlight_current_line(true);

    Glib::RefPtr<Gsv::LanguageManager> languages_manager = Gsv::LanguageManager::get_default();

    Glib::RefPtr<Gsv::Language> language = languages_manager->get_language("python"); //This is the GtkSourceView language ID.
    if(language)
    {
       //Createa a new buffer and set it, instead of getting the default buffer, in case libglade has tried to set it, using the wrong buffer type:
       Glib::RefPtr<Gsv::Buffer> buffer = Gsv::Buffer::create(language);
       buffer->set_highlight_syntax();
       m_text_view->set_buffer(buffer);
    }
  }

  //Dialog_Properties::set_modified(false);

  show_all_children();
}

Dialog_FieldCalculation::~Dialog_FieldCalculation()
{
}

void Dialog_FieldCalculation::set_field(const std::shared_ptr<const Field>& field, const Glib::ustring& table_name)
{
  //set_blocked();

  m_field = glom_sharedptr_clone(field); //Remember it so we save any details that are not in our UI.
  m_table_name = table_name;  //Used for lookup combo boxes.

  m_text_view->get_buffer()->set_text( field->get_calculation() );

  //set_blocked(false);


  //Dialog_Properties::set_modified(false);
}

std::shared_ptr<Field> Dialog_FieldCalculation::get_field() const
{
  std::shared_ptr<Field> field = glom_sharedptr_clone(m_field); //Start with the old details, to preserve anything that is not in our UI.

  field->set_calculation( m_text_view->get_buffer()->get_text() );

  return field;
}

bool Dialog_FieldCalculation::check_for_return_statement(const Glib::ustring& calculation)
{
  if(calculation.find("return") == Glib::ustring::npos)
  {
     Frame_Glom::show_ok_dialog(_("Calculation Error"), _("The calculation does not have a return statement."), *this);
     return false;
  }

  return true;
}

void Dialog_FieldCalculation::on_button_test()
{
  const auto calculation = m_text_view->get_buffer()->get_text();
  if(!check_for_return_statement(calculation))
    return;

  if(!UiUtils::script_check_for_pygtk2_with_warning(calculation, this))
    return;

  type_map_fields field_values;

  Document* document = get_document();
  if(document)
  {
    const auto fields = document->get_table_fields(m_table_name);
    for(Document::type_vec_fields::const_iterator iter = fields.begin(); iter != fields.end(); ++iter)
    {
      const std::shared_ptr<const Field> field = *iter;
      const auto example_value = Conversions::get_example_value(field->get_glom_type());
      field_values[field->get_name()] = example_value;
    }
  }

  //We need the connection when we run the script, so that the script may use it.
  std::shared_ptr<SharedConnection> sharedconnection = connect_to_server(this /* parent window */);

  Glib::ustring error_message;
  const auto value = glom_evaluate_python_function_implementation(
    Field::TYPE_TEXT,
    calculation,
    field_values, //TODO: Maybe use the field's type here.
    document,
    m_table_name,
    std::shared_ptr<Field>(), Gnome::Gda::Value(), // primary key - only used when setting values in the DB.
    sharedconnection->get_gda_connection(),
    error_message);

  if(error_message.empty())
    UiUtils::show_ok_dialog(_("Calculation result"), Glib::ustring::compose(_("The result of the calculation is:\n%1"), value.to_string()), *this, Gtk::MESSAGE_INFO);
  else
    UiUtils::show_ok_dialog( _("Calculation failed"), Glib::ustring::compose(_("The calculation failed with this error:\n%s"), error_message), *this, Gtk::MESSAGE_ERROR);

  //Show what fields would trigger the recalculation:
  std::shared_ptr<Field> temp = std::make_shared<Field>();
  temp->set_calculation(calculation);
  std::shared_ptr<LayoutItem_Field> layoutitem_temp = std::make_shared<LayoutItem_Field>();
  layoutitem_temp->set_full_field_details(temp);
  const auto triggered_fields = get_calculation_fields(m_table_name, layoutitem_temp);

  Glib::ustring field_names;
  for(type_list_const_field_items::const_iterator iter = triggered_fields.begin(); iter != triggered_fields.end(); ++iter)
  {
    field_names += ( (*iter)->get_layout_display_name() + ", " );
  }

  const auto triggered_relationships = temp->get_calculation_relationships();

  for(Field::type_list_strings::const_iterator iter = triggered_relationships.begin(); iter != triggered_relationships.end(); ++iter)
  {
    field_names += ( "related(" + *iter + "), " );
  }

  m_label_triggered_by->set_text(field_names);
}

} //namespace Glom
