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

#ifndef GLOM_MODE_DESIGN_DIALOG_IMAGEOBJECT_H
#define GLOM_MODE_DESIGN_DIALOG_IMAGEOBJECT_H

#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/builder.h>
#include <libglom/data_structure/layout/layoutitem_image.h>
#include <glom/base_db.h>
#include <glom/utility_widgets/imageglom.h>

namespace Glom
{

class Dialog_ImageObject
 : public Gtk::Dialog,
   public Base_DB //Give this class access to the current document, and to some utility methods.

{
public:
  static const char* glade_id;
  static const bool glade_developer;

  Dialog_ImageObject(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~Dialog_ImageObject();

  void set_imageobject(const std::shared_ptr<const LayoutItem_Image>& imageobject, const Glib::ustring& table_name, bool show_title = true);
  std::shared_ptr<LayoutItem_Image> get_imageobject() const;

private:
  void on_button_choose();

  Gtk::Box* m_box_title;
  Gtk::Entry* m_entry_title;
  ImageGlom* m_image;
  Gtk::Button* m_button_choose_image;

  std::shared_ptr<LayoutItem_Image> m_imageobject;
  Glib::ustring m_table_name;
};

} //namespace Glom

#endif // GLOM_MODE_DESIGN_DIALOG_IMAGEOBJECT_H
