/* Glom
 *
 * Copyright (C) 2011 Murray Cumming
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

#ifndef GLOM_TEST_SELFHOSTING_UTILS_H
#define GLOM_TEST_SELFHOSTING_UTILS_H

#include <libglom/document/document.h>
#include <libgdamm/datamodel.h>
#include <string>

bool test_create_and_selfhost(const std::string& example_filename, Glom::Document& document);

bool test_model_expected_size(const Glib::RefPtr<Gnome::Gda::DataModel>& data_model, guint columns_count, guint rows_count);
bool test_table_exists(const Glib::ustring& table_name, const Glom::Document& document);

void test_selfhosting_cleanup();

#endif //GLOM_TEST_SELFHOSTING_UTILS_H

