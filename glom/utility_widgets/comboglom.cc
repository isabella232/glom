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

#include "comboglom.h"
#include "../data_structure/glomconversions.h"
#include <gtkmm/messagedialog.h>
#include "../dialog_invalid_data.h"
#include "../data_structure/glomconversions.h"
#include "../application.h"
#include <glibmm/i18n.h>
//#include <sstream> //For stringstream

#include <locale>     // for locale, time_put
#include <ctime>     // for struct tm
#include <iostream>   // for cout, endl

ComboGlom::ComboGlom(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& /* refGlade */)
: Gtk::ComboBox(cobject)
{
  setup_menu();
  init();
}

ComboGlom::ComboGlom()
: ComboGlomChoicesBase()
{
  setup_menu();
  init();
}

ComboGlom::ComboGlom(const LayoutItem_Field& field_second)
: ComboGlomChoicesBase(field_second)
{
  setup_menu();
  init();
}

void ComboGlom::init()
{
  set_model(m_refModel);

  pack_start(m_Columns.m_col_first);

  if(m_with_second)
  {
    //We don't use this convenience method, because we want more control over the renderer.
    //and CellLayout gives no way to get the renderer back afterwards.
    //(well, maybe set_cell_data_func(), but that's a bit awkward.)
    //pack_start(m_Columns.m_col_second);

    Gtk::CellRenderer* cell_second = Gtk::manage(new Gtk::CellRendererText);
    cell_second->property_xalign() = 0.0;

    //Use the renderer:
    pack_start(*cell_second);

    //Make the renderer render the column:
    add_attribute(cell_second->_property_renderable(), m_Columns.m_col_second);
  }

  //if(m_glom_type == Field::TYPE_NUMERIC)
   // get_entry()->set_alignment(1.0); //Align numbers to the right.
}

ComboGlom::~ComboGlom()
{
}

void ComboGlom::check_for_change()
{
  Glib::ustring new_text = get_text();
  if(new_text != m_old_text)
  {
    //Validate the input:
    bool success = false;

    const LayoutItem_Field* layout_item = dynamic_cast<const LayoutItem_Field*>(get_layout_item());
    Gnome::Gda::Value value = GlomConversions::parse_value(layout_item->m_field.get_glom_type(), new_text, layout_item->m_numeric_format, success);

    if(success)
    {
      //Actually show the canonical text:
      set_value(value);
      m_signal_edited.emit(); //The text was edited, so tell the client code.
    }
    else
    {
      //Tell the user and offer to revert or try again:
      bool revert = glom_show_dialog_invalid_data(layout_item->m_field.get_glom_type());
      if(revert)
      {
        set_text(m_old_text);
      }
      else
        grab_focus(); //Force the user back into the same field, so that the field can be checked again and eventually corrected or reverted.
    }
  }
}

void ComboGlom::set_value(const Gnome::Gda::Value& value)
{
  const LayoutItem_Field* layout_item = dynamic_cast<const LayoutItem_Field*>(get_layout_item());
  if(layout_item)
    set_text(GlomConversions::get_text_for_gda_value(layout_item->m_field.get_glom_type(), value, layout_item->m_numeric_format));
}

void ComboGlom::set_text(const Glib::ustring& text)
{
  m_old_text = text;

  for(Gtk::TreeModel::iterator iter = m_refModel->children().begin(); iter != m_refModel->children().end(); ++iter)
  {
    Glib::ustring this_text = (*iter)[m_Columns.m_col_first];

    if(this_text == text)
    {
      set_active(iter);
      return; //success
    }
  }

  g_warning("ComboGlom::set_text(): no item found for: %s", text.c_str());

  //Not found, so mark it as blank:
  unset_active();
}

Gnome::Gda::Value ComboGlom::get_value() const
{
  const LayoutItem_Field* layout_item = dynamic_cast<const LayoutItem_Field*>(get_layout_item());
  bool success = false;

  const Glib::ustring text = get_text();
  return GlomConversions::parse_value(layout_item->m_field.get_glom_type(), text, layout_item->m_numeric_format, success);
}

Glib::ustring ComboGlom::get_text() const
{
  //Get the active row:
  Gtk::TreeModel::iterator active_row = get_active();
  if(active_row)
  {
    Gtk::TreeModel::Row row = *active_row;
    return row[m_Columns.m_col_first];
  }

  return Glib::ustring();
}

bool ComboGlom::on_button_press_event(GdkEventButton *event)
{
g_warning("ComboGlom::on_button_press_event()");

  //Enable/Disable items.
  //We did this earlier, but get_application is more likely to work now:
  App_Glom* pApp = get_application();
  if(pApp)
  {
    pApp->add_developer_action(m_refContextLayout); //So that it can be disabled when not in developer mode.
    pApp->add_developer_action(m_refContextAddField);
    pApp->add_developer_action(m_refContextAddRelatedRecords);
    pApp->add_developer_action(m_refContextAddGroup);

    pApp->update_userlevel_ui(); //Update our action's sensitivity.

    //Only show this popup in developer mode, so operators still see the default GtkEntry context menu.
    //TODO: It would be better to add it somehow to the standard context menu.
    if(pApp->get_userlevel() == AppState::USERLEVEL_DEVELOPER)
    {
      GdkModifierType mods;
      gdk_window_get_pointer( Gtk::Widget::gobj()->window, 0, 0, &mods );
      if(mods & GDK_BUTTON3_MASK)
      {
        //Give user choices of actions on this item:
        m_pMenuPopup->popup(event->button, event->time);
        return true; //We handled this event.
      }
    }

  }

  return Gtk::ComboBox::on_button_press_event(event);
}

App_Glom* ComboGlom::get_application()
{
  Gtk::Container* pWindow = get_toplevel();
  //TODO: This only works when the child widget is already in its parent.

  return dynamic_cast<App_Glom*>(pWindow);
}

void ComboGlom::on_changed()
{
  //Call base class:
  Gtk::ComboBox::on_changed();

  //This signal is emitted for every key press, but sometimes it's just to say that the active item has changed to "no active item",
  //if the text is not in the dropdown list:
  Gtk::TreeModel::iterator iter = get_active();
  if(iter)
  {
    //This is either a choice from the dropdown menu, or someone has typed in something that is in the drop-down menu.
    //TODO: If both ab, and abc, are in the menu, we are responding twice if the user types abc.
    check_for_change();
  }
  //Entry of text that is not in the menu will be handled by the ->get_entry() signal handlers._
}
