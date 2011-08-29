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

#ifndef GLOM_MODE_DESIGN_DIALOG_LINE_H
#define GLOM_MODE_DESIGN_DIALOG_LINE_H

#include <gtkmm.h>
#include <gtkmm/builder.h>
#include <libglom/data_structure/layout/layoutitem_line.h>
#include <glom/base_db.h>

namespace Glom
{

class Dialog_Line
 : public Gtk::Dialog,
   public Base_DB //Give this class access to the current document, and to some utility methods.

{
public:
  static const char* glade_id;
  static const bool glade_developer;
  
  Dialog_Line(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~Dialog_Line();

  void set_line(const sharedptr<const LayoutItem_Line>& line);
  sharedptr<LayoutItem_Line> get_line() const;

private:
  Gtk::SpinButton* m_spinbutton_line_width;
  Gtk::ColorButton* m_colorbutton;

  sharedptr<LayoutItem_Line> m_line;
};

} //namespace Glom

#endif // GLOM_MODE_DESIGN_DIALOG_LINE_H
