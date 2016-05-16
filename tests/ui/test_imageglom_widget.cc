/* Glom
 *
 * Copyright (C) 2016 Murray Cumming
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

#include <gtkmm/application.h>
#include <gtkmm/window.h>
#include <glibmm/miscutils.h>
#include <glom/utility_widgets/imageglom.h>
#include "tests/test_utils.h"
#include "tests/test_utils_images.h"

const std::string IMAGE_FILE_JPG1 = "test_image.jpg";
const std::string IMAGE_FILE_JPG2 = "test_image2.jpg";
const std::string IMAGE_FILE_PDF1 = "test_image.pdf";
const std::string IMAGE_FILE_PDF2 = "test_image2.pdf";

auto get_image(const std::string& name)
{
  const auto filename =
    Glib::build_filename(GLOM_TESTS_IMAGE_DATA_NOTINSTALLED, name);
  return get_value_for_image_from_file(filename);
}

void test_use_jpg()
{
  Glom::ImageGlom imageWidget;
  imageWidget.set_value(get_image(IMAGE_FILE_JPG1));
}

void test_reset_same_jpg()
{
  Glom::ImageGlom imageWidget;
  imageWidget.set_value(get_image(IMAGE_FILE_JPG1));
  imageWidget.set_value(get_image(IMAGE_FILE_JPG1));
}

void test_changing_jpg()
{
  Glom::ImageGlom imageWidget;
  imageWidget.set_value(get_image(IMAGE_FILE_JPG1));
  imageWidget.set_value(get_image(IMAGE_FILE_JPG2));
  imageWidget.set_value(get_image(IMAGE_FILE_JPG1));
}

void test_use_pdf()
{
  Glom::ImageGlom imageWidget;
  imageWidget.set_value(get_image(IMAGE_FILE_PDF1));
}

void test_reset_same_pdf()
{
  Glom::ImageGlom imageWidget;
  imageWidget.set_value(get_image(IMAGE_FILE_PDF1));
  imageWidget.set_value(get_image(IMAGE_FILE_PDF1));
}

void test_changing_pdf()
{
  Glom::ImageGlom imageWidget;
  imageWidget.set_value(get_image(IMAGE_FILE_PDF1));
  imageWidget.set_value(get_image(IMAGE_FILE_PDF2));
  imageWidget.set_value(get_image(IMAGE_FILE_PDF1));
}

void test_changing_from_jpg_to_pdf_to_jpg()
{
  Glom::ImageGlom imageWidget;
  imageWidget.set_value(get_image(IMAGE_FILE_JPG1));
  imageWidget.set_value(get_image(IMAGE_FILE_PDF1));
  imageWidget.set_value(get_image(IMAGE_FILE_JPG2));
}


int main(int argc, char *argv[])
{
  auto app =
    Gtk::Application::create(argc, argv, "org.glom.test_glade_derived_instantiation");

  test_use_jpg();
  test_reset_same_jpg();
  test_changing_jpg();

  test_use_pdf();
  test_reset_same_pdf();
  test_changing_pdf();

  test_changing_from_jpg_to_pdf_to_jpg();

  return EXIT_SUCCESS;
}
