/* Glom
 *
 * Copyright (C) 2001-2012 Murray Cumming
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

#ifndef GLOM_TRANSLATIONS_PO
#define GLOM_TRANSLATIONS_PO

#include <libglom/document/document.h>

namespace Glom
{

bool write_translations_to_po_file(Document* document, const Glib::ustring& po_file_uri, const Glib::ustring& translation_locale);

bool import_translations_from_po_file(Document* document, const Glib::ustring& po_file_uri, const Glib::ustring& translation_locale);

} //namespace Glom

#endif //GLOM_TRANSLATIONS_PO
