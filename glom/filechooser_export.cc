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

#include "filechooser_export.h"
#include "mode_data/dialog_layout_export.h"
#include <glom/libglom/utils.h>
#include <gtkmm/stock.h>
#include <glibmm/i18n.h>

namespace Glom
{

FileChooser_Export::FileChooser_Export()
: Gtk::FileChooserDialog(_("Export To File."), Gtk::FILE_CHOOSER_ACTION_SAVE),
  m_extra_widget(false, Utils::DEFAULT_SPACING_SMALL),
  m_button_format(_("Define Data _Format"), true /* use mnenomic */),
  m_pDialogLayout(0),
  m_document(0)
{
  add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  add_button(_("Export"), Gtk::RESPONSE_OK);

  m_extra_widget.pack_start(m_button_format, Gtk::PACK_SHRINK);
  m_button_format.signal_clicked().connect(
    sigc::mem_fun(*this, &FileChooser_Export::on_button_define_layout) );
  m_button_format.show();

  set_extra_widget(m_extra_widget);
  m_extra_widget.show();

  //TODO: Use a generic layout dialog?
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom_developer.glade", "window_data_layout_export");
#else
  std::auto_ptr<Gnome::Glade::XmlError> error;
  Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "window_data_layout_export", "", error);
  // Ignore error, refXml is checked below
#endif

  if(refXml)
  {
    Dialog_Layout_Export* dialog = 0;
    refXml->get_widget_derived("window_data_layout_export", dialog);
    if(dialog)
    {
      m_pDialogLayout = dialog;
      //add_view(m_pDialogLayout); //Give it access to the document.
      m_pDialogLayout->signal_hide().connect( sigc::mem_fun(*this, &FileChooser_Export::on_dialog_layout_hide) );
    }
  }
}

FileChooser_Export::~FileChooser_Export()
{
  delete m_pDialogLayout;
}

void FileChooser_Export::set_export_layout(const Document_Glom::type_mapLayoutGroupSequence& layout_groups, const Glib::ustring& table_name, Document_Glom* document)
{
  m_layout_groups = layout_groups;
  m_table_name = table_name;
  m_document = document;
  if(!m_document)
    std::cerr << "FileChooser_Export::set_export_layout() document is NULL." << std::endl;
}

void FileChooser_Export::on_button_define_layout()
{
  if(m_pDialogLayout)
  {
    m_pDialogLayout->set_layout_groups(m_layout_groups, m_document, m_table_name); //TODO: Use m_TableFields?
    m_pDialogLayout->set_transient_for(*this);
    set_modal(false);
    m_pDialogLayout->set_modal();
    m_pDialogLayout->show();
  }
}

void FileChooser_Export::on_dialog_layout_hide()
{
  m_pDialogLayout->get_layout_groups(m_layout_groups);
}

void FileChooser_Export::get_layout_groups(Document_Glom::type_mapLayoutGroupSequence& layout_groups) const
{
  layout_groups = m_layout_groups; //TODO_Performance: Avoid copying so much.
}

} //namespace Glom


