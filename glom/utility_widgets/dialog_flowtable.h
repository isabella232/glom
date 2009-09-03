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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef DIALOG_FLOWTABLE_H
#define DIALOG_FLOWTABLE_H

#include <gtkmm.h>
#include <gtkmm/builder.h>
#include "../mode_data/flowtablewithfields.h"
#include <glom/base_db.h>

namespace Glom
{

class Dialog_FlowTable
 : public Gtk::Dialog,
   public Base_DB //Give this class access to the current document, and to some utility methods.

{
public:
  Dialog_FlowTable(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~Dialog_FlowTable();

  void set_flowtable(FlowTableWithFields* flowtable);
  
  gint get_columns_count();
  Glib::ustring get_title();
    
private:
  Gtk::Entry* m_entry_title;
  Gtk::SpinButton* m_spin_columns;
   
  FlowTableWithFields* m_flowtable;
  sharedptr<LayoutGroup> m_layoutgroup;
};

} //namespace Glom

#endif //DIALOG_TEXTOBJECT_H
