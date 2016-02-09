/* Glom
 *
 * Copyright (C) 2001-2005 Murray Cumming
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

#include "layoutwidgetfield.h"
#include <glibmm/i18n.h>
#include <glom/appwindow.h>
#include "../mode_data/flowtablewithfields.h"
#include <giomm/menu.h>
#include <iostream>

namespace Glom
{

LayoutWidgetMenu::LayoutWidgetMenu()
{
  #ifndef GLOM_ENABLE_CLIENT_ONLY
  m_refActionGroup = Gio::SimpleActionGroup::create();

  m_refContextLayout = Gio::SimpleAction::create("choose-field");
  m_refContextLayoutProperties = Gio::SimpleAction::create("field-layout-properties");
  m_refContextAddField = Gio::SimpleAction::create("add-field");
  m_refContextAddRelatedRecords = Gio::SimpleAction::create("add-related-records");
  m_refContextAddNotebook = Gio::SimpleAction::create("add-notebook");
  m_refContextAddGroup = Gio::SimpleAction::create("add-group");
  m_refContextAddButton = Gio::SimpleAction::create("add-button");
  m_refContextAddText = Gio::SimpleAction::create("add-text");
  m_refContextDelete = Gio::SimpleAction::create("delete");
#endif // !GLOM_ENABLE_CLIENT_ONLY
}

#ifndef GLOM_ENABLE_CLIENT_ONLY

void LayoutWidgetMenu::add_action(const Glib::RefPtr<Gio::SimpleAction>& action, const Gio::ActionMap::ActivateSlot& slot)
{
  if(!action)
    return;

  m_refActionGroup->add_action(m_refContextLayout);
  action->signal_activate().connect(
    sigc::hide(slot));
}

void LayoutWidgetMenu::setup_menu(Gtk::Widget* widget)
{
  if(!widget)
  {
    std::cerr << G_STRFUNC << ": parent is NULL.\n";
    return;
  }

  add_action(m_refContextLayout,
    sigc::mem_fun(*this, &LayoutWidgetMenu::on_menupopup_activate_layout) );

  add_action(m_refContextLayoutProperties,
    sigc::mem_fun(*this, &LayoutWidgetMenu::on_menupopup_activate_layout_properties) );

  add_action(m_refContextAddField,
    sigc::bind( sigc::mem_fun(*this, &LayoutWidgetMenu::on_menupopup_add_item), enumType::FIELD ) );

  add_action(m_refContextAddRelatedRecords,
    sigc::bind( sigc::mem_fun(*this, &LayoutWidgetMenu::on_menupopup_add_item), enumType::PORTAL ) );

  add_action(m_refContextAddGroup,
    sigc::bind( sigc::mem_fun(*this, &LayoutWidgetMenu::on_menupopup_add_item), enumType::GROUP ) );

  add_action(m_refContextAddNotebook,
    sigc::bind( sigc::mem_fun(*this, &LayoutWidgetMenu::on_menupopup_add_item), enumType::NOTEBOOK ) );

  add_action(m_refContextAddButton,
    sigc::bind( sigc::mem_fun(*this, &LayoutWidgetMenu::on_menupopup_add_item), enumType::BUTTON ) );

  add_action(m_refContextAddText,
    sigc::bind( sigc::mem_fun(*this, &LayoutWidgetMenu::on_menupopup_add_item), enumType::TEXT ) );
  
  add_action(m_refContextDelete,
    sigc::mem_fun(*this, &LayoutWidgetMenu::on_menupopup_activate_delete) );

  //TODO: This does not work until this widget is in a container in the window:s
  auto pApp = get_appwindow();
  if(pApp)
  {
    pApp->add_developer_action(m_refContextLayout); //So that it can be disabled when not in developer mode.
    pApp->add_developer_action(m_refContextLayoutProperties); //So that it can be disabled when not in developer mode.
    pApp->add_developer_action(m_refContextAddField);
    pApp->add_developer_action(m_refContextAddRelatedRecords);
    pApp->add_developer_action(m_refContextAddNotebook);
    pApp->add_developer_action(m_refContextAddGroup);
    pApp->add_developer_action(m_refContextAddButton);
    pApp->add_developer_action(m_refContextAddText);

    pApp->update_userlevel_ui(); //Update our action's sensitivity. 
  }

  auto menu = Gio::Menu::create();
  menu->append(_("Choose Field"), "context.choose-field");
  menu->append(_("Field Layout Properties"), "context.field-layout-properties");
  menu->append(_("Add Related Records"), "context.add-related-records");
  menu->append(_("Add Notebook"), "context.add-notebook");
  menu->append(_("Add Group"), "context.add-group");
  menu->append(_("Add Button"), "context.add-button");
  menu->append(_("Add Text"), "context.add-text");
  menu->append(_("Delete"), "context.delete");


  m_pMenuPopup = std::make_unique<Gtk::Menu>(menu);
  m_pMenuPopup->attach_to_widget(*widget);

  if(pApp)
    m_refContextLayout->set_enabled(pApp->get_userlevel() == AppState::userlevels::DEVELOPER);

  //Make our popup menu work:
  widget->insert_action_group("context", m_refActionGroup);
}

void LayoutWidgetMenu::on_menupopup_add_item(enumType item)
{
  signal_layout_item_added().emit(item);
}

void LayoutWidgetMenu::on_menupopup_activate_layout()
{
  //finish_editing();

  //Ask the parent widget to show the layout dialog:
  signal_user_requested_layout().emit();
}

void LayoutWidgetMenu::on_menupopup_activate_layout_properties()
{
  //finish_editing();

  //Ask the parent widget to show the layout dialog:
  signal_user_requested_layout_properties().emit();
}

void LayoutWidgetMenu::on_menupopup_activate_delete()
{
  auto parent = dynamic_cast<Gtk::Widget*>(this);
  if(!parent)
  {
    // Should never happen:
    std::cerr << G_STRFUNC << ": this is not a Gtk::Widget\n";
    return;
  }

  LayoutWidgetBase* base = nullptr;
  do
  {
    parent = parent->get_parent();
    base = dynamic_cast<LayoutWidgetBase*>(parent);
    if(base && dynamic_cast<FlowTableWithFields*>(base))
      break;
  } while (parent);

  if(base)
  {
    auto group = 
      std::dynamic_pointer_cast<LayoutGroup>(base->get_layout_item());
    if(!group)
      return; 
 
    group->remove_item(get_layout_item());
    base->signal_layout_changed().emit();
  } 
}

#endif // !GLOM_ENABLE_CLIENT_ONLY

} //namespace Glom
