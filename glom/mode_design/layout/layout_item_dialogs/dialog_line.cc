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


#include "dialog_line.h"
#include <libglom/data_structure/glomconversions.h>
#include <iostream>

//#include <libgnome/gnome-i18n.h>
#include <glibmm/i18n.h>

namespace Glom
{

const char* Dialog_Line::glade_id("dialog_line");
const bool Dialog_Line::glade_developer(true);

Dialog_Line::Dialog_Line(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Dialog(cobject),
  m_spinbutton_line_width(0),
  m_colorbutton(0)
{
  builder->get_widget("spinbutton_line_width",  m_spinbutton_line_width);
  builder->get_widget("colorbutton",  m_colorbutton);

  //on_foreach_connect(this);

  //Dialog_Properties::set_modified(false);

  show_all_children();
}

Dialog_Line::~Dialog_Line()
{
}

void Dialog_Line::set_line(const sharedptr<const LayoutItem_Line>& line)
{
  if(!line)
  {
    std::cerr << G_STRFUNC << ": line is null" << std::endl;
  }

  //set_blocked();

  m_line = glom_sharedptr_clone(line); //Remember it so we save any details that are not in our UI.

  m_spinbutton_line_width->set_value(line->get_line_width());
  
  const Gdk::RGBA color( line->get_line_color() );
  m_colorbutton->set_rgba(color);

  //set_blocked(false);

  //Dialog_Properties::set_modified(false);
}

sharedptr<LayoutItem_Line> Dialog_Line::get_line() const
{
  if(!m_line)
  {
    std::cerr << G_STRFUNC <<  ": m_line is null" << std::endl;
  }
  
  sharedptr<LayoutItem_Line> result = glom_sharedptr_clone(m_line); //Start with the old details, to preserve anything that is not in our UI.

  if(!result)
  {
    std::cerr << G_STRFUNC << ": : result is null" << std::endl;
    return result;
  }

  result->set_line_width( m_spinbutton_line_width->get_value() );
  result->set_line_color( m_colorbutton->get_rgba().to_string() );

  return result;
}

} //namespace Glom





