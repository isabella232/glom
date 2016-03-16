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

#include <glom/mode_design/layout/layout_item_dialogs/dialog_group_by.h>
#include <glom/utils_ui.h>
#include <glom/glade_utils.h>
#include <libglom/utils.h>
#include <sstream> //For stringstream
#include <glibmm/i18n.h>

namespace Glom
{

const char* Dialog_GroupBy::glade_id("dialog_group_by");
const bool Dialog_GroupBy::glade_developer(true);

Dialog_GroupBy::Dialog_GroupBy(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Dialog(cobject),
  m_label_group_by(nullptr),
  m_label_sort_by(nullptr),
  m_label_secondary_fields(nullptr),
  m_button_field_group_by(nullptr),
  m_button_formatting_group_by(nullptr),
  m_button_field_sort_by(nullptr),
  m_button_secondary_fields(nullptr),
  m_comboboxentry_border_width(nullptr),
  m_dialog_choose_secondary_fields(nullptr),
  m_dialog_choose_sort_fields(nullptr)
{
  builder->get_widget("label_group_by", m_label_group_by);
  builder->get_widget("label_sort_by", m_label_sort_by);
  builder->get_widget("label_secondary_fields", m_label_secondary_fields);

  builder->get_widget("button_select_group_by", m_button_field_group_by);
  builder->get_widget("button_formatting_group_by", m_button_formatting_group_by);
  builder->get_widget("button_select_sort_by", m_button_field_sort_by);
  builder->get_widget("button_secondary_edit", m_button_secondary_fields);

  builder->get_widget_derived("comboboxentry_border_width", m_comboboxentry_border_width);

  //Connect signals:
  m_button_field_group_by->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_GroupBy::on_button_field_group_by));
  m_button_formatting_group_by->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_GroupBy::on_button_formatting_group_by));
  m_button_field_sort_by->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_GroupBy::on_button_field_sort_by));
  m_button_secondary_fields->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_GroupBy::on_button_secondary_fields));

  show_all_children();
}

Dialog_GroupBy::~Dialog_GroupBy()
{
  if(m_dialog_choose_secondary_fields)
  {
    remove_view(m_dialog_choose_secondary_fields);
    delete m_dialog_choose_secondary_fields;
  }

  if(m_dialog_choose_sort_fields)
  {
    remove_view(m_dialog_choose_sort_fields);
    delete m_dialog_choose_sort_fields;
  }
}

void Dialog_GroupBy::set_item(const std::shared_ptr<const LayoutItem_GroupBy>& item, const Glib::ustring& table_name)
{
  m_layout_item = glom_sharedptr_clone(item);
  m_table_name = table_name;

  update_labels();

  Glib::ustring border_width_as_text;
  std::stringstream the_stream;
  the_stream.imbue(std::locale("")); //Current locale.
  the_stream << m_layout_item->get_border_width();
  border_width_as_text = the_stream.str();
  m_comboboxentry_border_width->get_entry()->set_text(border_width_as_text);
}

std::shared_ptr<LayoutItem_GroupBy> Dialog_GroupBy::get_item() const
{
  std::stringstream the_stream;
  the_stream.imbue(std::locale("")); //Current locale.
  the_stream << m_comboboxentry_border_width->get_entry()->get_text();

  double border_width_as_number = 0;
  the_stream >> border_width_as_number;
  m_layout_item->set_border_width(border_width_as_number);
  return glom_sharedptr_clone(m_layout_item);
}

void Dialog_GroupBy::on_button_field_group_by()
{
  auto field = offer_field_list_select_one_field(m_layout_item->get_field_group_by(), m_table_name, this);
  if(field)
  {
    m_layout_item->set_field_group_by(field);
    set_item(m_layout_item, m_table_name); //Update the UI.
  }
}

void Dialog_GroupBy::on_button_formatting_group_by()
{
  if(m_layout_item)
  {
    auto field = offer_field_formatting(m_layout_item->get_field_group_by(), m_table_name, this, false /* no editing options. */);
    if(field)
    {
      m_layout_item->set_field_group_by(field);
      set_item(m_layout_item, m_table_name); //Update the UI.
    }
  }
}

void Dialog_GroupBy::on_button_field_sort_by()
{
  if(!m_dialog_choose_sort_fields)
  {
    Utils::get_glade_widget_derived_with_warning(m_dialog_choose_sort_fields);
    add_view(m_dialog_choose_sort_fields); //Give it access to the document.
  }

  if(m_dialog_choose_sort_fields)
  {
    m_dialog_choose_sort_fields->set_fields(m_table_name, m_layout_item->get_fields_sort_by());

    const auto dialog_response = Glom::UiUtils::dialog_run_with_help(m_dialog_choose_sort_fields);
    m_dialog_choose_sort_fields->hide();
    if(dialog_response == Gtk::RESPONSE_OK && m_dialog_choose_sort_fields->get_modified())
    {
      m_layout_item->set_fields_sort_by( m_dialog_choose_sort_fields->get_fields() );
    }
  }

  update_labels();
}

void Dialog_GroupBy::on_button_secondary_fields()
{
  if(!m_dialog_choose_secondary_fields)
  {
    Utils::get_glade_widget_derived_with_warning(m_dialog_choose_secondary_fields);
    if(m_dialog_choose_secondary_fields)
    {
      add_view(m_dialog_choose_secondary_fields); //Give it access to the document.
      m_dialog_choose_secondary_fields->set_title(_("Group By - Secondary Fields"));
    }
  }

  if(m_dialog_choose_secondary_fields)
  {
    m_dialog_choose_secondary_fields->set_fields(m_table_name, m_layout_item->get_secondary_fields()->m_list_items);

    const auto dialog_response = Glom::UiUtils::dialog_run_with_help(m_dialog_choose_secondary_fields);
    m_dialog_choose_secondary_fields->hide();
    if(dialog_response == Gtk::RESPONSE_OK && m_dialog_choose_secondary_fields->get_modified())
    {
      m_layout_item->get_secondary_fields()->remove_all_items(); //Free the existing member items.
      m_layout_item->get_secondary_fields()->m_list_items = m_dialog_choose_secondary_fields->get_fields();
    }
  }

  update_labels();
}

void Dialog_GroupBy::update_labels()
{
  //Group-by Field:
  if(m_layout_item->get_has_field_group_by())
  {
    m_label_group_by->set_text( m_layout_item->get_field_group_by()->get_layout_display_name() );
    m_button_formatting_group_by->set_sensitive(true);
  }
  else
  {
    m_label_group_by->set_text( Glib::ustring() );
    m_button_formatting_group_by->set_sensitive(false);
  }

  //Sort fields:
  m_label_sort_by->set_text( Utils::get_list_of_sort_fields_for_display(m_layout_item->get_fields_sort_by()) );

  //Secondary Fields:
  const Glib::ustring text_secondary_fields =
    Utils::get_list_of_layout_items_for_display(m_layout_item->get_secondary_fields());
  m_label_secondary_fields->set_text(text_secondary_fields);
}

} //namespace Glom
