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

#include "comboentry.h"
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

#ifdef GLOM_ENABLE_MAEMO
#include <hildon/hildon-touch-selector-entry.h>
#endif

namespace Glom
{

namespace DataWidgetChildren
{

ComboEntry::ComboEntry()
: Gtk::ComboBox(true /* has_entry */),
  ComboChoicesWithTreeModel(),
  m_cell_second(0)
{
#ifndef GLOM_ENABLE_CLIENT_ONLY
  setup_menu();
#endif // !GLOM_ENABLE_CLIENT_ONLY

  init();
}

Gtk::Entry* ComboEntry::get_entry()
{
  #ifndef GLOM_ENABLE_MAEMO
  return Gtk::ComboBox::get_entry();
  #else
  return m_maemo_selector.get_entry();
  #endif
}

const Gtk::Entry* ComboEntry::get_entry() const
{
  #ifndef GLOM_ENABLE_MAEMO
  return Gtk::ComboBox::get_entry();
  #else
  return m_maemo_selector.get_entry();
  #endif
}

void ComboEntry::init()
{
#ifndef GLOM_ENABLE_MAEMO
  set_model(m_refModel);
  set_entry_text_column(m_Columns.m_col_first);
#else
  //Maemo:
  set_selector(m_maemo_selector);

  //We don't use append_text_column(), because we want to specify no expand.
  //Glib::RefPtr<Hildon::TouchSelectorColumn> column =
  //  m_maemo_selector.append_text_column(m_refModel);
  // Only in the latest hildonmm: Glib::RefPtr<Hildon::TouchSelectorColumn> column =
  //  m_maemo_selector.append_column(m_refModel);
  Glib::RefPtr<Hildon::TouchSelectorColumn> column = Glib::wrap(hildon_touch_selector_append_column(
    HILDON_TOUCH_SELECTOR(m_maemo_selector.gobj()), GTK_TREE_MODEL(Glib::unwrap(m_refModel)), 0, static_cast<char*>(0)), true);

  column->pack_start(m_Columns.m_col_first, false);
  //Only in the latest hildonmm: column->set_text_column(m_Columns.m_col_first);
  column->set_property("text_column", 0);

  //Only in the latest hildonmm: m_maemo_selector->set_text_column(m_Columns.m_col_first);
  m_maemo_selector.set_text_column(0);


  //m_maemo_selector.set_model(0, m_refModel);
  //m_maemo_selector.set_text_column(0);
#endif

  //We use connect(slot, false) to connect before the default signal handler, because the default signal handler prevents _further_ handling.

  Gtk::Entry* entry = get_entry();
  if(!entry)
  {
    std::cerr << G_STRFUNC << ": get_entry() returned null." << std::endl;
  } else
  {
#ifndef GLOM_ENABLE_CLIENT_ONLY
    signal_button_press_event().connect(sigc::mem_fun(*this, &ComboEntry::on_entry_button_press_event), false);
#endif // GLOM_ENABLE_CLIENT_ONLY

    entry->signal_focus_out_event().connect(sigc::mem_fun(*this, &ComboEntry::on_entry_focus_out_event), false);
    entry->signal_activate().connect(sigc::mem_fun(*this, &ComboEntry::on_entry_activate));
  }
}

ComboEntry::~ComboEntry()
{
}

void ComboEntry::set_choices_related(const Document* document, const sharedptr<const Relationship>& relationship, const Glib::ustring& field, const Glib::ustring& field_second, bool show_all)
{
  if(!m_cell_second && !field_second.empty())
  {
    #ifndef GLOM_ENABLE_MAEMO
    //We don't use this convenience method, because we want more control over the renderer.
    //and CellLayout gives no way to get the renderer back afterwards.
    //(well, maybe set_cell_data_func(), but that's a bit awkward.)
    //pack_start(m_Columns.m_col_second);

    m_cell_second = Gtk::manage(new Gtk::CellRendererText);
    m_cell_second->set_property("xalign", 0.0);

    //Use the renderer:
    pack_start(*m_cell_second);

    //Make the renderer render the column:
    add_attribute(m_cell_second->_property_renderable(), m_Columns.m_col_second);
    #else //GLOM_ENABLE_MAEMO
    column->pack_start(m_Columns.m_col_second, false);
    #endif //GLOM_ENABLE_MAEMO
  }

  ComboChoicesWithTreeModel::set_choices_related(document, relationship, field, field_second, show_all);
}

void ComboEntry::set_layout_item(const sharedptr<LayoutItem>& layout_item, const Glib::ustring& table_name)
{
  //Call base class:
  ComboChoicesWithTreeModel::set_layout_item(layout_item, table_name);

  if(!layout_item)
    return;

  //Horizontal Alignment:
  FieldFormatting::HorizontalAlignment alignment =
    FieldFormatting::HORIZONTAL_ALIGNMENT_LEFT;
  sharedptr<LayoutItem_Field> layout_field =
    sharedptr<LayoutItem_Field>::cast_dynamic(layout_item);
  if(layout_field)
    alignment = layout_field->get_formatting_used_horizontal_alignment();

  const float x_align = (alignment == FieldFormatting::HORIZONTAL_ALIGNMENT_LEFT ? 0.0 : 1.0);

  Gtk::Entry* entry = get_entry();
  if(!entry)
  {
    std::cerr << G_STRFUNC << ": get_entry() returned null." << std::endl;
  }
  else
    entry->set_alignment(x_align);
}

void ComboEntry::check_for_change()
{
  Gtk::Entry* entry = get_entry();
  if(!entry)
  {
    std::cerr << G_STRFUNC << ": get_entry() returned null." << std::endl;
    return;
  }

  if(!(entry->get_editable()))
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
    entry->set_text(m_old_text);
  }

  const Glib::ustring new_text = entry->get_text();
  if(new_text != m_old_text)
  {
    //Validate the input:
    bool success = false;

    sharedptr<const LayoutItem_Field> layout_item = sharedptr<const LayoutItem_Field>::cast_dynamic(get_layout_item());
    Gnome::Gda::Value value = Conversions::parse_value(layout_item->get_glom_type(), entry->get_text(), layout_item->get_formatting_used().m_numeric_format, success);

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

bool ComboEntry::on_entry_focus_out_event(GdkEventFocus* /* event */)
{
  //bool result = Gtk::ComboBox::on_focus_out_event(event);

  //The user has finished editing.
  check_for_change();

  //Call base class:
  return false;
}

void ComboEntry::on_entry_activate()
{
  //Call base class:
  //get_entry()->on_activate();

  //The user has finished editing.
  check_for_change();
}

void ComboEntry::on_entry_changed()
{
  //The text is being edited, but the user has not finished yet.

  //Call base class:
  //gtk_entry()->on_changed();
}


void ComboEntry::set_value(const Gnome::Gda::Value& value)
{
  sharedptr<const LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::cast_dynamic(get_layout_item());
  if(!layout_item)
    return;

  set_text(Conversions::get_text_for_gda_value(layout_item->get_glom_type(), value, layout_item->get_formatting_used().m_numeric_format));

  //Show a different color if the value is numeric, if that's specified:
  if(layout_item->get_glom_type() == Field::TYPE_NUMERIC)
  {
    Gtk::Entry* entry = get_entry();
    if(!entry)
      return;

    const Glib::ustring fg_color =
    layout_item->get_formatting_used().get_text_format_color_foreground_to_use(value);
    if(!fg_color.empty())
      entry->modify_text(Gtk::STATE_NORMAL, Gdk::Color(fg_color));
    else
      entry->modify_text(Gtk::STATE_NORMAL, Gdk::Color());
  }
}

void ComboEntry::set_text(const Glib::ustring& text)
{
  m_old_text = text;

  #if GLOM_ENABLE_MAEMO
  for(Gtk::TreeModel::iterator iter = m_refModel->children().begin(); iter != m_refModel->children().end(); ++iter)
  {
    const Glib::ustring& this_text = (*iter)[m_Columns.m_col_first];

    if(this_text == text)
    {
      set_selected(iter);
    }
  }
  #endif //GLOM_ENABLE_MAEMO

  //Call base class:
  Gtk::Entry* entry = get_entry();
  if(!entry)
  {
    std::cerr << G_STRFUNC << ": get_entry() returned null." << std::endl;
  }
  else
    entry->set_text(text);
}

Gnome::Gda::Value ComboEntry::get_value() const
{
  bool success = false;

  const Gtk::Entry* entry = get_entry();
  if(!entry)
  {
    std::cerr << G_STRFUNC << ": get_entry() returned null." << std::endl;
    return Gnome::Gda::Value();
  }

  sharedptr<const LayoutItem_Field> layout_item = sharedptr<const LayoutItem_Field>::cast_dynamic(get_layout_item());
  return Conversions::parse_value(layout_item->get_glom_type(), entry->get_text(), layout_item->get_formatting_used().m_numeric_format, success);
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
bool ComboEntry::on_entry_button_press_event(GdkEventButton *event)
{
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

  return false; //We did not handle this event.
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

Application* ComboEntry::get_application()
{
  Gtk::Container* pWindow = get_toplevel();
  //TODO: This only works when the child widget is already in its parent.

  return dynamic_cast<Application*>(pWindow);
}

#ifndef GLOM_ENABLE_MAEMO
void ComboEntry::on_changed()
#else
void ComboEntry::on_changed(int /* column */)
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

void ComboEntry::set_read_only(bool read_only)
{
  Gtk::Entry* entry = get_entry();
  if(entry)
    entry->set_editable(!read_only);
}

} //namespace DataWidetChildren
} //namespace Glom
