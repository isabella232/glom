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
    std::cerr << "debug: Glom: get_and_show_pulse_dialog(): parent_window is NULL" << std::endl;

#ifdef GLIBMM_EXCEPTIONS_ENABLED
  Glib::RefPtr<Gtk::Builder> refXml = Gtk::Builder::create_from_file(Utils::get_glade_file_path("glom.glade"), "window_progress");
#else
  std::auto_ptr<Glib::Error> error;
  Glib::RefPtr<Gtk::Builder> refXml = Gtk::Builder::create_from_file(Utils::get_glade_file_path("glom.glade"), "window_progress", "", error);
  if(error.get())
    return 0;
#endif

  if(refXml)
  {
    Dialog_ProgressCreating* dialog_progress = 0;
    refXml->get_widget_derived("window_progress", dialog_progress);
    if(dialog_progress)
    {
      dialog_progress->set_message(_("Processing"), message);
      dialog_progress->set_modal();

      if(parent_window)
        dialog_progress->set_transient_for(*parent_window);

      dialog_progress->show();

      return dialog_progress;
    }
  }

  return 0;
}

} //namespace Utils
} //namespace Glom

