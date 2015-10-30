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

#ifndef GLOM_BOX_WITHBUTTONS_H
#define GLOM_BOX_WITHBUTTONS_H

#include <gtkmm/box.h>
#include "utility_widgets/adddel/adddel_withbuttons.h"

#include <libglom/document/document.h>
#include <libglom/connectionpool.h>
#include <libglom/appstate.h>
#include <glom/base_db.h>
#include <gtkmm/builder.h>

namespace Glom
{

/** A Gtk::Box base widget class, 
 * with some extra signals to allow derived classes to be used generically in 
 * Window_BoxHolder, allowing the dialog to respond to buttons in the box.
 */
class Box_WithButtons :
  public Gtk::Box
{
public: 
  Box_WithButtons();

  Box_WithButtons(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

  ///For use with libglademm's get_widget_derived():
  explicit Box_WithButtons(BaseObjectType* cobject);

  virtual ~Box_WithButtons();

  Gtk::Window* get_app_window();
  const Gtk::Window* get_app_window() const;

  //void show_hint(); //Public so that it can be called *after* this widget is added to its container.

  void set_button_cancel(Gtk::Button& button);

  //Signals:
  sigc::signal<void, Glib::ustring> signal_selected; //When an item is selected.
  sigc::signal<void> signal_cancelled; //When the cancel button is clicked.

  virtual Gtk::Widget* get_default_button();


private:
  //Signal handlers:
  void on_Button_Cancel();

  //virtual void hint_set(const Glib::ustring& strText);

  //Member data:
  //Glib::ustring m_strHint; //Help text.

  Gtk::Box m_Box_Buttons;
  Gtk::Button m_Button_Cancel; //Derived classes can use it if it's necessary.
};

} //namespace Glom

#endif //GLOM_BOX_WITHBUTTONS_H
