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

#include "dialog_identify_original.h"
#include <glom/utils_ui.h> //For bold_message()).
#include <glom/appwindow.h>

#include <iostream>

namespace Glom
{

const char* Dialog_IdentifyOriginal::glade_id("dialog_translation_identify_original");
const bool Dialog_IdentifyOriginal::glade_developer(true);

Dialog_IdentifyOriginal::Dialog_IdentifyOriginal(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Dialog(cobject),
  m_label_original(nullptr),
  m_combo_locale(nullptr)
{
  builder->get_widget("label_original", m_label_original);
  builder->get_widget_derived("combobox_locale", m_combo_locale);
  if(m_combo_locale)
    m_combo_locale->set_selected_locale(AppWindow::get_current_locale());
}

void Dialog_IdentifyOriginal::load_from_document()
{
  std::cout << "Dialog_IdentifyOriginal::load_from_document\n";

  if(m_label_original )
    m_label_original->set_markup( UiUtils::bold_message( IsoCodes::get_locale_name( get_document()->get_translation_original_locale()) ) );

  m_combo_locale->set_selected_locale(AppWindow::get_current_locale());

  View_Glom::load_from_document();
}

Glib::ustring Dialog_IdentifyOriginal::get_locale() const
{
  return m_combo_locale->get_selected_locale();
}

} //namespace Glom
