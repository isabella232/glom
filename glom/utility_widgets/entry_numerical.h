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

#ifndef ENTRY_NUMERICAL_H
#define ENTRY_NUMERICAL_H

#include <gtkmm/entry.h>

/**
  *@author Murray Cumming
  */

//Gtk::Entry with methods to get/set in terms of numbers.
//This makes code a little easier to read.
//TODO: Prevent non-number characters.
class Entry_Numerical : public Gtk::Entry
{
public: 
  Entry_Numerical();
  virtual ~Entry_Numerical();
  
  virtual guint get_value_as_guint();
  virtual void set_value(guint uiVal);
};

#endif
