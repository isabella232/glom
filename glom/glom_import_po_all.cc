/* Glom
 *
 * Copyright (C) 2012 Openismus GmbH
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

// For instance:
// glom_import_po_all /opt/gnome30/share/doc/glom/examples/example_music_collection.glom --input-path="/home/someone/po_Files"

#include "config.h"

#include <libglom/init.h>
#include <libglom/translations_po.h>
#include <libglom/utils.h>
#include <giomm/file.h>
#include <glibmm/optioncontext.h>
#include <glibmm/convert.h>
#include <glibmm/miscutils.h>
#include <iostream>
#include <stdexcept>

#include <glibmm/i18n.h>

class GlomCreateOptionGroup : public Glib::OptionGroup
{
public:
  GlomCreateOptionGroup();

  //These instances should live as long as the OptionGroup to which they are added,
  //and as long as the OptionContext to which those OptionGroups are added.
  std::string m_arg_filepath_po_input;
  bool m_arg_version;
};

GlomCreateOptionGroup::GlomCreateOptionGroup()
: Glib::OptionGroup("glom_import_po_all", _("Glom options"), _("Command-line options")),
  m_arg_version(false)
{
  Glib::OptionEntry entry; 
  entry.set_long_name("input-path");
  entry.set_short_name('o');
  entry.set_description(_("The path to a directory containing .po files, such as /home/someuser/po_files/ ."));
  add_entry_filename(entry, m_arg_filepath_po_input);

  entry.set_long_name("version");
  entry.set_short_name('V');
  entry.set_description(_("The version of this application."));
  add_entry(entry, m_arg_version);
}

int main(int argc, char* argv[])
{
  bindtextdomain(GETTEXT_PACKAGE, GLOM_LOCALEDIR);
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain(GETTEXT_PACKAGE);

  // Set the locale for any streams to the user's current locale,
  // We should not rely on the default locale of
  // any streams (we should always do an explicit imbue()),
  // but this is maybe a good default in case we forget.
  try
  {
    std::locale::global(std::locale(""));
  }
  catch(const std::runtime_error& ex)
  {
    //This has been known to throw an exception at least once:
    //https://bugzilla.gnome.org/show_bug.cgi?id=619445
    //This should tell us what the problem is:
    std::cerr << G_STRFUNC << ": exception from std::locale::global(std::locale(\"\")): " << ex.what() << std::endl;
    std::cerr << G_STRFUNC << ":   This can happen if the locale is not properly installed or configured." << std::endl;
  }

  
  Glom::libglom_init();
  
  Glib::OptionContext context;
  GlomCreateOptionGroup group;
  context.set_main_group(group);

  try
  {
    context.parse(argc, argv);
  }
  catch(const Glib::OptionError& ex)
  {
      std::cout << _("Error while parsing command-line options: ") << std::endl << ex.what() << std::endl;
      std::cout << _("Use --help to see a list of available command-line options.") << std::endl;
      return 0;
  }
  catch(const Glib::Error& ex)
  {
    std::cout << "Error: " << ex.what() << std::endl;
    return 0;
  }

  if(group.m_arg_version)
  {
    std::cout << PACKAGE_STRING << std::endl;
    return 0;
  }

  // Get a URI for a glom file:
  Glib::ustring input_uri;

  // The GOption documentation says that options without names will be returned to the application as "rest arguments".
  // I guess this means they will be left in the argv. Murray.
  if(input_uri.empty() && (argc > 1))
  {
     const char* pch = argv[1];
     if(pch)
       input_uri = pch;
  }

  if(input_uri.empty())
  {
    std::cerr << _("Please specify a glom file.") << std::endl;
    std::cerr << std::endl << context.get_help() << std::endl;
    return EXIT_FAILURE;
  }

  //Get a URI (file://something) from the filepath:
  Glib::RefPtr<Gio::File> file_input = Gio::File::create_for_commandline_arg(input_uri);

  //Make sure it is really a URI:
  input_uri = file_input->get_uri();

  if(!file_input->query_exists())
  {
    std::cerr << _("The Glom file does not exist.") << std::endl;
    std::cerr << G_STRFUNC << ": uri: " << input_uri << std::endl;

    std::cerr << std::endl << context.get_help() << std::endl;
    return EXIT_FAILURE;
  }

  Gio::FileType file_type = file_input->query_file_type();
  if(file_type == Gio::FILE_TYPE_DIRECTORY)
  {
    std::cerr << _("The Glom file path is a directory instead of a file.") << std::endl;
    std::cerr << std::endl << context.get_help() << std::endl;
    return EXIT_FAILURE;
  }

  //Check the po files path: 
  if(group.m_arg_filepath_po_input.empty())
  {
    std::cerr << _("Please specify the path to a directory containing po files.") << std::endl;
    std::cerr << std::endl << context.get_help() << std::endl;
    return EXIT_FAILURE;
  }

  //Get a URI (file://something) from the filepath:
  Glib::RefPtr<Gio::File> file_output = Gio::File::create_for_commandline_arg(group.m_arg_filepath_po_input);

  file_type = file_output->query_file_type();
  if(file_type != Gio::FILE_TYPE_DIRECTORY)
  {
    std::cerr << _("Glom: The po files directory path is not a directory.") << std::endl;
    std::cerr << std::endl << context.get_help() << std::endl;
    return EXIT_FAILURE;
  }


  // Load the document:
  Glom::Document document;
  document.set_file_uri(input_uri);
  int failure_code = 0;
  const auto test = document.load(failure_code);
  //std::cout << "Document load result=" << test << std::endl;

  if(!test)
  {
    std::cerr << G_STRFUNC << ": Document::load() failed with failure_code=" << failure_code << std::endl;
    return EXIT_FAILURE;
  }

  //Import all .po files from the directory:
  Glib::RefPtr<Gio::FileEnumerator> enumerator = file_output->enumerate_children();
  Glib::RefPtr<Gio::FileInfo> info;
  while( (info = enumerator->next_file()) )
  {
    Glib::RefPtr<Gio::File> child = file_output->get_child(info->get_name());
    if(child->query_file_type() == Gio::FILE_TYPE_DIRECTORY)
      continue;

    //Check that it has the .po file extension:
    const auto basename = child->get_basename();
    const std::string locale_id = 
      Glom::Utils::string_remove_suffix(basename, ".po");
    if(locale_id == basename)
      continue;
    
    document.set_allow_autosave(false); //Prevent saving while we modify the document.
    const bool succeeded = 
      Glom::import_translations_from_po_file(&document, child->get_uri(), locale_id);
    if(!succeeded)
    {
      std::cerr << Glib::ustring::compose(_("Po file import failed for locale: %1"), locale_id) << std::endl;
      return EXIT_FAILURE;
    }

    if(!document.save()) {
      std::cerr << Glib::ustring::compose(_("Po file import failed during document saving for locale: %1"), locale_id) << std::endl;
      return EXIT_FAILURE;
    }

    std::cout << Glib::ustring::compose(_("Po file imported for locale: %1 for file %2"), locale_id, input_uri) << std::endl;
  }

  Glom::libglom_deinit();

  return EXIT_SUCCESS;
}
