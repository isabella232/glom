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

#include "combo_textglade.h"
#include <gtk/gtkcombobox.h>

Combo_TextGlade::Combo_TextGlade(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& /* refGlade */)
: Gtk::ComboBox(cobject)
{
  m_model = Gtk::ListStore::create(m_text_columns);
  set_model(m_model);
  pack_start(m_text_columns.m_column);

  set_row_separator_func( sigc::mem_fun(*this, &Combo_TextGlade::on_row_separator) );
}


Combo_TextGlade::~Combo_TextGlade()
{

}

void Combo_TextGlade::append_separator()
{
  if(m_model)
  {
    Gtk::TreeModel::iterator iter = m_model->append();
    Gtk::TreeModel::Row row = *iter;
    row[m_text_columns.m_separator] = true;
  }
}

void Combo_TextGlade::append_text(const Glib::ustring& text)
{
  if(m_model)
  {
    Gtk::TreeModel::iterator iter = m_model->append();
    Gtk::TreeModel::Row row = *iter;
    row[m_text_columns.m_column] = text;
    row[m_text_columns.m_separator] = false;
  }
}

void Combo_TextGlade::prepend_text(const Glib::ustring& text)
{
  if(m_model)
  {
    Gtk::TreeModel::iterator iter = m_model->prepend();
    Gtk::TreeModel::Row row = *iter;
    row[m_text_columns.m_column] = text;
    row[m_text_columns.m_separator] = false;
  }
}

Glib::ustring Combo_TextGlade::get_active_text() const
{
  //We can not use gtk_combobox_get_active_text() here, because that can only be used if gtk_combo_box_new_text() has been used.

  Glib::ustring result;

  //Get the active row:
  Gtk::TreeModel::iterator active_row = get_active();
  if(active_row)
  {
    Gtk::TreeModel::Row row = *active_row;
    result = row[m_text_columns.m_column];
  }

  return result;
}

void Combo_TextGlade::clear_text()
{
  m_model->clear();
}

void Combo_TextGlade::set_active_text(const Glib::ustring& text)
{
  for(Gtk::TreeModel::iterator iter = m_model->children().begin(); iter != m_model->children().end(); ++iter)
  {
    Glib::ustring this_text = (*iter)[m_text_columns.m_column];

    if(this_text == text)
    {
      set_active(iter);
      return; //success
    }
  }

  //Not found, so mark it as blank:
  unset_active();
}

bool Combo_TextGlade::on_row_separator(const Glib::RefPtr<Gtk::TreeModel>& /* model */, const Gtk::TreeModel::iterator& iter)
{
  Gtk::TreeModel::Row row = *iter;
  return row[m_text_columns.m_separator];
}

///This ensures that something is selected:
void Combo_TextGlade::set_first_active()
{
  Gtk::TreeModel::iterator iter = m_model->children().begin();
  set_active(iter);
}




