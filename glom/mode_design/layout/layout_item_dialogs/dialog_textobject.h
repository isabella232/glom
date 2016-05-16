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

#ifndef GLOM_MODE_DESIGN_DIALOG_TEXTOBJECT_H
#define GLOM_MODE_DESIGN_DIALOG_TEXTOBJECT_H

#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/textview.h>
#include <gtkmm/builder.h>
#include <libglom/data_structure/layout/layoutitem_text.h>
#include <glom/base_db.h>

namespace Glom
{

class Dialog_TextObject
 : public Gtk::Dialog,
   public Base_DB //Give this class access to the current document, and to some utility methods.

{
public:
  static const char* glade_id;
  static const bool glade_developer;

  Dialog_TextObject(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

  void set_textobject(const std::shared_ptr<const LayoutItem_Text>& textobject, const Glib::ustring& table_name, bool show_title = true);
  std::shared_ptr<LayoutItem_Text> get_textobject() const;
  void get_textobject(std::shared_ptr<LayoutItem_Text>& textobject) const;

private:
  Gtk::Box* m_box_title;
  Gtk::Entry* m_entry_title;
  Gtk::TextView* m_text_view;

  std::shared_ptr<LayoutItem_Text> m_textobject;
  Glib::ustring m_table_name;
};

} //namespace Glom

#endif // GLOM_MODE_DESIGN_DIALOG_TEXTOBJECT_H
