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
#include <gtkmm/box.h>
#include <glom/bakery/dialog_offersave.h>
#include <glom/appwindow.h>
#include <glibmm/convert.h> 
#include <glibmm/i18n-lib.h>

namespace
{
  Glib::ustring get_confirmation_message(const Glib::ustring& file_uri)
  {
    Glib::ustring message = _("This document has unsaved changes. Would you like to save the document?");
    if(!file_uri.empty())
    {
      message += "\n\n";

      //TODO: Can we use filename_display_basename() with a URI?
      // No, almost certainly not. murrayc.
      message += Glib::ustring::compose(_("Document:\n%1"), Glib::filename_display_basename(file_uri));
    }

    return message;
  }
}

namespace GlomBakery
{


Dialog_OfferSave::Dialog_OfferSave(const Glib::ustring& file_uri)
: Gtk::MessageDialog( Glom::AppWindow::util_bold_message(_("Close without Saving")), true /* use markup */, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE)
{
  set_title(""); //The HIG says that alert dialogs should not have titles. The default comes from the message type.
  
  set_secondary_text(get_confirmation_message(file_uri));

  add_button(_("Discard"), BUTTON_Discard);
  auto cancel_button = add_button(_("_Cancel"), BUTTON_Cancel);
  add_button(_("_Save"), BUTTON_Save);

  // Otherwise Discard has focus initially which seems inconvenient:
  cancel_button->grab_focus();
}

Dialog_OfferSave::~Dialog_OfferSave()
{

}


} //namespace
