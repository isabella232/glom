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

#ifndef GLOM_UTILITY_WIDGETS_LAYOUT_WIDGET_BASE_H
#define GLOM_UTILITY_WIDGETS_LAYOUT_WIDGET_BASE_H

#include <gtkmm/widget.h>
#include <libglom/data_structure/layout/layoutitem.h>
#include <glom/mode_design/layout/treestore_layout.h> //Forthe enum.

#include "config.h" // For GLOM_ENABLE_CLIENT_ONLY

namespace Glom
{

class AppWindow;

class LayoutWidgetBase : virtual public sigc::trackable
{
public:
  LayoutWidgetBase();
  virtual ~LayoutWidgetBase();

  ///Takes ownership.
  virtual void set_layout_item(const std::shared_ptr<LayoutItem>& layout_item, const Glib::ustring& table_name);

  //The caller should call clone().
  std::shared_ptr<const LayoutItem> get_layout_item() const;
  std::shared_ptr<LayoutItem> get_layout_item();

  enum class enumType
  {
    FIELD,
    GROUP,
    NOTEBOOK,
    PORTAL,
    BUTTON,
    TEXT,
    IMAGE
  };

#ifndef GLOM_ENABLE_CLIENT_ONLY
  typedef sigc::signal<void> type_signal_layout_changed;

  /// Signals that the layout has changed, so it should be saved to the document again.
  type_signal_layout_changed signal_layout_changed();

  typedef sigc::signal<void, enumType> type_signal_layout_item_added;

  ///Requests the addition of an item:
  type_signal_layout_item_added signal_layout_item_added();

  //Allow a child widget to delegate to a parent widget:
  typedef sigc::signal<void> type_signal_user_requested_layout;
  type_signal_user_requested_layout signal_user_requested_layout();

  //Allow a child widget to delegate to a parent widget:
  typedef sigc::signal<void> type_signal_user_requested_layout_properties;
  type_signal_user_requested_layout_properties signal_user_requested_layout_properties();
#endif // !GLOM_ENABLE_CLIENT_ONLY

  virtual void set_read_only(bool read_only = true);

protected:
  virtual AppWindow* get_appwindow() const; // = 0;


  static void apply_formatting(Gtk::Widget& widget, const std::shared_ptr<const LayoutItem_WithFormatting>& layout_item);

protected: //TODO: Add accessor?
  std::shared_ptr<LayoutItem> m_pLayoutItem;

protected: //TODO: Add accessor?
  Glib::ustring m_table_name;

private:

#ifndef GLOM_ENABLE_CLIENT_ONLY
  /// Signals that the layout has changed, so it should be saved to the document again.
  type_signal_layout_changed m_signal_layout_changed;

  /// Requests the addition of an item.
  type_signal_layout_item_added m_signal_layout_item_added;

  type_signal_user_requested_layout m_signal_user_requested_layout;
  type_signal_user_requested_layout_properties m_signal_user_requested_layout_properties;
#endif // !GLOM_ENABLE_CLIENT_ONLY
};

} //namespace Glom

#endif // GLOM_UTILITYWIDGETS_LAYOUT_WIDGET_BASE_H
