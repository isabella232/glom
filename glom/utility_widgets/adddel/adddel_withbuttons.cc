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

#include "adddel_withbuttons.h"
#include <glom/utils_ui.h>
#include <libglom/utils.h>
#include <glibmm/i18n.h>

//#include <libgnome/gnome-i18n.h>

namespace Glom
{

AddDel_WithButtons::AddDel_WithButtons()
: m_ButtonBox(Gtk::Orientation::HORIZONTAL),
  m_Button_Add(_("_Add"), true),
  m_Button_Del(_("_Delete"), true),
  m_Button_Edit(_("_Open"), true)
{
  init();
}

AddDel_WithButtons::AddDel_WithButtons(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: AddDel(cobject, builder),
  m_ButtonBox(Gtk::Orientation::HORIZONTAL),
  m_Button_Add(_("_Add"), true),
  m_Button_Del(_("_Delete"), true),
  m_Button_Edit(_("_Open"), true)
{
  init();
}

void AddDel_WithButtons::init()
{
  m_ButtonBox.set_layout(Gtk::ButtonBoxStyle::END);
  m_ButtonBox.set_spacing(Utils::to_utype(UiUtils::DefaultSpacings::SMALL));

  //m_Button_Add.set_margin(Utils::to_utype(UiUtils::DefaultSpacings::SMALL));
  //m_Button_Del.set_margin(Utils::to_utype(UiUtils::DefaultSpacings::SMALL));
  //m_Button_Edit.set_margin(Utils::to_utype(UiUtils::DefaultSpacings::SMALL));

  setup_buttons();
  pack_start(m_ButtonBox, Gtk::PackOptions::SHRINK);

  //Link buttons to handlers:
  m_Button_Add.signal_clicked().connect(sigc::mem_fun(*this, &AddDel_WithButtons::on_button_add));
  m_Button_Del.signal_clicked().connect(sigc::mem_fun(*this, &AddDel_WithButtons::on_button_del));
  m_Button_Edit.signal_clicked().connect(sigc::mem_fun(*this, &AddDel_WithButtons::on_button_edit));
  m_Button_Extra.signal_clicked().connect(sigc::mem_fun(*this, &AddDel_WithButtons::on_button_extra));

  m_Button_Extra.hide();
}

void AddDel_WithButtons::on_button_add()
{
  if(m_auto_add)
  {
    auto iter = get_item_placeholder();
    if(iter)
    {
      guint first_visible = get_count_hidden_system_columns();
      select_item(iter, first_visible, true /* start_editing */);
    }
  }
  else
  {
    signal_user_requested_add().emit(); //Let the client code add the row explicitly, if it wants.
  }
}

void AddDel_WithButtons::on_button_del()
{
  on_MenuPopup_activate_Delete();
}

void AddDel_WithButtons::on_button_edit()
{
  on_MenuPopup_activate_Edit();
}

void AddDel_WithButtons::on_button_extra()
{
  auto refSelection = m_tree_view.get_selection();
  if(!refSelection)
    return;

  auto iter = refSelection->get_selected();
  if(!iter)
    return;

  if(get_is_placeholder_row(iter))
    return;

  signal_user_requested_extra()(iter);
}

void AddDel_WithButtons::set_allow_add(bool val)
{
  AddDel::set_allow_add(val);

  m_Button_Add.set_sensitive(val);
}

void AddDel_WithButtons::set_allow_delete(bool val)
{
  AddDel::set_allow_delete(val);

  m_Button_Del.set_sensitive(val);
}

void AddDel_WithButtons::set_allow_user_actions(bool value)
{
  AddDel::set_allow_user_actions(value);

  //add or remove buttons:
  if(value)
  {
    set_allow_user_actions(false); //Remove them first (Don't want to add them twice).

    //Ensure that the buttons are in the ButtonBox.
    setup_buttons();
  }
  else
  {
    //We don't just remove m_ButtonBox, because we want it to remain as a placeholder.
    m_ButtonBox.remove(m_Button_Add);
    m_ButtonBox.remove(m_Button_Del);
    m_ButtonBox.remove(m_Button_Edit);
    m_ButtonBox.remove(m_Button_Extra);
  }

  //Recreate popup menu with correct items:
  setup_menu(this);
}

void AddDel_WithButtons::setup_buttons()
{
  if(!get_allow_user_actions())
    return;

  m_ButtonBox.pack_end(m_Button_Add, Gtk::PackOptions::SHRINK);
  m_Button_Add.show();

  m_ButtonBox.pack_end(m_Button_Del, Gtk::PackOptions::SHRINK);
  m_Button_Del.show();

  m_ButtonBox.pack_end(m_Button_Edit, Gtk::PackOptions::SHRINK);
  m_Button_Edit.show();

  m_ButtonBox.pack_end(m_Button_Extra, Gtk::PackOptions::SHRINK);
  if(!m_label_extra.empty())
    m_Button_Extra.show();
  else
    m_Button_Extra.hide();
}

void AddDel_WithButtons::set_extra_button_label(const Glib::ustring& label)
{
  m_label_extra = label;
  m_Button_Extra.set_label(m_label_extra);
  m_Button_Extra.set_use_underline();

  if(!m_label_extra.empty())
    m_Button_Extra.show();
  else
    m_Button_Extra.hide();
}

void AddDel_WithButtons::set_edit_button_label(const Glib::ustring& label)
{
  m_Button_Edit.set_label(label);
  m_Button_Edit.set_use_underline();
}

} //namespace Glom

