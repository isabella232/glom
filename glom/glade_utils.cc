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

#include <glom/glade_utils.h>
#include <glibmm/i18n.h>

namespace Glom
{

namespace Utils
{


Dialog_ProgressCreating* get_and_show_pulse_dialog(const Glib::ustring& message, Gtk::Window* parent_window)
{
  if(!parent_window)
    std::cerr << G_STRFUNC << ": parent_window is NULL" << std::endl;

  Dialog_ProgressCreating* dialog_progress = 0;
  Utils::get_glade_widget_derived_with_warning(dialog_progress);
  if(!dialog_progress) //Unlikely and it already warns on stderr.
    return 0;
    
  dialog_progress->set_message(_("Processing"), message);
  dialog_progress->set_modal();

  if(parent_window)
    dialog_progress->set_transient_for(*parent_window);

  dialog_progress->show();

  return dialog_progress;
}

} //namespace Utils
} //namespace Glom

