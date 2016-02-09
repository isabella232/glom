/*
 * Copyright 2002 Murray Cumming
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "config.h"
#include <libglom/document/bakery/document.h>
#include <libglom/utils.h>
#include <giomm/file.h>
#include <iostream>
#include <glibmm/i18n-lib.h>

namespace GlomBakery
{

const guint BYTES_TO_PROCESS = 256;

Document::Document()
{
  m_bIsNew = true;
  m_bModified = false;
  m_bReadOnly = false;
  m_pView = nullptr;
}

Document::~Document()
{
}

Glib::ustring Document::get_file_uri() const
{
  return m_file_uri;
}

Glib::ustring Document::get_file_uri_with_extension(const Glib::ustring& uri)
{
  Glib::ustring result = uri;

  //Enforce file extension:
  if(!m_file_extension.empty())  //If there is an extension to enforce.
  {
    bool bAddExt = false;
    const auto strExt = '.' + get_file_extension();

    if(result.size() < strExt.size()) //It can't have the ext already if it's not long enough.
    {
      bAddExt = true; //It isn't there already.
    }
    else
    {
      const auto strEnd = result.substr(result.size() - strExt.size());
      if(strEnd != strExt) //If it doesn't already have the extension
        bAddExt = true;
    }

    //Add extension if necessay.
    if(bAddExt)
      result += strExt;

    //Note that this does not replace existing extensions, so it could be e.g. 'something.blah.theext'
  }

  return result;
}

void Document::set_file_uri(const Glib::ustring& file_uri, bool bEnforceFileExtension /* = false */)
{
  if(file_uri != m_file_uri)
    set_modified(); //Ready to save() for a Save As.

  m_file_uri = file_uri;

  //Enforce file extension:
  if(bEnforceFileExtension)
    m_file_uri = get_file_uri_with_extension(m_file_uri);
}

Glib::ustring Document::get_contents() const
{
  return m_strContents;
}

void Document::set_modified(bool bVal /* = true */)
{
  m_bModified = bVal;

  if(m_bModified)
  {
    m_bIsNew = false; //Can't be new if it's been modified.
  }

  //Allow the application or view to update it's UI accordingly:
  signal_modified().emit(m_bModified);
}

bool Document::get_modified() const
{
  return m_bModified;
}

bool Document::load(int& failure_code)
{
  //Initialize the output parameter:
  failure_code = Glom::Utils::to_utype(LoadFailureCodes::NONE);

  auto bTest = read_from_disk(failure_code);
  if(bTest)
  {
    bTest = load_after(failure_code); //may be overridden.
    if(bTest)
    {
      //Tell the View to show the new data:
      if(m_pView)
        m_pView->load_from_document();
    }
  }

  set_is_new(false);
  return bTest;
}

bool Document::load_from_data(const guchar* data, std::size_t length, int& failure_code)
{
  if(!data || !length)
    return false;

  //Initialize the output parameter:
  failure_code = 0;

  //We use the std::string constructor, because that takes the number
  //of bytes, rather than the ustring constructor, which takes the number
  //of characters.
  try
  {
    m_strContents = std::string((char*)data, length);
  }
  catch(const std::exception& ex)
  {
    std::cerr << G_STRFUNC << ": Exception instantiating std::string: " << ex.what() << std::endl;
    return false;
  }

  const auto bTest = load_after(failure_code); //may be overridden.
  if(bTest)
  {
    //Tell the View to show the new data:
    if(m_pView)
      m_pView->load_from_document();
  }

  set_is_new(false);
  return bTest;
}



bool Document::load_after(int& failure_code)
{
  //Called after text is read from disk, but before updating view.

  //Override this if necessary.
  //For instance, Document_XML parses the XML.

  failure_code = 0;
  return true;
}

bool Document::save()
{
  //Tell the view to update the data in this document.
  if(m_pView)
    m_pView->save_to_document();

  const auto bTest = save_before(); //This could be overridden.
  if(bTest)
    return write_to_disk();

  return bTest;
}

bool Document::save_before()
{
  //Called after view saves itself to document, but before writing to disk.

  //Override this if necessary.
  //For instance, Document_XML serializes its XML to text.

  return true;
}

bool Document::read_from_disk(int& failure_code)
{
  failure_code = Glom::Utils::to_utype(LoadFailureCodes::NONE);

  m_strContents.erase();

  // Open the input file for read access:
  if(m_file_uri.empty())
    return false;

  auto file = Gio::File::create_for_uri(m_file_uri);

  Glib::RefPtr<Gio::FileInputStream> stream;

  try
  {
    stream = file->read();
  }
  catch(const Gio::Error& ex)
  {
    std::cerr << G_STRFUNC << ": Error: " << ex.what() << std::endl;
    std::cerr << G_STRFUNC << ":  with m_file_uri=" << m_file_uri << std::endl;


    if(ex.code() == Gio::Error::NOT_FOUND)
      failure_code = Glom::Utils::to_utype(LoadFailureCodes::NOT_FOUND);
    //  std::cout << "  File not found: " << m_file_uri << std::endl;

    // If the operation was not successful, print the error and abort
    return false; //print_error(ex, input_uri_string);
  }

  // Read data from the input uri:
  guint buffer[BYTES_TO_PROCESS] = {0, }; // For each chunk.
  gsize bytes_read = 0;
  std::string data; //We use a std::string because we might not get whole UTF8 characters at a time. This might not be necessary.

  try
  {
    bool bContinue = true;
    while(bContinue)
    {
      bytes_read = stream->read(buffer, BYTES_TO_PROCESS);

      if(bytes_read == 0)
        bContinue = false; //stop because we reached the end.
      else
      {
        // Add the text to the string:
        data += std::string((char*)buffer, bytes_read);
      }
    }
  }
  catch(const Gio::Error& ex)
  {
    // If the operation was not successful, print the error and abort
    return false; //print_error(ex, input_uri_string);
  }

  m_strContents = data;

  set_modified(false);

  return true; //Success.
}

bool Document::write_to_disk()
{
  if(m_file_uri.empty())
  {
    std::cerr << G_STRFUNC << ": m_file_uri is empty.\n";
    return false;
  }

  //Write the changed data to disk:
  if(get_modified())
  {
    auto file = Gio::File::create_for_uri(m_file_uri);
    Glib::RefPtr<Gio::FileOutputStream> stream;

    //Create the file if it does not already exist:
    if(file->query_exists())
    {
      try
      {
        stream = file->replace(); //Instead of append_to().
      }
      catch(const Gio::Error& ex)
      {
        std::cerr << G_STRFUNC << ": " << ex.what() << std::endl;
        return false;
      }
    }
    else
    {
      //Make sure that all the parent directories exist, creating them if necessary:
      auto parent = file->get_parent();
      try
      {
        if(parent) //It will be empty if file was the root node of the filesystem.
          parent->make_directory_with_parents();
      }
      catch(const Gio::Error& ex)
      {
        //If it exists already then that's good.
        //Otherwise something unexpected happened.
        if(ex.code() == Gio::Error::EXISTS)
        {
          if(parent->query_file_type() != Gio::FILE_TYPE_DIRECTORY)
          {
            std::cerr << G_STRFUNC << ": This part of the URI is not a directory: " << parent->get_uri() <<  std::endl;
            std::cerr << G_STRFUNC << ":   using m_file_uri = " << m_file_uri << std::endl;
            return false;
          } 
        }
        else
        {
          std::cerr << G_STRFUNC << ": parent of uri=" << m_file_uri << "error=" << ex.what() << std::endl;
          return false;
        }
      }


      //Create the file:
      //By default files created are generally readable by everyone, but if we pass FILE_CREATE_PRIVATE in flags the file will be made readable only to the current user, to the level that is supported on the target filesystem.
      //TODO: Do we want to specify 0660 exactly? (means "this user and his group can read and write this non-executable file".)
      try
      {
        stream = file->create_file();
      }
      catch(const Gio::Error& ex)
      {
        std::cerr << G_STRFUNC << ": " << m_file_uri << ", error=" << ex.what() << std::endl;
        return false;
      }
    }

    if(!stream)
      return false;


    try
    {
      //Write the data to the output uri
      stream->write(m_strContents.data(), m_strContents.bytes());

      //Close the stream to make sure that the write really happens
      //even with glibmm 2.16.0 which had a refcount leak that stopped it.
      stream->close();
      stream.reset();
    }
    catch(const Gio::Error& ex)
    {
      // If the operation was not successful, print the error and abort
      std::cerr << G_STRFUNC << ": " << ex.what() << std::endl;
      return false; //print_error(ex, output_uri_string);
    }

    return true; //Success. (At doing nothing, because nothing needed to be done.)
  }
  else
    return true; //Success. (At doing nothing, because nothing needed to be done.)
}

Glib::ustring Document::get_name() const
{
  return util_file_uri_get_name(m_file_uri, m_file_extension);
}

//Note that Glib::filename_display_basename() shows %20 for spaces so it isn't very useful.
static Glib::ustring get_file_display_name(const Glib::ustring& uri)
{
  Glib::ustring result;

  if(uri.empty())
    return result;

  auto file = Gio::File::create_for_uri(uri);
  Glib::RefPtr<const Gio::FileInfo> file_info;

  try
  {
    file_info = file->query_info(G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << ": uri=" << uri << "): error: " << ex.what() << std::endl;
    return result;
  }

  if(!file_info)
    return result;

  return file_info->get_display_name();
}

Glib::ustring Document::util_file_uri_get_name(const Glib::ustring& file_uri, const Glib::ustring& file_extension)
{
  auto strResult = get_file_display_name(file_uri);

  //Remove the file extension:
  //TODO: Maybe g_filename_display_basename() and g_file_info_get_display_name() should do this.
  if(!strResult.empty() && !file_extension.empty())
  {
    const Glib::ustring strExt = '.' + file_extension;

    if(strResult.size() >= file_extension.size()) //It can't have the ext already if it's not long enough.
    {
      const auto strEnd = strResult.substr(strResult.size() - strExt.size());
      if(strEnd == strExt) //If it has the extension
      {
        strResult = strResult.substr(0, strResult.size() - strExt.size());
      }
    }
  }

  //Show untitled explicitly:
  //Also happens for file_uris with path but no name. e.g. /sub/sub/, which shouldn't happen.
  if(strResult.empty())
    strResult = _("Untitled");

  return strResult;
}

void Document::set_view(ViewBase* pView)
{
  m_pView = pView;
}

ViewBase* Document::get_view()
{
  return m_pView;
}

bool Document::get_read_only() const
{
  if(m_bReadOnly)
  {
    //An application might have used set_read_only() to make this document explicitly read_only, regardless of the positions of the storage location.
    return true;
  }
  else
  {
    if(m_file_uri.empty())
      return false; //It must be a default empty document, not yet saved, so it is not read-only.
    else
    {
      auto file = Gio::File::create_for_uri(m_file_uri);
      Glib::RefPtr<Gio::FileInfo> info;
      try
      {
        info = file->query_info(G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE);
      }
      catch(const Gio::Error& ex)
      {
        return false; //We should at least be able to read the permissions, so maybe the location is invalid. I'm not sure what the best return result here is.
      }

      if(!info)
        return false;

      const auto read_only = !info->get_attribute_boolean(G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE);
      return read_only;
    }
  }
}

void Document::set_read_only(bool bVal)
{
  m_bReadOnly = bVal;
}

bool Document::get_is_new() const
{
  return m_bIsNew;
}

void Document::set_is_new(bool bVal)
{
  if(bVal)
    set_modified(false); //can't be modified if it is new.

  m_bIsNew = bVal;
}

void Document::set_file_extension(const Glib::ustring& strVal)
{
  m_file_extension = strVal;
}

Glib::ustring Document::get_file_extension() const
{
  return m_file_extension; //TODO: get it from the mime-type system?
}

Document::type_signal_modified& Document::signal_modified()
{
  return signal_modified_;
}

} //namespace
