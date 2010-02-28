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

#include "labelglom.h"
#include <gtkmm/messagedialog.h>
#include <glom/application.h>
#include <glibmm/i18n.h>
#include <glom/mode_design/layout/layout_item_dialogs/dialog_textobject.h>
#include <glom/glade_utils.h>
#include "../mode_data/flowtablewithfields.h"
//#include <sstream> //For stringstream

namespace Glom
{

LabelGlom::LabelGlom()
{
  init();
}

LabelGlom::LabelGlom(const Glib::ustring& label, bool mnemonic)
: m_label(label, mnemonic)
{
  init();
}

LabelGlom::LabelGlom(const Glib::ustring& label, float xalign, float yalign, bool mnemonic)
: m_label(label, xalign, yalign, mnemonic)
{
  init();
}

LabelGlom::~LabelGlom()
{
}

void LabelGlom::init()
{
  add(m_label);
  m_label.show();
  set_events(Gdk::ALL_EVENTS_MASK);
  //This would be more efficient if we were only using the (base) EventBox to get events, 
  //but we also want to allow changing of the background color, so we don't use it: set_visible_window(false);
}

Application* LabelGlom::get_application()
{
  Gtk::Container* pWindow = get_toplevel();
  //TODO: This only works when the child widget is already in its parent.

  return dynamic_cast<Application*>(pWindow);
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void LabelGlom::on_menu_properties_activate()
{
  sharedptr<LayoutItem_Text> textobject = sharedptr<LayoutItem_Text>::cast_dynamic(m_pLayoutItem);
  if(!textobject)
    return;

  try
  {
    Glib::RefPtr<Gtk::Builder> refXml = Gtk::Builder::create_from_file(Utils::get_glade_file_path("glom_developer.glade"), "window_textobject");

    Dialog_TextObject* dialog = 0;
    refXml->get_widget_derived("window_textobject", dialog);

    if(dialog)
    {
      dialog->set_textobject(textobject, m_table_name);
      const int response = dialog->run();
      dialog->hide();
      if(response == Gtk::RESPONSE_OK)
      {
        //Get the chosen relationship:
        dialog->get_textobject(textobject);
      }
      signal_layout_changed().emit();

      delete dialog;
    }
  }
  catch(const Gtk::BuilderError& ex)
  {
    std::cerr << ex.what() << std::endl;
  }
}

bool LabelGlom::on_button_press_event(GdkEventButton *event)
{
  Application* pApp = get_application();
  if(pApp && pApp->get_userlevel() == AppState::USERLEVEL_DEVELOPER)
  {
    GdkModifierType mods;
    gdk_window_get_pointer( Gtk::Widget::gobj()->window, 0, 0, &mods );
    if(mods & GDK_BUTTON3_MASK)
    {
      //Give user choices of actions on this item:
      m_pPopupMenuUtils->popup(event->button, event->time);
      return true; //We handled this event.
    }
  }

  return Gtk::EventBox::on_button_press_event(event);
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

Gtk::Label* LabelGlom::get_label()
{
  return &m_label;
}

} //namespace Glom
