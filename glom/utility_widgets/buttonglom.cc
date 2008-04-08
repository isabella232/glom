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

#include "buttonglom.h"
#include <gtkmm/messagedialog.h>
#include "../application.h"
#include "../libglom/glade_utils.h"
#include "dialog_layoutitem_properties.h"
#include "../layout_item_dialogs/dialog_buttonscript.h"
#include <glibmm/i18n.h>
//#include <sstream> //For stringstream

namespace Glom
{

ButtonGlom::ButtonGlom(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& /* refGlade */)
: Gtk::Button(cobject)
{
  init();
}

ButtonGlom::ButtonGlom()
{
  init();
}

ButtonGlom::~ButtonGlom()
{
}

void ButtonGlom::init()
{

}

App_Glom* ButtonGlom::get_application()
{
  Gtk::Container* pWindow = get_toplevel();
  //TODO: This only works when the child widget is already in its parent.

  return dynamic_cast<App_Glom*>(pWindow);
}

void ButtonGlom::on_menu_properties_activate()
{
  try
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom_developer.glade", "dialog_layoutitem_properties");
    
    Dialog_LayoutItem_Properties* dialog = new Dialog_LayoutItem_Properties (0, refXml);
    refXml->get_widget_derived("dialog_layoutitem_properties", dialog);
    
    if (dialog)
    {
      dialog->set_label (get_label());
      if (dialog->run() == Gtk::RESPONSE_APPLY)
      {
        // We could just set the title of the layout item but than
        // we would need to redraw the whole layout.
        set_label (dialog->get_label()); 
        m_pLayoutItem->set_title (dialog->get_label());
      }
      delete dialog;
    }
  }
  catch(const Gnome::Glade::XmlError& ex)
  {
    std::cerr << ex.what() << std::endl;
  }
}

void ButtonGlom::on_menu_details_activate()
{
  try
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(Utils::get_glade_file_path("glom_developer.glade"), "window_button_script");

    Dialog_ButtonScript* dialog = 0;
    refXml->get_widget_derived("window_button_script", dialog);

    if(dialog)
    {
      sharedptr<LayoutItem_Button> layout_item = 
        sharedptr<LayoutItem_Button>::cast_dynamic(get_layout_item());
      dialog->set_script(layout_item, m_table_name);
      int response = Glom::Utils::dialog_run_with_help(dialog, "window_button_script");
      dialog->hide();
      if(response == Gtk::RESPONSE_OK)
      {
        dialog->get_script(layout_item);
      }

      delete dialog;
    }
  }
  catch(const Gnome::Glade::XmlError& ex)
  {
    std::cerr << ex.what() << std::endl;
  }
}

bool ButtonGlom::on_button_press_event(GdkEventButton *event)
{
  App_Glom* pApp = get_application();
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
  return Gtk::Button::on_button_press_event(event);
}

} //namespace Glom
