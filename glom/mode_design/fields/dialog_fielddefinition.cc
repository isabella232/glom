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

#include "dialog_fielddefinition.h"
#include "../../box_db_table.h"
//#include <libgnome/gnome-i18n.h>
#include <libintl.h>

Dialog_FieldDefinition::Dialog_FieldDefinition(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Dialog_Properties(cobject, refGlade)
{
  refGlade->get_widget_derived("combobox_type", m_pCombo_Type);

  refGlade->get_widget("entry_name", m_pEntry_Name);
  refGlade->get_widget("entry_title", m_pEntry_Title);
  
  refGlade->get_widget("checkbutton_unique",  m_pCheck_Unique);
  refGlade->get_widget("checkbutton_primarykey",  m_pCheck_PrimaryKey);
  refGlade->get_widget("checkbutton_autoincrement",  m_pCheck_AutoIncrement);  

  refGlade->get_widget("entry_default_value_simple",  m_pEntry_Default);
  refGlade->get_widget("label_default_value_simple",  m_pLabel_Default);

  refGlade->get_widget("box_default_value",  m_pBox_DefaultValue);
  refGlade->get_widget("checkbutton_lookup",  m_pCheck_Lookup);
  refGlade->get_widget("table_lookup",  m_pTable_Lookup);
  refGlade->get_widget_derived("combobox_lookup_relationship",  m_pCombo_LookupRelationship);
  refGlade->get_widget_derived("combobox_lookup_field",  m_pCombo_LookupField);


  //Connect signals:
  m_pCombo_Type->signal_changed().connect( sigc::mem_fun(*this, &Dialog_FieldDefinition::on_combo_type_changed) );
  m_pCombo_LookupRelationship->signal_changed().connect( sigc::mem_fun(*this, &Dialog_FieldDefinition::on_combo_lookup_relationship_changed) );
  m_pCheck_Lookup->signal_toggled().connect( sigc::mem_fun(*this, &Dialog_FieldDefinition::on_check_lookup_toggled) );

  //TODO:
  //Connect every widget to on_anything_changed():
  //foreachgettext( (GtkCallback)(&on_foreach), this); //Crashes several levels down.

  //on_anything_changed(); //In the meantime, we'll just start with [Apply] already enabled.

  //Make sure that the correct Type Details are showing:
  on_combo_type_changed();

  on_foreach_connect(*this);

  Dialog_Properties::set_modified(false);

  show_all_children();
}

Dialog_FieldDefinition::~Dialog_FieldDefinition()
{
}

void Dialog_FieldDefinition::set_field(const Field& field, const Glib::ustring& table_name)
{
  set_blocked();

  m_Field = field; //Remember it so we save any details that are not in our UI.
  m_table_name = table_name;  //Used for lookup combo boxes.

  //Set the Widgets from the field info:
  const Gnome::Gda::FieldAttributes& fieldInfo = field.get_field_info();

  m_pEntry_Name->set_text(fieldInfo.get_name());
  m_pCombo_Type->set_field_type( field.get_glom_type() );

  m_pCheck_Unique->set_active(fieldInfo.get_unique_key());
  m_pCheck_PrimaryKey->set_active(fieldInfo.get_primary_key());
  m_pCheck_AutoIncrement->set_active(fieldInfo.get_auto_increment());

  //Glom-specific details:

  //Default value
  bool disable_default_value = false;
  if(fieldInfo.get_auto_increment()) //Ignore default_values for auto_increment fields - it's just some obscure postgres code.
    disable_default_value = true;

  //Default value: simple:
  Glib::ustring default_value;
  if(!disable_default_value)
    default_value = m_Field.get_default_value_as_string();
  m_pEntry_Default->set_text(default_value);

  //Default value: lookup:

  m_pCheck_Lookup->set_active(m_Field.get_is_lookup());
  on_check_lookup_toggled();

  //Fill the lookup relationships combo:
  Document_Glom* document = dynamic_cast<Document_Glom*>(get_document());
  if(document)
  {
   Document_Glom::type_vecRelationships vecRelationships = document->get_relationships(table_name);
   m_pCombo_LookupRelationship->clear_text();

   for(Document_Glom::type_vecRelationships::iterator iter = vecRelationships.begin(); iter != vecRelationships.end(); ++iter)
   {
     m_pCombo_LookupRelationship->append_text(iter->get_name());
   }
  }
  
  Glib::ustring lookup_relationship_name;
  if(!disable_default_value)
    lookup_relationship_name = m_Field.get_lookup_relationship();
  m_pCombo_LookupRelationship->set_active_text(lookup_relationship_name);
  on_combo_lookup_relationship_changed(); //Put the correct list of fields in the fields combo.

  Glib::ustring lookup_field_name;
  if(!disable_default_value)
    lookup_field_name = m_Field.get_lookup_field();
  m_pCombo_LookupField->set_active_text(lookup_field_name);
    
  m_pEntry_Title->set_text(field.get_title());

  set_blocked(false);

  enforce_constraints();
  Dialog_Properties::set_modified(false);
}

Field Dialog_FieldDefinition::get_field() const
{
  Field field = m_Field; //Start with the old details, to preserve anything that is not in our UI.

  //Get the field info from the widgets:

  Gnome::Gda::FieldAttributes fieldInfo = field.get_field_info(); //Preserve previous information.

  fieldInfo.set_name(m_pEntry_Name->get_text());

  fieldInfo.set_gdatype( Field::get_gda_type_for_glom_type( m_pCombo_Type->get_field_type() ) );

  fieldInfo.set_unique_key(m_pCheck_Unique->get_active());
  fieldInfo.set_primary_key(m_pCheck_PrimaryKey->get_active());
  fieldInfo.set_auto_increment(m_pCheck_AutoIncrement->get_active());
 
  if(!fieldInfo.get_auto_increment()) //Ignore default_values for auto_increment fields - it's just some obscure postgres code.
  {
    //Simple default value:
    Glib::ustring default_value = m_pEntry_Default->get_text();
    fieldInfo.set_default_value( Gnome::Gda::Value(default_value) );
  }

  //Lookup:
  bool is_lookup = m_pCheck_Lookup->get_active();
  Glib::ustring relationship;
  if(is_lookup)
    relationship = m_pCombo_LookupRelationship->get_active_text();
  field.set_lookup_relationship(relationship);

  Glib::ustring lookup_field;
  if(is_lookup)
    lookup_field = m_pCombo_LookupField->get_active_text();
  field.set_lookup_field(lookup_field);

   
  Gnome::Gda::FieldAttributes field_info_copy = fieldInfo;
    
  field.set_field_info(fieldInfo);

  //Glom-specific details:

  field.set_title(m_pEntry_Title->get_text());

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
    m_pBox_DefaultValue->set_sensitive(false); //Disable all controls on the Notebook page.
    m_pEntry_Default->set_text(""); //Unique fields can not have default values. //TODO: People will be surprised when they lose information here. We should probably read the text as "" if the widget is disabled.
  }
  else
  {
    m_pBox_DefaultValue->set_sensitive(true);
  }
}

void Dialog_FieldDefinition::on_check_lookup_toggled()
{
  bool enable = m_pCheck_Lookup->get_active();
  m_pTable_Lookup->set_sensitive(enable);

  //re-disable it if it was not meant to be enabled:
  enforce_constraints();
}

void Dialog_FieldDefinition::on_combo_lookup_relationship_changed()
{
  //Get the relationship name:
  const Glib::ustring relationship_name = m_pCombo_LookupRelationship->get_active_text();
  if(!relationship_name.empty())
  {
    //Get the relationship details:
    Document_Glom* document = dynamic_cast<Document_Glom*>(get_document());
    if(document)
    {
      Relationship relationship;
      bool test = document->get_relationship(m_table_name, relationship_name, relationship);
      if(test)
      {
        Glib::ustring to_table = relationship.get_to_table();
        if(!to_table.empty())
        {
          //Get the fields in the other table, and add them to the combo:
          type_vecStrings vecFields = util_vecStrings_from_Fields(get_fields_for_table(m_table_name));
          m_pCombo_LookupField->clear_text();

          for(type_vecStrings::iterator iter = vecFields.begin(); iter != vecFields.end(); ++iter)
          {
            m_pCombo_LookupField->append_text(*iter);
          }
        }
      }
    }
  }

}


