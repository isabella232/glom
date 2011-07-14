/* Glom
 *
 * Copyright (C) 2001-2004 Murray Cumming
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

#include "config.h" // For GLOM_ENABLE_MAEMO

#include <glom/utils_ui.h>
#include <libglom/connectionpool.h>
#include <libglom/data_structure/layout/report_parts/layoutitem_fieldsummary.h>
#include <libglom/data_structure/glomconversions.h>

#include <libglom/data_structure/layout/layoutitem_image.h> // For GLOM_IMAGE_FORMAT
#include <gdkmm/pixbufloader.h>
#include <libgda/gda-blob-op.h> // For gda_blob_op_read_all()

#include <gtkmm/messagedialog.h>
#include <gtkmm/stock.h>

#include <giomm.h>

#ifdef GLOM_ENABLE_MAEMO
#include <hildonmm/note.h>
#endif

#include <glibmm/i18n.h>

#include <string.h> // for strchr
#include <sstream> //For stringstream

#include <iostream>
#include <fstream>

#include <locale>     // for locale, time_put
#include <ctime>     // for struct tm
#include <iostream>   // for cout, endl
#include <iomanip>

#include <stack>

namespace
{

// Basically copied from libgnome (gnome-help.c, Copyright (C) 2001 Sid Vicious
// Copyright (C) 2001 Jonathan Blandford <jrb@alum.mit.edu>), but C++ified
std::string locate_help_file(const std::string& path, const std::string& doc_name)
{
  // g_get_language_names seems not to be wrapped by glibmm
  const char* const* lang_list = g_get_language_names ();

  for(unsigned int j = 0; lang_list[j] != 0; ++j)
  {
    const char* lang = lang_list[j];

    /* This must be a valid language AND a language with
     * no encoding postfix.  The language will come up without
     * encoding next. */
    if(lang == 0 || strchr(lang, '.') != 0)
      continue;

    const char* exts[] = { "", ".xml", ".docbook", ".sgml", ".html", 0 };
    for(unsigned i = 0; exts[i] != 0; ++i)
    {
      std::string name = doc_name + exts[i];
      std::string full = Glib::build_filename(path, Glib::build_filename(lang, name));

      if(Glib::file_test(full, Glib::FILE_TEST_EXISTS))
        return full;
    }
  }

  return std::string();
}

} //anonymous namespace

namespace Glom
{

// Run dialog and response on Help if appropriate.
int Utils::dialog_run_with_help(Gtk::Dialog* dialog, const Glib::ustring& id)
{
  int result = dialog->run();
  
  //Maemo has no help system since Maemo 5, 
  //so we hide the buttons in anyway.
  #ifndef GLOM_ENABLE_MAEMO
  while (result == Gtk::RESPONSE_HELP)
  {
    show_help(id);
    result = dialog->run();
  }
  #endif //GLOM_ENABLE_MAEMO

  dialog->hide();
  return result;
}

/*
 * Help::show_help(const std::string& id)
 *
 * Launch a help browser with the glom help and load the given id if given
 * If the help cannot be found an error dialog will be shown
 */

// Maemo has no help system since Maemo 5 (Fremantle).
#ifndef GLOM_ENABLE_MAEMO
void Utils::show_help(const Glib::ustring& id)
{
  GError* err = 0;
  const gchar* pId;
  if(id.length())
  {
    pId = id.c_str();
  }
  else
  {
    pId = 0;
  }

  try
  {
    const char path[] = GLOM_DATADIR G_DIR_SEPARATOR_S "gnome"
                                     G_DIR_SEPARATOR_S "help"
                                     G_DIR_SEPARATOR_S "glom";
    std::string help_file = locate_help_file(path, "glom.xml");
    if(help_file.empty())
    {
      throw std::runtime_error(_("No help file available"));
    }
    else
    {
      std::string uri = "ghelp:" + help_file;
      if(pId) { uri += '?'; uri += pId; }

      // g_app_info_launch_default_for_uri seems not to be wrapped by giomm
      if(!g_app_info_launch_default_for_uri(uri.c_str(), 0, &err))
      {
        std::string message(err->message);
        g_error_free(err);
        throw std::runtime_error(message);
      }
    }
  }
  catch(const std::exception& ex)
  {
    const Glib::ustring message = _("Could not display help: ") + Glib::ustring(ex.what());
    Gtk::MessageDialog dialog(message, false, Gtk::MESSAGE_ERROR);
    dialog.run();
  }
}
#endif //GLOM_ENABLE_MAEMO

void Utils::show_ok_dialog(const Glib::ustring& title, const Glib::ustring& message, Gtk::Window* parent, Gtk::MessageType message_type)
{
#undef GLOM_ENABLE_MAEMO
#ifdef GLOM_ENABLE_MAEMO
  // TODO_maemo: Map message_type to a sensible stock_id?
  Hildon::Note dialog(Hildon::NOTE_TYPE_INFORMATION, parent, message);
#else
  Gtk::MessageDialog dialog("<b>" + title + "</b>", true /* markup */, message_type, Gtk::BUTTONS_OK);
  dialog.set_secondary_text(message);
  if(parent)
    dialog.set_transient_for(*parent);
#endif

  dialog.run();
}

void Utils::show_ok_dialog(const Glib::ustring& title, const Glib::ustring& message, Gtk::Window& parent, Gtk::MessageType message_type)
{
  show_ok_dialog(title, message, &parent, message_type);
}

namespace
{

static void on_window_hide(Glib::RefPtr<Glib::MainLoop> main_loop, sigc::connection handler_connection)
{
  handler_connection.disconnect(); //This should release a main_loop reference.
  main_loop->quit();

  //main_loop should be destroyed soon, because nothing else is using it.
}

} //anonymous namespace.

void Utils::show_window_until_hide(Gtk::Window* window)
{
  if(!window)
    return;

  Glib::RefPtr<Glib::MainLoop> main_loop = Glib::MainLoop::create(false /* not running */);

  //Stop the main_loop when the window is hidden:
  sigc::connection handler_connection; //TODO: There seems to be a crash if this is on the same line.
  handler_connection = window->signal_hide().connect( 
    sigc::bind(
      sigc::ptr_fun(&on_window_hide),
      main_loop, handler_connection
    ) );
  
  window->show();
  main_loop->run(); //Run and block until it is stopped by the hide signal handler.
}

Glib::ustring Utils::bold_message(const Glib::ustring& message)
{
  return "<b>" + message + "</b>";
}


Glib::RefPtr<Gdk::Pixbuf> Utils::get_pixbuf_for_gda_value(const Gnome::Gda::Value& value)
{
  Glib::RefPtr<Gdk::Pixbuf> result;

  if(value.get_value_type() == GDA_TYPE_BINARY || value.get_value_type() == GDA_TYPE_BLOB)
  {
    glong buffer_binary_length;
    gconstpointer buffer_binary;
    if(value.get_value_type() == GDA_TYPE_BLOB)
    {
      const GdaBlob* blob = value.get_blob();
      if(gda_blob_op_read_all(blob->op, const_cast<GdaBlob*>(blob)))
      {
        buffer_binary_length = blob->data.binary_length;
        buffer_binary = blob->data.data;
      }
      else
      {
        buffer_binary_length = 0;
        buffer_binary = 0;
        g_warning("Conversions::get_pixbuf_for_gda_value(): Failed to read BLOB data");
      }
    }
    else
    {
      buffer_binary = value.get_binary(buffer_binary_length);
    }

    /* Note that this is regular binary data, not escaped text representing the binary data: */
    if(buffer_binary && buffer_binary_length)
    {
      //typedef std::list<Gdk::PixbufFormat> type_list_formats;
      //const type_list_formats formats = Gdk::Pixbuf::get_formats();
      //std::cout << "Debug: Supported pixbuf formats:" << std::endl;
      //for(type_list_formats::const_iterator iter = formats.begin(); iter != formats.end(); ++iter)
      //{
      //  std::cout << " name=" << iter->get_name() << ", writable=" << iter->is_writable() << std::endl;
      //}

      Glib::RefPtr<Gdk::PixbufLoader> refPixbufLoader;      
      try
      {
        refPixbufLoader = Gdk::PixbufLoader::create();
      }
      catch(const Gdk::PixbufError& ex)
      {
        refPixbufLoader.reset();
        std::cerr << "PixbufLoader::create failed: " << ex.what() << std::endl;
      }

      if(refPixbufLoader)
      {
        guint8* puiData = (guint8*)buffer_binary;
        try
        {
          refPixbufLoader->write(puiData, static_cast<gsize>(buffer_binary_length));
          result = refPixbufLoader->get_pixbuf();

          refPixbufLoader->close(); //This throws if write() threw, so it must be inside the try block.
        }

        catch(const Glib::Exception& ex)
        {
          g_warning("Conversions::get_pixbuf_for_gda_value(): PixbufLoader::write() failed: %s", ex.what().c_str());
        }
      }

      //TODO: load the image, using the mime type stored elsewhere.
      //pixbuf = Gdk::Pixbuf::create_from_data(
    }

  }

  return result;
}

static int get_width_for_text(Gtk::Widget& widget, const Glib::ustring& text)
{
  //Get the width required for this string in the current font:
  Glib::RefPtr<Pango::Layout> refLayout = widget.create_pango_layout(text);
  int width = 0;
  int height = 0;
  refLayout->get_pixel_size(width, height);
  int result = width;

  //Add a bit more:
  result += 10;

  return result;
}

int Utils::get_suitable_field_width_for_widget(Gtk::Widget& widget, const sharedptr<const LayoutItem_Field>& field_layout, bool or_title)
{
  int result = 150; //Suitable default.

  const Field::glom_field_type field_type = field_layout->get_glom_type();

  Glib::ustring example_text;
  switch(field_type)
  {
    case(Field::TYPE_DATE):
    {
      Glib::Date date(31, Glib::Date::Month(12), 2000);
      example_text = Conversions::get_text_for_gda_value(field_type, Gnome::Gda::Value(date));
      break;
    }
    case(Field::TYPE_TIME):
    {
      Gnome::Gda::Time time = {0, 0, 0, 0, 0};
      time.hour = 24;
      time.minute = 59;
      time.second = 59;
      example_text = Conversions::get_text_for_gda_value(field_type, Gnome::Gda::Value(time));
      break;
    }
    case(Field::TYPE_NUMERIC):
    {
#ifdef GLOM_ENABLE_MAEMO
      //Maemo's screen is not so big, so don't be so generous:
      example_text = "EUR 9999999";
#else
      example_text = "EUR 9999999999";
#endif //GLOM_ENABLE_MAEMO
      break;
    }
    case(Field::TYPE_TEXT):
    case(Field::TYPE_IMAGE): //Give images the same width as text fields, so they will often line up.
    {
      //if(!field_layout->get_text_format_multiline()) //Use the full width for multi-line text.
#ifdef GLOM_ENABLE_MAEMO
        //Maemo's screen is not so big, so don't be so generous:
        example_text = "AAAAAAAAAAAAAAAA";
#else
        example_text = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
#endif //GLOM_ENABLE_MAEMO

      break;
    }
    default:
    {
      break;
    }
  }


  if(!example_text.empty())
  {
    //Get the width required for this string in the current font:
    result = get_width_for_text(widget, example_text);
  }

  if(or_title)
  {
    //Make sure that there's enough space for the title too.
    const int title_width = get_width_for_text(widget, field_layout->get_title());
    if(title_width > result)
      result = title_width;
  }

  return result;
}


std::string Utils::get_filepath_with_extension(const std::string& filepath, const std::string& extension)
{
  std::string result = filepath;

  bool add_ext = false;
  const std::string str_ext = "." + extension;

  if(result.size() < str_ext.size()) //It can't have the ext already if it's not long enough.
  {
    add_ext = true; //It isn't there already.
  }
  else
  {
    const Glib::ustring strEnd = result.substr(result.size() - str_ext.size());
    if(strEnd != str_ext) //If it doesn't already have the extension
      add_ext = true;
  }

  //Add extension if necessay.
  if(add_ext)
    result += str_ext;

  //TODO: Do not replace existing extensions, so it could be e.g. 'something.blah.theext'

  return result;
}


//static:
Glib::RefPtr<Gdk::Pixbuf> Utils::image_scale_keeping_ratio(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf, int target_height, int target_width)
{
  if( (target_height == 0) || (target_width == 0) )
    return Glib::RefPtr<Gdk::Pixbuf>(); //This shouldn't happen anyway.

  if(!pixbuf)
    return pixbuf;

  enum enum_scale_mode
  {
    SCALE_WIDTH,
    SCALE_HEIGHT,
    SCALE_BOTH,
    SCALE_NONE
  };

  enum_scale_mode scale_mode = SCALE_NONE; //Start with either the width or height, and scale the other according to the ratio.

  const int pixbuf_height = pixbuf->get_height();
  const int pixbuf_width = pixbuf->get_width();

  if(pixbuf_height > target_height)
  {
    if(pixbuf_width > target_width)
    {
      scale_mode = SCALE_BOTH;
    }
    else
    {
      //Only the height is bigger:
      scale_mode = SCALE_HEIGHT;
    }
  }
  else if(pixbuf_width > target_width)
  {
    //Only the height is bigger:
    scale_mode = SCALE_WIDTH;
  }

  if(scale_mode == SCALE_NONE)
    return pixbuf;
  else if(scale_mode == SCALE_HEIGHT)
  {
    const float ratio = (float)target_height / (float)pixbuf_height; 
    target_width = (int)((float)pixbuf_width * ratio);
  }
  else if(scale_mode == SCALE_WIDTH)
  {
    const float ratio = (float)target_width / (float) pixbuf_width;
    target_height = (int)((float)pixbuf_height * ratio);
  }
  else if(scale_mode == SCALE_BOTH)
  {
    const float ratio = std::min(
      (float)target_width / (float) pixbuf_width,
      (float)target_height / (float) pixbuf_height);
    target_width = (int)((float)pixbuf_width * ratio);
    target_height = (int)((float)pixbuf_height * ratio);
  }

 if( (target_height == 0) || (target_width == 0) )
 {
   return Glib::RefPtr<Gdk::Pixbuf>(); //This shouldn't happen anyway. It seems to happen sometimes though, when ratio is very small.
 }

  return pixbuf->scale_simple(target_width, target_height, Gdk::INTERP_NEAREST);
}

bool Utils::show_warning_no_records_found(Gtk::Window& transient_for)
{
  const Glib::ustring message = _("Your find criteria did not match any records in the table.");

#ifdef GLOM_ENABLE_MAEMO
  Hildon::Note dialog(Hildon::NOTE_TYPE_CONFIRMATION_BUTTON, transient_for, message);
#else
  Gtk::MessageDialog dialog(Utils::bold_message(_("No Records Found")), true, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_NONE);
  dialog.set_secondary_text(message);
  dialog.set_transient_for(transient_for);
#endif


  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(_("New Find"), Gtk::RESPONSE_OK);

  const bool find_again = (dialog.run() == Gtk::RESPONSE_OK);
  return find_again;
}


} //namespace Glom
