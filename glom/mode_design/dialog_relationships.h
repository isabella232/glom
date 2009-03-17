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

#ifndef DIALOG_RELATIONSHIPS_H
#define DIALOG_RELATIONSHIPS_H

#include "dialog_design.h"
#include "box_db_table_relationships.h"

namespace Glom
{

class Dialog_Relationships : public Dialog_Design
{
public:
  Dialog_Relationships(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~Dialog_Relationships();

  virtual bool init_db_details(const Glib::ustring& table_name);

  virtual void on_hide();

private:

  Box_DB_Table_Relationships* m_box;
};

} //namespace Glom

#endif //DIALOG_RELATIONSHIPS_H
