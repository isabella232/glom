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

#include "glom/utility_widgets/adddel/adddel.h"
#include "dialog_properties.h"
#include <gtkmm/togglebutton.h>
#include <gtkmm/textview.h>
#include <iostream>

namespace Glom
{

Dialog_Properties::Dialog_Properties(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Window(cobject),
  m_block(false),
  m_modified(false)
{
  builder->get_widget("button_cancel", m_pButton_Cancel);
  builder->get_widget("button_save", m_pButton_Save);

  //In general, we don't want to allow changes to windows underneath while editing properties.
  //Also, if we don't set this then seconday windows (from a modal dialog) will be on top but unusable.
  set_modal();

  //Connect signal handlers:
  m_pButton_Cancel->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Properties::on_button_cancel) );
  m_pButton_Save->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Properties::on_button_save) );

  show_all_children();
}

Dialog_Properties::~Dialog_Properties()
{
}

Dialog_Properties::type_signal_apply Dialog_Properties::signal_apply()
{
  return m_signal_apply;
}

void Dialog_Properties::on_button_save()
{
  signal_apply().emit();
}

void Dialog_Properties::on_button_cancel()
{
  hide();
}

void Dialog_Properties::add(Gtk::Widget& /*widget */)
{
  //TODO: Remove this method?
  //Connect the widgets signals:
  //on_foreach_connect(widget);
}

void Dialog_Properties::widget_connect_changed_signal(Gtk::Widget* widget)
{
  if(!widget)
  {
    std::cerr << G_STRFUNC << ": widget is null." << std::endl;
    return;
  }

  Gtk::ComboBox* pCombo = dynamic_cast<Gtk::ComboBox*>(widget);
  if(pCombo) //If it is actually a Combo:
  {
    pCombo->signal_changed().connect(sigc::mem_fun(*this, &Dialog_Properties::on_anything_changed));
  }
  else
  {
    Gtk::Entry* pEntry = dynamic_cast<Gtk::Entry*>(widget);
    if(pEntry) //If it is actually an Entry:
    {
      pEntry->signal_changed().connect(sigc::mem_fun(*this, &Dialog_Properties::on_anything_changed));
    }
    else
    {
      Gtk::ToggleButton* pToggleButton = dynamic_cast<Gtk::ToggleButton*>(widget);
      if(pToggleButton)
      {
        pToggleButton->signal_toggled().connect( sigc::mem_fun(*this, &Dialog_Properties::on_anything_changed) );
      }
      else
      {
        Gtk::TextView* pTextView = dynamic_cast<Gtk::TextView*>(widget);
        if(pTextView)
        {
          pTextView->get_buffer()->signal_changed().connect( sigc::mem_fun(*this, &Dialog_Properties::on_anything_changed) );
        }
        else
        {
          AddDel* pAddDel = dynamic_cast<AddDel*>(widget);
          if(pAddDel)
          {
            pAddDel->signal_user_changed().connect( sigc::mem_fun(*this, &Dialog_Properties::on_adddel_user_changed) );
          }
        }
      }
    }
  }
}

void Dialog_Properties::on_adddel_user_changed(const Gtk::TreeModel::iterator& /* iter */, guint /* col */)
{
  on_anything_changed();
}

void Dialog_Properties::on_anything_changed()
{
  if(!m_block)
  {
    //Something (e.g. an edit or a combo) changed.
    //So we need to activate the [Save] button:

    enforce_constraints();
    set_modified();
  }
}

void Dialog_Properties::on_foreach_connect(Gtk::Widget* widget)
{
  if(!widget)
  {
    std::cerr << G_STRFUNC << ": widget is null." << std::endl;
    return;
  }

  widget_connect_changed_signal(widget); //Connect the appropriate signal

  //Recurse through children:
  Gtk::Container* pContainer = dynamic_cast<Gtk::Container*>(widget);
  if(pContainer)
  {
    pContainer->foreach( sigc::mem_fun(*this, &Dialog_Properties::on_foreach_connect)); //recursive
  }
}

void Dialog_Properties::set_blocked(bool val)
{
  m_block = val;
}

void Dialog_Properties::set_modified(bool modified)
{
  m_modified = true;

  m_pButton_Save->set_sensitive(modified);
  m_pButton_Cancel->set_sensitive(true);
}

void Dialog_Properties::enforce_constraints()
{

}

} //namespace Glom
