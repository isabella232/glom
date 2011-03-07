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

#ifndef GLOM_UTILITY_WIDGETS_NOTEBOOK_GLOM_H
#define GLOM_UTILITY_WIDGETS_NOTEBOOK_GLOM_H

#include <gtkmm.h>
#include "layoutwidgetmenu.h"
#include <libglom/data_structure/layout/layoutitem_notebook.h>
#include <gtkmm/builder.h>

namespace Glom
{

class Application;
class NotebookLabel;

class NotebookGlom
: public Gtk::Notebook,
  public LayoutWidgetMenu
{
public:
  explicit NotebookGlom(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  explicit NotebookGlom();
  virtual ~NotebookGlom();

protected:
  friend class NotebookLabel;
  void delete_from_layout();  
  
protected:
  void init();

  virtual Application* get_application();
};

} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_NOTEBOOK_GLOM_H

