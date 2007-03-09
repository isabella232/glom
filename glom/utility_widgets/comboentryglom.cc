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

#include "comboentryglom.h"
#include <glom/libglom/data_structure/glomconversions.h>
#include <gtkmm/messagedialog.h>
#include "../dialog_invalid_data.h"
#include <glom/libglom/data_structure/glomconversions.h>
#include "../application.h"
#include <glibmm/i18n.h>
//#include <sstream> //For stringstream

#include <locale>     // for locale, time_put
#include <ctime>     // for struct tm
#include <iostream>   // for cout, endl

namespace Glom
{

ComboEntryGlom::ComboEntryGlom(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& /* refGlade */)
: Gtk::ComboBoxEntry(cobject)
{
  setup_menu();
  init();
}

ComboEntryGlom::ComboEntryGlom()
: ComboGlomChoicesBase()
{
  setup_menu();
  init();
}

ComboEntryGlom::ComboEntryGlom(const sharedptr<LayoutItem_Field>& field_second)
: ComboGlomChoicesBase(field_second)
{
  setup_menu();
  init();
}

void ComboEntryGlom::init()
{
  set_model(m_refModel);

  set_text_column(m_Columns.m_col_first);

  //We use connect(slot, false) to connect before the default signal handler, because the default signal handler prevents _further_ handling.
  get_entry()->signal_button_press_event().connect(sigc::mem_fun(*this, &ComboEntryGlom::on_entry_button_press_event), false);
  get_entry()->signal_focus_out_event().connect(sigc::mem_fun(*this, &ComboEntryGlom::on_entry_focus_out_event), false);
  get_entry()->signal_activate().connect(sigc::mem_fun(*this, &ComboEntryGlom::on_entry_activate));

  if(m_with_second)
  {
    //We don't use this convenience method, because we want more control over the renderer.
    //and CellLayout gives no way to get the renderer back afterwards.
    //(well, maybe set_cell_data_func(), but that's a bit awkward.)
    //pack_start(m_Columns.m_col_second);

    Gtk::CellRenderer* cell_second = Gtk::manage(new Gtk::CellRendererText);
    cell_second->property_xalign() = 0.0f;

    //Use the renderer:
    pack_start(*cell_second);

    //Make the renderer render the column:
    add_attribute(cell_second->_property_renderable(), m_Columns.m_col_second);
  }
}

ComboEntryGlom::~ComboEntryGlom()
{
}

void ComboEntryGlom::set_layout_item(const sharedptr<LayoutItem>& layout_item, const Glib::ustring& table_name)
{
  //Call base class:
  ComboGlomChoicesBase::set_layout_item(layout_item, table_name);

  //Right-align numbers:
  sharedptr<const LayoutItem_Field> layout_field = sharedptr<const LayoutItem_Field>::cast_dynamic(get_layout_item());
  if(layout_field && layout_field->get_glom_type() == Field::TYPE_NUMERIC)
      get_entry()->set_alignment(1.0); //Align numbers to the right.
}

void ComboEntryGlom::check_for_change()
{
  if(!(get_entry()->get_editable()))
  {
    //Don't allow editing via the menu either, if the Entry is non-editable.

    //Give the user some kind of warning.
    //We could just remove the menu (by using a normal Entry for read-only fields with choices), 
    //but I think that the choice is a useful recognisable visual hint about the field,
    //which shouldn't change sometimes just because the field is read-only.
    Gtk::Window* top_level_window = get_application();
    if(top_level_window)
      Frame_Glom::show_ok_dialog(_("Read-only field."), _("This field may not be edited here."), *top_level_window, Gtk::MESSAGE_INFO);

    //Change the entry back to the old value:
    get_entry()->set_text(m_old_text);
  }

  Glib::ustring new_text = get_entry()->get_text();
  if(new_text != m_old_text)
  {
    //Validate the input:
    bool success = false;

    sharedptr<const LayoutItem_Field> layout_item = sharedptr<const LayoutItem_Field>::cast_dynamic(get_layout_item());
    Gnome::Gda::Value value = Conversions::parse_value(layout_item->get_glom_type(), get_entry()->get_text(), layout_item->get_formatting_used().m_numeric_format, success);

    if(success)
    {
      //Actually show the canonical text:
      set_value(value);
      m_signal_edited.emit(); //The text was edited, so tell the client code.
    }
    else
    {
      //Tell the user and offer to revert or try again:
      const bool revert = glom_show_dialog_invalid_data(layout_item->get_glom_type());
      if(revert)
      {
        set_text(m_old_text);
      }
      else
        grab_focus(); //Force the user back into the same field, so that the field can be checked again and eventually corrected or reverted.
    }
  }
}

bool ComboEntryGlom::on_entry_focus_out_event(GdkEventFocus* /* event */)
{
  //bool result = Gtk::ComboBoxEntry::on_focus_out_event(event);

  //The user has finished editing.
  check_for_change();

  //Call base class:
  return false;
}

void ComboEntryGlom::on_entry_activate()
{ 
  //Call base class:
  //get_entry()->on_activate();

  //The user has finished editing.
  check_for_change();
}

void ComboEntryGlom::on_entry_changed()
{
  //The text is being edited, but the user has not finished yet.

  //Call base class:
  //gtk_entry()->on_changed();
}


void ComboEntryGlom::set_value(const Gnome::Gda::Value& value)
{
  sharedptr<const LayoutItem_Field> layout_item = sharedptr<const LayoutItem_Field>::cast_dynamic(get_layout_item());
  if(layout_item)
  {
    set_text(Conversions::get_text_for_gda_value(layout_item->get_glom_type(), value, layout_item->get_formatting_used().m_numeric_format));
  }
}

void ComboEntryGlom::set_text(const Glib::ustring& text)
{
  m_old_text = text;

  //Call base class:
  get_entry()->set_text(text);
}

Gnome::Gda::Value ComboEntryGlom::get_value() const
{
  bool success = false;

  sharedptr<const LayoutItem_Field> layout_item = sharedptr<const LayoutItem_Field>::cast_dynamic(get_layout_item());
  return Conversions::parse_value(layout_item->get_glom_type(), get_entry()->get_text(), layout_item->get_formatting_used().m_numeric_format, success);
}

bool ComboEntryGlom::on_entry_button_press_event(GdkEventButton *event)
{
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

  return false; //We did not handle this event.
}

App_Glom* ComboEntryGlom::get_application()
{
  Gtk::Container* pWindow = get_toplevel();
  //TODO: This only works when the child widget is already in its parent.

  return dynamic_cast<App_Glom*>(pWindow);
}

void ComboEntryGlom::on_changed()
{
  //Call base class:
  Gtk::ComboBoxEntry::on_changed();

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

void ComboEntryGlom::set_read_only(bool read_only)
{
  Gtk::Entry* entry = get_entry();
  if(entry)
    entry->set_editable(!read_only);
}


} //namespace Glom
