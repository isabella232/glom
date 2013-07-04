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

#ifndef GLOM_MODE_DESIGN_DIALOG_FIELD_LAYOUT_H
#define GLOM_MODE_DESIGN_DIALOG_FIELD_LAYOUT_H

#include <gtkmm/dialog.h>
#include <gtkmm/label.h>
#include <gtkmm/entry.h>
#include <glom/utility_widgets/dialog_properties.h>
#include <libglom/document/view.h>
#include <glom/box_withbuttons.h>
#include <glom/mode_design/comboentry_currency.h>
#include "box_formatting.h"

namespace Glom
{

class Dialog_FieldLayout
 : public Gtk::Dialog,
   public View_Composite_Glom //Give it access to the document.
{
public:
  static const char* glade_id;
  static const bool glade_developer;

  Dialog_FieldLayout(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~Dialog_FieldLayout();

  /**
   * @param document The document, so that the dialog can load the previous layout, and save changes.
   * @param field The starting field information.
   * @param table_name The field's table.
   */
  virtual void set_field(const std::shared_ptr<const LayoutItem_Field>& field, const Glib::ustring& table_name, bool show_editable_options = true);

  std::shared_ptr<LayoutItem_Field> get_field_chosen() const;

private:
  void on_radiobutton_custom_formatting();
  void enforce_constraints();

  Gtk::Label* m_label_field_name;
  Gtk::CheckButton* m_checkbutton_editable;

  Gtk::CheckButton* m_radiobutton_title_default;
  Gtk::Label* m_label_title_default;
  Gtk::CheckButton* m_radiobutton_title_custom;
  Gtk::Entry* m_entry_title_custom;


  Gtk::Box* m_box_formatting_placeholder;
  Gtk::RadioButton* m_radiobutton_custom_formatting;
  Box_Formatting* m_box_formatting;

  mutable std::shared_ptr<LayoutItem_Field> m_layout_item;

  Glib::ustring m_table_name;
};

} //namespace Glom

#endif // GLOM_MODE_DESIGN_DIALOG_FIELD_LAYOUT_H
