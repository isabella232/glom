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

#include "dialog_choose_date.h"
#include <bakery/App/App_Gtk.h> //For util_bold_message().
//#include <libgnome/gnome-i18n.h>
#include <glibmm/i18n.h>

namespace Glom
{

Dialog_ChooseDate::Dialog_ChooseDate()
: m_calendar(0)
{
}

Dialog_ChooseDate::Dialog_ChooseDate(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::Dialog(cobject),
  m_calendar(0)
{
  refGlade->get_widget("calendar", m_calendar);

  m_calendar->signal_day_selected_double_click().connect(sigc::mem_fun(*this, &Dialog_ChooseDate::on_day_selected_double_click));
}

Dialog_ChooseDate::~Dialog_ChooseDate()
{
}

void Dialog_ChooseDate::set_date_chosen(const Glib::ValueBase& value)
{
  if(G_VALUE_TYPE(value.gobj()) == G_TYPE_DATE) //Otherwise GtkCalendar defaults to the current (today's) date.
  {
    Gnome::Gda::Date date = value.get_date();

    guint month = date.month;
    if(month != 0)
      --month; //Gtk::Calendar months start at 0.

    m_calendar->select_month(month, date.year);
    m_calendar->select_day(date.day);
  }
}

Glib::ValueBase Dialog_ChooseDate::get_date_chosen() const
{
  Gnome::Gda::Date date = {0, 0, 0};
  guint year = 0;
  guint month = 0;
  guint day = 0;

  m_calendar->get_date(year, month, day);
  date.year = year;

  date.month = month + 1; //GtkCalendar months start at 0.
  if(date.month > 12)
    date.month = 12;

  date.day = day;

  return Glib::ValueBase(date);
}

void Dialog_ChooseDate::on_day_selected_double_click()
{
  //Close the dialog:
  response(Gtk::RESPONSE_OK);
}

} //namespace Glom


