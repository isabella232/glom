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
#include <glom/application.h>
#include <glom/utils_ui.h>
#include <glom/glade_utils.h>
#include <libglom/data_structure/glomconversions.h>
#include <glom/utility_widgets/dialog_image_load_progress.h>
#include <glom/utility_widgets/dialog_image_save_progress.h>

#include <iostream>   // for cout, endl

namespace Glom
{

ImageGlom::ImageGlom()
: m_image(Gtk::Stock::MISSING_IMAGE, Gtk::ICON_SIZE_DIALOG), //The widget is invisible if we don't specify an image.
  m_pMenuPopup_UserMode(0)
{
  init();
}

ImageGlom::ImageGlom(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& /* builder */)
: Gtk::EventBox(cobject),
  m_image(Gtk::Stock::MISSING_IMAGE, Gtk::ICON_SIZE_DIALOG), //The widget is invisible if we don't specify an image.
  m_pMenuPopup_UserMode(0)
{
  init();
}

void ImageGlom::init()
{
  m_read_only = false;

#ifndef GLOM_ENABLE_CLIENT_ONLY
  setup_menu();
#endif // !GLOM_ENABLE_CLIENT_ONLY

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

void ImageGlom::set_layout_item(const sharedptr<LayoutItem>& layout_item, const Glib::ustring& table_name)
{
  LayoutWidgetField::set_layout_item(layout_item, table_name);
#ifdef GTKMM_ATKMM_ENABLED
  get_accessible()->set_name(layout_item->get_name());
#endif  
}

bool ImageGlom::on_button_press_event(GdkEventButton *event)
{
  GdkModifierType mods;
  gdk_window_get_pointer( gtk_widget_get_window (Gtk::Widget::gobj()), 0, 0, &mods );

  //Enable/Disable items.
  //We did this earlier, but get_application is more likely to work now:
  Application* pApp = get_application();
  if(pApp)
  {
#ifndef GLOM_ENABLE_CLIENT_ONLY
    pApp->add_developer_action(m_refContextLayout); //So that it can be disabled when not in developer mode.
    pApp->add_developer_action(m_refContextAddField);
    pApp->add_developer_action(m_refContextAddRelatedRecords);
    pApp->add_developer_action(m_refContextAddGroup);

    pApp->update_userlevel_ui(); //Update our action's sensitivity.
#endif // !GLOM_ENABLE_CLIENT_ONLY

    //Only show this popup in developer mode, so operators still see the default GtkEntry context menu.
    //TODO: It would be better to add it somehow to the standard context menu.
#ifndef GLOM_ENABLE_CLIENT_ONLY
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
#endif // !GLOM_ENABLE_CLIENT_ONLY
    {
      // We cannot be in developer mode in client only mode.
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

Application* ImageGlom::get_application()
{
  Gtk::Container* pWindow = get_toplevel();
  //TODO: This only works when the child widget is already in its parent.

  return dynamic_cast<Application*>(pWindow);
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
  // Remember original data 
  m_original_data = Gnome::Gda::Value();
  m_original_data = value;
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = Utils::get_pixbuf_for_gda_value(value);

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
  if(m_original_data.get_value_type() != G_TYPE_NONE)
    return m_original_data;
  
  if(m_pixbuf_original)
  {
    try
    {
      gchar* buffer = 0;
      gsize buffer_size = 0;
      std::vector<Glib::ustring> list_keys;
      std::vector<Glib::ustring> list_values;
      //list_keys.push_back("quality"); //For jpeg only.
      //list_values.push_back("95");

      m_pixbuf_original->save_to_buffer(buffer, buffer_size, GLOM_IMAGE_FORMAT, list_keys, list_values); //Always store images as the standard format in the database.

      //g_warning("ImageGlom::get_value(): debug: to db: ");
      //for(int i = 0; i < 10; ++i)
      //  g_warning("%02X (%c), ", (guint8)buffer[i], buffer[i]);

      GdaBinary* bin = g_new(GdaBinary, 1);
      bin->data = reinterpret_cast<guchar*>(buffer);
      bin->binary_length = buffer_size;

      m_original_data = Gnome::Gda::Value();
      m_original_data.Glib::ValueBase::init(GDA_TYPE_BINARY);
      gda_value_take_binary(m_original_data.gobj(), bin);

      buffer = 0;
      return m_original_data;
    }
    catch(const Glib::Exception& ex)
    {
      std::cerr << G_STRFUNC << ": " << ex.what() << std::endl;
    }
  }

  return Gnome::Gda::Value();
}

bool ImageGlom::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
  const bool result = Gtk::EventBox::on_draw(cr);

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
        Glib::RefPtr<Gdk::Pixbuf> pixbuf_scaled = Utils::image_scale_keeping_ratio(pixbuf, allocation.get_height(), allocation.get_width());
        if(!pixbuf_scaled)
        {
          std::cerr << "Utils::image_scale_keeping_ratio() returned NULL pixbuf." << std::endl;
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

void ImageGlom::on_menupopup_activate_open_file()
{
  open_with();
}

void ImageGlom::on_menupopup_activate_open_file_with()
{
  Application* pApp = get_application();

  //Offer the user a choice of suitable applications:
  Gtk::AppChooserDialog dialog(GLOM_IMAGE_FORMAT_MIME_TYPE);
  if(pApp)
    dialog.set_transient_for(*pApp);

  if(dialog.run() != Gtk::RESPONSE_OK)
    return;
  
  Glib::RefPtr<Gio::AppInfo> app_info = dialog.get_app_info();
  if(!app_info)
  {
    std::cerr << G_STRFUNC << ": app_info was null." << std::endl;
  }
  
  open_with(app_info);
}

void ImageGlom::open_with(const Glib::RefPtr<Gio::AppInfo>& app_info)
{
  //Get a temporary file path:
  std::string filepath;
  const int filehandle = Glib::file_open_tmp(filepath);
  ::close(filehandle);
  
  if(filepath.empty())
  {
    std::cerr << G_STRFUNC << ": Glib::file_open_tmp() returned an empty filepath" << std::endl;
    return;
  }
  
  const Glib::ustring uri = Glib::filename_to_uri(filepath);
  
  if(!save_file(uri))
    return;

  if(app_info)
  {
    std::vector<std::string> vec_uris;
    vec_uris.push_back(uri);
    std::cout << "app_info: " << app_info->get_name() << ", uri=" << uri << std::endl;
    app_info->launch_uris(vec_uris, 0); //TODO: Get a GdkAppLaunchContext?
  }
  else
  {
    //TODO: Avoid duplication in xsl_utils.cc, by moving this into a utility function:  
#ifdef G_OS_WIN32
    // gtk_show_uri doesn't seem to work on Win32, at least not for local files
    // We use Windows API instead.
    // TODO: Check it again and file a bug if necessary.
    ShellExecute(0, "open", uri.c_str(), 0, 0, SW_SHOW);
#else
    //Use the GNOME browser:
    GError* gerror = 0;
    if(!gtk_show_uri(0 /* screen */, uri.c_str(), GDK_CURRENT_TIME, &gerror))
    {
      std::cerr << G_STRFUNC << ": " << gerror->message << std::endl;
      g_error_free(gerror);
    }
#endif //G_OS_WIN32
  }
}


static Glib::RefPtr<Gtk::FileFilter> get_file_filter_images()
{
  //Get image formats only:
  Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
  filter->set_name(_("Images"));
  filter->add_pixbuf_formats();
  
  return filter;
}

void ImageGlom::on_menupopup_activate_save_file()
{
  Application* pApp = get_application();

  Gtk::FileChooserDialog dialog(_("Save Image"), Gtk::FILE_CHOOSER_ACTION_SAVE);
  if(pApp)
    dialog.set_transient_for(*pApp);
          
  dialog.add_filter( get_file_filter_images() );

  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);
  const int response = dialog.run();
  dialog.hide();
  if(response != Gtk::RESPONSE_OK)
    return;
    
  const Glib::ustring uri = dialog.get_uri();
  if(uri.empty())
    return;
    
  save_file(uri);
}

bool ImageGlom::save_file(const Glib::ustring& uri)
{
  DialogImageSaveProgress* dialog_save = 0;
  Utils::get_glade_widget_derived_with_warning(dialog_save);
  if(!dialog_save)
    return false;
    
  // Automatically delete the dialog when we no longer need it:
  std::auto_ptr<Gtk::Dialog> dialog_keeper(dialog_save);

  Application* pApp = get_application();
  if(pApp)
    dialog_save->set_transient_for(*pApp);

  dialog_save->set_pixbuf(m_pixbuf_original);
  dialog_save->save(uri);

  //TODO: Use this when we do async saving:
  //dialog_save->run();
  return true;
}

void ImageGlom::on_menupopup_activate_select_file()
{
  if(m_read_only)
    return;
    
  Application* pApp = get_application();

  //TODO: Use Hildon::FileChooser for Maemo.
  Gtk::FileChooserDialog dialog(_("Choose Image"), Gtk::FILE_CHOOSER_ACTION_OPEN);
  if(pApp)
    dialog.set_transient_for(*pApp);
          
  dialog.add_filter( get_file_filter_images() );

  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(_("Select"), Gtk::RESPONSE_OK);
  int response = dialog.run();
  dialog.hide();

  if((response != Gtk::RESPONSE_CANCEL) && (response != Gtk::RESPONSE_DELETE_EVENT))
  {
    const Glib::ustring uri = dialog.get_uri();
    if(!uri.empty())
    {
      DialogImageLoadProgress* dialog;
      Utils::get_glade_widget_derived_with_warning(dialog);
      if(dialog)
      {
        // Automatically delete the dialog when we no longer need it:
        std::auto_ptr<Gtk::Dialog> dialog_keeper(dialog);

        if(pApp)
          dialog->set_transient_for(*pApp);

        dialog->load(uri);

        if(dialog->run() == Gtk::RESPONSE_ACCEPT)
        {
          GdaBinary* bin = g_new(GdaBinary, 1);
          std::auto_ptr<GdaBinary> image_data = dialog->get_image_data();
          bin->data = image_data->data;
          bin->binary_length = image_data->binary_length;

          m_original_data = Gnome::Gda::Value();
          m_original_data.Glib::ValueBase::init(GDA_TYPE_BINARY);
          gda_value_take_binary(m_original_data.gobj(), bin);

          m_pixbuf_original = dialog->get_pixbuf();
          m_image.set(m_pixbuf_original); //Load the image.
          scale();
          signal_edited().emit();
        }
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

  m_pixbuf_clipboard.reset();
}

void ImageGlom::on_menupopup_activate_copy()
{
  if(m_pixbuf_original)
  {
    //When copy is used, store it here until it is pasted.
    m_pixbuf_clipboard = m_pixbuf_original->copy(); //TODO: Get it from the DB, when we stop storing the original here instead of just the preview.
  }
  else
    m_pixbuf_clipboard.reset();

  Glib::RefPtr<Gtk::Clipboard> refClipboard = Gtk::Clipboard::get();

  //Targets:
  std::vector<Gtk::TargetEntry> listTargets;
  listTargets.push_back( Gtk::TargetEntry(GLOM_IMAGE_FORMAT_MIME_TYPE) );

  refClipboard->set( listTargets, sigc::mem_fun(*this, &ImageGlom::on_clipboard_get), sigc::mem_fun(*this, &ImageGlom::on_clipboard_clear) );
}

void ImageGlom::on_clipboard_received_image(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf)
{
  if(m_read_only)
    return;

  if(pixbuf)
  {
    // Clear original data of previous image
    m_original_data = Gnome::Gda::Value();

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

  m_pixbuf_original.reset();
  m_image.set(Gtk::Stock::MISSING_IMAGE, Gtk::ICON_SIZE_DIALOG);
  signal_edited().emit();
}

void ImageGlom::setup_menu_usermode()
{
  m_refActionGroup_UserModePopup = Gtk::ActionGroup::create();

  m_refActionGroup_UserModePopup->add(Gtk::Action::create("ContextMenu_UserMode", "Context Menu") );
  
  m_refActionOpenFile =  Gtk::Action::create("ContextOpenFile", Gtk::Stock::OPEN);
  m_refActionOpenFileWith =  Gtk::Action::create("ContextOpenFileWith", Gtk::Stock::OPEN, _("Open With"));
  m_refActionSaveFile =  Gtk::Action::create("ContextSaveFile", Gtk::Stock::SAVE);
  m_refActionSelectFile =  Gtk::Action::create("ContextSelectFile", Gtk::Stock::EDIT, _("Choose File"));
  m_refActionCopy = Gtk::Action::create("ContextCopy", Gtk::Stock::COPY);
  m_refActionPaste = Gtk::Action::create("ContextPaste", Gtk::Stock::PASTE);
  m_refActionClear = Gtk::Action::create("ContextClear", Gtk::Stock::CLEAR);

  m_refActionGroup_UserModePopup->add(m_refActionOpenFile,
    sigc::mem_fun(*this, &ImageGlom::on_menupopup_activate_open_file) );

  m_refActionGroup_UserModePopup->add(m_refActionOpenFileWith,
    sigc::mem_fun(*this, &ImageGlom::on_menupopup_activate_open_file_with) );
    
  m_refActionGroup_UserModePopup->add(m_refActionSaveFile,
    sigc::mem_fun(*this, &ImageGlom::on_menupopup_activate_save_file) );
    
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
        "    <menuitem action='ContextOpenFile'/>"
        "    <menuitem action='ContextOpenFileWith'/>"
        "    <menuitem action='ContextSaveFile'/>"
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
