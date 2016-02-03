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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#ifndef GLOM_TRANSLATIONS_PO
#define GLOM_TRANSLATIONS_PO

#include <libglom/document/document.h>

namespace Glom
{

/** Create a pot template file that can be used by translators to create a new .po file.
 * @param document The document whose translations should be written to a .po file.
 * @param pot_file The filepath at which to create a .po file.
 */
bool write_pot_file(const std::shared_ptr<Document>& document, const Glib::ustring& pot_file_uri);

/** Create a po file containing the translations from the Glom document.
 * @param document The document whose translations should be written to a .po file.
 * @param po_file The filepath at which to create a .po file.
 * @param translation_locale For instance, de_DE.
 * @param locale_name For instance, Deutsch, to identify the translation team.
 */
bool write_translations_to_po_file(const std::shared_ptr<Document>& document, const Glib::ustring& po_file_uri, const Glib::ustring& translation_locale, const Glib::ustring& locale_name = Glib::ustring());

/** Parse a po file, storing its translations in the Glom document.
 * @param document The document into which the translations should be stored.
 * @param po_file The filepath at which to find a .po file.
 * @param translation_locale For instance, de_DE.
 */
bool import_translations_from_po_file(const std::shared_ptr<Document>& document, const Glib::ustring& po_file_uri, const Glib::ustring& translation_locale);

/** Get a hint about what the text is for.
 * This is also necessary to uniquely identify the item,
 * because not all text with the same contents should be translated the same 
 * way in all languages - the context might change the translation.
 */ 
Glib::ustring get_po_context_for_item(const std::shared_ptr<const TranslatableItem>& item, const Glib::ustring& hint);

} //namespace Glom

#endif //GLOM_TRANSLATIONS_PO
