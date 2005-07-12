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

#include "imageglom.h"
#include <glibmm/i18n.h>
#include "../application.h"
//#include <sstream> //For stringstream

#include <iostream>   // for cout, endl

ImageGlom::ImageGlom()
: m_image(Gtk::Stock::MISSING_IMAGE, Gtk::ICON_SIZE_DIALOG) //The widget is invisible if we don't specify an image.
{
  setup_menu();
  
  add(m_image);
}

ImageGlom::~ImageGlom()
{
}


bool ImageGlom::on_button_press_event(GdkEventButton *event)
{
  GdkModifierType mods;
  gdk_window_get_pointer( Gtk::Widget::gobj()->window, 0, 0, &mods );
      
  //Enable/Disable items.
  //We did this earlier, but get_application is more likely to work now:
  App_Glom* pApp = get_application();
  if(pApp)
  {
    pApp->add_developer_action(m_refContextLayout); //So that it can be disabled when not in developer mode.
    pApp->add_developer_action(m_refContextAddField);
    pApp->add_developer_action(m_refContextAddRelatedRecords);
    pApp->add_developer_action(m_refContextAddGroup);

    pApp->update_userlevel_ui(); //Update our action's sensitivity.

    //Only show this popup in developer mode, so operators still see the default GtkEntry context menu.
    //TODO: It would be better to add it somehow to the standard context menu.
    if(pApp->get_userlevel() == AppState::USERLEVEL_DEVELOPER)
    {
      if(mods & GDK_BUTTON3_MASK)
      {
        //Give user choices of actions on this item:
        m_pMenuPopup->popup(event->button, event->time);
        return true; //We handled this event.
      }
    }
  
    //Single-click to select file:
    if(mods & GDK_BUTTON1_MASK)
    {
      Gtk::FileChooserDialog dialog(_("Choose image"), Gtk::FILE_CHOOSER_ACTION_OPEN);
      dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
      dialog.add_button(_("Select"), Gtk::RESPONSE_OK);
      int response = dialog.run();
      dialog.hide();
      if(response != Gtk::RESPONSE_CANCEL)
      {
        const std::string filepath = dialog.get_filename();
        if(!filepath.empty())
        {
          Glib::RefPtr<Gdk::Pixbuf> pixbuf;
          try
          {
            pixbuf = Gdk::Pixbuf::create_from_file(filepath);
            
            //Scale it down to the preferred size:
            //TODO : This will be silly if we allow different image sizes on different layouts:
            const Gtk::Allocation allocation = m_image.get_allocation();
            const int pixbuf_height = pixbuf->get_height();
            const int pixbuf_width = pixbuf->get_width();
            
            if( (pixbuf_height > allocation.get_height()) ||
                (pixbuf_width > allocation.get_width()) )
            {
              pixbuf = scale_keeping_ratio(pixbuf, allocation.get_height(), allocation.get_width());
            }
            
            if(pixbuf)
              m_image.set(pixbuf); //Load the image.
          }
          catch(const Glib::Exception& ex)
          {
            Frame_Glom::show_ok_dialog(_("Image loading failed"), _("The image file could not be opened:\n") + ex.what(), *pApp);
          } 
          
          return true; //We handled this event.
        }
      }
    }
  }

  return Gtk::EventBox::on_button_press_event(event);
}

App_Glom* ImageGlom::get_application()
{
  Gtk::Container* pWindow = get_toplevel();
  //TODO: This only works when the child widget is already in its parent.

  return dynamic_cast<App_Glom*>(pWindow);
}

void ImageGlom::set_value(const Gnome::Gda::Value& /* value */)
{
  //TODO
}

Gnome::Gda::Value ImageGlom::get_value() const
{
  Gnome::Gda::Value result;
  //TODO
  return result;
}

//static:
Glib::RefPtr<Gdk::Pixbuf> ImageGlom::scale_keeping_ratio(const Glib::RefPtr<Gdk::Pixbuf> pixbuf, int target_height, int target_width)
{
  enum enum_scale_mode
  {
    SCALE_WIDTH,
    SCALE_HEIGHT,
    SCALE_NONE
  };
    
  enum_scale_mode scale_mode = SCALE_NONE; //Start with either the width or height, and scale the other according to the ratio.

  const int pixbuf_height = pixbuf->get_height();
  const int pixbuf_width = pixbuf->get_width();
  
  if(pixbuf_height > target_height)
  {
    if(pixbuf_width > target_width)
    {
      //Both are bigger than the target, so find the biggest one:
      if(pixbuf_width > pixbuf_height)
        scale_mode = SCALE_WIDTH;
      else
        scale_mode = SCALE_HEIGHT;
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
    float ratio = (float)target_height / (float)pixbuf_height; 
    target_width = (int)((float)pixbuf_width * ratio);
  }
  else if(scale_mode == SCALE_WIDTH)
  {
    float ratio = (float)target_width / (float) pixbuf_width;
    target_height = (int)((float)pixbuf_height * ratio);
  }
  
  return pixbuf->scale_simple(target_width, target_height, Gdk::INTERP_NEAREST);
}
