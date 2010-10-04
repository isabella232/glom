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
#include <glom/mode_data/datawidget/cellcreation.h>
#include <glom/mode_data/datawidget/treemodel_db_withextratext.h>
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
: ComboChoicesWithTreeModel()
{
#ifndef GLOM_ENABLE_CLIENT_ONLY
  setup_menu();
#endif // !GLOM_ENABLE_CLIENT_ONLY

  init();
}

Gtk::Entry* ComboEntry::get_entry()
{
  #ifndef GLOM_ENABLE_MAEMO
  return Gtk::ComboBoxEntry::get_entry();
  #else
  return m_maemo_selector.get_entry();
  #endif
}

const Gtk::Entry* ComboEntry::get_entry() const
{
  #ifndef GLOM_ENABLE_MAEMO
  return Gtk::ComboBoxEntry::get_entry();
  #else
  return m_maemo_selector.get_entry();
  #endif
}

void ComboEntry::init()
{
  //We use connect(slot, false) to connect before the default signal handler, because the default signal handler prevents _further_ handling.
  Gtk::Entry* entry = get_entry();
  entry->signal_button_press_event().connect(sigc::mem_fun(*this, &ComboEntry::on_entry_button_press_event), false);
  entry->signal_focus_out_event().connect(sigc::mem_fun(*this, &ComboEntry::on_entry_focus_out_event), false);
  entry->signal_activate().connect(sigc::mem_fun(*this, &ComboEntry::on_entry_activate));
}

ComboEntry::~ComboEntry()
{
}


void ComboEntry::set_choices_fixed(const FieldFormatting::type_list_values& list_values)
{
  ComboChoicesWithTreeModel::set_choices_fixed(list_values);

  //Show model in the view:
  Glib::RefPtr<Gtk::TreeModel> model = get_choices_model();
  if(!model)
  {
    std::cerr << G_STRFUNC << ": model is null." << std::endl;
    return;
  }

  set_model(model);
  set_text_column(0);

  const guint columns_count = model->get_n_columns();
  for(guint i = 0; i < columns_count; ++i)
  {
    Gtk::CellRendererText* cell = 0;
    if(i == 0)
    {
      //Get the default column, created by set_text_column():
      cell = dynamic_cast<Gtk::CellRendererText*>(get_first_cell());

      //Unpack and repack it with expand=false instead of expand=true:
      //We don't expand the first column, so we can align the other columns.
      cell->reference();
      clear();
      pack_start(*cell, false);
      cell->unreference();
    }
    else
    {
      //Create the cell:
      cell = Gtk::manage(new Gtk::CellRendererText);
      pack_start(*cell, true);
    }

    //Make the renderer render the column:
    add_attribute(*cell, "text", i);

    cell->property_xalign() = 0.0f;
  }
}

void ComboEntry::set_choices_related(const Document* document, const sharedptr<const LayoutItem_Field>& layout_field, const Gnome::Gda::Value& foreign_key_value)
{
  ComboChoicesWithTreeModel::set_choices_related(document, layout_field, foreign_key_value);

  Glib::RefPtr<Gtk::TreeModel> model = get_choices_model();
  if(!model)
  {
    std::cerr << G_STRFUNC << ": model is null." << std::endl;
    return;
  }

  //Show the model in the view:
  set_model(model);
  //clear() breaks GtkComboBoxEntry. TODO: Fix the C code? clear();
  
  //The DB model has a special virtual text column,
  //and the simple model just has text in all columns:
  Glib::RefPtr<DbTreeModelWithExtraText> model_db = 
    Glib::RefPtr<DbTreeModelWithExtraText>::cast_dynamic(model);
  if(model_db)
  {
    const int text_col = model_db->get_text_column();
    //const GType debug_type = model_db->get_column_type(text_col);
    //std::cout << "DEBUG: text_col=" << text_col << ", debug_type=" << g_type_name(debug_type) << std::endl;
    set_text_column(text_col);
  }
  else
  {
    std::cerr << G_STRFUNC << ": The model is not a DbTreeModelWithExtraText." << std::endl;
    return;
  }
  
  //const guint columns_count = model->get_n_columns();
  guint i = 0;
  for(type_vec_const_layout_items::const_iterator iter = m_db_layout_items.begin(); iter != m_db_layout_items.end(); ++iter)
  {
    const sharedptr<const LayoutItem> layout_item = *iter;
    Gtk::CellRenderer* cell = 0;

    if(i == 0)
    {
      //Get the default column, created by set_text_column():
      cell = get_first_cell();
      if(!cell)
        std::cerr << G_STRFUNC << ": get_first_cell() returned null." << std::endl;
      else
      {
        //Unpack and repack it with expand=false instead of expand=true:
        //We don't expand the first column, so we can align the other columns.
        cell->reference();
        clear();
        pack_start(*cell, false);
        cell->unreference();
      }
    }
    else
    {
      //Create the cell:
      cell = create_cell(layout_item, m_table_name, document, get_fixed_cell_height(*this));
      pack_start(*cell, true);
      
      cell_connect_cell_data_func(this, cell, i);
    }
    
    ++i;
  }
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
  get_entry()->set_alignment(x_align);
}

void ComboEntry::check_for_change()
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

  const Glib::ustring new_text = get_entry()->get_text();
  if(new_text != m_old_text)
  {
    //Validate the input:
    bool success = false;

    sharedptr<const LayoutItem_Field> layout_item = sharedptr<const LayoutItem_Field>::cast_dynamic(get_layout_item());
    const Gnome::Gda::Value value = Conversions::parse_value(layout_item->get_glom_type(), get_entry()->get_text(), layout_item->get_formatting_used().m_numeric_format, success);

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
  //bool result = Gtk::ComboBoxEntry::on_focus_out_event(event);

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
  get_entry()->set_text(text);
}

Gnome::Gda::Value ComboEntry::get_value() const
{
  bool success = false;

  sharedptr<const LayoutItem_Field> layout_item = sharedptr<const LayoutItem_Field>::cast_dynamic(get_layout_item());
  return Conversions::parse_value(layout_item->get_glom_type(), get_entry()->get_text(), layout_item->get_formatting_used().m_numeric_format, success);
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
  Gtk::ComboBoxEntry::on_changed();

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
