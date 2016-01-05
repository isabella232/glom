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

#ifndef GLOM_MODE_DESIGN_USERS_DIALOG_CHOOSE_USER_H
#define GLOM_MODE_DESIGN_USERS_DIALOG_CHOOSE_USER_H

#include <gtkmm/dialog.h>
#include <gtkmm/builder.h>
#include <gtkmm/entry.h>
#include "../../utility_widgets/combo_textglade.h"

namespace Glom
{

class Dialog_ChooseUser : public Gtk::Dialog
{
public:
  static const char* glade_id;
  static const bool glade_developer;

  Dialog_ChooseUser(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

  typedef std::vector<Glib::ustring> type_vec_strings;
  void set_user_list(const type_vec_strings& users);

  Glib::ustring get_user() const;

private:
  Combo_TextGlade* m_combo_name;
};

} //namespace Glom

#endif //GLOM_MODE_DESIGN_USERS_DIALOG_CHOOSER_USER_H
