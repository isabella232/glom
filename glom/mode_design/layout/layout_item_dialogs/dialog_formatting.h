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

#ifndef GLOM_MODE_DESIGN_DIALOG_FORMATTING_H
#define GLOM_MODE_DESIGN_DIALOG_FORMATTING_H

#include <gtkmm/dialog.h>
#include <glom/utility_widgets/dialog_properties.h>
#include <libglom/document/view.h>
#include <glom/box_withbuttons.h>
#include <glom/mode_design/comboentry_currency.h>
#include "box_formatting.h"

namespace Glom
{

/** This dialog lets the user choose the formatting for non-field items.
 * Field items should use Dialog_FieldLayout instead.
 */
class Dialog_Formatting
 : public Gtk::Dialog,
   public View_Composite_Glom //Give it access to the document.
{
public:
  Dialog_Formatting();
  virtual ~Dialog_Formatting();

  /**
   * @param document The document, so that the dialog can load the previous layout, and save changes.
   * @param field The starting item information.
   */
  void set_item(const std::shared_ptr<const LayoutItem_WithFormatting>& field, bool show_numeric);

  /** Set the @a layout_item's formatting to the formatting specified in the
   * dialog by the user.
   */
  void use_item_chosen(const std::shared_ptr<LayoutItem_WithFormatting>& layout_item);

private:
  void enforce_constraints();

  Box_Formatting* m_box_formatting;
};

} //namespace Glom

#endif // GLOM_MODE_DESIGN_DIALOG_FORMATTING_H
