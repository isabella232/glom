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

#ifndef GLOM_FILECHOOSER_EXPORT_H
#define GLOM_FILECHOOSER_EXPORT_H

#include <libglom/document/document.h>
#include <gtkmm/filechooserdialog.h>

namespace Glom
{

class Dialog_Layout_Export;

class FileChooser_Export :
  public Gtk::FileChooserDialog
{
public: 
  FileChooser_Export();
  virtual ~FileChooser_Export();

  void set_export_layout(const Document::type_list_const_layout_groups& layout_groups, const Glib::ustring& table_name, const std::shared_ptr<Document>& document);

  void get_layout_groups(Document::type_list_const_layout_groups& layout_groups) const;

private:

#ifndef GLOM_ENABLE_CLIENT_ONLY
  void on_button_define_layout();
  void on_dialog_layout_hide();
#endif

  //Member widgets:
  Gtk::Box m_extra_widget;

#ifndef GLOM_ENABLE_CLIENT_ONLY
  Gtk::Button m_button_format;
  Dialog_Layout_Export* m_pDialogLayout;
#endif //GLOM_ENABLE_CLIENT_ONLY

  Glib::ustring m_table_name;

  Document::type_list_const_layout_groups m_layout_groups;
  std::shared_ptr<Document> m_document;
};

} //namespace Glom

#endif // GLOM_FILESCHOOSER_EXPORT_H
