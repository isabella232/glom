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

#include "table_columns.h"
#include <gtkmm/label.h>

Table_Columns::Table_Columns()
{
  m_uiNextRow = 0;
}

Table_Columns::~Table_Columns()
{
}

void Table_Columns::add_row(Gtk::Widget& widgetA, Gtk::Widget& widgetB)
{
  if(m_vecJustification.size() < 2)
    m_vecJustification.resize(2, Gtk::JUSTIFY_LEFT);

  Alignment_Justified* pAlignmentA = Gtk::manage(new Alignment_Justified());
  pAlignmentA->set_justification(m_vecJustification[0]);
  pAlignmentA->add(widgetA);

  Alignment_Justified* pAlignmentB = Gtk::manage(new Alignment_Justified());
  pAlignmentB->set_justification(m_vecJustification[1]);
  pAlignmentB->add(widgetB);

  attach(*pAlignmentA, 0, 1, m_uiNextRow, m_uiNextRow+1);
  attach(*pAlignmentB, 1, 2, m_uiNextRow, m_uiNextRow+1);

  m_uiNextRow++;

  widgetA.show();
  widgetB.show();
}
	
void Table_Columns::add_row(const Glib::ustring& strText, Gtk::Widget& widgetB)
{
  Gtk::Label* pLabel = Gtk::manage( new Gtk::Label(strText) );
  add_row( *pLabel, widgetB );
}

void Table_Columns::add_row(Gtk::Widget& widgetA)
{
  Alignment_Justified* pAlignment = Gtk::manage(new Alignment_Justified());
  pAlignment->set_justification(Gtk::JUSTIFY_CENTER);
  pAlignment->add(widgetA);

  attach(*pAlignment, 0, 2, m_uiNextRow, m_uiNextRow+1);

  m_uiNextRow++;
}

void Table_Columns::set_column_justification(guint col, Gtk::Justification justification)
{
  if(col >= m_vecJustification.size())
    m_vecJustification.resize(col + 1, Gtk::JUSTIFY_LEFT);

  m_vecJustification[col] = justification;
}
