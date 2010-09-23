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

#ifndef GLOM_DOCUMENT_VIEW_H
#define GLOM_DOCUMENT_VIEW_H

#include <libglom/document/document.h>
#include <libglom/document/bakery/view/view_composite.h>

namespace Glom
{

///The base View for the document.
typedef GlomBakery::View<Document> View_Glom;

typedef GlomBakery::View_Composite<Document> View_Composite_Glom;

} //namespace Glom

#endif // GLOM_DOCUMENT_VIEW_H
