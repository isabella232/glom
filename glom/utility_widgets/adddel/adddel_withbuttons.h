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

#ifndef GLOM_UTILITY_WIDGETS_ADDDEL_WITHBUTTONS_H
#define GLOM_UTILITY_WIDGETS_ADDDEL_WITHBUTTONS_H

#include "adddel.h"
#include <gtkmm/buttonbox.h>

namespace Glom
{

class AddDel_WithButtons : public AddDel
{
public: 
  AddDel_WithButtons();
  AddDel_WithButtons(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

  void set_allow_add(bool val = true) override;
  void set_allow_delete(bool val = true) override;
  void set_allow_user_actions(bool bVal = true) override;

  /**
   * @param label The button label text, including the mnemonic underline.
   */
  void set_edit_button_label(const Glib::ustring& label);

  /** Set the label of the extra button, if any.
   * If there is no label text (the default) then the button will not be shown.
   *
   * @param label The button label text, including the mnemonic underline.
   */
  void set_extra_button_label(const Glib::ustring& label);

private:
  void init();
  void setup_buttons();

  void on_button_add();
  void on_button_del();
  void on_button_edit();
  void on_button_extra();

  void show_all_vfunc() override;

  //member widgets:
  Gtk::ButtonBox m_ButtonBox;
  Gtk::Button m_Button_Add;
  Gtk::Button m_Button_Del;
  Gtk::Button m_Button_Edit;
  Gtk::Button m_Button_Extra;

  Glib::ustring m_label_extra;
};

} //namespace Glom

#endif // GLOM_UTLITY_WIDGETS_ADDDEL_WITHBUTTONS_H
