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

#include <glom/mode_design/layout/dialog_layout_list.h>
#include <glom/mode_design/layout/dialog_choose_field.h>
#include <glom/mode_design/layout/layout_item_dialogs/dialog_field_layout.h>
#include <glom/frame_glom.h>
#include <libglom/utils.h> //For bold_message()).

//#include <libgnome/gnome-i18n.h>
#include <glibmm/i18n.h>

namespace Glom
{

Dialog_Layout_List::Dialog_Layout_List(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Dialog_Layout_Details(cobject, builder)
{
  //These do not make sense in a list:
  m_button_add_notebook->hide();
  m_button_add_related->hide();
  m_button_add_related_calendar->hide();
  m_button_add_group->hide();

  //We don't want this part of the dialog:
  //(We share one glade definition for several dialogs.)
  Gtk::Frame* box_calendar = 0;
  builder->get_widget("frame_calendar", box_calendar); 
  box_calendar->hide();

  //We don't use this column:
  if(m_treeview_column_group_columns)
    m_treeview_column_group_columns->set_visible(false);

  //We do use this column:
  if(m_treeview_column_column_width)
    m_treeview_column_column_width->set_visible();
}

Dialog_Layout_List::~Dialog_Layout_List()
{
}


} //namespace Glom
