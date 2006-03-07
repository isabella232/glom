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

#include "dialog_field_layout.h"
#include "../data_structure/glomconversions.h"
#include <glibmm/i18n.h>

Dialog_FieldLayout::Dialog_FieldLayout(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::Dialog(cobject),
  m_label_field_name(0),
  m_checkbutton_editable(0),
  m_radiobutton_title_default(0),
  m_label_title_default(0),
  m_radiobutton_title_custom(0),
  m_entry_title_custom(0),
  m_box_formatting_placeholder(0),
  m_radiobutton_custom_formatting(0),
  m_box_formatting(0)
{
  refGlade->get_widget("label_field_name", m_label_field_name);
  refGlade->get_widget("checkbutton_editable", m_checkbutton_editable);

  refGlade->get_widget("radiobutton_use_title_default", m_radiobutton_title_default);
  refGlade->get_widget("label_title_default", m_label_title_default);
  refGlade->get_widget("radiobutton_use_title_custom", m_radiobutton_title_custom);
  refGlade->get_widget("entry_title_custom", m_entry_title_custom);

  //Get the place to put the Formatting stuff:
  refGlade->get_widget("radiobutton_use_custom", m_radiobutton_custom_formatting);
  refGlade->get_widget("box_formatting_placeholder", m_box_formatting_placeholder);


  //Get the formatting stuff:
  try
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXmlFormatting = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "box_formatting");
    refXmlFormatting->get_widget_derived("box_formatting", m_box_formatting);
  }
  catch(const Gnome::Glade::XmlError& ex)
  {
    std::cerr << ex.what() << std::endl;
  }

  m_box_formatting_placeholder->pack_start(*m_box_formatting);
  add_view(m_box_formatting);

  m_radiobutton_custom_formatting->signal_toggled().connect(sigc::mem_fun(*this, &Dialog_FieldLayout::on_radiobutton_custom_formatting));

  show_all_children();
}

Dialog_FieldLayout::~Dialog_FieldLayout()
{
  remove_view(m_box_formatting);
}

void Dialog_FieldLayout::set_field(const sharedptr<const LayoutItem_Field>& field, const Glib::ustring& table_name)
{
  m_layout_item = glom_sharedptr_clone(field);
  m_table_name = table_name;

  m_label_field_name->set_text( field->get_layout_display_name() );

  m_checkbutton_editable->set_active( field->get_editable() );

  //Calculated fields can never be edited:
  sharedptr<const Field> field_details = field->get_full_field_details();
  const bool editable_allowed = field_details && !field_details->get_has_calculation();
  m_checkbutton_editable->set_sensitive(editable_allowed);

  //Custom title:
  Glib::ustring title_custom;
  if(field->get_title_custom())
    title_custom = field->get_title_custom()->get_title();

  m_radiobutton_title_custom->set_active( !title_custom.empty() );
  m_entry_title_custom->set_text(title_custom);
  m_label_title_default->set_text(field->get_title_or_name());

  //Formatting:
  m_radiobutton_custom_formatting->set_active( !field->get_formatting_use_default() );
  m_box_formatting->set_formatting(field->m_formatting, table_name, field->get_full_field_details());

  enforce_constraints();
}

sharedptr<LayoutItem_Field> Dialog_FieldLayout::get_field_chosen() const
{
  m_layout_item->set_editable( m_checkbutton_editable->get_active() );

  m_layout_item->set_formatting_use_default( !m_radiobutton_custom_formatting->get_active() );
  m_box_formatting->get_formatting(m_layout_item->m_formatting);

  sharedptr<CustomTitle> title_custom = sharedptr<CustomTitle>::create();
  if(m_radiobutton_title_custom->get_active())
  {
    std::cout << "setting title custom: " << m_entry_title_custom->get_text() << std::endl;
    title_custom->set_title(m_entry_title_custom->get_text());
  }

  m_layout_item->set_title_custom(title_custom);

  return glom_sharedptr_clone(m_layout_item);
}

void Dialog_FieldLayout::on_radiobutton_custom_formatting()
{
  enforce_constraints();
}

void Dialog_FieldLayout::enforce_constraints()
{
  //Enable/Disable the custom formatting widgets:
  const bool custom = m_radiobutton_custom_formatting->get_active();
  m_box_formatting->set_sensitive(custom);
}


