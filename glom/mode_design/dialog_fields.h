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

#ifndef DIALOG_FIELDS_H
#define DIALOG_FIELDS_H

#include "dialog_design.h"
#include "fields/box_db_table_definition.h"

namespace Glom
{

class Dialog_Fields : public Dialog_Design
{
public:
  static const char* glade_id;
  static const bool glade_developer;

  Dialog_Fields(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~Dialog_Fields();

  bool init_db_details(const Glib::ustring& table_name) override;

private:

  Box_DB_Table_Definition* m_box;
};

} //namespace Glom

#endif //DIALOG_FIELDS_H
