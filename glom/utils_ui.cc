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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#include "config.h" // For GLOM_ENABLE_CLIENT_ONLY

#include <glom/utils_ui.h>
#include <glom/appwindow.h>
#include <libglom/connectionpool.h>
#include <libglom/data_structure/layout/report_parts/layoutitem_fieldsummary.h>
#include <libglom/data_structure/glomconversions.h>

#include <libglom/data_structure/layout/layoutitem_image.h> // For GLOM_IMAGE_FORMAT
#include <gdkmm/pixbufloader.h>
#include <libgda/gda-blob-op.h> // For gda_blob_op_read_all()

#include <gtkmm/messagedialog.h>
#include <glibmm/convert.h>
#include <glibmm/miscutils.h>
#include <glibmm/fileutils.h>

#include <giomm/file.h>
#include <glibmm/main.h>
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

// For ShellExecute:
#ifdef G_OS_WIN32
# include <windows.h>
#endif

namespace
{

static void on_css_parsing_error(const Glib::RefPtr<const Gtk::CssSection>& section, const Glib::Error& error,
    const Glib::ustring& error_clue)
{
  std::cerr << G_STRFUNC << ": Parsing error: " << error.what() << std::endl;
  std::cerr << "  css: " << error_clue << std::endl;

  if(section)
  {
    const auto file = section->get_file();
    if (file)
    {
      std::cerr << " URI = " << file->get_uri() << std::endl;
    }

    std::cerr << " start_line = " << section->get_start_line()+1
      << ", end_line = " << section->get_end_line()+1 << std::endl;
    std::cerr << " start_position = " << section->get_start_position()
      << ", end_position = " << section->get_end_position() << std::endl;
  }
}

static Glib::RefPtr<Gtk::CssProvider> create_css_provider(Gtk::Widget& widget, const Glib::ustring& error_clue)
{
  // Add a StyleProvider so we can change the color, background color, and font.
  // This was easier before Gtk::Widget::override_color() was deprecated.
  Glib::RefPtr<Gtk::CssProvider> css_provider = Gtk::CssProvider::create();

  Glib::RefPtr<Gtk::StyleContext> refStyleContext = widget.get_style_context();
  if(refStyleContext)
    refStyleContext->add_provider(css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  css_provider->signal_parsing_error().connect(
    sigc::bind(
      sigc::ptr_fun(&on_css_parsing_error),
      error_clue));

  return css_provider;
}


static void load_into_css_provider(Gtk::Widget& widget, const Glib::ustring& css)
{
  auto css_provider = create_css_provider(widget, css);

  try
  {
    css_provider->load_from_data(css);
  }
  catch(const Gtk::CssProviderError& ex)
  {
    std::cerr << G_STRFUNC << ": Gtk::CssProvider::load_from_data() failed with CssProviderError: "
      << ex.what() << std::endl;
    std::cerr << "  with css: " << css << std::endl;
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << ": Gtk::CssProvider::load_from_data() failed with Error: "
      << ex.what() << std::endl;
    std::cerr << "  with css: " << css << std::endl;
  }
}

} //anonymous namespace

namespace Glom
{

// Run dialog and response on Help if appropriate.
int UiUtils::dialog_run_with_help(Gtk::Dialog* dialog, const Glib::ustring& id)
{
  int result = dialog->run();
  
  while (result == Gtk::RESPONSE_HELP)
  {
    show_help(dialog, id);
    result = dialog->run();
  }

  dialog->hide();
  return result;
}

/*
 * Help::show_help(const std::string& id)
 *
 * Launch a help browser with the glom help and load the given id if given
 * If the help cannot be found an error dialog will be shown
 */
void UiUtils::show_help(Gtk::Window* parent_window, const Glib::ustring& id)
{
  //TODO: Check that this actually works for any dialog that has an ID in the help files.
  Glib::ustring uri = "help:glom";
  if (!id.empty())
    uri += "/" + id;
    
  try
  {
    //Use the GNOME help browser:
    GError* gerror = nullptr;
    Glib::RefPtr<Gdk::Screen> screen;
    if(parent_window)
      screen = parent_window->get_screen();

    if(!gtk_show_uri(screen ? screen->gobj() : nullptr,
      uri.c_str(), GDK_CURRENT_TIME, &gerror))
    {
      std::cerr << G_STRFUNC << ": " << gerror->message << std::endl;

      const Glib::ustring message(gerror->message);
      g_error_free(gerror);
      throw std::runtime_error(message);
    }
  }
  catch(const std::exception& ex)
  {
    const auto message = Glib::ustring::compose(_("Could not display help: %1"), Glib::ustring(ex.what()));
    Gtk::MessageDialog dialog(message, false, Gtk::MESSAGE_ERROR);
    dialog.run();
  }
}

void UiUtils::show_ok_dialog(const Glib::ustring& title, const Glib::ustring& message, Gtk::Window* parent, Gtk::MessageType message_type)
{
  Gtk::MessageDialog dialog("<b>" + title + "</b>", true /* markup */, message_type, Gtk::BUTTONS_OK);
  dialog.set_secondary_text(message);
  if(parent)
    dialog.set_transient_for(*parent);

  dialog.run();
}

void UiUtils::show_ok_dialog(const Glib::ustring& title, const Glib::ustring& message, Gtk::Window& parent, Gtk::MessageType message_type)
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

void UiUtils::show_window_until_hide(Gtk::Window* window)
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

Glib::ustring UiUtils::bold_message(const Glib::ustring& message)
{
  return "<b>" + message + "</b>";
}


Glib::RefPtr<Gdk::Pixbuf> UiUtils::get_pixbuf_for_gda_value(const Gnome::Gda::Value& value)
{
  Glib::RefPtr<Gdk::Pixbuf> result;

  if(value.get_value_type() == GDA_TYPE_BINARY || value.get_value_type() == GDA_TYPE_BLOB)
  {
    glong buffer_binary_length;
    gconstpointer buffer_binary;
    if(value.get_value_type() == GDA_TYPE_BLOB)
    {
      const auto blob = value.get_blob();
      if(gda_blob_op_read_all(blob->op, const_cast<GdaBlob*>(blob)))
      {
        buffer_binary_length = blob->data.binary_length;
        buffer_binary = blob->data.data;
      }
      else
      {
        buffer_binary_length = 0;
        buffer_binary = 0;
        std::cerr << G_STRFUNC << ": Failed to read BLOB data" << std::endl;
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
      //for(const auto& item : formats)
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
        std::cerr << G_STRFUNC << ": PixbufLoader::create failed: " << ex.what() << std::endl;
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
          std::cerr << G_STRFUNC << ": PixbufLoader::write() failed: " << ex.what() << std::endl;
        }
      }

      //TODO: load the image, using the mime type stored elsewhere.
      //pixbuf = Gdk::Pixbuf::create_from_data(
    }

  }

  return result;
}

namespace {

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

} //anonymous namespace.

int UiUtils::get_suitable_field_width_for_widget(Gtk::Widget& widget, const std::shared_ptr<const LayoutItem_Field>& field_layout, bool or_title, bool for_treeview)
{
  int result = 150; //Suitable default.

  const auto field_type = field_layout->get_glom_type();

  Glib::ustring example_text;
  switch(field_type)
  {
    case(Field::glom_field_type::DATE):
    {
      const Glib::Date date(31, Glib::Date::Month(12), 2000);
      example_text = Conversions::get_text_for_gda_value(field_type, Gnome::Gda::Value(date));
      break;
    }
    case(Field::glom_field_type::TIME):
    {
      Gnome::Gda::Time time = {0, 0, 0, 0, 0};
      time.hour = 24;
      time.minute = 59;
      time.second = 59;
      example_text = Conversions::get_text_for_gda_value(field_type, Gnome::Gda::Value(time));
      break;
    }
    case(Field::glom_field_type::NUMERIC):
    {
      if(for_treeview)
        example_text = "EUR 999.99";
      else
        example_text = "EUR 9999999999";
        
      break;
    }
    case(Field::glom_field_type::TEXT):
    case(Field::glom_field_type::IMAGE): //Give images the same width as text fields, so they will often line up.
    {
      if(for_treeview)
        example_text = "AAAAAAAAAAAA";
      else
        example_text = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";

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
    const auto title_width = get_width_for_text(widget, item_get_title(field_layout));
    if(title_width > result)
      result = title_width;
  }

  return result;
}


std::string UiUtils::get_filepath_with_extension(const std::string& filepath, const std::string& extension)
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
    const auto strEnd = result.substr(result.size() - str_ext.size());
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
Glib::RefPtr<Gdk::Pixbuf> UiUtils::image_scale_keeping_ratio(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf, int target_height, int target_width)
{
  if( (target_height == 0) || (target_width == 0) )
    return Glib::RefPtr<Gdk::Pixbuf>(); //This shouldn't happen anyway.

  if(!pixbuf)
    return pixbuf;

  enum class enum_scale_mode
  {
    WIDTH,
    HEIGHT,
    BOTH,
    NONE
  };

  enum_scale_mode scale_mode = enum_scale_mode::NONE; //Start with either the width or height, and scale the other according to the ratio.

  const auto pixbuf_height = pixbuf->get_height();
  const auto pixbuf_width = pixbuf->get_width();

  if(pixbuf_height > target_height)
  {
    if(pixbuf_width > target_width)
    {
      scale_mode = enum_scale_mode::BOTH;
    }
    else
    {
      //Only the height is bigger:
      scale_mode = enum_scale_mode::HEIGHT;
    }
  }
  else if(pixbuf_width > target_width)
  {
    //Only the height is bigger:
    scale_mode = enum_scale_mode::WIDTH;
  }

  if(scale_mode == enum_scale_mode::NONE)
    return pixbuf;
  else if(scale_mode == enum_scale_mode::HEIGHT)
  {
    const float ratio = (float)target_height / (float)pixbuf_height; 
    target_width = (int)((float)pixbuf_width * ratio);
  }
  else if(scale_mode == enum_scale_mode::WIDTH)
  {
    const float ratio = (float)target_width / (float) pixbuf_width;
    target_height = (int)((float)pixbuf_height * ratio);
  }
  else if(scale_mode == enum_scale_mode::BOTH)
  {
    const auto ratio = std::min(
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

bool UiUtils::show_warning_no_records_found(Gtk::Window& transient_for)
{
  const auto message = _("Your find criteria did not match any records in the table.");

  Gtk::MessageDialog dialog(UiUtils::bold_message(_("No Records Found")), true, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_NONE);
  dialog.set_secondary_text(message);
  dialog.set_transient_for(transient_for);

  dialog.add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
  dialog.add_button(_("New Find"), Gtk::RESPONSE_OK);

  const auto find_again = (dialog.run() == Gtk::RESPONSE_OK);
  return find_again;
}


void UiUtils::show_report_in_browser(const std::string& filepath, Gtk::Window* parent_window)
{
  //Give the user a clue, in case the web browser opens in the background, for instance in a new tab:
  if(parent_window)
    show_ok_dialog(_("Report Finished"), _("The report will now be opened in your web browser."), *parent_window, Gtk::MESSAGE_INFO);

#ifdef G_OS_WIN32
  // gtk_show_uri doesn't seem to work on Win32, at least not for local files
  // We use Windows API instead.
  // TODO: Check it again and file a bug if necessary.
  ShellExecute(0, "open", filepath.c_str(), 0, 0, SW_SHOW);
#else

  Glib::ustring uri;
  try
  {
    uri = Glib::filename_to_uri(filepath);
  }
  catch(const Glib::ConvertError& ex)
  {
    std::cerr << G_STRFUNC << ": Could not convert filepath to URI: " << filepath << std::endl;
    return;
  }

  //Use the GNOME browser:
  GError* gerror = nullptr;
  Glib::RefPtr<Gdk::Screen> screen;
  if(parent_window)
    screen = parent_window->get_screen();

  if(!gtk_show_uri(screen ? screen->gobj() : nullptr,
    uri.c_str(), GDK_CURRENT_TIME, &gerror))
  {
    std::cerr << G_STRFUNC << ": " << gerror->message << std::endl;
    g_error_free(gerror);
  }
#endif //G_OS_WIN32
}

std::string UiUtils::get_icon_path(const Glib::ustring& filename)
{
  return  "/org/gnome/glom/data/icons/" + filename;
}

bool UiUtils::script_check_for_pygtk2_with_warning(const Glib::ustring& script, Gtk::Window* parent_window)
{
  if(!Utils::script_check_for_pygtk2(script))
  {
    UiUtils::show_ok_dialog(_("Script Uses PyGTK 2"),
      _("Glom cannot run this script because it uses pygtk 2, but Glom uses GTK+ 3, and attempting to use pygtk 2 would cause Glom to crash."), parent_window, Gtk::MESSAGE_ERROR);
    return false;
  }

  return true;
}

void UiUtils::treeview_delete_all_columns(Gtk::TreeView* treeview)
{
  if(!treeview)
    return;

  //We use this instead of just Gtk::TreeView::remove_all_columns()
  //because that deletes columns as a side-effect of unreferencing them,
  //and that behaviour might be fixed in gtkmm sometime,
  //and whether they should be deleted by that would depend on whether we used Gtk::manage().
  //Deleting them explicitly is safer and clearer. murrayc.

  //Remove all View columns:
  typedef std::vector<Gtk::TreeView::Column*> type_vec_columns;
  type_vec_columns vecViewColumns (treeview->get_columns());

  for (type_vec_columns::iterator iter (vecViewColumns.begin ()), columns_end (vecViewColumns.end ());
    iter != columns_end;
    ++iter)
  {
    Gtk::TreeView::Column* pViewColumn (*iter);
    if(!pViewColumn)
      continue;

    GtkTreeViewColumn* weak_ptr = nullptr;
    g_object_add_weak_pointer (G_OBJECT (pViewColumn->gobj()), (gpointer*)&weak_ptr);

    //Keep the object alive, instead of letting gtk_tree_view_remove_column() delete it by reducing its reference to 0,
    //so we can explicitly delete it.
    //This feels safer, considering some strange crashes I've seen when using Gtk::TreeView::remove_all_columns(),
    //though that might have been just because we didn't reset m_treeviewcolumn_button. murrayc.
    pViewColumn->reference();
    treeview->remove_column(*pViewColumn);
    delete pViewColumn; //This should cause it to be removed.

    if(weak_ptr)
    {
      std::cerr << G_STRFUNC << ": The GtkTreeViewColumn was not destroyed as expected." << std::endl;
    }
  }
}

void UiUtils::container_remove_all(Gtk::Container& container)
{
  //Remove all (usally just one) widgets from m_vbox_parent:
  //Gtk::Bin::remove() is easier but after GtkAlignment was deprecated, there is no suitable widget.
  for(const auto& child : container.get_children())
  {
    if(child)
      container.remove(*child);
  }
}

void UiUtils::load_font_into_css_provider(Gtk::Widget& widget, const Glib::ustring& pango_font_name)
{
  const Pango::FontDescription font_desc(pango_font_name);
  const auto font_family = font_desc.get_family();
  const auto font_size = font_desc.get_size();
  const auto css =
    "* {\n" +
    (font_family.empty() ? "" : "    font-family: " + font_desc.get_family() + ";\n") +
    (font_size == 0 ? "" : "    font-size: " + std::to_string(font_size / PANGO_SCALE) + "pt;\n") +
    "}";

  load_into_css_provider(widget, css);
}

void UiUtils::load_color_into_css_provider(Gtk::Widget& widget, const Glib::ustring& color)
{
  const auto css = "* { color: " + color + "; }";
  load_into_css_provider(widget, css);
}

void UiUtils::load_background_color_into_css_provider(Gtk::Widget& widget, const Glib::ustring& color)
{
  auto css = "* { background-color: " + color + "; }";
/*
      "GtkTextView {\n"
      "  background-color: " + color + "; }\n"
      "GtkTextView:backdrop {\n"
      "  background-color: " + color + "; }\n"
      "GtkTextView.view:selected {\n"
      "  background-color: " + color + "; }\n"
      "GtkTextView.view:insensitive {\n"
      "  background-color: " + color + "; }"
*/

  load_into_css_provider(widget, css);
}

} //namespace Glom
