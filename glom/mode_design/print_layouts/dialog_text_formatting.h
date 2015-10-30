/* Glom
 *
 * Copyright (C) 2007 Murray Cumming
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

#ifndef GLOM_DIALOG_TEXT_FORMATTING_H
#define GLOM_DIALOG_TEXT_FORMATTING_H

#include <gtkmm/dialog.h>
#include <gtkmm/builder.h>
#include <libglom/document/document.h>
#include <glom/mode_design/layout/layout_item_dialogs/box_formatting.h>

namespace Glom
{

//TODO: Rename this? It seems to be only used for Print Layouts. Or why not use Dialog_Formatting instead?
/** A dialog with a titled frame, a label for the table title, and a close button.
 */
class Dialog_TextFormatting
: public Gtk::Window,
  public View_Composite_Glom
{
public:
  static const char* glade_id;
  static const bool glade_developer;

  Dialog_TextFormatting(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~Dialog_TextFormatting();

  //Allow direct access, for convenience:
  Gtk::Box* m_box_formatting_placeholder;
  Box_Formatting* m_box_formatting;

private:
  void on_button_close();
};

} //namespace Glom

#endif //GLOM_DIALOG_TEXT_FORMATTING_H
