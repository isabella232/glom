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

#ifndef GLOM_TRANSLATIONS_DIALOG_CHANGE_LANGUAGE_H
#define GLOM_TRANSLATIONS_DIALOG_CHANGE_LANGUAGE_H

#include "combobox_locale.h"
#include <gtkmm/dialog.h>
#include <gtkmm/builder.h>

namespace Glom
{

/**
 */
class Dialog_ChangeLanguage
  : public Gtk::Dialog
{
public:
  static const char* glade_id;
  static const bool glade_developer;

  Dialog_ChangeLanguage(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

  Glib::ustring get_locale() const;

private:
  ComboBox_Locale* m_combo_locale;
};

} //namespace Glom

#endif //GLOM_TRANSLATIONS_DIALOG_CHANGE_LANGUAGE_H

