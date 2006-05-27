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

#ifndef GLOM_DIALOG_RELATIONSHIPS_OVERVIEW
#define GLOM_DIALOG_RELATIONSHIPS_OVERVIEW

#include <glom/libglom/document/document_glom.h>
#include <gtkmm/dialog.h>
#include <libglademm.h>
#include "relationships_canvas.h"

namespace Glom
{

class Dialog_RelationshipsOverview 
  : public Gtk::Dialog,
    public Base_DB //Give it access to the document, and to the database utilities
{
public:
  Dialog_RelationshipsOverview(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Dialog_RelationshipsOverview();

protected:

  Gtk::ScrolledWindow* m_scrolledwindow_canvas;
  RelationshipsCanvas m_canvas;
  //Gnome::Canvas::Group m_canvas_group;
};

} //namespace Glom

#endif //GLOM_DIALOG_RELATIONSHIPS_OVERVIEW
