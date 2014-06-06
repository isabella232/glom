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

#ifndef GLOM_MODE_DATA_DIALOG_NEW_RECORD_H
#define GLOM_MODE_DATA_DIALOG_NEW_RECORD_H

#include <gtkmm/dialog.h>
#include <libglom/document/document.h>
#include <glom/base_db.h>
#include <glom/mode_data/box_data_details.h>

namespace Glom
{

namespace DataWidgetChildren
{

class Dialog_NewRecord
  : public Gtk::Dialog,
    public View_Composite_Glom
{
public:
  static const char* glade_id;
  static const bool glade_developer;

  Dialog_NewRecord();
  Dialog_NewRecord(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~Dialog_NewRecord();

  bool init_db_details(const Glib::ustring& table_name, const Glib::ustring& layout_platform);

  bool get_id_chosen(Gnome::Gda::Value& chosen_id) const;

private:

  void setup();

  Gtk::Label* m_label_table_name;
  Gtk::Box* m_vbox_parent;

  Glib::ustring m_table_name;
  Glib::ustring m_layout_platform;

  Box_Data_Details m_box_details;
};

} //namespace DataWidetChildren
} //namespace Glom

#endif //GLOM_MODE_DATA_DIALOG_NEW_RECORD_H
