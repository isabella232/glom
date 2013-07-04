/* Glom
 *
 * Copyright (C) 2008 Johannes Schmid
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

#ifndef GLOM_DIALOG_FLOWTABLE_H
#define GLOM_DIALOG_FLOWTABLE_H

#include <gtkmm/dialog.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/entry.h>
#include <gtkmm/builder.h>
#include "../mode_data/flowtablewithfields.h"
#include <glom/base_db.h>

namespace Glom
{

//TODO: Is this used?
class Dialog_FlowTable
 : public Gtk::Dialog,
   public Base_DB //Give this class access to the current document, and to some utility methods.

{
public:
  static const char* glade_id;
  static const bool glade_developer;
	
  Dialog_FlowTable(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~Dialog_FlowTable();

  void set_flowtable(FlowTableWithFields* flowtable);
  
  gint get_columns_count() const;

  //TODO: Isn't this the same as Window::get_title()?
  //  Probably, yes, at least when it is properly const.
  //  so, TODO: find out what calls it.
  Glib::ustring get_title();
    
private:
  Gtk::Entry* m_entry_title;
  Gtk::SpinButton* m_spin_columns;
   
  FlowTableWithFields* m_flowtable;
  std::shared_ptr<LayoutGroup> m_layoutgroup;
};

} //namespace Glom

#endif // GLOM_DIALOG_FLOWTABLE_H
