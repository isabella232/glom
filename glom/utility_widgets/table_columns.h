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

#ifndef TABLE_COLUMNS_H
#define TABLE_COLUMNS_H

#include "gtkmm/table.h"
#include "alignment_justified.h"
#include <vector>

namespace Glom
{

class Table_Columns : public Gtk::Table
{
public: 
  Table_Columns();
  virtual ~Table_Columns();
  
  void set_column_justification(guint col, Gtk::Justification justification);
  
  void add_row(Gtk::Widget& widgetA, Gtk::Widget& widgetB);
  void add_row(const Glib::ustring& strText, Gtk::Widget& widgetB);
  
  void add_row(Gtk::Widget& widgetA); //Span 2 columns.
    
protected:
  typedef std::vector<Gtk::Justification> type_vecJustification;
  type_vecJustification m_vecJustification;

  guint m_uiNextRow;
};

} //namespace Glom

#endif
