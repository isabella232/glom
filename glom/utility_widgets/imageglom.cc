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
#include "../data_structure/glomconversions.h"
//#include <sstream> //For stringstream

#include <iostream>   // for cout, endl

ImageGlom::ImageGlom()
: m_image(Gtk::Stock::MISSING_IMAGE, Gtk::ICON_SIZE_DIALOG), //The widget is invisible if we don't specify an image.
  m_pMenuPopup_UserMode(0)
{
  setup_menu();
  setup_menu_usermode();
  
  m_image.set_size_request(150, 150);
  m_image.show();
  
  m_frame.set_shadow_type(Gtk::SHADOW_ETCHED_IN); //Without this, the image widget has no borders and is completely invisible when empty.
  m_frame.add(m_image);
  m_frame.show();
  
  add(m_frame);
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
    else
    {
      if(mods & GDK_BUTTON3_MASK)
      {
        //Give user choices of actions on this item:
        m_pMenuPopup_UserMode->popup(event->button, event->time);
        return true; //We handled this event.
      }
    }
  
    //Single-click to select file:
    if(mods & GDK_BUTTON1_MASK)
    {
      on_menupopup_activate_select_file();
      return true; //We handled this event.

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

bool ImageGlom::get_has_original_data() const
{
  return true; //TODO.
}
  
void ImageGlom::set_value(const Gnome::Gda::Value& value)
{ 
  g_warning("debug: set_value(): start");
  
  bool pixbuf_set = false;
  
  if(value.get_value_type() == Gnome::Gda::VALUE_TYPE_BINARY)
  {
    glong size = 0;
    const gpointer pData = value.get_binary(&size);
    if(size && pData)
    {
      //libgda does not currently properly unescape binary data,
      //so pData is actually a null terminated string, of escaped binary data.
      //This workaround should be removed when libgda is fixed:
      size_t buffer_binary_length = 0;
      guchar* buffer_binary =  Glom_PQunescapeBytea((const guchar*)pData /* must be null-terminated */, &buffer_binary_length); //freed by us later.
      if(buffer_binary)
      {
        //typedef std::list<Gdk::PixbufFormat> type_list_formats;
        //const type_list_formats formats = Gdk::Pixbuf::get_formats();
        //std::cout << "Debug: Supported pixbuf formats:" << std::endl;
        //for(type_list_formats::const_iterator iter = formats.begin(); iter != formats.end(); ++iter)
        //{
        //  std::cout << " name=" << iter->get_name() << ", writable=" << iter->is_writable() << std::endl;
        //}
        
        Glib::RefPtr<Gdk::PixbufLoader> refPixbufLoader;
        
        // PixbufLoader::create() is broken in gtkmm before 2.6.something,
        // so let's do this in C so it works with all 2.6 versions:
        GError* error = 0;
        GdkPixbufLoader* loader = gdk_pixbuf_loader_new_with_type("png", &error);
        if(!error)
          refPixbufLoader = Glib::wrap(loader);
        
        /*
        try
        {
          refPixbufLoader = Gdk::PixbufLoader::create("png");
          g_warning("debug a1");
        }
        catch(const Gdk::PixbufError& ex)
        {
          refPixbufLoader.clear();
          g_warning("PixbufLoader::create failed: %s",ex.what().c_str());
        }
        */
        
        if(refPixbufLoader)
        {
          try
          {
            guint8* puiData = (guint8*)buffer_binary;
            
            //g_warning("ImageGlom::set_value(): debug: from db: ");
            //for(int i = 0; i < 10; ++i)
            //  g_warning("%02X (%c), ", (guint8)puiData[i], (char)puiData[i]);
              
             g_warning("  debug: debug z1.");
            refPixbufLoader->write(puiData, (glong)buffer_binary_length);
            g_warning("  debug: debug z2.");
            m_pixbuf_original = refPixbufLoader->get_pixbuf();
            m_image.set(m_pixbuf_original);
            pixbuf_set = true;
            
            g_warning("  debug: debug zscale1.");
            scale();
            g_warning("  debug: debug zscale2.");
          }
          catch(const Glib::Exception& ex)
          {
            g_warning("ImageGlom::set_value(): PixbufLoader::write() failed: %s", ex.what().c_str());
          }
          
          refPixbufLoader->close();
          
          free(buffer_binary);
          g_warning("  debug: debug z3.");
        }
      }
            
      //TODO: load the image, using the mime type stored elsewhere.
      //pixbuf = Gdk::Pixbuf::create_from_data(
    }
    
  }
  
  if(!pixbuf_set)
  {
    g_warning("debug: set_value(): setting to empty image.");
    m_image.set(Gtk::Stock::MISSING_IMAGE, Gtk::ICON_SIZE_DIALOG);
  }
  
 g_warning("debug: set_value(): end");
 
}

Gnome::Gda::Value ImageGlom::get_value() const
{
 g_warning("debug: get_value(): start");
 
  //TODO: Return the data from the file that was just chosen.
  //Don't store the original here any longer than necessary,
  Gnome::Gda::Value result; //TODO: Initialize it as binary.
  
  if(m_pixbuf_original)
  {
    try
    {
      gchar* buffer = 0;
      gsize buffer_size = 0;
      std::list<Glib::ustring> list_empty;
      m_pixbuf_original->save_to_buffer(buffer, buffer_size, "png", list_empty, list_empty); //Always store images as PNG in the database.
      
      //g_warning("ImageGlom::get_value(): debug: to db: ");
      //for(int i = 0; i < 10; ++i)
      //  g_warning("%02X (%c), ", (guint8)buffer[i], buffer[i]);
          
      result.set(buffer, buffer_size);
      
      g_free(buffer);
      buffer = 0;
    }
    catch(const Glib::Exception& /* ex */)
    {
    
    }
  }
  
  g_warning("debug: get_value(): end");
  return result;
}

void ImageGlom::scale()
{
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = m_pixbuf_original;
  
  if(pixbuf)
  {
    const Gtk::Allocation allocation = m_image.get_allocation();
    const int pixbuf_height = pixbuf->get_height();
    const int pixbuf_width = pixbuf->get_width();
          
    if( (pixbuf_height > allocation.get_height()) ||
        (pixbuf_width > allocation.get_width()) )
    {
      Glib::RefPtr<Gdk::Pixbuf> pixbuf_scaled = scale_keeping_ratio(pixbuf, allocation.get_height(), allocation.get_width());
      m_image.set(pixbuf_scaled);
    }
  }
  else
    g_warning("ImageGlom::scale(): attempt to scale a null pixbuf.");
}

//static:
Glib::RefPtr<Gdk::Pixbuf> ImageGlom::scale_keeping_ratio(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf, int target_height, int target_width)
{
  if( (target_height == 0) || (target_width == 0) )
    return Glib::RefPtr<Gdk::Pixbuf>(); //This shouldn't happen anyway.
    
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
  
  g_warning("debug: before scale_simple: target_width=%d, target_height=%d", target_width, target_height);
  return pixbuf->scale_simple(target_width, target_height, Gdk::INTERP_NEAREST);
}

void ImageGlom::on_menupopup_activate_select_file()
{
  Gtk::FileChooserDialog dialog(_("Choose image"), Gtk::FILE_CHOOSER_ACTION_OPEN);
  
  //Get image formats only:
  Gtk::FileFilter filter;
  filter.add_pixbuf_formats();
  dialog.add_filter(filter);
  
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(_("Select"), Gtk::RESPONSE_OK);
  int response = dialog.run();
  dialog.hide();
  
  if(response != Gtk::RESPONSE_CANCEL)
  {
    const std::string filepath = dialog.get_filename();
    if(!filepath.empty())
    {
      try
      {
        m_pixbuf_original = Gdk::Pixbuf::create_from_file(filepath);
        if(m_pixbuf_original)
        {
          m_image.set(m_pixbuf_original); //Load the image.
          scale();
          signal_edited().emit();
        }
        else
        {
          g_warning("ImageGlom::on_menupopup_activate_select_file(): file load failed.");
        }
      }
      catch(const Glib::Exception& ex)
      {
        App_Glom* pApp = get_application();
        if(pApp)
          Frame_Glom::show_ok_dialog(_("Image loading failed"), _("The image file could not be opened:\n") + ex.what(), *pApp);
      }
    }
  }        
}

void ImageGlom::on_menupopup_activate_copy()
{

}

void ImageGlom::on_menupopup_activate_paste()
{

}

void ImageGlom::setup_menu_usermode()
{
  m_refActionGroup_UserModePopup = Gtk::ActionGroup::create();
  
  m_refActionGroup_UserModePopup->add(Gtk::Action::create("ContextMenu_UserMode", "Context Menu") );
  m_refActionSelectFile =  Gtk::Action::create("ContextSelectFile", Gtk::Stock::EDIT, _("Choose File"));
  m_refActionCopy = Gtk::Action::create("ContextCopy", Gtk::Stock::COPY);
  m_refActionPaste = Gtk::Action::create("ContextPaste", Gtk::Stock::PASTE);
 
  
  m_refActionGroup_UserModePopup->add(m_refActionSelectFile,
    sigc::mem_fun(*this, &ImageGlom::on_menupopup_activate_select_file) );

  m_refActionGroup_UserModePopup->add(m_refActionCopy,
    sigc::mem_fun(*this, &ImageGlom::on_menupopup_activate_copy) );

  m_refActionGroup_UserModePopup->add(m_refActionPaste,
    sigc::mem_fun(*this, &ImageGlom::on_menupopup_activate_paste) );

  m_refUIManager_UserModePopup = Gtk::UIManager::create();

  m_refUIManager_UserModePopup->insert_action_group(m_refActionGroup_UserModePopup);

  //TODO: add_accel_group(m_refUIManager_UserModePopup->get_accel_group());

  try
  {
    Glib::ustring ui_info = 
        "<ui>"
        "  <popup name='ContextMenu_UserMode'>"
        "    <menuitem action='ContextSelectFile'/>"
        "    <menuitem action='ContextCopy'/>"
        "    <menuitem action='ContextPaste'/>"
        "  </popup>"
        "</ui>";

    m_refUIManager_UserModePopup->add_ui_from_string(ui_info);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << "building menus failed: " <<  ex.what();
  }

  //Get the menu:
  m_pMenuPopup_UserMode = dynamic_cast<Gtk::Menu*>( m_refUIManager_UserModePopup->get_widget("/ContextMenu_UserMode") ); 
  if(!m_pMenuPopup_UserMode)
    g_warning("menu not found");
}
