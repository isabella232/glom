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

#ifndef GLOM_MODE_DATA_LAYOUT_WIDGET_BASE_H
#define GLOM_MODE_DATA_LAYOUT_WIDGET_BASE_H

#include "../data_structure/layout/layoutitem.h"
#include <gtkmm.h>
#include "../mode_data/treestore_layout.h" //Forthe enum.

class App_Glom;

class LayoutWidgetBase : virtual public sigc::trackable
{
public: 
  LayoutWidgetBase();
  virtual ~LayoutWidgetBase();

  ///Takes ownership.
  void set_layout_item(LayoutItem* layout_item, const Glib::ustring& table_name);

  //The caller should call clone().
  const LayoutItem* get_layout_item() const;
  LayoutItem* get_layout_item();

  //Popup-menu:
  virtual void setup_menu();
  virtual void on_menupopup_activate_layout();
  virtual void on_menupopup_activate_layout_properties();
  virtual void on_menupopup_add_item(TreeStore_Layout::enumType item);


  typedef sigc::signal<void> type_signal_layout_changed;
  type_signal_layout_changed signal_layout_changed();

  typedef sigc::signal<void, TreeStore_Layout::enumType> type_signal_layout_item_added;
  type_signal_layout_item_added signal_layout_item_added();

  //Allow a child widget to delegate to a parent widget:
  typedef sigc::signal<void> type_signal_user_requested_layout;
  type_signal_user_requested_layout signal_user_requested_layout(); 

  //Allow a child widget to delegate to a parent widget:
  typedef sigc::signal<void> type_signal_user_requested_layout_properties;
  type_signal_user_requested_layout_properties signal_user_requested_layout_properties();

protected:
  virtual App_Glom* get_application() const; // = 0;

  LayoutItem* m_pLayoutItem;
  Glib::ustring m_table_name;

  type_signal_layout_changed m_signal_layout_changed;
  type_signal_layout_item_added m_signal_layout_item_added;

  type_signal_user_requested_layout m_signal_user_requested_layout;
  type_signal_user_requested_layout_properties m_signal_user_requested_layout_properties;

  Gtk::Menu* m_pMenuPopup;
  Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;
  Glib::RefPtr<Gtk::UIManager> m_refUIManager;
  Glib::RefPtr<Gtk::Action> m_refContextLayout, m_refContextLayoutProperties, m_refContextAddField, m_refContextAddRelatedRecords, m_refContextAddGroup;

};

#endif //GLOM_MODE_DATA_LAYOUT_WIDGET_BASE_H
