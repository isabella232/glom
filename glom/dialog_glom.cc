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
 
#include "dialog_glom.h"

Dialog_Glom::Dialog_Glom(Box_DB* pBox)
{
  set_border_width(6);
 
  m_pBox = pBox;

  if(m_pBox)
  {
    m_pBox->signal_cancelled.connect(sigc::mem_fun(*this, &Dialog_Glom::on_Box_cancelled));
    m_pBox->show();
  }

  set_has_separator(false);
}

Dialog_Glom::~Dialog_Glom()
{
}

void Dialog_Glom::on_Box_cancelled()
{
  hide();
}
