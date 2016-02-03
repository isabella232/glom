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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#include "tests/test_selfhosting_utils.h"
#include <libglom/init.h>
#include <libglom/report_builder.h>
#include <glib.h> //For g_assert()
#include <iostream>
#include <cstdlib> //For EXIT_SUCCESS and EXIT_FAILURE


static bool test(Glom::Document::HostingMode hosting_mode)
{
  auto document = std::make_shared<Glom::Document>();
  const bool recreated = 
    test_create_and_selfhost_from_example("example_music_collection.glom", document, hosting_mode);
  if(!recreated)
  {
    std::cerr << G_STRFUNC << ": Recreation failed." << std::endl;
    return false;
  }

  const auto report_temp = 
    Glom::ReportBuilder::create_standard_list_report(document, "albums");

  Glom::FoundSet found_set; //TODO: Test a where clause.
  found_set.m_table_name = "albums";

  const std::locale locale("en_US.UTF-8"); //Instead of just "" (current locale) so we get the same results each time.
  Glom::ReportBuilder report_builder(locale);
  report_builder.set_document(document);
  const Glib::ustring html = 
    report_builder.report_build(found_set, report_temp);

  if(html.empty())
  {
    std::cerr << G_STRFUNC << ": Failed: html was empty." << std::endl;
    return false;
  }

  if(html.find("Bruce Springsteen") == std::string::npos)
  {
    std::cerr << G_STRFUNC << ": Failed: html did not contain the expected text." << std::endl;
    return false;
  }

  test_selfhosting_cleanup();
    
  return true;
}

int main()
{
  Glom::libglom_init();
  
  const auto result = test_all_hosting_modes(sigc::ptr_fun(&test));

  Glom::libglom_deinit();

  return result;
}
