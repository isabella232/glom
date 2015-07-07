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


#include "dialog_script_library.h"
#include <glom/python_embed/glom_python.h>
#include <libglom/data_structure/glomconversions.h>
#include <glom/glade_utils.h>
#include <glom/utils_ui.h>
#include <glom/mode_design/script_library/dialog_new_script.h>
#include <gtksourceviewmm/languagemanager.h>
#include <glom/appwindow.h>


//#include <libgnome/gnome-i18n.h>
#include <glibmm/i18n.h>

namespace Glom
{

const char* Dialog_ScriptLibrary::glade_id("dialog_script_library");
const bool Dialog_ScriptLibrary::glade_developer(true);

Dialog_ScriptLibrary::Dialog_ScriptLibrary(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Dialog(cobject)
{
  //Get child widgets:
  builder->get_widget_derived("combobox_name", m_combobox_name);
  builder->get_widget("textview_script",  m_text_view);
  builder->get_widget("button_check",  m_button_check);
  builder->get_widget("button_add",  m_button_add);
  builder->get_widget("button_remove",  m_button_remove);

  //Connect signals:
  m_button_check->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_ScriptLibrary::on_button_check) );
  m_button_add->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_ScriptLibrary::on_button_add) );
  m_button_remove->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_ScriptLibrary::on_button_remove) );
  m_combobox_name->signal_changed().connect( sigc::mem_fun(*this, &Dialog_ScriptLibrary::on_combo_name_changed) );
  //connect_each_widget(this);

  //Dialog_Properties::set_modified(false);

  //Set the SourceView to do syntax highlighting for Python:
  Glib::RefPtr<Gsv::LanguageManager> languages_manager = Gsv::LanguageManager::get_default();

  Glib::RefPtr<Gsv::Language> language = languages_manager->get_language("python"); //This is the GtkSourceView language ID.
  if(language)
  {
     //Create a new buffer and set it, instead of getting the default buffer, in case libglade has tried to set it, using the wrong buffer type:
     Glib::RefPtr<Gsv::Buffer> buffer = Gsv::Buffer::create(language);
     buffer->set_highlight_syntax();
     m_text_view->set_buffer(buffer);
  }

  show_all_children();
}

Dialog_ScriptLibrary::~Dialog_ScriptLibrary()
{
}

void Dialog_ScriptLibrary::on_button_check()
{
  const auto script = m_text_view->get_buffer()->get_text();

  //TODO: glom_execute_python_function_implementation(script);
}

void Dialog_ScriptLibrary::on_button_add()
{
  //Save any outstanding changes:
  save_current_script();

  Document* document = get_document();
  if(!document)
    return;

  Dialog_NewScript* dialog = nullptr;
  Utils::get_glade_widget_derived_with_warning(dialog);
  if(!dialog) //Unlikely and it already warns on stderr.
    return;
    
  dialog->set_transient_for(*this);
  const auto dialog_response = Glom::UiUtils::dialog_run_with_help(dialog);
  dialog->hide();
  if(dialog_response != Gtk::RESPONSE_OK)
    return;

  const auto name = dialog->m_entry_name->get_text();
  delete dialog;

  if(name.empty())
    return; //TODO Warn and retry.

  if(!(document->get_library_module(name).empty())) //Don't add one that already exists.
    return; //TODO Warn and retry.

  document->set_library_module(name, Glib::ustring());

  save_current_script();

  load_from_document(); //Fill the combo.
  m_combobox_name->set_active_text(name);
}

void Dialog_ScriptLibrary::on_button_remove()
{
  Document* document = get_document();
  if(!document)
    return;

  Gtk::MessageDialog dialog(UiUtils::bold_message(_("Remove library script")), true, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE );
  dialog.set_secondary_text(_("Do you really want to delete this script? This data can not be recovered"));
  dialog.add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
  dialog.add_button(_("_Remove"), Gtk::RESPONSE_OK);
  dialog.set_transient_for(*this);
  const auto dialog_response = dialog.run();
  dialog.hide();
  if(dialog_response != Gtk::RESPONSE_OK)
    return;

  const auto name = m_combobox_name->get_active_text();
  if(!name.empty())
  {
    document->remove_library_module(name); //TODO: Show warning dialog.
    load_from_document(); //Fill the combo.
  }
}


void Dialog_ScriptLibrary::on_combo_name_changed()
{
  //Save the old script:
  save_current_script();

  //Lod the new script:
  load_current_script();
}

void Dialog_ScriptLibrary::load_current_script()
{
  Document* document = get_document();
  if(!document)
    return;

  //Get the selected module name:
  const auto name = m_combobox_name->get_active_text();

  //Get the module's script text:
  Glib::ustring script;
  if(!name.empty())
  {
    script = document->get_library_module(name);
  }
 
  //Show the script text:
  m_text_view->get_buffer()->set_text(script);

  m_current_name = name;
}

void Dialog_ScriptLibrary::save_current_script()
{
  Document* document = get_document();
  if(!document)
    return;

  //Get the current module name:
  const Glib::ustring name = m_current_name; /* We might be saving the current script in response to a change in the combo. */

  //Set the module's script text:
  if(!name.empty())
  {
    //Get the script text:
    const auto script = m_text_view->get_buffer()->get_text();

    document->set_library_module(name, script);
  }
}

void Dialog_ScriptLibrary::load_from_document()
{
  const Document* document = get_document();
  if(!document)
    return;

  m_combobox_name->remove_all();
 
  for(const auto& name : document->get_library_module_names())
  {
    m_combobox_name->append(name);
  }

  //Show the current script, or the first one, if there is one:
  if(m_current_name.empty())
  {
    m_combobox_name->set_first_active();
    m_current_name = m_combobox_name->get_active_text();
  }
  else
  {
    m_combobox_name->set_active_text(m_current_name);
  }

  //TODO: Maybe already done by the signal handler:
  if(m_current_name.empty())
    m_text_view->get_buffer()->set_text(Glib::ustring());
  else
    load_current_script();
}

void Dialog_ScriptLibrary::save_to_document()
{
  Document* document = get_document();
  if(!document)
    return;

  save_current_script();  
}



} //namespace Glom




