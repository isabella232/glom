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

#include "dialog_fielddefinition.h"
#include "dialog_fieldcalculation.h"
#include <glom/glade_utils.h>
#include <glom/utils_ui.h>
#include <glom/appwindow.h>
#include "../../box_db_table.h"
#include <libglom/db_utils.h>
//#include <libgnome/gnome-i18n.h>
#include <glibmm/i18n.h>

namespace Glom
{

const char* Dialog_FieldDefinition::glade_id("window_field_definition_edit");
const bool Dialog_FieldDefinition::glade_developer(true);

Dialog_FieldDefinition::Dialog_FieldDefinition(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Dialog_Properties(cobject, builder),
  m_pDataWidget_DefaultValueSimple(nullptr)
{
  builder->get_widget_derived("combobox_type", m_pCombo_Type);

  builder->get_widget("entry_name", m_pEntry_Name);
  builder->get_widget("entry_title", m_pEntry_Title);

  builder->get_widget("checkbutton_unique",  m_pCheck_Unique);
  builder->get_widget("checkbutton_primarykey",  m_pCheck_PrimaryKey);
  builder->get_widget("checkbutton_autoincrement",  m_pCheck_AutoIncrement);

  builder->get_widget("hbox_default_value_simple",  m_pBox_DefaultValueSimple);

  builder->get_widget("box_default_value",  m_pBox_ValueTab);

  builder->get_widget("radiobutton_userentry",  m_pRadio_UserEntry);
  builder->get_widget("alignment_userentry",  m_pAlignment_UserEntry);

  builder->get_widget("checkbutton_lookup",  m_pCheck_Lookup);
  builder->get_widget("table_lookup",  m_pTable_Lookup);
  builder->get_widget_derived("combobox_lookup_relationship",  m_pCombo_LookupRelationship);
  builder->get_widget("combobox_lookup_field",  m_pCombo_LookupField);

  builder->get_widget("radiobutton_calculate",  m_pRadio_Calculate);
  builder->get_widget("alignment_calculate",  m_pAlignment_Calculate);
  builder->get_widget("textview_calculate",  m_pTextView_Calculation);
  builder->get_widget("button_edit_calculation",  m_pButton_EditCalculation);

  //Connect signals:
  if(m_pCombo_Type)
  m_pCombo_Type->signal_changed().connect( sigc::mem_fun(*this, &Dialog_FieldDefinition::on_combo_type_changed) );

  if(m_pCombo_LookupRelationship)
    m_pCombo_LookupRelationship->signal_changed().connect( sigc::mem_fun(*this, &Dialog_FieldDefinition::on_combo_lookup_relationship_changed) );

  if(m_pCheck_Lookup)
    m_pCheck_Lookup->signal_toggled().connect( sigc::mem_fun(*this, &Dialog_FieldDefinition::on_check_lookup_toggled) );

  if(m_pRadio_Calculate)
    m_pRadio_Calculate->signal_toggled().connect( sigc::mem_fun(*this, &Dialog_FieldDefinition::on_radio_calculate_toggled) );

  if(m_pRadio_UserEntry)
    m_pRadio_UserEntry->signal_toggled().connect( sigc::mem_fun(*this, &Dialog_FieldDefinition::on_radio_userentry_toggled) );

  if(m_pButton_EditCalculation)
    m_pButton_EditCalculation->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_FieldDefinition::on_button_edit_calculation) );

  //TODO:
  //Connect every widget to on_anything_changed():
  //foreach_( (GtkCallback)(&on_foreach), this); //Crashes several levels down.

  //on_anything_changed(); //In the meantime, we'll just start with [Apply] already enabled.

  //Make sure that the correct Type Details are showing:
  on_combo_type_changed();


  connect_each_widget(this);
  connect_each_widget(m_pBox_DefaultValueSimple);
  connect_each_widget(m_pBox_ValueTab);

  Dialog_Properties::set_modified(false);

  //This is mostly just to exercise some widgets during simple tests:
  enforce_constraints();

  show_all_children();
}

Dialog_FieldDefinition::~Dialog_FieldDefinition()
{
}

void Dialog_FieldDefinition::set_field(const std::shared_ptr<const Field>& field, const Glib::ustring& table_name)
{
  set_blocked();

  m_Field = glom_sharedptr_clone(field); //Remember it so we save any details that are not in our UI.
  m_table_name = table_name;  //Used for lookup combo boxes.

  //Set the Widgets from the field info:
  //const Glib::RefPtr<Gnome::Gda::Column>& fieldInfo = field->get_field_info();

  m_pEntry_Name->set_text(field->get_name());
  m_pCombo_Type->set_field_type( field->get_glom_type() );

  m_pCheck_Unique->set_active(field->get_unique_key());
  m_pCheck_PrimaryKey->set_active(field->get_primary_key());
  m_pCheck_AutoIncrement->set_active(field->get_auto_increment());

  //Glom-specific details:

  //Default value
  bool disable_default_value = false;
  if(field->get_auto_increment()) //Ignore default_values for auto_increment fields - it's just some obscure postgres code.
    disable_default_value = true;

  //Default value: simple:
  Gnome::Gda::Value default_value;
  if(!disable_default_value)
    default_value = m_Field->get_default_value();

  //Create an appropriate DataWidget for the default value:
  delete m_pDataWidget_DefaultValueSimple;
  m_pDataWidget_DefaultValueSimple = nullptr;

  //We use a regular DataWidget for the default value, so we can reuse its functionality,
  //but it's not a real field - hence the special title.
  std::shared_ptr<LayoutItem_Field> layout_item = std::make_shared<LayoutItem_Field>();
  std::shared_ptr<Field> field_default_value = glom_sharedptr_clone(m_Field);
  field_default_value->set_name("glom_temp_default_value");
  field_default_value->set_title_original(_("Default Value"));
  layout_item->set_full_field_details(field_default_value);
  m_pDataWidget_DefaultValueSimple = Gtk::manage( new DataWidget(layout_item, "", get_document()) );
  if(!m_pDataWidget_DefaultValueSimple->get_data_child_widget())
    std::cerr << G_STRFUNC << ": The DataWidget did not create a child widget." << std::endl;

  connect_each_widget(m_pDataWidget_DefaultValueSimple);

  Gtk::Label* pLabel = m_pDataWidget_DefaultValueSimple->get_label();
  if(!pLabel->get_text().empty())
  {
    pLabel->set_valign(Gtk::ALIGN_START); //Because the widget might be multiline.
    m_pBox_DefaultValueSimple->pack_start(*pLabel, Gtk::PACK_SHRINK);
  }

  m_pBox_DefaultValueSimple->pack_end(*m_pDataWidget_DefaultValueSimple, Gtk::PACK_EXPAND_WIDGET);
  m_pDataWidget_DefaultValueSimple->set_value(default_value);
  m_pDataWidget_DefaultValueSimple->show();

  //Default value: lookup:

  m_pCheck_Lookup->set_active(m_Field->get_is_lookup());
  on_check_lookup_toggled();

  //Fill the lookup relationships combo:
  Document* document = dynamic_cast<Document*>(get_document());
  if(document)
  {
    //Get the relationships used by this table, excluding relationships triggered
    //by this field itself (to avoid circular self-lookups):
    const auto vecRelationships = document->get_relationships_excluding_triggered_by(table_name, m_Field->get_name());
    m_pCombo_LookupRelationship->set_relationships(vecRelationships);
  }

  std::shared_ptr<Relationship> lookup_relationship;
  if(!disable_default_value)
    lookup_relationship = m_Field->get_lookup_relationship();

  m_pCombo_LookupRelationship->set_selected_relationship(lookup_relationship);
  on_combo_lookup_relationship_changed(); //Put the correct list of fields in the fields combo.

  Glib::ustring lookup_field_name;
  if(!disable_default_value)
    lookup_field_name = m_Field->get_lookup_field();
  m_pCombo_LookupField->set_active_text(lookup_field_name);


  //Calculation:
  const auto calculation = field->get_calculation();
  if(calculation.empty())
   m_pRadio_UserEntry->set_active();
  else
    m_pRadio_Calculate->set_active();

  on_check_lookup_toggled();

  //std::cout << "debug: dialog_fielddefinition.c:: m_pTextView_Calculation.gobj() gtype = " << G_OBJECT_TYPE_NAME(m_pTextView_Calculation->gobj()) << std::endl;
  m_pTextView_Calculation->get_buffer()->set_text(calculation);
  //std::cout << "  debug: dialog_fielddefinition.c:: after get_buffer()" << std::endl;

  m_pEntry_Title->set_text(item_get_title(field));

  set_blocked(false);

  enforce_constraints();
  Dialog_Properties::set_modified(false);
}

std::shared_ptr<Field> Dialog_FieldDefinition::get_field() const
{
  std::shared_ptr<Field> field = glom_sharedptr_clone(m_Field); //Start with the old details, to preserve anything that is not in our UI.
  // const_cast is necessary and save here for the window (jhs)
  std::shared_ptr<SharedConnection> sharedcnc = connect_to_server(const_cast<Dialog_FieldDefinition*>(this));
  Glib::RefPtr<Gnome::Gda::Connection> cnc = sharedcnc->get_gda_connection();

  //Get the field info from the widgets:

  Glib::RefPtr<Gnome::Gda::Column> fieldInfo = field->get_field_info(); //Preserve previous information.

  fieldInfo->set_name(m_pEntry_Name->get_text());

  fieldInfo->set_g_type( Field::get_gda_type_for_glom_type( m_pCombo_Type->get_field_type() ) );

  fieldInfo->set_auto_increment(m_pCheck_AutoIncrement->get_active());

  if(!fieldInfo->get_auto_increment()) //Ignore default_values for auto_increment fields - it's just some obscure postgres code.
  {
    //Simple default value:
    fieldInfo->set_default_value( m_pDataWidget_DefaultValueSimple->get_value() );
  }

  //Lookup:
  const auto is_lookup = m_pCheck_Lookup->get_active();
  std::shared_ptr<Relationship> relationship;
  if(is_lookup)
    relationship = m_pCombo_LookupRelationship->get_selected_relationship();
  field->set_lookup_relationship(relationship);

  Glib::ustring lookup_field;
  if(is_lookup)
    lookup_field = m_pCombo_LookupField->get_active_text();
  field->set_lookup_field(lookup_field);


  //Calculation:
  if(m_pRadio_Calculate)
    field->set_calculation(m_pTextView_Calculation->get_buffer()->get_text());

  Glib::RefPtr<Gnome::Gda::Column> field_info_copy = fieldInfo;

  field->set_field_info(fieldInfo);

  //Glom-specific details:

  field->set_unique_key(m_pCheck_Unique->get_active());
  field->set_primary_key(m_pCheck_PrimaryKey->get_active());

  field->set_title(m_pEntry_Title->get_text(), AppWindow::get_current_locale());

  return field;
}


void Dialog_FieldDefinition::on_combo_type_changed()
{
  //Display appropriate Table of widgets for this type:

  //m_Frame_TypeDetails.remove();

  //Use FieldType static method to categorise field type:
 // glom_field_type fieldType = m_pCombo_Type->get_field_type();

}

void Dialog_FieldDefinition::enforce_constraints()
{
  if(m_pCheck_PrimaryKey->get_active())
  {
    m_pCheck_Unique->set_active(true); //Primary keys must be unique.
    m_pCheck_Unique->set_sensitive(false); //Stop the user from disagreeing with that.
  }
  else
    m_pCheck_Unique->set_sensitive(true);

  if(m_pCheck_Unique->get_active() || m_pCheck_AutoIncrement->get_active())
  {
    m_pBox_ValueTab->set_sensitive(false); //Disable all controls on the Notebook page.
    m_pDataWidget_DefaultValueSimple->set_value( Gnome::Gda::Value() ); //Unique fields cannot have default values. //TODO: People will be surprised when they lose information here. We should probably read the text as "" if the widget is disabled.
  }
  else
  {
    m_pBox_ValueTab->set_sensitive(true);
  }

  const auto enable_calc = m_pRadio_Calculate->get_active();
  m_pAlignment_Calculate->set_sensitive(enable_calc);

  const auto enable_userentry = m_pRadio_UserEntry->get_active();
  m_pAlignment_UserEntry->set_sensitive(enable_userentry);
}

void Dialog_FieldDefinition::on_check_lookup_toggled()
{
  bool enable = m_pCheck_Lookup->get_active();
  m_pTable_Lookup->set_sensitive(enable);

  //re-disable it if it was not meant to be enabled:
  enforce_constraints();
}

void Dialog_FieldDefinition::on_radio_calculate_toggled()
{
  bool enable = m_pRadio_Calculate->get_active();
  m_pAlignment_Calculate->set_sensitive(enable);

  //re-disable it if it was not meant to be enabled:
  enforce_constraints();
}

void Dialog_FieldDefinition::on_radio_userentry_toggled()
{
  bool enable = m_pRadio_UserEntry->get_active();
  m_pAlignment_UserEntry->set_sensitive(enable);

  //re-disable it if it was not meant to be enabled:
  enforce_constraints();
}


void Dialog_FieldDefinition::on_combo_lookup_relationship_changed()
{
  //Get the fields that are avaiable from the new relationship:

  m_pCombo_LookupField->remove_all();

  //Get the relationship name:
  std::shared_ptr<const Relationship> relationship = m_pCombo_LookupRelationship->get_selected_relationship();
  if(relationship)
  {
    //Get the relationship details:
    Document* document = dynamic_cast<Document*>(get_document());
    if(document)
    {
      const auto to_table = relationship->get_to_table();
      if(!to_table.empty())
      {
        //Get the fields in the other table, and add them to the combo:
        const auto fields_in_to_table = DbUtils::get_fields_for_table(document, to_table);
        for(const auto& field : fields_in_to_table)
        {
          m_pCombo_LookupField->append(field->get_name());
        }
      }
    }
  }

}

void Dialog_FieldDefinition::on_button_edit_calculation()
{
  //TODO: Share a global instance, to make this quicker?
  Dialog_FieldCalculation* dialog = nullptr;
  Utils::get_glade_widget_derived_with_warning(dialog);
  if(!dialog) //Unlikely and it already warns on stderr.
    return;

  add_view(dialog); //Give it access to the document.

  m_Field->set_calculation( m_pTextView_Calculation->get_buffer()->get_text() );
  dialog->set_field(m_Field, m_table_name);
  //TODO: dialog.set_transient_for(*get_app_window());
  const auto response = Glom::UiUtils::dialog_run_with_help(dialog);
  if(response == Gtk::RESPONSE_OK)
  {
    m_pTextView_Calculation->get_buffer()->set_text( dialog->get_field()->get_calculation() );
  }

  remove_view(dialog);
  delete dialog;
}

} //namespace Glom
