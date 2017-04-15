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

#include "dialog_choose_date.h"
//#include <libgnome/gnome-i18n.h>

namespace Glom
{

namespace DataWidgetChildren
{

const char* Dialog_ChooseDate::glade_id("dialog_choose_date");
const bool Dialog_ChooseDate::glade_developer(false);

Dialog_ChooseDate::Dialog_ChooseDate()
: m_calendar(nullptr)
{
}

Dialog_ChooseDate::Dialog_ChooseDate(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Dialog(cobject),
  m_calendar(nullptr)
{
  builder->get_widget("calendar", m_calendar);

  m_calendar->signal_day_selected_double_click().connect(sigc::mem_fun(*this, &Dialog_ChooseDate::on_day_selected_double_click));
}

void Dialog_ChooseDate::set_date_chosen(const Gnome::Gda::Value& value)
{
  if(value.get_value_type() == G_TYPE_DATE) //Otherwise GtkCalendar defaults to the current (today's) date.
  {
    Glib::Date date = value.get_date();

    guint month = static_cast<guint>(date.get_month());
    if(month != 0)
      --month; //Gtk::Calendar months start at 0.

    m_calendar->select_month(month, date.get_year());
    m_calendar->select_day(date.get_day());
  }
}

Gnome::Gda::Value Dialog_ChooseDate::get_date_chosen() const
{
  guint year = 0;
  guint month = 0;
  guint day = 0;

  m_calendar->get_date(year, month, day);

  ++ month; //GtkCalendar months start at 0.
  if(month > 12) month = 12;

  Glib::Date date(day, static_cast<Glib::Date::Month>(month), year);
  return Gnome::Gda::Value(date);
}

void Dialog_ChooseDate::on_day_selected_double_click()
{
  //Close the dialog:
  response(Gtk::ResponseType::OK);
}

} //namespace DataWidetChildren
} //namespace Glom


