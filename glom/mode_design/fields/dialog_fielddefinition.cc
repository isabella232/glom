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


Dialog_FieldDefinition::Dialog_FieldDefinition(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Dialog_Properties(cobject, refGlade)
{
  refGlade->get_widget_derived("combobox_type", m_pCombo_Type);

  refGlade->get_widget("entry_name", m_pEntry_Name);
  refGlade->get_widget("entry_title", m_pEntry_Title);
  
  refGlade->get_widget("checkbutton_unique",  m_pCheck_Unique);
  refGlade->get_widget("checkbutton_primarykey",  m_pCheck_PrimaryKey);
  refGlade->get_widget("checkbutton_autoincrement",  m_pCheck_AutoIncrement);  

  refGlade->get_widget("entry_defaultvalue",  m_pEntry_Default);
  refGlade->get_widget("label_defaultvalue",  m_pLabel_Default);  

  //Connect signals:
  m_pCombo_Type->signal_changed().connect( sigc::mem_fun(*this, &Dialog_FieldDefinition::on_combo_type_changed));

  //TODO:
  //Connect every widget to on_anything_changed():
  //foreachgettext( (GtkCallback)(&on_foreach), this); //Crashes several levels down.

  //on_anything_changed(); //In the meantime, we'll just start with [Apply] already enabled.

  //Make sure that the correct Type Details are showing:
  on_combo_type_changed();

  on_foreach_connect(*this);

  set_modified(false);
  
  show_all_children();
}

Dialog_FieldDefinition::~Dialog_FieldDefinition()
{
}

void Dialog_FieldDefinition::set_field(const Field& field)
{
  set_blocked();

  m_Field = field; //Remember it so we save any details that are not in our UI.
  
  //Set the Widgets from the field info:
  const Gnome::Gda::FieldAttributes& fieldInfo = field.get_field_info();

  m_pEntry_Name->set_text(fieldInfo.get_name());
  m_pCombo_Type->set_field_type( field.get_field_type().get_glom_type() );

  m_pCheck_Unique->set_active(fieldInfo.get_unique_key());
  m_pCheck_PrimaryKey->set_active(fieldInfo.get_primary_key());
  m_pCheck_AutoIncrement->set_active(fieldInfo.get_auto_increment());

  Glib::ustring default_value = "";
  if(!fieldInfo.get_auto_increment()) //Ignore default_values for auto_increment fields - it's just some obscure postgres code.
    default_value = m_Field.get_default_value_as_string();
    
  m_pEntry_Default->set_text(default_value);

  //Optional type details:
  /* TODO_port:
  m_Entry_TypeDetails_M.set_value(fieldType.get_MaxLength());
  m_Entry_TypeDetails_Numeric_M.set_value(fieldType.get_MaxLength());
  m_Entry_TypeDetails_Numeric_D.set_value(fieldType.get_DecimalsCount());

  m_Check_Signed.set_active(fieldType.get_Signed());
  */
  
  //Glom-specific details:
  m_pEntry_Title->set_text(field.get_title());

  set_blocked(false);

  enforce_constraints();
  set_modified(false);
}

Field Dialog_FieldDefinition::get_field() const
{
  Field field = m_Field; //Start with the old details, to preserve anything that is not in our UI.

  //Get the field info from the widgets:

  Gnome::Gda::FieldAttributes fieldInfo = field.get_field_info(); //Preserve previous information.

  fieldInfo.set_name(m_pEntry_Name->get_text());

  FieldType fieldtypetemp( m_pCombo_Type->get_field_type() );
  fieldInfo.set_gdatype(fieldtypetemp.get_gda_type());

  fieldInfo.set_unique_key(m_pCheck_Unique->get_active());
  fieldInfo.set_primary_key(m_pCheck_PrimaryKey->get_active());
  fieldInfo.set_auto_increment(m_pCheck_AutoIncrement->get_active());
 
  if(!fieldInfo.get_auto_increment()) //Ignore default_values for auto_increment fields - it's just some obscure postgres code.
  {
    Glib::ustring default_value = m_pEntry_Default->get_text();
    fieldInfo.set_default_value( Gnome::Gda::Value(default_value) );
  }
      
  Gnome::Gda::FieldAttributes field_info_copy = fieldInfo;
    
  //Optional type details:
  /* TODO_port:
  if(Gnome::Gda::FieldAttributes::FieldType::get_TypeCategory(m_pCombo_Type->get_field_type()) == mysqlcppapi::FieldType::TYPE_CATEGORY_Numeric)
  {
    fieldType.set_MaxLength( m_Entry_TypeDetails_M.get_value_as_guint() );
  }
  else
  {
    fieldType.set_MaxLength( m_Entry_TypeDetails_Numeric_M.get_value_as_guint() );
  }

  fieldType.set_DecimalsCount(m_Entry_TypeDetails_Numeric_D.get_value_as_guint());

  fieldType.set_Signed(m_Check_Signed.get_active());
  */

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
 // FieldType::enumTypes fieldType = m_pCombo_Type->get_field_type();

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

  if(m_pCheck_Unique->get_active())
  {
    m_pLabel_Default->set_sensitive(false); //Disable the label because a disabled entry does not look disabled.
    m_pEntry_Default->set_text(""); //Unique fields can not have default values. //TODO: People will be surprised when they lost information here. We should probably read the text as "" if the widget is disabled.
    m_pEntry_Default->set_sensitive(false); //Stop the user from disagreeing with that.
  }
  else
  {
    m_pLabel_Default->set_sensitive(true);
    m_pEntry_Default->set_sensitive(true);
  }
}

