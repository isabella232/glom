/* Glom
 *
 * Copyright (C) 2001-2016 Murray Cumming
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

#include <libglom/file_utils.h>
#include <libglom/string_utils.h>
#include <glibmm/miscutils.h>
#include <glibmm/fileutils.h>
#include <glibmm/convert.h>
#include <giomm/resource.h>
#include <libglom/utils.h>
#include <fstream>
#include <iostream>

namespace Glom
{

namespace FileUtils
{

bool file_exists(const Glib::ustring& uri)
{
  if(uri.empty())
     return false;

  //Check whether file exists already:
  // Try to examine the input file.
  auto file = Gio::File::create_for_uri(uri);
  return file_exists(file);
}

bool file_exists(const Glib::RefPtr<Gio::File>& file)
{
  try
  {
    return file->query_exists();
  }
  catch(const Gio::Error& /* ex */)
  {
    return false; //Something went wrong. It does not exist.
  }
}

bool delete_file(const std::string& uri)
{
  auto file = Gio::File::create_for_uri(uri);
  if(file->query_file_type() == Gio::FILE_TYPE_DIRECTORY)
  {
    std::cerr << G_STRFUNC << ": The file is a directory.\n";
    return false;
  }

  try
  {
    if(!file->remove())
      return false;
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << ": Exception from Gio::File: " << ex.what() << std::endl;
    return false;
  }

  return true;
}

/** For instance, to find the first file in the directory with a .glom extension.
 */
static Glib::ustring get_directory_child_with_suffix(const Glib::ustring& uri_directory, const std::string& suffix, bool recursive)
{
  auto directory = Gio::File::create_for_uri(uri_directory);
  auto enumerator = directory->enumerate_children();

  auto info = enumerator->next_file();
  while(info)
  {
    Glib::RefPtr<const Gio::File> child = directory->get_child(info->get_name());

    const Gio::FileType file_type = child->query_file_type();
    if(file_type == Gio::FILE_TYPE_REGULAR)
    {
      //Check the filename:
      const auto basename = child->get_basename();
      if(Utils::string_remove_suffix(basename, suffix) != basename)
        return child->get_uri();
    }
    else if(recursive && file_type == Gio::FILE_TYPE_DIRECTORY)
    {
      //Look in sub-directories too:
      const Glib::ustring result = get_directory_child_with_suffix(child->get_uri(), suffix, recursive);
      if(!result.empty())
        return result;
    }

    info = enumerator->next_file();
  }

  return Glib::ustring();
}

Glib::ustring get_file_uri_without_extension(const Glib::ustring& uri)
{
  if(uri.empty())
    return uri;

  auto file = Gio::File::create_for_uri(uri);
  if(!file)
    return uri; //Actually an error.

  const Glib::ustring filename_part = file->get_basename();

  const Glib::ustring::size_type pos_dot = filename_part.rfind(".");
  if(pos_dot == Glib::ustring::npos)
    return uri; //There was no extension, so just return the existing URI.
  else
  {
    const Glib::ustring filename_part_without_ext = filename_part.substr(0, pos_dot);

    //Use the Gio::File API to manipulate the URI:
    auto parent = file->get_parent();
    auto file_without_extension = parent->get_child(filename_part_without_ext);

    return file_without_extension->get_uri();
  }
}

std::string get_file_path_without_extension(const std::string& filepath)
{
  if(filepath.empty())
    return filepath;

  auto file = Gio::File::create_for_path(filepath);
  if(!file)
    return filepath; //Actually an error.

  const Glib::ustring filename_part = file->get_basename();

  const Glib::ustring::size_type pos_dot = filename_part.rfind(".");
  if(pos_dot == Glib::ustring::npos)
    return filepath; //There was no extension, so just return the existing URI.
  else
  {
    const Glib::ustring filename_part_without_ext = filename_part.substr(0, pos_dot);

    //Use the Gio::File API to manipulate the URI:
    auto parent = file->get_parent();
    auto file_without_extension = parent->get_child(filename_part_without_ext);

    return file_without_extension->get_path();
  }
}

std::string get_temp_file_path(const std::string& prefix, const std::string& extension)
{
  //Get a temporary file path:
  std::string filepath;
  try
  {
    const std::string prefix_pattern = prefix + "XXXXXX" + extension;
    const int filehandle = Glib::file_open_tmp(filepath, prefix_pattern);
    ::close(filehandle);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << ": Glib::file_open_tmp() failed\n";
    return filepath;
  }

  if(filepath.empty())
  {
    std::cerr << G_STRFUNC << ": Glib::file_open_tmp() returned an empty filepath\n";
  }

  return filepath;
}

Glib::ustring get_temp_file_uri(const std::string& prefix, const std::string& extension)
{
  try
  {
    const auto filepath = get_temp_file_path(prefix, extension);
    return Glib::filename_to_uri(filepath);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << ": Exception from filename_to_uri(): " << ex.what() << std::endl;
    return std::string();
  }
}

std::string get_temp_directory_path(const std::string& prefix)
{
  std::string result;

  const auto pattern = Glib::build_filename(
          Glib::get_tmp_dir(), prefix + "XXXXXX");

  //We must copy the pattern, because mkdtemp() modifies it:
  char* c_pattern = g_strdup(pattern.c_str());

  const char* filepath = g_mkdtemp(c_pattern);
  if(filepath)
    result = filepath;

  return result;
}

Glib::ustring get_temp_directory_uri(const std::string& prefix)
{
  try
  {
    const auto filepath = get_temp_directory_path(prefix);
    return Glib::filename_to_uri(filepath);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << ": Exception from filename_to_uri(): " << ex.what() << std::endl;
    return Glib::ustring();
  }
}


Glib::ustring create_local_image_uri(const Gnome::Gda::Value& value)
{
  static guint m_temp_image_uri_number = 0;

  Glib::ustring result;

  if(value.get_value_type() == GDA_TYPE_BINARY)
  {
    long size = 0;
    gconstpointer pData = value.get_binary(size);
    if(size && pData)
    {
      // Note that this is regular binary data, not escaped text representing the data:

      //Save the image to a temporary file and provide the file URI.
      char pchExtraNum[10];
      sprintf(pchExtraNum, "%d", m_temp_image_uri_number);
      result = ("/tmp/glom_report_image_" + Glib::ustring(pchExtraNum) + ".png");
      ++m_temp_image_uri_number;

      std::fstream the_stream(result, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
      if(the_stream)
      {
        the_stream.write((char*)pData, size);
      }
    }
    else
      std::cerr << G_STRFUNC << ": binary GdaValue contains no data.\n";
  }
  //else
  //  std::cerr << G_STRFUNC << ": type != BINARY\n";

  if(result.empty())
    result = "/tmp/glom_report_image_invalid.png";

  return ("file://" + result);
}


} //namespace FileUtils

} //namespace Glom
