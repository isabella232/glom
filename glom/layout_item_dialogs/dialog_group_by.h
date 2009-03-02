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

#ifndef GLOM_LAYOUT_ITEM_DIALOGS_GROUP_BY_H
#define GLOM_LAYOUT_ITEM_DIALOGS_GROUP_BY_H

#include <gtkmm.h>
#include "../utility_widgets/dialog_properties.h"
#include <glom/libglom/document/document_glom.h>
#include "../box_withbuttons.h"
#include "../utility_widgets/combo_textglade.h"
#include "../utility_widgets/comboentry_currency.h"
#include "dialog_groupby_secondaryfields.h"
#include "dialog_groupby_sortfields.h"
#include "comboentry_borderwidth.h"

namespace Glom
{

class Dialog_GroupBy
 : public Gtk::Dialog,
   public Base_DB
{
public:
  Dialog_GroupBy(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Dialog_GroupBy();

  /**
   * @param item The starting information.
   * @param table_name The item's table.
   */
  virtual void set_item(const sharedptr<const LayoutItem_GroupBy>& item, const Glib::ustring& table_name);


  sharedptr<LayoutItem_GroupBy> get_item() const;

private:
  //Signal handlers:
  void on_button_field_group_by();
  void on_button_formatting_group_by();
  void on_button_field_sort_by();
  void on_button_secondary_fields();
  void on_dialog_secondary_fields_hide();

  void update_labels();

  Gtk::Label* m_label_group_by;
  Gtk::Label* m_label_sort_by;
  Gtk::Label* m_label_secondary_fields;
  Gtk::Button* m_button_field_group_by;
  Gtk::Button* m_button_formatting_group_by;
  Gtk::Button* m_button_field_sort_by;
  Gtk::Button* m_button_secondary_fields;
  ComboEntry_BorderWidth* m_comboboxentry_border_width;

  Dialog_GroupBy_SecondaryFields* m_dialog_choose_secondary_fields;
  Dialog_GroupBy_SortFields* m_dialog_choose_sort_fields;

  mutable sharedptr<LayoutItem_GroupBy> m_layout_item;

  Glib::ustring m_table_name;
};

} //namespace Glom

#endif //GLOM_LAYOUT_ITEM_DIALOGS_GROUP_BY_H
