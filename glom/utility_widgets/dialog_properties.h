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

#ifndef GLOM_DIALOG_PROPERTIES_H
#define GLOM_DIALOG_PROPERTIES_H

#include <gtkmm/window.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/button.h>
#include <gtkmm/combobox.h>
#include <gtkmm/builder.h>

namespace Glom
{

class Dialog_Properties : public Gtk::Window
{
public:
  Dialog_Properties(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

  //Add a widget (probably a container) to the top half of the dialog:
  void add(Gtk::Widget& widget) override;

  virtual void set_modified(bool modified = true);

  //int page_number
  typedef sigc::signal<void()> type_signal_apply;
  type_signal_apply signal_apply();

protected:
  //Signal handlers:
  void on_button_save();
  void on_button_cancel();

  void on_anything_changed();
  void on_adddel_user_changed(const Gtk::TreeModel::iterator& /* iter */, guint /* col */);

  /// Disable/enable other controls when a control is selected.
  virtual void enforce_constraints();

  /** Handle the relevant signal for each of the widget's child widgets,
   * calling set_modified(true) in the signal handler.
   */
  void connect_each_widget(Gtk::Widget* widget);

private:
  void on_foreach_connect(Gtk::Widget& widget);
  void widget_connect_changed_signal(Gtk::Widget* widget);

protected:
  void set_blocked(bool val = true);

  type_signal_apply m_signal_apply;

  //Child widgets:

  //PlaceHolder m_Frame; //For the top-half. See add().
  Gtk::Button* m_button_save;
  Gtk::Button* m_button_cancel;

  bool m_block;

  bool m_modified;

  //typedef std::list<sigc::connection> type_listConnections; //Store the connections so that we can remove them later.
};

} //namespace Glom

#endif //GLOM_DIALOG_PROPERTIES_H
