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

#ifndef GLOM_DIALOG_PROGRESS_CREATING_H
#define GLOM_DIALOG_PROGRESS_CREATING_H

#include <gtkmm/builder.h>
#include <gtkmm/window.h>
#include <gtkmm/label.h>
#include <gtkmm/progressbar.h>

namespace Glom
{

/** Use this to show the user that something is happening.
 * Call pulse() repeatedly to show that we are still working. 
 */
class Dialog_ProgressCreating
  : public Gtk::Window
{
public:
  Dialog_ProgressCreating(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~Dialog_ProgressCreating();

  void set_message(const Glib::ustring& title, const Glib::ustring& secondary_text);

  void pulse();

  // TODO: This should probably derive from Gtk::Dialog... armin.
  void response(int response_id);
  int run();

  virtual bool on_delete_event(GdkEventAny* event);

private:
  Gtk::ProgressBar* m_progress;
  Gtk::Label* m_label_message;

  int m_response_id;
  bool m_running;
};

} //namespace Glom

#endif //GLOM_DIALOG_PROGRESS_CREATING_H

