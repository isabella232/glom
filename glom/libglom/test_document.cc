/* Glom
 *
 * Copyright (C) 2009 Murray Cumming
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

#include <libglom/document/document_glom.h>
#include <libglom/init.h>

int
main()
{
  Glom::libglom_init();

  Glib::ustring uri;

  try
  {
    uri = Glib::filename_to_uri("/home/murrayc/checkouts/gnome224/glom/examples/example_music_collection.glom");
  }
  catch(const Glib::ConvertError& ex)
  {
    std::cerr << "Exception from Glib::filename_to_uri(): " << ex.what();
    return 1;
  }

  std::cout << "URI=" << uri << std::endl;

  Glom::Document_Glom document;
  document.set_file_uri(uri);
  bool test = document.load();
  std::cout << "Document load result=" << test << std::endl;

  if(!test)
    return 1;

  typedef std::vector<Glib::ustring> type_vecstrings;
  const type_vecstrings table_names = document.get_table_names();
  for(type_vecstrings::const_iterator iter = table_names.begin(); iter != table_names.end(); ++iter)
  {
    std::cout << "Table: " << *iter << std::endl;
  }
  
  Glom::libglom_deinit();


  return 0;
}
