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

#ifndef GLOM_MODE_DATA_DIALOG_CHOOSE_ID_H
#define GLOM_MODE_DATA_DIALOG_CHOOSE_ID_H

#include <gtkmm/dialog.h>
#include <libglom/document/document.h>
#include <glom/base_db.h>
#include <glom/mode_find/box_data_details_find.h>
#include <glom/mode_data/box_data_list.h>

namespace Glom
{

namespace DataWidgetChildren
{

class Dialog_ChooseID
  : public Gtk::Dialog,
    public View_Composite_Glom
{
public:
  static const char* glade_id;
  static const bool glade_developer;

  Dialog_ChooseID();
  Dialog_ChooseID(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~Dialog_ChooseID();

  bool init_db_details(const Glib::ustring& table_name, const Glib::ustring& layout_platform);

  bool get_id_chosen(Gnome::Gda::Value& chosen_id) const;

  enum class enumStage
  {
    INVALID,
    FIND,
    SELECT
  };

private:

  void setup();
  void update_ui_for_stage();

  void on_button_quickfind();
  void on_box_find_criteria(const Gnome::Gda::SqlExpr& where_clause);
  void on_box_select_selected(const Gnome::Gda::Value& primary_key);

  Gtk::Label* m_label_table_name;
  Gtk::Box* m_box_quick_find; //Only show this when in Find mode.
  Gtk::Entry* m_entry_quick_find;
  Gtk::Button* m_button_quick_find;
  Gtk::Box* m_vbox_parent;

  Glib::ustring m_table_name;
  Glib::ustring m_layout_platform;

  std::shared_ptr<Document> m_document;
  Gnome::Gda::Value m_id_chosen;

  Box_Data_Details_Find m_box_find;
  Box_Data_List m_box_select;

  enumStage m_stage;
};

} //namespace DataWidetChildren
} //namespace Glom

#endif //GLOM_MODE_DATA_DIALOG_CHOOSE_ID_H
