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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#ifndef GLOM_UTILITY_WIDGETS_IMAGE_GLOM_H
#define GLOM_UTILITY_WIDGETS_IMAGE_GLOM_H

#include <gtkmm/eventbox.h>
#include <gtkmm/image.h>
#include <gtkmm/menu.h>
#include <libglom/data_structure/field.h>
#include "layoutwidgetfield.h"
#include <gtkmm/frame.h>
#include <gtkmm/builder.h>
#include <gtkmm/scrolledwindow.h>
#include <giomm/appinfo.h>
#include <giomm/simpleactiongroup.h>
#include <evince-view.h>

namespace Glom
{

class AppWindow;

class ImageGlom
: public Gtk::EventBox,
  public LayoutWidgetField
{
public:
  ImageGlom();
  explicit ImageGlom(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);


  virtual ~ImageGlom();

  void set_layout_item(const std::shared_ptr<LayoutItem>& layout_item, const Glib::ustring& table_name) override;

  void set_value(const Gnome::Gda::Value& value) override;
  Gnome::Gda::Value get_value() const override;

  //Optionally use this instead of set_value(), to avoid creating an unnecessary Value.
  //void set_pixbuf(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf);

  void do_choose_image();

  void set_read_only(bool read_only = true) override;

  void on_ev_job_finished(EvJob* job);

private:
  void init();
  void init_widgets(bool use_evince);
  void clear_image_from_widgets();
  void clear_original_data();

  void on_size_allocate(Gtk::Allocation& allocation) override;

  bool on_button_press_event(GdkEventButton *event) override;

  void on_menupopup_activate_open_file();
  void on_menupopup_activate_open_file_with();
  void on_menupopup_activate_save_file();
  void on_menupopup_activate_select_file();
  void on_menupopup_activate_copy();
  void on_menupopup_activate_paste();
  void on_menupopup_activate_clear();

  void on_clipboard_get(Gtk::SelectionData& selection_data, guint /* info */);
  void on_clipboard_clear();
  void on_clipboard_received_image(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf);

  AppWindow* get_appwindow() const override;

  void setup_menu_usermode();
  void show_image_data();

  void popup_menu(guint button, guint32 activate_time);

  const GdaBinary* get_binary() const;

  //Get a pixbuf scaled down to the current size allocation:
  Glib::RefPtr<Gdk::Pixbuf> get_scaled_image();

  Glib::ustring save_to_temp_file(bool show_progress = true);
  bool save_file(const Glib::ustring& uri);
  bool save_file_sync(const Glib::ustring& uri);
  void open_with(const Glib::RefPtr<Gio::AppInfo>& app_info =  Glib::RefPtr<Gio::AppInfo>());

  Glib::ustring get_mime_type() const;
  static void fill_evince_supported_mime_types();
  static void fill_gdkpixbuf_supported_mime_types();

  mutable Gnome::Gda::Value m_original_data; // Original file data (mutable so that we can create it in get_value() if it does not exist yet)

  Gtk::Frame m_frame;

  //For anything supported by Evince:
  std::unique_ptr<Gtk::ScrolledWindow> m_ev_scrolled_window;
  EvView* m_ev_view;
  EvDocumentModel* m_ev_document_model;

  //For anything supported by GdkPixbuf,
  //or for representative thumbnails and icons:
  std::unique_ptr<Gtk::Image> m_image;
  Glib::RefPtr<Gdk::Pixbuf> m_pixbuf_original; //Only stored temporarily, because it could be big.
  Glib::RefPtr<Gdk::Pixbuf> m_pixbuf_clipboard; //When copy is used, store it here until it is pasted.


  std::unique_ptr<Gtk::Menu> m_menu_popup_user_mode;

  //TODO: Use just the Gio::ActionGroup type when it derives from Gio::ActionMap.
  Glib::RefPtr<Gio::SimpleActionGroup> m_action_group_user_mode_popup;

  //We use Gio::SimpleAction rather than Gio::Action
  //because Gio::Action has no way to enable/disable it.
  Glib::RefPtr<Gio::SimpleAction> m_action_open_file, m_action_open_file_with,
    m_action_save_file, m_action_select_file, m_action_copy, m_action_paste, m_action_clear;

  bool m_read_only;

  typedef std::vector<Glib::ustring> type_vec_ustrings;
  static type_vec_ustrings m_evince_supported_mime_types;
  static type_vec_ustrings m_gdkpixbuf_supported_mime_types;
};

} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_COMBOENTRY_GLOM_H

