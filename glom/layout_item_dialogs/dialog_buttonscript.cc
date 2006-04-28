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


#include "dialog_buttonscript.h"
#include "../python_embed/glom_python.h"
#include <glom/libglom/data_structure/glomconversions.h>

//#include <libgnome/gnome-i18n.h>
#include <glibmm/i18n.h>

Dialog_ButtonScript::Dialog_ButtonScript(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::Dialog(cobject)
{
  refGlade->get_widget("textview_calculation",  m_text_view);
  refGlade->get_widget("button_test",  m_button_test);

  m_button_test->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_ButtonScript::on_button_test) );
  //on_foreach_connect(*this);

  //Dialog_Properties::set_modified(false);

  show_all_children();
}

Dialog_ButtonScript::~Dialog_ButtonScript()
{
}

void Dialog_ButtonScript::set_script(const sharedptr<const LayoutItem_Button>& script, const Glib::ustring& table_name)
{
  //set_blocked();

  m_script = glom_sharedptr_clone(script); //Remember it so we save any details that are not in our UI.
  m_table_name = table_name;  //Used for lookup combo boxes.

  m_text_view->get_buffer()->set_text( script->get_script() );

  //set_blocked(false);

  //Dialog_Properties::set_modified(false);
}

sharedptr<LayoutItem_Button> Dialog_ButtonScript::get_script() const
{
  sharedptr<LayoutItem_Button> result = glom_sharedptr_clone(m_script); //Start with the old details, to preserve anything that is not in our UI.

  result->set_script( m_text_view->get_buffer()->get_text() );

  return result;
}

void Dialog_ButtonScript::on_button_test()
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

  glom_execute_python_function_implementation(calculation, field_values, //TODO: Maybe use the field's type here.
    document, m_table_name);
}




