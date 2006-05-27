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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "dialog_change_language.h"
#include <glibmm/i18n.h>

namespace Glom
{

Dialog_ChangeLanguage::Dialog_ChangeLanguage(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::Dialog(cobject),
  m_combo_locale(0)
{
  refGlade->get_widget_derived("combobox_locale", m_combo_locale);

  m_combo_locale->set_selected_locale(TranslatableItem::get_current_locale());
}

Dialog_ChangeLanguage::~Dialog_ChangeLanguage()
{
}

Glib::ustring Dialog_ChangeLanguage::get_locale() const
{
  return m_combo_locale->get_selected_locale();
}

} //namespace Glom
