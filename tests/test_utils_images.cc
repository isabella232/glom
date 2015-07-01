/* Glom
 *
 * Copyright (C) 2012 Murray Cumming
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

#include "tests/test_utils_images.h"
#include <gdkmm/wrap_init.h> //TODO: Add a Gdk::init() to gtkmm.
#include <gdkmm/pixbufloader.h>
#include <iostream>

bool check_value_is_an_image(const Gnome::Gda::Value& value)
{
  g_assert(!value.is_null());
  g_assert(value.get_value_type() == GDA_TYPE_BINARY);
  long data_length = 0;
  const auto data = value.get_binary(data_length);
  g_assert(data);
  g_assert(data_length);
  //Check that it can be interpreted as an image:
  //Luckily, the use of GdkPixbufLoader here does not seem to require an X display.
  Gdk::wrap_init();
  Glib::RefPtr<Gdk::PixbufLoader> refPixbufLoader;      
  try
  {
    refPixbufLoader = Gdk::PixbufLoader::create();
  }
  catch(const Gdk::PixbufError& ex)
  {
    std::cerr << G_STRFUNC << ": PixbufLoader::create failed: " << ex.what() << std::endl;
    return false;
  }

  Glib::RefPtr<Gdk::Pixbuf> pixbuf;
  guint8* puiData = (guint8*)data;
  try
  {
    refPixbufLoader->write(puiData, static_cast<gsize>(data_length));
    pixbuf = refPixbufLoader->get_pixbuf();
    refPixbufLoader->close(); //This throws if write() threw, so it must be inside the try block.
  }
  catch(const Glib::Exception& ex)
  {
    std::cerr << G_STRFUNC << ": PixbufLoader::write() failed: " << ex.what() << std::endl;
    return false;
  }

  return true;
}

