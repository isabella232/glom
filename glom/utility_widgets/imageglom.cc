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
#include <glom/libglom/data_structure/glomconversions.h>
//#include <sstream> //For stringstream

#include <iostream>   // for cout, endl

namespace Glom
{

ImageGlom::ImageGlom()
: m_image(Gtk::Stock::MISSING_IMAGE, Gtk::ICON_SIZE_DIALOG), //The widget is invisible if we don't specify an image.
  m_pMenuPopup_UserMode(0)
{
  init();
}

ImageGlom::ImageGlom(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& /* refGlade */)
: Gtk::EventBox(cobject),
  m_image(Gtk::Stock::MISSING_IMAGE, Gtk::ICON_SIZE_DIALOG), //The widget is invisible if we don't specify an image.
  m_pMenuPopup_UserMode(0)
{
  init();
}

void ImageGlom::init()
{
  m_read_only = false;

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

void ImageGlom::set_pixbuf(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf)
{
  m_pixbuf_original = pixbuf;
  m_image.set(m_pixbuf_original);

  scale();
}

void ImageGlom::set_value(const Gnome::Gda::Value& value)
{
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = Conversions::get_pixbuf_for_gda_value(value);
  if(pixbuf)
  {
    set_pixbuf(pixbuf);
    scale();
  }
  else
  {
    /*
    std::cout << "Debug: Setting MISSING_IMAGE" << std::endl;
    
    //Check that this stock icon size is really available,
    //though it would be a distro error if it is not.
    Glib::RefPtr<Gtk::Style> style = get_style();
    if(style)
    {
      /std::cout << "Debug: Setting MISSING_IMAGE 3" << std::endl;

      const Gtk::IconSet iconset = style->lookup_icon_set(Gtk::Stock::MISSING_IMAGE);

      std::cout << "Debug: Setting MISSING_IMAGE 4" << std::endl;

      typedef std::vector<Gtk::IconSize> type_vecSizes;
      type_vecSizes sizes = iconset.get_sizes();
      type_vecSizes::iterator iterFind = std::find(sizes.begin(), sizes.end(), Gtk::ICON_SIZE_DIALOG);
      if(iterFind != sizes.end())
      {
     */
        m_image.set(Gtk::Stock::MISSING_IMAGE, Gtk::ICON_SIZE_DIALOG);
     /*
      }
      else
      {
        std::cerr << "Glom: The current theme does not seem to havae the Gtk::Stock::MISSING_IMAGE icon in size Gtk::ICON_SIZE_DIALOG" << std::endl;

        if(!sizes.empty() && (sizes[0] > 0))
        {
          std::cerr << "  Using alternative stock icon size." << std::endl;
          m_image.set(Gtk::Stock::MISSING_IMAGE, sizes[0]);
        }
        else
        {
          std::cerr << "  No alternative stock icon size available either, for this stock icon." << std::endl;
          m_image.set("");
        }
      }
    }
    else
    {
      std::cerr << "Glom: No Gtk::Style available for this widget (yet), so not setting MISSING_IMAGE icon." << std::endl;
      m_image.set("");
    }
    */

    m_pixbuf_original = Glib::RefPtr<Gdk::Pixbuf>();
  }
}

Gnome::Gda::Value ImageGlom::get_value() const
{
  //TODO: Return the data from the file that was just chosen.
  //Don't store the original here any longer than necessary,
  Gnome::Gda::Value result;
  
  if(m_pixbuf_original)
  {
    try
    {
      gchar* buffer = 0;
      gsize buffer_size = 0;
      std::list<Glib::ustring> list_keys;
      std::list<Glib::ustring> list_values;
      //list_keys.push_back("quality"); //For jpeg only.
      //list_values.push_back("95");

      m_pixbuf_original->save_to_buffer(buffer, buffer_size, GLOM_IMAGE_FORMAT, list_keys, list_values); //Always store images as the standard format in the database.

      //g_warning("ImageGlom::get_value(): debug: to db: ");
      //for(int i = 0; i < 10; ++i)
      //  g_warning("%02X (%c), ", (guint8)buffer[i], buffer[i]);

      result.init(GDA_TYPE_BINARY);
      result.set(reinterpret_cast<const guchar*>(buffer), buffer_size);

      g_free(buffer);
      buffer = 0;
    }
    catch(const Glib::Exception& /* ex */)
    {

    }
  }

  return result;
}

bool ImageGlom::on_expose_event(GdkEventExpose* event)
{
  const bool result = Gtk::EventBox::on_expose_event(event);
  scale();
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
      if(allocation.get_height() > 10 || allocation.get_width() > 10)
      {
        Glib::RefPtr<Gdk::Pixbuf> pixbuf_scaled = scale_keeping_ratio(pixbuf, allocation.get_height(), allocation.get_width());
        if(!pixbuf_scaled)
        {
          std::cerr << "ImageGlom::scale(): scale_keeping_ratio() returned NULL pixbuf." << std::endl;
        }
        else 
        {
          //Don't set a new pixbuf if the dimenstions have not changed:
          Glib::RefPtr<const Gdk::Pixbuf> pixbuf_in_image;

          if(m_image.get_storage_type() == Gtk::IMAGE_PIXBUF) //Prevent warning.
            pixbuf_in_image = m_image.get_pixbuf();

          if( !pixbuf_in_image || (pixbuf_in_image->get_height() != pixbuf_scaled->get_height()) || (pixbuf_in_image->get_width() != pixbuf_scaled->get_width()) )
            m_image.set(pixbuf_scaled);
        }
      }
    }
  }
  //else
  //  g_warning("ImageGlom::scale(): attempt to scale a null pixbuf.");
}

//static:
Glib::RefPtr<Gdk::Pixbuf> ImageGlom::scale_keeping_ratio(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf, int target_height, int target_width)
{
  if( (target_height == 0) || (target_width == 0) )
    return Glib::RefPtr<Gdk::Pixbuf>(); //This shouldn't happen anyway.

  if(!pixbuf)
    return pixbuf;

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
    const float ratio = (float)target_height / (float)pixbuf_height; 
    target_width = (int)((float)pixbuf_width * ratio);
  }
  else if(scale_mode == SCALE_WIDTH)
  {
    const float ratio = (float)target_width / (float) pixbuf_width;
    target_height = (int)((float)pixbuf_height * ratio);
  }

 if( (target_height == 0) || (target_width == 0) )
 {
   return Glib::RefPtr<Gdk::Pixbuf>(); //This shouldn't happen anyway. It seems to happen sometimes though, when ratio is very small.
 }

  return pixbuf->scale_simple(target_width, target_height, Gdk::INTERP_NEAREST);
}

void ImageGlom::on_menupopup_activate_select_file()
{
  if(m_read_only)
    return;

  Gtk::FileChooserDialog dialog(_("Choose image"), Gtk::FILE_CHOOSER_ACTION_OPEN);

  //Get image formats only:
  Gtk::FileFilter filter;
  filter.set_name(_("Images"));
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
          Frame_Glom::show_ok_dialog(_("Image loading failed"), _("The image file could not be opened:\n") + ex.what(), *pApp, Gtk::MESSAGE_ERROR);
      }
    }
  }
}

void ImageGlom::on_clipboard_get(Gtk::SelectionData& selection_data, guint /* info */)
{
  //info is meant to indicate the target, but it seems to be always 0,
  //so we use the selection_data's target instead.

  const std::string target = selection_data.get_target(); 

  if(target == GLOM_IMAGE_FORMAT_MIME_TYPE)
  {
    // This set() override uses an 8-bit text format for the data.
    selection_data.set_pixbuf(m_pixbuf_clipboard);
  }
  else
  {
    g_warning("ExampleWindow::on_clipboard_get(): Unexpected clipboard target format.");
  }
}

void ImageGlom::on_clipboard_clear()
{
  if(m_read_only)
    return;

  m_pixbuf_clipboard.clear();
}

void ImageGlom::on_menupopup_activate_copy()
{
  if(m_pixbuf_original)
  {
    //When copy is used, store it here until it is pasted.
    m_pixbuf_clipboard = m_pixbuf_original->copy(); //TODO: Get it from the DB, when we stop storing the original here instead of just the preview.
  }
  else
    m_pixbuf_clipboard.clear();

  Glib::RefPtr<Gtk::Clipboard> refClipboard = Gtk::Clipboard::get();

  //Targets:
  std::list<Gtk::TargetEntry> listTargets;

  listTargets.push_back( Gtk::TargetEntry(GLOM_IMAGE_FORMAT_MIME_TYPE) );

  refClipboard->set( listTargets, sigc::mem_fun(*this, &ImageGlom::on_clipboard_get), sigc::mem_fun(*this, &ImageGlom::on_clipboard_clear) );
}

void ImageGlom::on_clipboard_received_image(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf)
{
  if(m_read_only)
    return;

  if(pixbuf)
  {
    m_pixbuf_original = pixbuf;

    m_image.set(m_pixbuf_original); //Load the image.
    scale();
    signal_edited().emit();
  }
}


void ImageGlom::on_menupopup_activate_paste()
{
  if(m_read_only)
    return;

  //Tell the clipboard to call our method when it is ready:
  Glib::RefPtr<Gtk::Clipboard> refClipboard = Gtk::Clipboard::get();

  if(refClipboard)
    refClipboard->request_image( sigc::mem_fun(*this, &ImageGlom::on_clipboard_received_image) );
}

void ImageGlom::on_menupopup_activate_clear()
{
  if(m_read_only)
    return;

  m_pixbuf_original.clear();
  m_image.set(Gtk::Stock::MISSING_IMAGE, Gtk::ICON_SIZE_DIALOG);
  signal_edited().emit();
}

void ImageGlom::setup_menu_usermode()
{
  m_refActionGroup_UserModePopup = Gtk::ActionGroup::create();

  m_refActionGroup_UserModePopup->add(Gtk::Action::create("ContextMenu_UserMode", "Context Menu") );
  m_refActionSelectFile =  Gtk::Action::create("ContextSelectFile", Gtk::Stock::EDIT, _("Choose File"));
  m_refActionCopy = Gtk::Action::create("ContextCopy", Gtk::Stock::COPY);
  m_refActionPaste = Gtk::Action::create("ContextPaste", Gtk::Stock::PASTE);
  m_refActionClear = Gtk::Action::create("ContextClear", Gtk::Stock::CLEAR);

  m_refActionGroup_UserModePopup->add(m_refActionSelectFile,
    sigc::mem_fun(*this, &ImageGlom::on_menupopup_activate_select_file) );

  m_refActionGroup_UserModePopup->add(m_refActionCopy,
    sigc::mem_fun(*this, &ImageGlom::on_menupopup_activate_copy) );

  m_refActionGroup_UserModePopup->add(m_refActionPaste,
    sigc::mem_fun(*this, &ImageGlom::on_menupopup_activate_paste) );

  m_refActionGroup_UserModePopup->add(m_refActionClear,
    sigc::mem_fun(*this, &ImageGlom::on_menupopup_activate_clear) );

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
        "    <menuitem action='ContextClear'/>"
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

void ImageGlom::do_choose_image()
{
  on_menupopup_activate_select_file();
}

void ImageGlom::set_read_only(bool read_only)
{
  m_read_only = read_only;
}


} //namespace Glom
