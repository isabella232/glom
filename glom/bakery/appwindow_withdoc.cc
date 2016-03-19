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
#include <glom/bakery/appwindow_withdoc.h>
#include <glom/bakery/dialog_offersave.h>
#include <libglom/algorithms_utils.h>
#include <libglom/utils.h>
#include <giomm/file.h>
#include <glibmm/i18n.h>

namespace GlomBakery
{

AppWindow_WithDoc::type_list_strings AppWindow_WithDoc::m_mime_types;

AppWindow_WithDoc::AppWindow_WithDoc(const Glib::ustring& appname)
: AppWindow(appname),
  m_document(nullptr),
  m_bCloseAfterSave(false)
{
}

AppWindow_WithDoc::~AppWindow_WithDoc()
{
  //TODO: Should the views have weak_ptr to m_document?
}

//static
void AppWindow_WithDoc::add_mime_type(const Glib::ustring& mime_type)
{
  Glom::Utils::add_unique(m_mime_types, mime_type);
}

void AppWindow_WithDoc::on_menu_file_close()
{
  if(m_document->get_modified())
  {
    //Offer to save changes:
    m_bCloseAfterSave = true; //Checked in FileChooser signal handler.
    offer_to_save_changes(); //If a File|Exit is in progress, this could cancel it.
  }

  on_document_close();

  if(!get_operation_cancelled())
  {
    //Note that this can result in this appwindow being deleted,
    //so we do it last.
    ui_hide();
  }
}

bool AppWindow_WithDoc::open_document_from_data(const guchar* data, std::size_t length)
{
  int failure_code = 0;
  const auto bTest = m_document->load_from_data(data, length, failure_code);

  bool bOpenFailed = false;
  if(!bTest) //if open failed.
    bOpenFailed = true;
  else
  {
    //if open succeeded then let the App respond:
    const auto test = on_document_load();
    if(!test)
      bOpenFailed = true; //The application didn't like something about the just-loaded document.
    else
    {
      update_window_title();
      set_document_modified(false); //disables menu and toolbar Save items.
    }
  }

  if(bOpenFailed)
  {
    ui_warning_load_failed(failure_code);

    return false; //failed.
  }
  else
    return true;
}

bool AppWindow_WithDoc::open_document(const Glib::ustring& file_uri)
{
  //Open it:

  //Load it into a new instance unless the current document is just a default new.
  if(!(get_document()->get_is_new())) //if it's not new.
  {
    //New instance:
    new_instance(file_uri);
    return true;
  }

  auto pApp = this; //Replace the default new document in this instance.

  //Open it.
  pApp->m_document->set_file_uri(file_uri);
  int failure_code = 0;
  const auto bTest = pApp->m_document->load(failure_code);

  bool bOpenFailed = false;
  bool bShowError = false;
  if(!bTest) //if open failed.
  {
    bOpenFailed = true;
    bShowError = true;
  }
  else
  {
    //if open succeeded then let the App respond:
    const auto test = pApp->on_document_load();
    if(!test)
      bOpenFailed = true; //The application didn't like something about the just-loaded document.
    else
    {
      pApp->update_window_title();
      set_document_modified(false); //disables menu and toolbar Save items.

      //Update document history list:
      //(Getting the file URI again, in case it has changed while being opened,
      //for instance if it was a template file that was saved as a real file.)
      if(pApp->m_document)
        document_history_add(file_uri);
    }
  }

  if(bOpenFailed)
  {
    if(bShowError)
      ui_warning_load_failed(failure_code);

    //Make sure that non-existant files are removed from the history list:
    if(failure_code == Glom::Utils::to_utype(Document::LoadFailureCodes::NOT_FOUND))
      document_history_remove(file_uri);

    //re-initialize document.
    pApp->m_document.reset();
    pApp->init_create_document();

    return false; //failed.
  }

  return true; //success.
}

void AppWindow_WithDoc::on_menu_file_open()
{
  //Display File Open dialog and respond to choice:

  //Bring document window to front, to make it clear which document is being changed:
  ui_bring_to_front();

  //Ask user to choose file to open:
  auto file_uri = ui_file_select_open();
  if(!file_uri.empty())
    open_document(file_uri);
}

//This is for calling directly, use the other override for the signal handler:
void AppWindow_WithDoc::offer_saveas()
{
  on_menu_file_saveas();
}

bool AppWindow_WithDoc::file_exists(const Glib::ustring& uri)
{
  if(uri.empty())
    return false;

  //Check whether file exists already:
  {
    // Try to examine the input file.
    auto file = Gio::File::create_for_uri(uri);

    try
    {
      return file->query_exists();
    }
    catch(const Gio::Error& /* ex */)
    {
      return false; //Something went wrong. It does not exist.
    }
  }
}

void AppWindow_WithDoc::on_menu_file_saveas()
{
  //Display File Save dialog and respond to choice:

  //Bring document window to front, to make it clear which document is being saved:
  //This doesn't work: TODO.
  ui_bring_to_front();

  //Show the save dialog:
  const auto file_uriOld = m_document->get_file_uri();

  auto file_uri = ui_file_select_save(file_uriOld); //Also asks for overwrite confirmation.
  if(!file_uri.empty())
  {
    //Enforce the file extension:
    file_uri = m_document->get_file_uri_with_extension(file_uri);

    m_document->set_file_uri(file_uri, true); //true = enforce file extension
    const auto bTest = m_document->save();

    if(!bTest)
    {
      ui_warning(_("Save failed."), _("There was an error while saving the file. Your changes have not been saved."));
    }
    else
    {
      //Disable Save and SaveAs menu items:
      after_successful_save();
    }

    update_window_title();


    //Close if this save was a result of a File|Close or File|Exit:.
    //if(bTest && m_bCloseAfterSave) //Don't close if the save failed.
    //{
    //  on_menu_file_close(); //This could be the second time, but now there are no unsaved changes.
    //}
  }
  else
  {
    cancel_close_or_exit();
  }
}

void AppWindow_WithDoc::on_menu_file_save()
{
  if(m_document)
  {
    //If there is already a filepath, then save to that location:
    if(!(m_document->get_file_uri().empty()))
    {
      auto bTest = m_document->save();

      if(bTest)
      {
        //Disable Save and SaveAs menu items:
        after_successful_save();

        //Close the document if this save was in response to a 'Do you want to save before closing?':
        //if(m_bCloseAfterSave) // || m_bExiting
        //  close_mark_or_destroy();
      }
      else
      {
        //The save failed. Tell the user and don't do anything else:
        ui_warning(_("Save failed."), _("There was an error while saving the file. Your changes have not been saved."));

        cancel_close_or_exit();
      }
    }
    else
    {
      //If there is no filepath, then ask for one and save to that location:
      offer_saveas();
    }
  }

  if(!m_bCloseAfterSave) //Don't try to do anything after closing - this instance would not exist anymore.
  {
    update_window_title();
  }

}

void AppWindow_WithDoc::init()
{
  init_create_document();

  //Call base method:
  AppWindow::init();

  on_document_load(); //Show default empty document in the View.

  set_document_modified(false); //Disable Save menu item.
}

void AppWindow_WithDoc::init_create_document()
{
  //Overrides should call this base method at the end.

  if(!m_document)
  {
    m_document = std::make_shared<Document>();
  }

  m_document->set_is_new(true); //Can't be modified if it's just been created.

  m_document->signal_modified().connect(sigc::mem_fun(*this, &AppWindow_WithDoc::on_document_modified));

  update_window_title();
}

std::shared_ptr<Document> AppWindow_WithDoc::get_document()
{
  return m_document;
}

std::shared_ptr<const Document> AppWindow_WithDoc::get_document() const
{
  return m_document;
}

void AppWindow_WithDoc::offer_to_save_changes()
{
  if(m_document)
  {
    if(m_document->get_modified())
    {
      set_operation_cancelled(false); //Initialize it again. It might be set later in this method by cancel_close_or_exit().

      //The document has unsaved changes,
      //so ask the user whether the document should be saved:
      auto buttonClicked = ui_offer_to_save_changes();

      //Respond to button that was clicked:
      switch(buttonClicked)
      {
        case(enumSaveChanges::Save):
        {
          on_menu_file_save(); //If File|Exit is in progress, this could cancel it.
          break;
        }

        case(enumSaveChanges::Discard):
        {
          //Close if this save offer was a result of a FileClose (It probably always is):
          //close_mark_or_destroy();
          //Do nothing - the caller will probably hide() the window to have it deleted.
          break;
        }

        case(enumSaveChanges::Cancel): //Cancel.
        {
          cancel_close_or_exit();
          break;
        }

        default:
        {
          break;
        }
      }
    }
  }
}

void AppWindow_WithDoc::cancel_close_or_exit()
{
  set_operation_cancelled();
  m_bCloseAfterSave = false;

  //exit_destroy_marked_instances(); //Clean up after an exit.
}

bool AppWindow_WithDoc::on_document_load()
{
  //Show document contents:
  if(m_document)
  {
    auto pView = m_document->get_view();
    if(pView)
      pView->load_from_document();

    //Set document as unmodified (this could have been wrongly set during the load):
    set_document_modified(false);

    return true;
  }
  else
    return false; //I can't think of any reason why this would happen.

  //If you are not using Views, then override this to fill your various windows with stuff according to the contents of the document.
}

void AppWindow_WithDoc::on_document_close()
{
}

void AppWindow_WithDoc::update_window_title()
{

}

void AppWindow_WithDoc::on_document_modified(bool /* modified */)
{
  //Change the displayed 'modified' status.
  //This method could be overridden to e.g. enable a Save icon or enable the Save menu item.
  //TODO: enable/disable the Save menu item.

  ui_show_modification_status();

  //Change title bar:
  update_window_title();
}

void AppWindow_WithDoc::set_document_modified(bool bModified /* = true */)
{
  m_document->set_modified(bModified);

  //Enable/Disable Save menu item and toolbar item:
  ui_show_modification_status();
}

void AppWindow_WithDoc::on_menu_edit_copy()
{
  auto pView = m_document->get_view();
  if(pView)
    pView->clipboard_copy();
}

void AppWindow_WithDoc::on_menu_edit_paste()
{
  auto pView = m_document->get_view();
  if(pView)
    pView->clipboard_paste();
}

void AppWindow_WithDoc::on_menu_edit_clear()
{
  auto pView = m_document->get_view();
  if(pView)
    pView->clipboard_clear();
}

void AppWindow_WithDoc::after_successful_save()
{
  set_document_modified(false); //enables/disables menu and toolbar widgets.

  //Update document history list:
  document_history_add(m_document->get_file_uri());
}

void AppWindow_WithDoc::document_history_add(const Glib::ustring& /* file_uri */)
{
  //Override this.
}

void AppWindow_WithDoc::document_history_remove(const Glib::ustring& /* file_uri */)
{
  //Override this.
}

void AppWindow_WithDoc::ui_warning_load_failed(int)
{
  ui_warning(_("Open Failed."), _("The document could not be opened."));
}

} //namespace
