/* Glom
 *
 * Copyright (C) 2001-2009 Murray Cumming
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

#ifndef GLOM_LIBGLOM_INIT_H
#define GLOM_LIBGLOM_INIT_H

namespace Glom
{

/** This must be used by applications other than Glom,
 * which are unlikely to otherwise initialize the libraries used by libglom.
 * Glom uses it too, just to avoid duplicating code.
 */
void libglom_init();

void libglom_deinit();

} //namespace Glom

#endif //GLOM_LIBGLOM_INIT_H
