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

#ifndef DIALOG_SCRIPTLIBRARY_H
#define DIALOG_SCRIPTLIBRARY_H

#include <gtkmm/dialog.h>
#include <gtkmm/builder.h>
#include <libglom/data_structure/layout/layoutitem_button.h>
#include <glom/base_db.h>
#include <gtksourceviewmm/view.h>
#include <glom/utility_widgets/combo_textglade.h>

namespace Glom
{

class Dialog_ScriptLibrary
 : public Gtk::Dialog,
   public Base_DB //Give this class access to the current document, and to some utility methods.

{
public:
  static const char* glade_id;
  static const bool glade_developer;

  Dialog_ScriptLibrary(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~Dialog_ScriptLibrary();

  virtual void load_from_document() override;
  virtual void save_to_document() override;

private:
  void on_button_add();
  void on_button_remove();
  void on_button_check();
  void on_combo_name_changed();



  void load_current_script();
  void save_current_script();

  Combo_TextGlade* m_combobox_name;

  Gsv::View* m_text_view;
  Gtk::Button* m_button_check;
  Gtk::Button* m_button_add;
  Gtk::Button* m_button_remove;

  Glib::ustring m_current_name;
};

} //namespace Glom

#endif //DIALOG_SCRIPTLIBRARY_H
