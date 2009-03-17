/*
 * Copyright 2000 Murray Cumming
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

#ifdef BAKERY_MAEMO_ENABLED
#include <hildon-fmmm/file-chooser-dialog.h>
#include <hildonmm/note.h>
#endif // BAKERY_MAEMO_ENABLED

#include <glom/bakery/GtkDialogs.h>
#include <glom/bakery/Dialog_OfferSave.h>
#include <glom/bakery/App_Gtk.h>
#include <gtkmm/stock.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/filechooserdialog.h>
#include <giomm.h>
#include <glibmm/i18n-lib.h>


namespace GlomBakery
{

void GtkDialogs::ui_warning(App& app, const Glib::ustring& text, const Glib::ustring& secondary_text)
{
  Gtk::Window* pWindow = dynamic_cast<Gtk::Window*>(&app);

#ifdef BAKERY_MAEMO_ENABLED
  Hildon::Note dialog(Hildon::NOTE_TYPE_INFORMATION, text, Gtk::Stock::DIALOG_WARNING);
#else
  Gtk::MessageDialog dialog(App_Gtk::util_bold_message(text), true /* use markup */, Gtk::MESSAGE_WARNING);
  dialog.set_secondary_text(secondary_text);

  dialog.set_title(""); //The HIG says that alert dialogs should not have titles. The default comes from the message type.
#endif

  if(pWindow)
    dialog.set_transient_for(*pWindow);

  dialog.run();
}

Glib::ustring GtkDialogs::ui_file_select_open(App& app, const Glib::ustring& starting_folder_uri)
{
  Gtk::Window* pWindow = dynamic_cast<Gtk::Window*>(&app);

#ifdef BAKERY_MAEMO_ENABLED
  Hildon::FileChooserDialog fileChooser_Open(Gtk::FILE_CHOOSER_ACTION_OPEN);
#else
  Gtk::FileChooserDialog fileChooser_Open(_("Open Document"), Gtk::FILE_CHOOSER_ACTION_OPEN);
  fileChooser_Open.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  fileChooser_Open.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);
  fileChooser_Open.set_default_response(Gtk::RESPONSE_OK);
#endif // BAKERY_MAEMO_ENABLED

  if(pWindow)
    fileChooser_Open.set_transient_for(*pWindow);

  if(!starting_folder_uri.empty())
    fileChooser_Open.set_current_folder_uri(starting_folder_uri);

  const int response_id = fileChooser_Open.run();
  fileChooser_Open.hide();
  if(response_id != Gtk::RESPONSE_CANCEL)
  {
    return fileChooser_Open.get_uri();
  }
  else
    return Glib::ustring();
}

static bool uri_is_writable(const Glib::RefPtr<const Gio::File>& uri)
{
  if(!uri)
    return false;

  Glib::RefPtr<const Gio::FileInfo> file_info;

#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
    file_info = uri->query_info(G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE);
  }
  catch(const Glib::Error& /* ex */)
  {
    return false;
  }
#else
  std::auto_ptr<Gio::Error> error;
  file_info = uri->query_info(G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE, Gio::FILE_QUERY_INFO_NONE, error);
  if(error.get())
    return false;
#endif

  if(file_info)
  {
    return file_info->get_attribute_boolean(G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE);
  }
  else
    return true; //Not every URI protocol supports access rights, so assume that it's writable and complain later.
}

Glib::ustring GtkDialogs::ui_file_select_save(App& app, const Glib::ustring& old_file_uri)
{
  Gtk::Window* pWindow = dynamic_cast<Gtk::Window*>(&app);

#ifdef BAKERY_MAEMO_ENABLED
  Hildon::FileChooserDialog fileChooser_Save(Gtk::FILE_CHOOSER_ACTION_SAVE);
#else
  Gtk::FileChooserDialog fileChooser_Save(_("Save Document"), Gtk::FILE_CHOOSER_ACTION_SAVE);
  fileChooser_Save.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  fileChooser_Save.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);
  fileChooser_Save.set_default_response(Gtk::RESPONSE_OK);
#endif // BAKERY_MAEMO_ENABLED

 if(pWindow)
    fileChooser_Save.set_transient_for(*pWindow);

  fileChooser_Save.set_do_overwrite_confirmation(); //Ask the user if the file already exists.

  //Make the save dialog show the existing filename, if any:
  if(!old_file_uri.empty())
  {
    //Just start with the parent folder,
    //instead of the whole name, to avoid overwriting:
    Glib::RefPtr<Gio::File> gio_file = Gio::File::create_for_uri(old_file_uri);
    if(gio_file)
    {
      Glib::RefPtr<Gio::File> parent = gio_file->get_parent();
      if(parent)
      {
        const Glib::ustring uri_parent = parent->get_uri();
        fileChooser_Save.set_uri(uri_parent);
      }
    }
  }


  //bool tried_once_already = false;

  bool try_again = true;
  while(try_again)
  {
    try_again = false;

    //Work around bug #330680 "GtkFileChooserDialog is too small when shown a second time.":
    //(Commented-out because the workaround doesn't work)
    /*
    if(tried_once_already)
    {
      fileChooser_Save.set_default_size(-1, 600); 
    }
    else
      tried_once_already = true;
    */

    const int response_id = fileChooser_Save.run();
    fileChooser_Save.hide();
    if(response_id != Gtk::RESPONSE_CANCEL)
    {
      const Glib::ustring uri = fileChooser_Save.get_uri();

      Glib::RefPtr<Gio::File> gio_file = Gio::File::create_for_uri(uri);

      //If the file exists (the FileChooser offers a "replace?" dialog, so this is possible.):
      if(App_WithDoc::file_exists(uri))
      {
        //Check whether we have rights to the file to change it:
        //Really, GtkFileChooser should do this for us.
        if(!uri_is_writable(gio_file))
        {
           //Warn the user:
           ui_warning(app, _("Read-only File."), _("You may not overwrite the existing file, because you do not have sufficient access rights."));
           try_again = true; //Try again.
           continue;
        }
      }

      //Check whether we have rights to the directory, to create a new file in it:
      //Really, GtkFileChooser should do this for us.
      Glib::RefPtr<const Gio::File> gio_file_parent = gio_file->get_parent();
      if(gio_file_parent)
      {
        if(!uri_is_writable(gio_file_parent))
        {
          //Warn the user:
           ui_warning(app, _("Read-only Directory."), _("You may not create a file in this directory, because you do not have sufficient access rights."));
           try_again = true; //Try again.
           continue;
        }
      }

      if(!try_again)
        return uri;
    }
    else
      return Glib::ustring(); //The user cancelled.
  }
}

App_WithDoc::enumSaveChanges GtkDialogs::ui_offer_to_save_changes(App& app, const Glib::ustring& file_uri)
{
  App_WithDoc::enumSaveChanges result = App_WithDoc::SAVECHANGES_Cancel;

  GlomBakery::Dialog_OfferSave* pDialogQuestion = new GlomBakery::Dialog_OfferSave(file_uri);

  Gtk::Window* pWindow = dynamic_cast<Gtk::Window*>(&app);
  if(pWindow)
    pDialogQuestion->set_transient_for(*pWindow);

  GlomBakery::Dialog_OfferSave::enumButtons buttonClicked = (GlomBakery::Dialog_OfferSave::enumButtons)pDialogQuestion->run();
  delete pDialogQuestion;
  pDialogQuestion = 0;

  if(buttonClicked == GlomBakery::Dialog_OfferSave::BUTTON_Save)
     result = App_WithDoc::SAVECHANGES_Save;
  else if(buttonClicked == GlomBakery::Dialog_OfferSave::BUTTON_Discard)
     result = App_WithDoc::SAVECHANGES_Discard;
  else
     result = App_WithDoc::SAVECHANGES_Cancel;

  return result;
}

} //namespace
