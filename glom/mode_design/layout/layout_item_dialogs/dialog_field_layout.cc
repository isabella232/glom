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

#include "dialog_field_layout.h"
#include <glom/glade_utils.h>
#include <glom/appwindow.h>

namespace Glom
{

const char* Dialog_FieldLayout::glade_id("dialog_layout_field_properties");
const bool Dialog_FieldLayout::glade_developer(true);

Dialog_FieldLayout::Dialog_FieldLayout(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Dialog(cobject),
  m_label_field_name(nullptr),
  m_checkbutton_editable(nullptr),
  m_radiobutton_title_default(nullptr),
  m_label_title_default(nullptr),
  m_radiobutton_title_custom(nullptr),
  m_entry_title_custom(nullptr),
  m_box_formatting_placeholder(nullptr),
  m_radiobutton_custom_formatting(nullptr),
  m_box_formatting(nullptr)
{
  builder->get_widget("label_field_name", m_label_field_name);
  builder->get_widget("checkbutton_editable", m_checkbutton_editable);

  builder->get_widget("radiobutton_use_title_default", m_radiobutton_title_default);
  builder->get_widget("label_title_default", m_label_title_default);
  builder->get_widget("radiobutton_use_title_custom", m_radiobutton_title_custom);
  builder->get_widget("entry_title_custom", m_entry_title_custom);

  //Get the place to put the Formatting stuff:
  builder->get_widget("radiobutton_use_custom", m_radiobutton_custom_formatting);
  builder->get_widget("box_formatting_placeholder", m_box_formatting_placeholder);

  //Get the formatting stuff:
  Utils::box_pack_start_glade_child_widget_derived_with_warning(m_box_formatting_placeholder, m_box_formatting);
  add_view(m_box_formatting);

  m_radiobutton_custom_formatting->signal_toggled().connect(sigc::mem_fun(*this, &Dialog_FieldLayout::on_radiobutton_custom_formatting));

  show_all_children();
}

Dialog_FieldLayout::~Dialog_FieldLayout()
{
  remove_view(m_box_formatting);
}

void Dialog_FieldLayout::set_field(const std::shared_ptr<const LayoutItem_Field>& field, const Glib::ustring& table_name, bool show_editable_options)
{
  m_layout_item = glom_sharedptr_clone(field);

  m_table_name = table_name;

  m_label_field_name->set_text( field->get_layout_display_name() );

  m_checkbutton_editable->set_active( field->get_editable() );

  //Calculated fields can never be edited:
  auto field_details = field->get_full_field_details();
  const bool editable_allowed = field_details && !field_details->get_has_calculation();
  m_checkbutton_editable->set_sensitive(editable_allowed);

  if(!show_editable_options)
    m_checkbutton_editable->hide();

  //Custom title:
  Glib::ustring title_custom;
  if(field->get_title_custom())
    title_custom = item_get_title(field->get_title_custom());

  m_radiobutton_title_custom->set_active( field->get_title_custom() && field->get_title_custom()->get_use_custom_title() );
  m_entry_title_custom->set_text(title_custom);
  m_label_title_default->set_text(field->get_title_or_name_no_custom(AppWindow::get_current_locale()));

  //Formatting:
  m_radiobutton_custom_formatting->set_active( !field->get_formatting_use_default() );
  m_box_formatting->set_formatting_for_field(field->m_formatting, table_name, field->get_full_field_details());
  if(!show_editable_options)
    m_box_formatting->set_is_for_non_editable();

  enforce_constraints();
}

std::shared_ptr<LayoutItem_Field> Dialog_FieldLayout::get_field_chosen() const
{
  m_layout_item->set_editable( m_checkbutton_editable->get_active() );

  m_layout_item->set_formatting_use_default( !m_radiobutton_custom_formatting->get_active() );
  m_box_formatting->get_formatting(m_layout_item->m_formatting);

  auto title_custom = std::make_shared<CustomTitle>();
  title_custom->set_use_custom_title(m_radiobutton_title_custom->get_active()); //For instance, tell it to really use a blank title.
  title_custom->set_title(m_entry_title_custom->get_text(), AppWindow::get_current_locale());

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
  const auto custom = m_radiobutton_custom_formatting->get_active();
  m_box_formatting->set_sensitive(custom);
}

} //namespace Glom
