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

#include "dialog_copy_translation.h"
#include <glom/appwindow.h>

namespace Glom
{

const char* Dialog_CopyTranslation::glade_id("dialog_translation_copy");
const bool Dialog_CopyTranslation::glade_developer(true);

Dialog_CopyTranslation::Dialog_CopyTranslation(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Dialog(cobject),
  m_combo_locale(nullptr)
{
  Gtk::Builder::get_widget_derived(builder, "combobox_locale", m_combo_locale);
  if(m_combo_locale)
    m_combo_locale->set_selected_locale(AppWindow::get_current_locale());
}

Glib::ustring Dialog_CopyTranslation::get_locale() const
{
  return m_combo_locale->get_selected_locale();
}

} //namespace Glom
