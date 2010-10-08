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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "layoutwidgetfield.h"
#include <glibmm/i18n.h>
#include <glom/application.h>
#include "../mode_data/flowtablewithfields.h"
#include <iostream>

namespace Glom
{

LayoutWidgetMenu::LayoutWidgetMenu()
#ifndef GLOM_ENABLE_CLIENT_ONLY
  : m_pMenuPopup(0)
#endif
{
  #ifndef GLOM_ENABLE_CLIENT_ONLY
  m_refActionGroup = Gtk::ActionGroup::create();

  m_refActionGroup->add(Gtk::Action::create("ContextMenu", "Context Menu") );
  m_refContextLayout =  Gtk::Action::create("ContextLayout", _("Choose Field"));
  m_refContextLayoutProperties =  Gtk::Action::create("ContextLayoutProperties", _("Field Layout Properties"));
  m_refContextAddField =  Gtk::Action::create("ContextAddField", _("Add Field"));
  m_refContextAddRelatedRecords =  Gtk::Action::create("ContextAddRelatedRecords", _("Add Related Records"));
  m_refContextAddNotebook =  Gtk::Action::create("ContextAddNotebook", _("Add Notebook"));
  m_refContextAddGroup =  Gtk::Action::create("ContextAddGroup", _("Add Group"));
  m_refContextAddButton =  Gtk::Action::create("ContextAddButton", _("Add Button"));
  m_refContextAddText =  Gtk::Action::create("ContextAddText", _("Add Text"));
  m_refContextDelete = Gtk::Action::create("ContextDelete", _("Delete"));
#endif // !GLOM_ENABLE_CLIENT_ONLY
}

LayoutWidgetMenu::~LayoutWidgetMenu()
{
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void LayoutWidgetMenu::setup_menu()
{
  m_refActionGroup->add(m_refContextLayout,
    sigc::mem_fun(*this, &LayoutWidgetMenu::on_menupopup_activate_layout) );

  m_refActionGroup->add(m_refContextLayoutProperties,
    sigc::mem_fun(*this, &LayoutWidgetMenu::on_menupopup_activate_layout_properties) );

  m_refActionGroup->add(m_refContextAddField,
    sigc::bind( sigc::mem_fun(*this, &LayoutWidgetMenu::on_menupopup_add_item), TYPE_FIELD ) );

  m_refActionGroup->add(m_refContextAddRelatedRecords,
    sigc::bind( sigc::mem_fun(*this, &LayoutWidgetMenu::on_menupopup_add_item), TYPE_PORTAL ) );

  m_refActionGroup->add(m_refContextAddGroup,
    sigc::bind( sigc::mem_fun(*this, &LayoutWidgetMenu::on_menupopup_add_item), TYPE_GROUP ) );

  m_refActionGroup->add(m_refContextAddNotebook,
    sigc::bind( sigc::mem_fun(*this, &LayoutWidgetMenu::on_menupopup_add_item), TYPE_NOTEBOOK ) );

  m_refActionGroup->add(m_refContextAddButton,
    sigc::bind( sigc::mem_fun(*this, &LayoutWidgetMenu::on_menupopup_add_item), TYPE_BUTTON ) );

  m_refActionGroup->add(m_refContextAddText,
    sigc::bind( sigc::mem_fun(*this, &LayoutWidgetMenu::on_menupopup_add_item), TYPE_TEXT ) );
  
  m_refActionGroup->add(m_refContextDelete,
    sigc::mem_fun(*this, &LayoutWidgetMenu::on_menupopup_activate_delete) );

  //TODO: This does not work until this widget is in a container in the window:s
  Application* pApp = get_application();
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

  m_refUIManager = Gtk::UIManager::create();

  m_refUIManager->insert_action_group(m_refActionGroup);

  //TODO: add_accel_group(m_refUIManager->get_accel_group());

  try
  {
    Glib::ustring ui_info = 
        "<ui>"
        "  <popup name='ContextMenu'>"
        "    <menuitem action='ContextLayout'/>"
        "    <menuitem action='ContextLayoutProperties'/>"
        "    <menuitem action='ContextAddField'/>"
        "    <menuitem action='ContextAddRelatedRecords'/>"
        "    <menuitem action='ContextAddNotebook'/>"
        "    <menuitem action='ContextAddGroup'/>"
        "    <menuitem action='ContextAddButton'/>"
        "    <menuitem action='ContextAddText'/>"
        "    <separator />"
        "    <menuitem action='ContextDelete' />"
        "  </popup>"
        "</ui>";

    m_refUIManager->add_ui_from_string(ui_info);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << "building menus failed: " <<  ex.what();
  }

  //Get the menu:
  m_pMenuPopup = dynamic_cast<Gtk::Menu*>( m_refUIManager->get_widget("/ContextMenu") ); 
  if(!m_pMenuPopup)
    g_warning("menu not found");


  if(pApp)
    m_refContextLayout->set_sensitive(pApp->get_userlevel() == AppState::USERLEVEL_DEVELOPER);
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
  Gtk::Widget* parent = dynamic_cast<Gtk::Widget*>(this);
  if(!parent)
  {
    // Should never happen!
    std::cerr << "LayoutWidgetUtils is no Gtk::Widget" << std::endl;
    return;
  }

  LayoutWidgetBase* base = 0;
  do
  {
    parent = parent->get_parent();
    base = dynamic_cast<LayoutWidgetBase*>(parent);
    if(base && dynamic_cast<FlowTableWithFields*>(base))
      break;
  } while (parent);

  if(base)
  {
    sharedptr<LayoutGroup> group = 
      sharedptr<LayoutGroup>::cast_dynamic(base->get_layout_item());
    if(!group)
      return; 
 
    group->remove_item(get_layout_item());
    base->signal_layout_changed().emit();
  } 
}

#endif // !GLOM_ENABLE_CLIENT_ONLY

} //namespace Glom
