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

#ifndef GLOM_MODE_DATA_LAYOUT_WIDGET_MENU_H
#define GLOM_MODE_DATA_LAYOUT_WIDGET_MENU_H

#include "layoutwidgetbase.h"
#include <gtkmm/builder.h>
#include <gtkmm/menu.h>
#include <giomm/simpleactiongroup.h>

namespace Glom
{

class LayoutWidgetMenu : public LayoutWidgetBase
{
public: 
  LayoutWidgetMenu();
  
  //Popup-menu:
#ifndef GLOM_ENABLE_CLIENT_ONLY
  /**
   * @widget The widget instance, such as "this" in a derived class.
   */
  virtual void setup_menu(Gtk::Widget* widget); //TODO: Make this protected?

  virtual void on_menupopup_activate_layout();
  virtual void on_menupopup_activate_layout_properties();
  void on_menupopup_add_item(enumType item);
  void on_menupopup_activate_delete();
#endif // !GLOM_ENABLE_CLIENT_ONLY  
    
protected:
#ifndef GLOM_ENABLE_CLIENT_ONLY    
  std::unique_ptr<Gtk::Menu> m_menu_popup;

  //TODO_Performance: //Presumably we waste lots of memory by having this in each layout widget. Maybe we can use one shared menu.
  Glib::RefPtr<Gio::SimpleActionGroup> m_action_group;

  Glib::RefPtr<Gio::SimpleAction> m_context_layout, m_context_layout_properties;
  Glib::RefPtr<Gio::SimpleAction> m_context_add_field, m_context_add_related_records,
    m_context_add_group, m_context_add_notebook, m_context_add_button, m_context_add_text;
  Glib::RefPtr<Gio::SimpleAction> m_context_delete;
#endif // GLOM_ENABLE_CLIENT_ONLY

private:
  void add_action(const Glib::RefPtr<Gio::SimpleAction>& action, const Gio::ActionMap::ActivateSlot& slot);
};

} //namespace Glom

#endif //GLOM_MODE_DATA_LAYOUT_WIDGET_MENU_H
