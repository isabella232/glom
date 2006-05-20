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

#ifndef GLOM_MODE_DATA_CALCINPROGRESS_H
#define GLOM_MODE_DATA_CALCINPROGRESS_H

#include <glom/libglom/data_structure/field.h>

class CalcInProgress
{
public:
  CalcInProgress();

  sharedptr<const Field> m_field; 
  Gnome::Gda::Value m_value; //If it's been calculated.
  bool m_calc_in_progress;
  bool m_calc_finished;
};

#endif //GLOM_MODE_DATA_CALCINPROGRESS_H

