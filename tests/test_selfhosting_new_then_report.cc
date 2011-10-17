/* Glom
 *
 * Copyright (C) 2010 Openismus GmbH
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
71 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "tests/test_selfhosting_utils.h"
#include <libglom/init.h>
#include <libglom/report_builder.h>
#include <glib.h> //For g_assert()
#include <iostream>
#include <cstdlib> //For EXIT_SUCCESS and EXIT_FAILURE

int main()
{
  Glom::libglom_init();

  Glom::Document document;
  const bool recreated = 
    test_create_and_selfhost("example_music_collection.glom", document);
  g_assert(recreated);

  const Glom::sharedptr<const Glom::Report> report_temp = 
    Glom::ReportBuilder::create_standard_list_report(&document, "albums");

  Glom::FoundSet found_set; //TODO: Test a where clause.
  found_set.m_table_name = "albums";
  Glom::ReportBuilder report_builder;
  report_builder.set_document(&document);
  const std::string filepath = 
    report_builder.report_build(found_set, report_temp);
  if(filepath.empty())
  {
    test_selfhosting_cleanup();
    return EXIT_FAILURE;
  }

  /*
  if(filecontents.find("Bruce Springsteen") == std::string::npos)
  {
    test_selfhosting_cleanup();
    return EXIT_FAILURE;
  }
  */

  test_selfhosting_cleanup();

  Glom::libglom_deinit();

  return EXIT_SUCCESS;
}
