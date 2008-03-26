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
#include "../application.h"
#include <glibmm/i18n.h>
#include "dialog_layoutitem_properties.h"
//#include <sstream> //For stringstream

namespace Glom
{

LabelGlom::LabelGlom()
{
#ifndef GLOM_ENABLE_CLIENT_ONLY
  setup_menu();
#endif // !GLOM_ENABLE_CLIENT_ONLY

  init();
}

LabelGlom::LabelGlom(const Glib::ustring& label, float xalign, float yalign, bool mnemonic, bool title)
: m_label(label, xalign, yalign, mnemonic),
  m_title(title)
{
#ifndef GLOM_ENABLE_CLIENT_ONLY
  setup_menu();
#endif // !GLOM_ENABLE_CLIENT_ONLY

  init();
}

LabelGlom::~LabelGlom()
{
}

void LabelGlom::init()
{
  add(m_label);
  m_label.show();
  set_events (Gdk::BUTTON_PRESS_MASK);
  m_refUtilDetails->set_visible(false);
}

App_Glom* LabelGlom::get_application()
{
  Gtk::Container* pWindow = get_toplevel();
  //TODO: This only works when the child widget is already in its parent.

  return dynamic_cast<App_Glom*>(pWindow);
}

void LabelGlom::on_menu_properties_activate()
{
  try
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom_developer.glade", "dialog_layoutitem_properties");
    
    Dialog_LayoutItem_Properties* dialog = new Dialog_LayoutItem_Properties (0, refXml);
    refXml->get_widget_derived("dialog_layoutitem_properties", dialog);
    
    if (dialog)
    {
      dialog->set_label (m_label.get_label());
      if (dialog->run() == Gtk::RESPONSE_APPLY)
      {
        Glib::ustring label(dialog->get_label());
        sharedptr<LayoutItem_Text> layoutitem_text = sharedptr<LayoutItem_Text>::cast_dynamic(m_pLayoutItem);
        if (layoutitem_text)
        {
          // We could just set the title of the layout item but than
          // we would need to redraw the whole layout.
          m_label.set_label (label);
          if (m_title)
          {
            std::cout << "label: " << label << std::endl;
            layoutitem_text->set_title (label);
          }
          else
          {
            layoutitem_text->set_text (label);
          }
        }
      }
      delete dialog;
    }
  }
  catch(const Gnome::Glade::XmlError& ex)
  {
    std::cerr << ex.what() << std::endl;
  }
}

bool LabelGlom::on_button_press_event(GdkEventButton *event)
{
  App_Glom* pApp = get_application();
  if(pApp->get_userlevel() == AppState::USERLEVEL_DEVELOPER)
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

} //namespace Glom
