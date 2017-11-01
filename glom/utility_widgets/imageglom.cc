/* Glom
 *
 * Copyright (C) 2001-2011 Murray Cumming
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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#include "imageglom.h"
#include <glibmm/i18n.h>
#include <glom/appwindow.h>
#include <glom/utils_ui.h>
#include <glom/glade_utils.h>
#include <libglom/algorithms_utils.h>
#include <libglom/data_structure/glomconversions.h>
#include <glom/utility_widgets/dialog_image_load_progress.h>
#include <glom/utility_widgets/dialog_image_save_progress.h>
#include <libglom/utils.h>
#include <libglom/file_utils.h>
#include <gtkmm/appchooserdialog.h>
#include <gtkmm/filechooserdialog.h>
#include <giomm/contenttype.h>
#include <giomm/menu.h>
#include <libgda/gda-blob-op.h>
#include <glibmm/convert.h>

#ifdef G_OS_WIN32
#include <windows.h>
#endif

#include <iostream>   // for cout, endl

namespace Glom
{

ImageGlom::type_vec_ustrings ImageGlom::m_evince_supported_mime_types;
ImageGlom::type_vec_ustrings ImageGlom::m_gdkpixbuf_supported_mime_types;

ImageGlom::ImageGlom()
//: m_ev_view(nullptr),
  //m_ev_document_model(nullptr)
{
  init();
}

ImageGlom::ImageGlom(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& /* builder */)
: Gtk::EventBox(cobject) /*,
  m_ev_view(nullptr),
  m_ev_document_model(nullptr) */
{
  init();
}

void ImageGlom::clear_image_from_widgets()
{
  if(m_image)
  {
    m_image->set(Glib::RefPtr<Gdk::Pixbuf>()); //TODO: Add an unset() to gtkmm.
  }

  /*
  if(m_ev_document_model)
  {
    g_object_unref(m_ev_document_model);
    m_ev_document_model = nullptr;
  }
  */
}

void ImageGlom::init_widgets(bool /* use_evince */)
{
  clear_image_from_widgets();

  m_frame.remove();

  Gtk::Widget* widget = nullptr;

  /*
  if(use_evince)
  {
    if(!m_ev_view)
    {
      m_ev_view = EV_VIEW(ev_view_new());
      gtk_widget_show(GTK_WIDGET(m_ev_view));

      m_ev_scrolled_window = std::make_unique<Gtk::ScrolledWindow>();
      gtk_container_add(GTK_CONTAINER(m_ev_scrolled_window->gobj()), GTK_WIDGET(m_ev_view));

      //gtk_widget_add_events(GTK_WIDGET(m_ev_view), GDK_BUTTON_PRESS_MASK);

      //Connect the the EvView's button-press-event signal,
      //because we don't get it otherwise.
      //For some reason this is not necessary with the GtkImage.
      auto cppEvView = Glib::wrap(GTK_WIDGET(m_ev_view));
      cppEvView->signal_button_press_event().connect(
        sigc::mem_fun(*this, &ImageGlom::on_button_press_event), false);
    }

    m_image.reset();

    widget = m_ev_scrolled_window.get();
  }
  else
  */
  {
    m_image = std::make_unique<Gtk::Image>();
    /*
    if(m_ev_view)
    {
      gtk_widget_destroy(GTK_WIDGET(m_ev_view));
      m_ev_view = nullptr;
      m_ev_scrolled_window.reset();
    }
    */

    widget = m_image.get();
  }

  widget->show();
  m_frame.add(*widget);
}

void ImageGlom::init()
{
  m_read_only = false;

#ifndef GLOM_ENABLE_CLIENT_ONLY
  setup_menu(this);
#endif // !GLOM_ENABLE_CLIENT_ONLY

  setup_menu_usermode();

  m_frame.set_shadow_type(Gtk::ShadowType::ETCHED_IN); //Without this, the image widget has no borders and is completely invisible when empty.
  m_frame.show();

  add(m_frame);
}

void ImageGlom::set_layout_item(const std::shared_ptr<LayoutItem>& layout_item, const Glib::ustring& table_name)
{
  LayoutWidgetField::set_layout_item(layout_item, table_name);
#ifdef GTKMM_ATKMM_ENABLED
  get_accessible()->set_name(layout_item->get_name());
#endif
}

bool ImageGlom::on_button_press_event(Gdk::EventButton& button_event)
{
  auto gdkwindow = get_window();
  Gdk::ModifierType mods;
  int x = 0;
  int y = 0;
  gdkwindow->get_device_position(button_event.get_device(), x, y, mods);

  //Enable/Disable items.
  //We did this earlier, but get_appwindow is more likely to work now:
  auto pApp = get_appwindow();
  if(pApp)
  {
#ifndef GLOM_ENABLE_CLIENT_ONLY
    pApp->add_developer_action(m_context_layout); //So that it can be disabled when not in developer mode.
    pApp->add_developer_action(m_context_add_field);
    pApp->add_developer_action(m_context_add_related_records);
    pApp->add_developer_action(m_context_add_group);

    pApp->update_userlevel_ui(); //Update our action's sensitivity.
#endif // !GLOM_ENABLE_CLIENT_ONLY

    //Only show this popup in developer mode, so operators still see the default GtkEntry context menu.
    //TODO: It would be better to add it somehow to the standard context menu.
#ifndef GLOM_ENABLE_CLIENT_ONLY
    if(pApp->get_userlevel() == AppState::userlevels::DEVELOPER)
    {
      if((mods & Gdk::ModifierType::BUTTON3_MASK) == Gdk::ModifierType::BUTTON3_MASK)
      {
        //Give user choices of actions on this item:
        popup_menu(button_event);

        return true; //We handled this event.
      }
    }
    else
#endif // !GLOM_ENABLE_CLIENT_ONLY
    {
      // We cannot be in developer mode in client only mode.
      if((mods & Gdk::ModifierType::BUTTON3_MASK) == Gdk::ModifierType::BUTTON3_MASK)
      {
        //Give user choices of actions on this item:
        popup_menu(button_event);

        return true; //We handled this event.
      }
    }

    //Single-click to select file:
    if((mods & Gdk::ModifierType::BUTTON1_MASK) == Gdk::ModifierType::BUTTON1_MASK)
    {
      on_menupopup_activate_select_file();
      return true; //We handled this event.

    }
  }

  return Gtk::EventBox::on_button_press_event(button_event);
}

AppWindow* ImageGlom::get_appwindow() const
{
  auto pWindow = const_cast<Gtk::Container*>(get_toplevel());
  //TODO: This only works when the child widget is already in its parent.

  return dynamic_cast<AppWindow*>(pWindow);
}

/*
void ImageGlom::set_pixbuf(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf)
{
  m_pixbuf_original = pixbuf;
  show_image_data();
}
*/

void ImageGlom::set_value(const Gnome::Gda::Value& value)
{
  // Remember original data
  clear_original_data();
  m_original_data = value;
  show_image_data();
}

Gnome::Gda::Value ImageGlom::get_value() const
{
  return m_original_data;
}

void ImageGlom::on_size_allocate(const Gtk::Allocation& allocation, int baseline, Gtk::Allocation& out_clip)
{
  Gtk::EventBox::on_size_allocate(allocation, baseline, out_clip);

  //Resize the GtkImage if necessary:
  if(m_pixbuf_original)
  {
    const auto pixbuf_scaled = get_scaled_image();
    m_image->set(pixbuf_scaled);
  }
}

/*
static void image_glom_ev_job_finished(EvJob* job, void* user_data)
{
  g_assert(job);

  auto self = static_cast<ImageGlom*>(user_data);
  g_assert(self);

  self->on_ev_job_finished(job);
}

void ImageGlom::on_ev_job_finished(EvJob* job)
{
  if(ev_job_is_failed (job)) {
    g_warning ("%s", job->error->message);
    g_object_unref (job);

    return;
  }

  ev_document_model_set_document(m_ev_document_model, job->document);
  ev_document_model_set_page(m_ev_document_model, 1);
  g_object_unref (job);

  //TODO: Show that we are no longer loading.
  //ev_view_set_loading(m_ev_view, FALSE);
}
*/

const GdaBinary* ImageGlom::get_binary() const
{
  const GdaBinary* gda_binary = nullptr;
  if(m_original_data.get_value_type() == GDA_TYPE_BINARY)
    gda_binary = gda_value_get_binary(m_original_data.gobj());
  else if(m_original_data.get_value_type() == GDA_TYPE_BLOB)
  {
    const auto gda_blob = gda_value_get_blob(m_original_data.gobj());
    const auto op = gda_blob ? gda_blob_get_op(const_cast<GdaBlob*>(gda_blob)) : nullptr;
    if(gda_blob && op && gda_blob_op_read_all(op, const_cast<GdaBlob*>(gda_blob))) {
      gda_binary = gda_blob_get_binary(const_cast<GdaBlob*>(gda_blob));
    }
  }

  return gda_binary;
}

Glib::ustring ImageGlom::get_mime_type() const
{
  const auto gda_binary = get_binary();

  if(!gda_binary)
    return Glib::ustring();

  const auto data = gda_binary_get_data(const_cast<GdaBinary*>(gda_binary)); 
  if(!data)
    return Glib::ustring();

  bool uncertain = false;
  const auto datalen = gda_binary_get_size(const_cast<GdaBinary*>(gda_binary));
  const auto result = Gio::content_type_guess(std::string(),
    static_cast<const guchar*>(data), datalen,
    uncertain);

  //std::cout << G_STRFUNC << ": mime_type=" << result << ", uncertain=" << uncertain << std::endl;
  return result;
}

void ImageGlom::fill_evince_supported_mime_types()
{
  //Fill the static list if it has not already been filled:
  if(!m_evince_supported_mime_types.empty())
    return;

/*
  //Discover what mime types libevview can support.
  //Older versions supported image types too, via GdkPixbuf,
  //but that support was then removed.
  auto types_list = ev_backends_manager_get_all_types_info();
  if(!types_list)
  {
    return;
  }

  for(GList* l = types_list; l; l = g_list_next(l))
  {
    EvTypeInfo *info = (EvTypeInfo *)l->data;
    if(!info)
      continue;

    const char* mime_type = nullptr;
    int i = 0;
    while((mime_type = info->mime_types[i++]))
    {
      if(mime_type)
        m_evince_supported_mime_types.emplace_back(mime_type);
      //std::cout << "evince supported mime_type=" << mime_type << std::endl;
    }
  }
*/
}

void ImageGlom::fill_gdkpixbuf_supported_mime_types()
{
  //Fill the static list if it has not already been filled:
  if(!m_gdkpixbuf_supported_mime_types.empty())
    return;

  for(const auto& format : Gdk::Pixbuf::get_formats())
  {
    const auto mime_types = format.get_mime_types();
    m_gdkpixbuf_supported_mime_types.insert(
      m_gdkpixbuf_supported_mime_types.end(),
      mime_types.begin(), mime_types.end());
  }
}

void ImageGlom::show_image_data()
{
  bool use_evince = false;

  const auto mime_type = get_mime_type();

  //std::cout << "mime_type=" << mime_type << std::endl;

  fill_evince_supported_mime_types();
  if(Utils::find_exists(m_evince_supported_mime_types, mime_type))
  {
    use_evince = true;
  }

  init_widgets(use_evince);

  //Clear all possible display widgets:
  m_pixbuf_original.reset();

  if(use_evince)
  {
    // Try loading from data in memory:
    // TODO: Uncomment this if this API is added: https://bugzilla.gnome.org/show_bug.cgi?id=654832
    /*
    const auto gda_binary = get_binary();
    if(!gda_binary || !gda_binary->data || !gda_binary->binary_length)
    {
       std::cerr << G_STRFUNC << "Data was null or empty.\n";
      return;
    }

    EvJob *job = ev_job_load_new_with_data(
      (char*)gda_binary->data, gda_binary->binary_length);
    */
    //TODO: Test failure asynchronously.

    /*
    const auto uri = save_to_temp_file(false / don't show progress /);
    if(uri.empty())
    {
      std::cerr << G_STRFUNC << "Could not save temp file to show in the EvView.\n";
    }

    EvJob *job = ev_job_load_new(uri.c_str());

    m_ev_document_model = ev_document_model_new();
    ev_view_set_model(m_ev_view, m_ev_document_model);
    ev_document_model_set_continuous(m_ev_document_model, FALSE); //Show only one page.

    //TODO: Show that we are loading.
    //ev_view_set_loading(m_ev_view, TRUE);

    g_signal_connect (job, "finished",
      G_CALLBACK (image_glom_ev_job_finished), this);
    ev_job_scheduler_push_job (job, EV_JOB_PRIORITY_NONE);
    */
  }
  else
  {
    //Use GtkImage instead:
    Glib::RefPtr<const Gio::Icon> icon;

    bool use_gdkpixbuf = false;
    fill_gdkpixbuf_supported_mime_types();
    if(Utils::find_exists(m_gdkpixbuf_supported_mime_types, mime_type))
    {
      use_gdkpixbuf = true;
    }

    if(use_gdkpixbuf)
    {
      //Try to use GdkPixbuf's loader:
      m_pixbuf_original = UiUtils::get_pixbuf_for_gda_value(m_original_data);
    }
    else
    {
      //Get an icon for the file type;
      icon = Gio::content_type_get_icon(mime_type);
    }

    if(m_pixbuf_original)
    {
      auto pixbuf_scaled = get_scaled_image();
      m_image->set(pixbuf_scaled);
    }
    else if(icon)
    {
      m_image->set(icon, Gtk::BuiltinIconSize::DIALOG);
    }
    else
    {
      m_image->set_from_icon_name("image-missing", Gtk::BuiltinIconSize::DIALOG);
    }
  }
}

Glib::RefPtr<Gdk::Pixbuf> ImageGlom::get_scaled_image()
{
  auto pixbuf = m_pixbuf_original;

  if(!pixbuf)
    return pixbuf;

  const auto allocation = m_image->get_allocation();
  const auto pixbuf_height = pixbuf->get_height();
  const auto pixbuf_width = pixbuf->get_width();

  const auto allocation_height = allocation.get_height();
  const auto allocation_width = allocation.get_width();

  //std::cout << "pixbuf_height=" << pixbuf_height << ", pixbuf_width=" << pixbuf_width << std::endl;
  //std::cout << "allocation_height=" << allocation.get_height() << ", allocation_width=" << allocation.get_width() << std::endl;

  if( (pixbuf_height > allocation_height) ||
      (pixbuf_width > allocation_width) )
  {
    if(true) //allocation_height > 10 || allocation_width > 10)
    {
      auto pixbuf_scaled = UiUtils::image_scale_keeping_ratio(pixbuf, allocation_height, allocation_width);

      //Don't set a new pixbuf if the dimensions have not changed:
      Glib::RefPtr<Gdk::Pixbuf> pixbuf_in_image;

      if(m_image->get_storage_type() == Gtk::Image::Type::PIXBUF) //Prevent warning.
        pixbuf_in_image = m_image->get_pixbuf();

      if( !pixbuf_in_image || !pixbuf_scaled || (pixbuf_in_image->get_height() != pixbuf_scaled->get_height()) || (pixbuf_in_image->get_width() != pixbuf_scaled->get_width()) )
      {
        /*
        std::cout << "get_scale(): returning scaled\n";
        if(pixbuf_scaled)
        {
          std::cout << "scaled height=" << pixbuf_scaled->get_height() << ", scaled width=" << pixbuf_scaled->get_width() << std::endl;
        }
        */

        return pixbuf_scaled;
      }
      else
      {
        //Return the existing one,
        //instead of a new one with the same contents,
        //so no unnecessary changes will be triggered.
        return pixbuf_in_image;
      }
    }
  }

  //std::cout << "get_scaled(): returning original\n";
  return pixbuf;
}

void ImageGlom::on_menupopup_activate_open_file()
{
  open_with();
}

void ImageGlom::on_menupopup_activate_open_file_with()
{
  auto pApp = get_appwindow();

  //Offer the user a choice of suitable applications:
  const auto mime_type = get_mime_type();
  if(mime_type.empty())
  {
    std::cerr << G_STRFUNC << ": mime_type is empty.\n";
  }

  Gtk::AppChooserDialog dialog(mime_type);
  if(pApp)
    dialog.set_transient_for(*pApp);

  if(dialog.run() != Gtk::ResponseType::OK)
    return;

  auto app_info = dialog.get_app_info();
  if(!app_info)
  {
    std::cerr << G_STRFUNC << ": app_info was null.\n";
  }

  open_with(app_info);
}

static void make_file_read_only(const Glib::ustring& uri)
{
  std::string filepath;

  try
  {
    filepath = Glib::filename_from_uri(uri);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << "Exception: " << ex.what() << std::endl;
    return;
  }

  if(filepath.empty())
  {
    std::cerr << G_STRFUNC << ": filepath is empty.\n";
  }

  const auto result = chmod(filepath.c_str(), S_IRUSR);
  if(result != 0)
  {
    std::cerr << G_STRFUNC << ": chmod() failed.\n";
  }

  //Setting the attribute via gio gives us this exception:
  //"Setting attribute access::can-write not supported"
  /*
  auto file = Gio::File::create_for_uri(uri);

  Glib::RefPtr<Gio::FileInfo> file_info;

  try
  {
    file_info = file->query_info(G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << ": query_info() failed: " << ex.what() << std::endl;
    return;
  }

  if(!file_info)
  {
    std::cerr << G_STRFUNC << ": : file_info is null\n";
    return;
  }

  const bool can_write =
    file_info->get_attribute_boolean(G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE);
  if(!can_write)
    return;

  file_info->set_attribute_boolean(G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE, false);

  try
  {
    file->set_attributes_from_info(file_info);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << ": set_attributes_from_info() failed: " << ex.what() << std::endl;
  }
  */
}

Glib::ustring ImageGlom::save_to_temp_file(bool show_progress)
{
  Glib::ustring uri = FileUtils::get_temp_file_uri("glom_image");
  if(uri.empty())
  {
    std::cerr << G_STRFUNC << ": : uri is empty.\n";
  }

  bool saved = false;
  if(show_progress)
    saved = save_file(uri);
  else
    saved = save_file_sync(uri);

  if(!saved)
  {
    uri = Glib::ustring();
    std::cerr << G_STRFUNC << ": save_file() failed.\n";
  }
  else
  {
    //Don't let people easily edit the saved file,
    //because they would lose data when it is automatically deleted later.
    //Also they might think that editing it will change it in the database.
    make_file_read_only(uri);
  }

  return uri;
}

void ImageGlom::open_with(const Glib::RefPtr<Gio::AppInfo>& app_info)
{
  const auto uri = save_to_temp_file();
  if(uri.empty())
    return;

  if(app_info)
  {
    app_info->launch_uri(uri); //TODO: Get a GdkAppLaunchContext?
  }
  else
  {
    //TODO: Avoid duplication in xsl_utils.cc, by moving this into a utility function:
#ifdef G_OS_WIN32
    // gtk_show_uri doesn't seem to work on Win32, at least not for local files
    // We use Windows API instead.
    // TODO: Check it again and file a bug if necessary.
    // TODO: and this might not be necessary with Gio::AppInfo::launch_default_for_uri().
    //   Previously we used gtk_show_uri().
    ShellExecute(0, "open", uri.c_str(), 0, 0, SW_SHOW);
#else
    Gio::AppInfo::launch_default_for_uri(uri);
#endif //G_OS_WIN32
  }
}


static void set_file_filter_images(Gtk::FileChooser& file_chooser)
{
  //Get image formats only:
  auto filter = Gtk::FileFilter::create();
  filter->set_name(_("Images"));
  filter->add_pixbuf_formats();
  file_chooser.add_filter(filter);

  //ev_document_factory_add_filters(GTK_WIDGET(file_chooser.gobj()), nullptr);

  //Make Images the currently-selected one:
  file_chooser.set_filter(filter);

  /*  ev_document_factory_add_filters() add this already:
  filter = Gtk::FileFilter::create();
  filter->set_name(_("All Files"));
  filter->add_pattern("*");
  file_chooser.add_filter(filter);
  */
}

void ImageGlom::on_menupopup_activate_save_file()
{
  auto pApp = get_appwindow();

  Gtk::FileChooserDialog dialog(_("Save Image"), Gtk::FileChooser::Action::SAVE);
  if(pApp)
    dialog.set_transient_for(*pApp);

  set_file_filter_images(dialog);

  dialog.add_button(_("_Cancel"), Gtk::ResponseType::CANCEL);
  dialog.add_button(_("_Save"), Gtk::ResponseType::OK);
  const auto response = dialog.run();
  dialog.hide();
  if(response != Gtk::ResponseType::OK)
    return;

  const auto uri = dialog.get_uri();
  if(uri.empty())
    return;

  save_file(uri);
}

bool ImageGlom::save_file_sync(const Glib::ustring& uri)
{
  //TODO: We should still do this asynchronously,
  //even when we don't use the dialog's run() to do that
  //because we don't want to offer feedback.
  //Ideally, EvView would just load from data anyway.

  const auto gda_binary = get_binary();
  if(!gda_binary)
  {
    std::cerr << G_STRFUNC << ": GdaBinary is null\n";
    return false;
  }

  const auto data = gda_binary_get_data(const_cast<GdaBinary*>(gda_binary));
  if(!data)
  {
    std::cerr << G_STRFUNC << ": GdaBinary::data is null\n";
    return false;
  }

  try
  {
    const auto filepath = Glib::filename_from_uri(uri);
    const auto datalen = gda_binary_get_size(const_cast<GdaBinary*>(gda_binary));
    Glib::file_set_contents(filepath, static_cast<const char*>(data), datalen);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << "Exception: " << ex.what() << std::endl;
    return false;
  }

  return true;
}

bool ImageGlom::save_file(const Glib::ustring& uri)
{
  DialogImageSaveProgress* dialog_save = nullptr;
  Utils::get_glade_widget_derived_with_warning(dialog_save);
  if(!dialog_save)
    return false;

  // Automatically delete the dialog when we no longer need it:
  std::shared_ptr<Gtk::Dialog> dialog_keeper(dialog_save);

  auto pApp = get_appwindow();
  if(pApp)
    dialog_save->set_transient_for(*pApp);

  const auto gda_binary = get_binary();
  if(!gda_binary)
    return false;

  dialog_save->set_image_data(gda_binary);
  dialog_save->save(uri);

  dialog_save->run();

  return true;
}

void ImageGlom::on_menupopup_activate_select_file()
{
  if(m_read_only)
    return;

  auto pApp = get_appwindow();

  Gtk::FileChooserDialog dialog(_("Choose Image"), Gtk::FileChooser::Action::OPEN);
  if(pApp)
    dialog.set_transient_for(*pApp);

  set_file_filter_images(dialog);

  dialog.add_button(_("_Cancel"), Gtk::ResponseType::CANCEL);
  dialog.add_button(_("Select"), Gtk::ResponseType::OK);
  int response = dialog.run();
  dialog.hide();

  if((response != Gtk::ResponseType::CANCEL) && (response != Gtk::ResponseType::DELETE_EVENT))
  {
    const auto uri = dialog.get_uri();
    if(!uri.empty())
    {
      DialogImageLoadProgress* dialog_progress = nullptr;
      Utils::get_glade_widget_derived_with_warning(dialog_progress);
      if(dialog_progress)
      {
        // Automatically delete the dialog when we no longer need it:
        std::shared_ptr<Gtk::Dialog> dialog_keeper(dialog_progress);

        if(pApp)
          dialog_progress->set_transient_for(*pApp);

        dialog_progress->load(uri);

        if(dialog_progress->run() == Gtk::ResponseType::ACCEPT)
        {
          // This takes ownership of the GdaBinary from the dialog:
          auto image_data = dialog_progress->get_image_data();

          clear_original_data();

          g_value_unset(m_original_data.gobj());
          g_value_init(m_original_data.gobj(), GDA_TYPE_BINARY);
          gda_value_take_binary(m_original_data.gobj(), image_data.get());

          show_image_data();
          signal_edited().emit();
        }
      }
    }
  }
}

void ImageGlom::clear_original_data()
{
  m_original_data = Gnome::Gda::Value();
}

void ImageGlom::on_clipboard_get(Gtk::SelectionData& selection_data, guint /* info */)
{
  //info is meant to indicate the target, but it seems to be always 0,
  //so we use the selection_data's target instead.

  const auto target = selection_data.get_target();

  const auto mime_type = get_mime_type();
  if(mime_type.empty())
  {
    std::cerr << G_STRFUNC << ": mime_type is empty.\n";
  }

  if(target == mime_type)
  {
    const auto gda_binary = get_binary();
    if(!gda_binary)
      return;

    const auto data = gda_binary_get_data(const_cast<GdaBinary*>(gda_binary));
    if(!data)
      return;

    const auto datalen = gda_binary_get_size(const_cast<GdaBinary*>(gda_binary));
    selection_data.set(mime_type, 8, static_cast<guchar*>(data), datalen);

    // This set() override uses an 8-bit text format for the data.
    //selection_data.set_pixbuf(m_pixbuf_clipboard);
  }
  else
  {
    std::cout << "ExampleWindow::on_clipboard_get(): Unexpected clipboard target format. expected: " << mime_type << std::endl;
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

  auto refClipboard = Gtk::Clipboard::get();

  //Targets:
  const auto mime_type = get_mime_type();
  if(mime_type.empty())
  {
    std::cerr << G_STRFUNC << ": mime_type is empty.\n";
  }

  std::vector<Gtk::TargetEntry> listTargets;
  listTargets.emplace_back( Gtk::TargetEntry(mime_type) );

  refClipboard->set( listTargets, sigc::mem_fun(*this, &ImageGlom::on_clipboard_get), sigc::mem_fun(*this, &ImageGlom::on_clipboard_clear) );
}

void ImageGlom::on_clipboard_received_image(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf)
{
  if(m_read_only)
    return;

  if(pixbuf)
  {
    clear_original_data();

    m_pixbuf_original = pixbuf;
    show_image_data();

    signal_edited().emit();
  }
}


void ImageGlom::on_menupopup_activate_paste()
{
  if(m_read_only)
    return;

  //Tell the clipboard to call our method when it is ready:
  auto refClipboard = Gtk::Clipboard::get();

  if(refClipboard)
    refClipboard->request_image( sigc::mem_fun(*this, &ImageGlom::on_clipboard_received_image) );
}

void ImageGlom::on_menupopup_activate_clear()
{
  if(m_read_only)
    return;

  clear_original_data();
  show_image_data();
  signal_edited().emit();
}

void ImageGlom::setup_menu_usermode()
{
  //Create the Gio::ActionGroup and associate it with this widget:
  m_action_group_user_mode_popup = Gio::SimpleActionGroup::create();

  m_action_open_file = m_action_group_user_mode_popup->add_action("open-file",
    sigc::mem_fun(*this, &ImageGlom::on_menupopup_activate_open_file) );

  m_action_open_file_with = m_action_group_user_mode_popup->add_action("open-fil-ewith",
    sigc::mem_fun(*this, &ImageGlom::on_menupopup_activate_open_file_with) );

  m_action_save_file = m_action_group_user_mode_popup->add_action("save-file",
    sigc::mem_fun(*this, &ImageGlom::on_menupopup_activate_save_file) );

  m_action_select_file = m_action_group_user_mode_popup->add_action("select-file",
    sigc::mem_fun(*this, &ImageGlom::on_menupopup_activate_select_file) );

  m_action_copy = m_action_group_user_mode_popup->add_action("copy",
    sigc::mem_fun(*this, &ImageGlom::on_menupopup_activate_copy) );

  m_action_paste = m_action_group_user_mode_popup->add_action("paste",
    sigc::mem_fun(*this, &ImageGlom::on_menupopup_activate_paste) );

  m_action_clear = m_action_group_user_mode_popup->add_action("clear",
    sigc::mem_fun(*this, &ImageGlom::on_menupopup_activate_clear) );

  insert_action_group("imagecontext", m_action_group_user_mode_popup);


  //Create the UI for the menu whose items will activate the actions,
  //when this UI (a GtkMenu) is added and shown:

  auto menu = Gio::Menu::create();
  menu->append(_("_Open File"), "context.open-file");
  menu->append(_("Open File With"), "context.open-file-with");
  menu->append(_("Select File"), "context.select-file");
  menu->append(_("_Copy"), "context.copy");
  menu->append(_("_Paste"), "context.paste");
  menu->append(_("_Clear"), "context.clear");

  m_menu_popup_user_mode = std::make_unique<Gtk::Menu>(menu);
  m_menu_popup_user_mode->attach_to_widget(*this);
}

void ImageGlom::do_choose_image()
{
  on_menupopup_activate_select_file();
}

void ImageGlom::set_read_only(bool read_only)
{
  m_read_only = read_only;
}

void ImageGlom::popup_menu(Gdk::EventButton& event)
{
  if(!m_menu_popup_user_mode)
  {
    std::cerr << G_STRFUNC << ": m_menu_popup_user_mode is null\n";
    return;
  }

  m_menu_popup_user_mode->popup_at_pointer(event);

  m_action_select_file->set_enabled();
}


} //namespace Glom
