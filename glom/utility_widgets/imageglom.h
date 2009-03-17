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

#ifndef GLOM_UTILITY_WIDGETS_IMAGE_GLOM_H
#define GLOM_UTILITY_WIDGETS_IMAGE_GLOM_H

#include <gtkmm.h>
#include <libglom/data_structure/field.h>
#include "layoutwidgetfield.h"
#include <gtkmm/builder.h>

namespace Glom
{

class App_Glom;

class ImageGlom
: public Gtk::EventBox,
  public LayoutWidgetField
{
public:
  ImageGlom();
  explicit ImageGlom(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);


  virtual ~ImageGlom();

  virtual void set_value(const Gnome::Gda::Value& value);
  virtual Gnome::Gda::Value get_value() const;
  virtual bool get_has_original_data() const;

  //Optionally use this instead of set_value(), to avoid creating an unnecessary Value.
  void set_pixbuf(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf);

  void do_choose_image();

  void set_read_only(bool read_only = true);

  static Glib::RefPtr<Gdk::Pixbuf> scale_keeping_ratio(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf, int target_height, int target_width);

private:
  void init();

  // Note that these are normal signal handlers when glibmm was compiled
  // without default signal handler API.
  virtual bool on_expose_event(GdkEventExpose* event);

  virtual bool on_button_press_event(GdkEventButton *event);

  void on_menupopup_activate_select_file();
  void on_menupopup_activate_copy();
  void on_menupopup_activate_paste();
  void on_menupopup_activate_clear();

  void on_clipboard_get(Gtk::SelectionData& selection_data, guint /* info */);
  void on_clipboard_clear();
  void on_clipboard_received_image(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf);

  virtual App_Glom* get_application();

  void setup_menu_usermode();
  void scale();

 
  Gtk::Image m_image;
  Gtk::Frame m_frame;
  Glib::RefPtr<Gdk::Pixbuf> m_pixbuf_original; //Only stored temporarily, because it could be big.
  Glib::RefPtr<Gdk::Pixbuf> m_pixbuf_clipboard; //When copy is used, store it here until it is pasted.

  Gtk::Menu* m_pMenuPopup_UserMode;
  Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup_UserModePopup;
  Glib::RefPtr<Gtk::UIManager> m_refUIManager_UserModePopup;
  Glib::RefPtr<Gtk::Action> m_refActionSelectFile, m_refActionCopy, m_refActionPaste, m_refActionClear;

  bool m_read_only;
};

} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_COMBOENTRY_GLOM_H

