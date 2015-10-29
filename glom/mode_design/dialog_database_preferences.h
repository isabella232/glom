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

#ifndef GLOM_DIALOG_DATABASE_PREFERENCES_H
#define GLOM_DIALOG_DATABASE_PREFERENCES_H

#include <gtkmm/dialog.h>
#include <gtkmm/builder.h>
#include <glom/variablesmap.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtksourceviewmm/view.h>
#include <glom/base_db.h>
#include <libglom/data_structure/system_prefs.h>
#include <libglom/connectionpool.h>
#include "utility_widgets/imageglom.h"

namespace Glom
{

class Dialog_Database_Preferences
  : public Gtk::Dialog,
    public Base_DB
{
public:
  static const char* glade_id;
  static const bool glade_developer;

  Dialog_Database_Preferences(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~Dialog_Database_Preferences();

  std::shared_ptr<SharedConnection> connect_to_server_with_connection_settings() const;

  void load_from_document() override;
  void save_to_document() override;

private:
  void on_response(int response_id) override;

  void on_button_choose_image();
  void on_button_test_script();
  void on_treeview_cell_edited_next_value(const Glib::ustring& path_string, const Glib::ustring& new_text);
  int on_autoincrements_sort(const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b);

  //Tree model columns:
  class ModelColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:

    ModelColumns()
    { add(m_col_table); add(m_col_field); add(m_col_next_value); }

    Gtk::TreeModelColumn<Glib::ustring> m_col_table;
    Gtk::TreeModelColumn<Glib::ustring> m_col_field;
    Gtk::TreeModelColumn<long> m_col_next_value;
  };

  ModelColumns m_columns;

  Glib::RefPtr<Gtk::ListStore> m_model_autoincrements;


  Glom::VariablesMap m_glade_variables_map;
  Gtk::TreeView* m_treeview_autoincrements;

  ImageGlom* m_image;
  Gtk::Button* m_button_choose_image;

  Gsv::View* m_text_view_script;
  Gtk::Button* m_button_test_script;

  SystemPrefs m_system_prefs;
};

} //namespace Glom

#endif //GLOM_DIALOG_DATABASE_PREFERENCES_H
