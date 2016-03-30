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

#include "filechooser_export.h"
#include <glom/mode_design/layout/dialog_layout_export.h>
#include <glom/utils_ui.h>
#include <glom/glade_utils.h>
#include <libglom/utils.h>
#include <glibmm/i18n.h>

namespace Glom
{

FileChooser_Export::FileChooser_Export()
: Gtk::FileChooserDialog(_("Export to File"), Gtk::FILE_CHOOSER_ACTION_SAVE),
  m_extra_widget(Gtk::ORIENTATION_HORIZONTAL, Utils::to_utype(UiUtils::DefaultSpacings::SMALL)),
#ifndef GLOM_ENABLE_CLIENT_ONLY
  m_button_format(_("Define Data _Format"), true /* use mnenomic */),
  m_dialog_layout(nullptr),
#endif //GLOM_ENABLE_CLIENT_ONLY
  m_document(nullptr)
{
  add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
  add_button(_("_Export"), Gtk::RESPONSE_OK);

  m_extra_widget.pack_start(m_button_format, Gtk::PACK_SHRINK);

#ifndef GLOM_ENABLE_CLIENT_ONLY
  m_button_format.signal_clicked().connect(
    sigc::mem_fun(*this, &FileChooser_Export::on_button_define_layout) );
  m_button_format.show();
#endif

  set_extra_widget(m_extra_widget);
  m_extra_widget.show();

#ifndef GLOM_ENABLE_CLIENT_ONLY
  //TODO: Use a generic layout dialog?
  Dialog_Layout_Export* dialog = nullptr;
  Utils::get_glade_widget_derived_with_warning(dialog);
  if(!dialog)
    return;
  
  m_dialog_layout = dialog;
  //add_view(m_dialog_layout); //Give it access to the document.
  m_dialog_layout->signal_hide().connect( sigc::mem_fun(*this, &FileChooser_Export::on_dialog_layout_hide) );
#endif //GLOM_ENABLE_CLIENT_ONLY
}

FileChooser_Export::~FileChooser_Export()
{
#ifndef GLOM_ENABLE_CLIENT_ONLY
  delete m_dialog_layout;
  m_dialog_layout = nullptr;
#endif //GLOM_ENABLE_CLIENT_ONLY
}

void FileChooser_Export::set_export_layout(const Document::type_list_const_layout_groups& layout_groups, const Glib::ustring& table_name, const std::shared_ptr<Document>& document)
{
  m_layout_groups = layout_groups;
  m_table_name = table_name;
  m_document = document;
  if(!m_document)
    std::cerr << G_STRFUNC << ": FileChooser_Export::set_export_layout() document is NULL.\n";
}

//We only allow a full export in client-only mode, 
//to avoid building a large part of the layout definition code.
#ifndef GLOM_ENABLE_CLIENT_ONLY
void FileChooser_Export::on_button_define_layout()
{
  if(!m_dialog_layout)
    return;

  m_dialog_layout->set_layout_groups(m_layout_groups, m_document, m_table_name); //TODO: Use m_TableFields?
  m_dialog_layout->set_transient_for(*this);
  set_modal(false);
  m_dialog_layout->set_modal();
  m_dialog_layout->show();
}

void FileChooser_Export::on_dialog_layout_hide()
{
  if(m_dialog_layout)
    m_dialog_layout->get_layout_groups(m_layout_groups);
}
#endif //GLOM_ENABLE_CLIENT_ONLY

void FileChooser_Export::get_layout_groups(Document::type_list_const_layout_groups& layout_groups) const
{
  layout_groups = m_layout_groups;
}

} //namespace Glom


