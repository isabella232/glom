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

#ifndef GLOM_MODE_DESIGN_DIALOG_BUTTONSCRIPT_H
#define GLOM_MODE_DESIGN_DIALOG_BUTTONSCRIPT_H

#include <gtkmm.h>
#include <gtkmm/builder.h>
#include <libglom/data_structure/layout/layoutitem_button.h>
#include <glom/base_db.h>

#include <gtksourceviewmm/sourceview.h>

namespace Glom
{

class Dialog_ButtonScript
 : public Gtk::Dialog,
   public Base_DB //Give this class access to the current document, and to some utility methods.

{
public:
  static const char* glade_id;
  static const bool glade_developer;
  
  Dialog_ButtonScript(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~Dialog_ButtonScript();

  void set_script(const sharedptr<const LayoutItem_Button>& script, const Glib::ustring& table_name);
  sharedptr<LayoutItem_Button> get_script() const;
  void get_script (const sharedptr<LayoutItem_Button>& script) const;

private:
  void on_button_test_script();

  Gtk::Entry* m_entry_title;
  gtksourceview::SourceView* m_text_view_script;
  Gtk::Button* m_button_test_script;

  sharedptr<LayoutItem_Button> m_script;
  Glib::ustring m_table_name;
};

} //namespace Glom

#endif // GLOM_MODE_DESIGN_DIALOG_BUTTONSCRIPT_H
