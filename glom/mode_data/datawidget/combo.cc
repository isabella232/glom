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

#include "combo.h"
#include <libglom/data_structure/glomconversions.h>
#include <gtkmm/messagedialog.h>
#include <glom/dialog_invalid_data.h>
#include <libglom/data_structure/glomconversions.h>
#include <glom/application.h>
#include <glibmm/i18n.h>
//#include <sstream> //For stringstream

#include <locale>     // for locale, time_put
#include <ctime>     // for struct tm
#include <iostream>   // for cout, endl

namespace Glom
{

namespace DataWidgetChildren
{

ComboGlom::ComboGlom()
: ComboChoicesWithTreeModel()
{
#ifndef GLOM_ENABLE_CLIENT_ONLY
  setup_menu();
#endif // !GLOM_ENABLE_CLIENT_ONLY

  init();
}

void ComboGlom::init()
{
  #ifdef GLOM_ENABLE_MAEMO
  //Maemo:
  set_selector(m_maemo_selector);
  m_maemo_selector.set_model(0, m_refModel);

  Glib::RefPtr<Hildon::TouchSelectorColumn> column =
    m_maemo_selector.append_text_column(m_refModel);
  column->set_property("text-column", 0); // TODO: Add a TextSelectorColumn::set_text_column() method?

  column->pack_start(m_Columns.m_col_first, false);
  #endif //GLOM_ENABLE_MAEMO

  //if(m_glom_type == Field::TYPE_NUMERIC)
   // get_entry()->set_alignment(1.0); //Align numbers to the right.
}

ComboGlom::~ComboGlom()
{
}

void ComboGlom::create_model(guint columns_count)
{
  //Create the model itself:
  ComboChoicesWithTreeModel::create_model(columns_count);

  //Show the model in the view:
  set_model(m_refModel);

  for(guint i = 0; i < columns_count; ++i)
  {
    Gtk::CellRendererText* cell = Gtk::manage(new Gtk::CellRendererText);
    cell->set_property("xalign", 0.0);

    //Use the renderer:
    pack_start(*cell);

    //Make the renderer render the column:
    add_attribute(*cell, "text", i);
  }
}

void ComboGlom::check_for_change()
{
  Glib::ustring new_text = get_text();
  if(new_text != m_old_text)
  {
    //Validate the input:
    bool success = false;

    sharedptr<const LayoutItem_Field> layout_item = sharedptr<const LayoutItem_Field>::cast_dynamic(get_layout_item());
    const Gnome::Gda::Value value = Conversions::parse_value(layout_item->get_glom_type(), new_text, layout_item->get_formatting_used().m_numeric_format, success);

    if(success)
    {
      //Actually show the canonical text:
      set_value(value);
      m_signal_edited.emit(); //The text was edited, so tell the client code.
    }
    else
    {
      //Tell the user and offer to revert or try again:
      bool revert = glom_show_dialog_invalid_data(layout_item->get_glom_type());
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
  sharedptr<const LayoutItem_Field> layout_item = sharedptr<const LayoutItem_Field>::cast_dynamic(get_layout_item());
  if(!layout_item)
    return;

  set_text(Conversions::get_text_for_gda_value(layout_item->get_glom_type(), value, layout_item->get_formatting_used().m_numeric_format));

  //Show a different color if the value is numeric, if that's specified:
  if(layout_item->get_glom_type() == Field::TYPE_NUMERIC)
  {
    std::vector<Gtk::CellRenderer*> cells = get_cells();
    if(cells.empty())
      return;

    Gtk::CellRendererText* cell = dynamic_cast<Gtk::CellRendererText*>(cells[0]);
    if(!cell)
      return;

    const Glib::ustring fg_color =
    layout_item->get_formatting_used().get_text_format_color_foreground_to_use(value);
    if(fg_color.empty())
    {
      //GtkComboBox doesn't interpret "" as an unset. TODO: Fix that?
      cell->property_foreground_set() = false;
    }
    else
      cell->property_foreground() = fg_color;
  }
}

void ComboGlom::set_text(const Glib::ustring& text)
{
  m_old_text = text;

  for(Gtk::TreeModel::iterator iter = m_refModel->children().begin(); iter != m_refModel->children().end(); ++iter)
  {
    const Gtk::TreeModel::Row row = *iter;
    Glib::ustring this_text;
    row.get_value(0, this_text);

    if(this_text == text)
    {
      #ifndef GLOM_ENABLE_MAEMO
      set_active(iter);
      #else
      set_selected(iter);
      #endif //GLOM_ENABLE_MAEMO
      return; //success
    }
  }

  //It's OK to pass "" to this method to unset any items:
  if(!text.empty())
  {
    g_warning("ComboGlom::set_text(): no item found for: %s", text.c_str());
  }

  //Not found, so mark it as blank:
  #ifndef GLOM_ENABLE_MAEMO
  unset_active();
  #else
  unselect();
  #endif
}

Gnome::Gda::Value ComboGlom::get_value() const
{
  sharedptr<const LayoutItem_Field> layout_item = sharedptr<const LayoutItem_Field>::cast_dynamic(get_layout_item());
  bool success = false;

  const Glib::ustring text = get_text();
  return Conversions::parse_value(layout_item->get_glom_type(), text, layout_item->get_formatting_used().m_numeric_format, success);
}

Glib::ustring ComboGlom::get_text() const
{
  //Get the active row:
  #ifndef GLOM_ENABLE_MAEMO
  Gtk::TreeModel::iterator iter = get_active();
  #else
  ComboGlom* unconst = const_cast<ComboGlom*>(this);
  Gtk::TreeModel::iterator iter = unconst->get_selected();
  #endif //GLOM_ENABLE_MAEMO

  if(iter)
  {
    const Gtk::TreeModel::Row row = *iter;
    Glib::ustring text;
    row.get_value(0, text);
    return text;
  }

  return Glib::ustring();
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
bool ComboGlom::on_button_press_event(GdkEventButton *event)
{
g_warning("ComboGlom::on_button_press_event()");

  //Enable/Disable items.
  //We did this earlier, but get_application is more likely to work now:
  Application* pApp = get_application();
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
      gdk_window_get_pointer( gtk_widget_get_window (Gtk::Widget::gobj()), 0, 0, &mods );
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
#endif // !GLOM_ENABLE_CLIENT_ONLY

Application* ComboGlom::get_application()
{
  Gtk::Container* pWindow = get_toplevel();
  //TODO: This only works when the child widget is already in its parent.

  return dynamic_cast<Application*>(pWindow);
}


#ifndef GLOM_ENABLE_MAEMO
void ComboGlom::on_changed()
#else
void ComboGlom::on_changed(int /* column */)
#endif
{
  //Call base class:
  Gtk::ComboBox::on_changed();

  //This signal is emitted for every key press, but sometimes it's just to say that the active item has changed to "no active item",
  //if the text is not in the dropdown list:
  #ifndef GLOM_ENABLE_MAEMO
  Gtk::TreeModel::iterator iter = get_active();
  #else
  Gtk::TreeModel::iterator iter = get_selected();
  #endif //GLOM_ENABLE_MAEMO

  if(iter)
  {
    //This is either a choice from the dropdown menu, or someone has typed in something that is in the drop-down menu.
    //TODO: If both ab, and abc, are in the menu, we are responding twice if the user types abc.
    check_for_change();
  }
  //Entry of text that is not in the menu will be handled by the ->get_entry() signal handlers._
}

void ComboGlom::set_read_only(bool /* read_only */)
{
  //TODO
}

} //namespace DataWidetChildren
} //namespace Glom
