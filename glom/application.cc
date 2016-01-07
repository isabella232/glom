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

#include <glom/application.h>
#include <glom/appwindow.h>
#include <glom/main_local_options.h>
#include <glom/glade_utils.h>
#include <glibmm/optioncontext.h>
#include <iostream>

#include <glibmm/i18n.h>

namespace Glom
{

// We use Gio::APPLICATION_NON_UNIQUE because we have some singletons and other static data,
// to simplify our code.
// We also want to prevent all instances from crashing when one instance crashes.
Application::Application()
: Gtk::Application("org.glom.Glom", Gio::APPLICATION_HANDLES_OPEN | Gio::APPLICATION_HANDLES_COMMAND_LINE | Gio::APPLICATION_NON_UNIQUE)
{
}

Glib::RefPtr<Application> Application::create()
{
  return Glib::RefPtr<Application>( new Application() );
}

void Application::create_window(const Glib::RefPtr<Gio::File>& file)
{
  //std::cout << G_STRFUNC << ": debug" << std::endl;

  AppWindow* window = nullptr;
  Glom::Utils::get_glade_widget_derived_with_warning(window);
  g_assert(window);

  window->set_show_sql_debug(m_remote_option_group.m_arg_debug_sql);
  window->set_stop_auto_server_shutdown(m_remote_option_group.m_arg_stop_auto_server_shutdown);

  //Make sure that the application runs for as long this window is still open:
  add_window(*window);

  //Delete the window when it is hidden:
  window->signal_hide().connect(sigc::bind<Gtk::Window*>(sigc::mem_fun(*this,
    &Application::on_window_hide), window));
      
  Glib::ustring input_uri;
  if(file) //If it's empty then this is a new empty file, as a result of an activation rather than an open.
  {
    input_uri = file->get_uri();
  }

  const auto test = window->init_with_document(input_uri, m_remote_option_group.m_arg_restore); //Sets it up and shows it.
  if(!test) //The user could cancel the offer of a new or existing database.
  {
    window->hide(); //This will cause it to be deleted by on_window_hide.
  }
}

void Application::on_window_hide(Gtk::Window* window)
{
  delete window;
}

void Application::on_activate()
{
  //std::cout << G_STRFUNC << ": debug" << std::endl;

  // The application has been started, so let's show a window:
  create_window();
}

void Application::on_startup()
{
  //Call the base class:
  Gtk::Application::on_startup();

  //TODO: Remove this if there is ever an easier way to make 'accel's from the .glade file just work.
  //See https://bugzilla.gnome.org/show_bug.cgi?id=708905

  //TODO: Use set_accels_for_action() instead? The documentation is not helpful.
  //See https://bugzilla.gnome.org/show_bug.cgi?id=721367#c5

  //From window_main.glade:
  set_accel_for_action("file.new", "<Primary>n");
  set_accel_for_action("file.open", "<Primary>o");
  set_accel_for_action("win.close", "<Primary>w");
  set_accel_for_action("win.print", "<Primary>p"); //Not in the main window, but in the Relationships Overview window, and maybe others.
  set_accel_for_action("win.print-preview", "<Shift><Primary>p"); //Not in the main window, but in the Print Layout window, and maybe others.
  set_accel_for_action("edit.copy", "<Primary>c");
  set_accel_for_action("edit.paste", "<Primary>v");
  set_accel_for_action("edit.find", "<Primary>f");
}

void Application::on_open(const Gio::Application::type_vec_files& files,
  const Glib::ustring& hint)
{
  //std::cout << G_STRFUNC << ": debug" << std::endl;
  
  // The application has been asked to open some files,
  // so let's open a new window for each one.
  //std::cout << "debug: files.size()=" << files.size() << std::endl;
  for(guint i = 0; i < files.size(); i++)
  {
    auto file = files[i];
    if(!file)
    {
      std::cerr << G_STRFUNC << ": file is null." << std::endl;
    }
    else
      create_window(file);
  }

  Gtk::Application::on_open(files, hint);
}


int Application::on_command_line(const Glib::RefPtr<Gio::ApplicationCommandLine>& command_line)
{
  //std::cout << G_STRFUNC << ": debug" << std::endl;
  
  //Parse command-line arguments that were passed either to the main (first) instance
  //or to subsequent instances.
  //Note that this parsing is happening in the main (not remote) instance.
  int argc = 0;
  char** argv =	command_line->get_arguments(argc);

  Glib::OptionContext context;
  context.set_main_group(m_remote_option_group);
  
  //Note that these options should really be parsed in main(),
  //but we do it here because of glib bug: https://bugzilla.gnome.org/show_bug.cgi?id=634990#c6
  //Handling the two groups together here is possible due to our use of Gio::APPLICATION_NON_UNIQUE .
  LocalOptionGroup local_group;
  context.add_group(local_group);

  try
  {
    context.parse(argc, argv);
  }
  catch(const Glib::OptionError& ex)
  {
    std::cout << _("Error while parsing command-line options: ") << std::endl << ex.what() << std::endl;
    std::cout << _("Use --help to see a list of available command-line options.") << std::endl;
    return EXIT_FAILURE;
  }
  catch(const Glib::Error& ex)
  {
    std::cout << "Error: " << ex.what() << std::endl;
    return EXIT_FAILURE;
  }

  //Get command-line parameters, if any:
  if(!local_group.handle_options())
    return EXIT_FAILURE;
    
  bool stop = false;
  const auto date_check_ok = local_group.get_debug_date_check_result(stop);
  if(stop)
  {
    //This command-line option is documented as stopping afterwards.
    return date_check_ok ? EXIT_SUCCESS : EXIT_FAILURE;
  }


  Glib::ustring input_uri = m_remote_option_group.m_arg_filename;

  // The GOption documentation says that options without names will be returned to the application as "rest arguments".
  // I guess this means they will be left in the argv. Murray.
  if(input_uri.empty() && (argc > 1))
  {
     const char* pch = argv[1];
     if(pch)
       input_uri = pch;
  }

  Glib::RefPtr<Gio::File> file;
  if(!input_uri.empty())
  {
    //Get a URI (file://something) from the filepath:
    file = Gio::File::create_for_commandline_arg(input_uri);

    if(!file->query_exists())
    {
      std::cerr << _("Glom: The file does not exist.") << std::endl;
      std::cerr << G_STRFUNC << ": uri: " << input_uri << std::endl;

      std::cerr << std::endl << context.get_help() << std::endl;
      return EXIT_FAILURE;
    }

    const auto file_type = file->query_file_type();
    if(file_type == Gio::FILE_TYPE_DIRECTORY)
    {
      std::cerr << _("Glom: The file path is a directory instead of a file.") << std::endl;

      std::cerr << std::endl << context.get_help() << std::endl;
      return EXIT_FAILURE;
    }

    //std::cout << "URI = " << input_uri << std::endl;
  }

  //debugging:
  //input_uri = "file:///home/murrayc/cvs/gnome212/glom/examples/example_smallbusiness.glom";

  if(file)
  {
    open(file); //TODO: Find out why calling open() with a null File causes an infinite loop.
  }
  else
  {
    //Open a new "document" instead:
    activate();
  }

  //The local instance will eventually exit with this status code:
  return EXIT_SUCCESS;
}

} //namespace Glom
