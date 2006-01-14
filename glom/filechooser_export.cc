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
#include <gtkmm/stock.h>
#include <glibmm/i18n.h>

FileChooser_Export::FileChooser_Export()
: Gtk::FileChooserDialog(_("Export To File."), Gtk::FILE_CHOOSER_ACTION_SAVE),
  m_extra_widget(false, 6),
  m_button_format(_("Define Data _Format"), true /* use mnenomic */)
{
  add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  add_button(_("Export"), Gtk::RESPONSE_OK);

  m_extra_widget.pack_start(m_button_format, Gtk::PACK_SHRINK);
  m_button_format.show();
  set_extra_widget(m_extra_widget);
  m_extra_widget.show();
}

FileChooser_Export::~FileChooser_Export()
{
}


