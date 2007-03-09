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

#ifndef GLOM_MODE_DATA_DIALOG_CHOOSE_DATE_H
#define GLOM_MODE_DATA_DIALOG_CHOOSE_DATE_H

#include <gtkmm/dialog.h>
#include <glom/libglom/document/document_glom.h>
#include "../base_db.h"
#include "../mode_find/box_data_details_find.h"
#include "../mode_data/box_data_list.h"

namespace Glom
{

class Dialog_ChooseDate
  : public Gtk::Dialog
{
public:
  Dialog_ChooseDate();
  Dialog_ChooseDate(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Dialog_ChooseDate();

  void set_date_chosen(const Gnome::Gda::Value& value);
  Gnome::Gda::Value get_date_chosen() const;

protected:
  void on_day_selected_double_click();

  Gtk::Calendar* m_calendar;
};

} //namespace Glom

#endif //GLOM_MODE_DATA_DIALOG_CHOOSE_DATE_H
