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

#ifndef GLOM_LAYOUT_ITEM_DIALOGS_FIELD_SUMMARY_H
#define GLOM_LAYOUT_ITEM_DIALOGS_FIELD_SUMMARY_H

#include <gtkmm.h>
#include "../utility_widgets/dialog_properties.h"
#include "../document/document_glom.h"
#include "../data_structure/layout/report_parts/layoutitem_fieldsummary.h"
#include "../box_db.h"
#include "../utility_widgets/combo_textglade.h"
#include "combo_summarytype.h"

class Dialog_FieldSummary
 : public Gtk::Dialog,
   public Base_DB
{
public:
  Dialog_FieldSummary(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Dialog_FieldSummary();

  /**
   * @param item The starting information.
   * @param table_name The item's table.
   */
  virtual void set_item(const sharedptr<const LayoutItem_FieldSummary>& item, const Glib::ustring& table_name);


  sharedptr<LayoutItem_FieldSummary> get_item() const;

protected:
  //Signal handlers:
  void on_button_field();

  Gtk::Label* m_label_field;
  Combo_SummaryType* m_combo_summarytype;
  Gtk::Button* m_button_field;

  mutable sharedptr<LayoutItem_FieldSummary> m_layout_item;

  Glib::ustring m_table_name;
};

#endif //GLOM_LAYOUT_ITEM_DIALOGS_FIELD_SUMMARY_H
