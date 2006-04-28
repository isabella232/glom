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

#ifndef DIALOG_TEXTOBJECT_H
#define DIALOG_TEXTOBJECT_H

#include <gtkmm.h>
#include <libglademm.h>
#include <glom/libglom/data_structure/layout/layoutitem_text.h>
#include "../base_db.h"

class Dialog_TextObject
 : public Gtk::Dialog,
   public Base_DB //Give this class access to the current document, and to some utility methods.

{
public:
  Dialog_TextObject(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Dialog_TextObject();

  void set_textobject(const sharedptr<const LayoutItem_Text>& textobject, const Glib::ustring& table_name);
  sharedptr<LayoutItem_Text> get_textobject() const;

protected:
  Gtk::Entry* m_entry_title;
  Gtk::TextView* m_text_view;

  sharedptr<LayoutItem_Text> m_textobject;
  Glib::ustring m_table_name;
};

#endif //DIALOG_TEXTOBJECT_H
