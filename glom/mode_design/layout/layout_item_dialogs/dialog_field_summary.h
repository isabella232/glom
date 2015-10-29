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

#ifndef GLOM_LAYOUT_ITEM_DIALOGS_FIELD_SUMMARY_H
#define GLOM_LAYOUT_ITEM_DIALOGS_FIELD_SUMMARY_H

#include <gtkmm/dialog.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <glom/utility_widgets/dialog_properties.h>
#include <libglom/document/document.h>
#include <libglom/data_structure/layout/report_parts/layoutitem_fieldsummary.h>
#include <glom/box_withbuttons.h>
#include "combo_summarytype.h"

namespace Glom
{

class Dialog_FieldSummary
 : public Gtk::Dialog,
   public Base_DB
{
public:
  static const char* glade_id;
  static const bool glade_developer;

  Dialog_FieldSummary(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~Dialog_FieldSummary();

  /**
   * @param item The starting information.
   * @param table_name The item's table.
   */
  void set_item(const std::shared_ptr<const LayoutItem_FieldSummary>& item, const Glib::ustring& table_name);


  std::shared_ptr<LayoutItem_FieldSummary> get_item() const;

private:
  //Signal handlers:
  void on_button_field();

  Gtk::Label* m_label_field;
  Combo_SummaryType* m_combo_summarytype;
  Gtk::Button* m_button_field;

  mutable std::shared_ptr<LayoutItem_FieldSummary> m_layout_item;

  Glib::ustring m_table_name;
};

} //namespace Glom

#endif //GLOM_LAYOUT_ITEM_DIALOGS_FIELD_SUMMARY_H
