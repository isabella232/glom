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

#ifndef GLOM_MODE_DATA_CALCINPROGRESS_H
#define GLOM_MODE_DATA_CALCINPROGRESS_H

#include <libglom/data_structure/field.h>

namespace Glom
{

class CalcInProgress
{
public:
  CalcInProgress();

  std::shared_ptr<const Field> m_field; 
  Gnome::Gda::Value m_value; //If it's been calculated.
  bool m_calc_in_progress;
  bool m_calc_finished;
};

} //namespace Glom

#endif //GLOM_MODE_DATA_CALCINPROGRESS_H

