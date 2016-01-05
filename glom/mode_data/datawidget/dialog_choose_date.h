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

#ifndef GLOM_MODE_DATA_DIALOG_CHOOSE_DATE_H
#define GLOM_MODE_DATA_DIALOG_CHOOSE_DATE_H

#include <gtkmm/dialog.h>
#include <libglom/document/document.h>
#include <glom/base_db.h>
//#include <glom/mode_find/box_data_details_find.h>
#include <glom/mode_data/box_data_list.h>
#include <gtkmm/calendar.h>

namespace Glom
{

namespace DataWidgetChildren
{

class Dialog_ChooseDate
  : public Gtk::Dialog
{
public:
  static const char* glade_id;
  static const bool glade_developer;

  Dialog_ChooseDate();
  Dialog_ChooseDate(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

  void set_date_chosen(const Gnome::Gda::Value& value);
  Gnome::Gda::Value get_date_chosen() const;

private:
  void on_day_selected_double_click();

  Gtk::Calendar* m_calendar;
};

} //namespace DataWidetChildren
} //namespace Glom

#endif //GLOM_MODE_DATA_DIALOG_CHOOSE_DATE_H
