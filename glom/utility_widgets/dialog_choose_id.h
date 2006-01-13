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

#ifndef GLOM_MODE_DATA_DIALOG_CHOOSE_ID_H
#define GLOM_MODE_DATA_DIALOG_CHOOSE_ID_H

#include <gtkmm/dialog.h>
#include "../document/document_glom.h"
#include "../base_db.h"
#include "../mode_find/box_data_details_find.h"
#include "../mode_data/box_data_list.h"

class Dialog_ChooseID
  : public Gtk::Dialog,
    public Base_DB
{
public:
  Dialog_ChooseID();
  Dialog_ChooseID(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Dialog_ChooseID();

  virtual bool init_db_details(const Glib::ustring& table_name);

  bool get_id_chosen(Gnome::Gda::Value& chosen_id) const;

  enum enumStage
  {
    STAGE_INVALID,
    STAGE_FIND,
    STAGE_SELECT
  };

protected:

  void setup();
  void update_ui_for_stage();

  void on_button_quickfind();
  void on_box_find_criteria(const Glib::ustring& where_clause);
  void on_box_select_selected(const Gnome::Gda::Value& primary_key);

  Gtk::Label* m_label_table_name;
  Gtk::HBox* m_pBox_QuickFind; //Only show this when in Find mode.
  Gtk::Entry* m_pEntry_QuickFind;
  Gtk::Button* m_pButton_QuickFind;
  Gtk::Alignment* m_alignment_parent;

  Glib::ustring m_table_name;

  Document_Glom* m_document;
  Gnome::Gda::Value m_id_chosen;

  Box_Data_Details_Find m_box_find;
  Box_Data_List m_box_select;

  enumStage m_stage;
};

#endif //GLOM_MODE_DATA_DIALOG_CHOOSE_ID_H
