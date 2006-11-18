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


#include "dialog_script_library.h"
#include "../python_embed/glom_python.h"
#include <glom/libglom/data_structure/glomconversions.h>
#include <gtksourceviewmm/sourcelanguagesmanager.h>


//#include <libgnome/gnome-i18n.h>
#include <glibmm/i18n.h>

namespace Glom
{

Dialog_ScriptLibrary::Dialog_ScriptLibrary(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::Dialog(cobject)
{
  //Get child widgets:
  refGlade->get_widget("comboboxentry_name", m_comboboxentry_name);
  refGlade->get_widget("textview_script",  m_text_view);
  refGlade->get_widget("button_check",  m_button_check);

  //Setup the combo model:
  m_combo_model = Gtk::ListStore::create(m_combo_model_columns);
  m_comboboxentry_name->set_model(m_combo_model);
  m_comboboxentry_name->set_text_column(m_combo_model_columns.m_name);

  //Connect signals:
  m_button_check->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_ScriptLibrary::on_button_check) );
  m_comboboxentry_name->signal_changed().connect( sigc::mem_fun(*this, &&Dialog_ScriptLibrary::on_comboentry_name_changed) );
  //on_foreach_connect(*this);

  //Dialog_Properties::set_modified(false);

  //Set the SourceView to do syntax highlighting for Python:
  //TODO: Shouldn't this be a singleton?
  Glib::RefPtr<gtksourceview::SourceLanguagesManager> languages_manager = gtksourceview::SourceLanguagesManager::create();

  Glib::RefPtr<gtksourceview::SourceLanguage> language = languages_manager->get_language_from_mime_type("application/x-python");
  if(language)
  {
     //Create a new buffer and set it, instead of getting the default buffer, in case libglade has tried to set it, using the wrong buffer type:
     Glib::RefPtr<gtksourceview::SourceBuffer> buffer = gtksourceview::SourceBuffer::create(language);
     buffer->set_highlight();
     m_text_view->set_buffer(buffer);
  }

  show_all_children();
}

Dialog_ScriptLibrary::~Dialog_ScriptLibrary()
{
}

void Dialog_ScriptLibrary::on_button_check()
{
  const Glib::ustring script = m_text_view->get_buffer()->get_text();

  //TODO: glom_execute_python_function_implementation(script);
}

void Dialog_ScriptLibrary::on_comboentry_name_changed()
{
  save_current_script();
}

void Dialog_ScriptLibrary::load_current_script()
{
  Document_Glom* document = get_document();
  if(!document)
    return;

  //Get the selected module name:
  Gtk::TreeModel::itertator iter = m_comboboxentry_name->get_active();
  Gtk::TreeModel::Row row = *iter;
  const Glib::ustring name = iter[m_combo_model_columns.m_name];

  //Get the module's script text:
  Glib::ustring script;
  if(!name.empty())
  {
    script = document->get_library_module(name);
  }
 
  //Show the script text:
  m_text_view->get_buffer()->set_text(script);
}

void Dialog_ScriptLibrary::save_current_script()
{
  Document_Glom* document = get_document();
  if(!document)
    return;

  //Get the current module name:
  const Glib::ustring name = m_current_name; /* We might be saving the current script in response to a change in the combo. */

  //Set the module's script text:
  if(!name.empty())
  {
    //Get the script text:
    const Glib::ustring script = m_text_view->get_buffer()->get_text();

    document->set_library_module(name, script);
  }
}

void Dialog_ScriptLibrary::load_from_document()
{
  Document_Glom* document = get_document();
  if(!document)
    return;

  const std::vector<Glib::ustring> module_names = document->get_library_module_names();
  m_combo_model->clear();
  for(std::vector<Glib::ustring>::const_iterator iter = module_name.begin(); iter != module_names.end(); ++iter)
  {
    const Glib::ustring name = *iter;
    //const Glib::ustring script = document->get_library_module(*iter);
    Gtk::TreeModel::itertator iter = m_combo_model->append();
    Gtk::TreeModel::Row row = *iter;
    row[m_combo_model_columns.m_name] = name;
  }  
}

void Dialog_ScriptLibrary::save_to_document()
{
  Document_Glom* document = get_document();
  if(!document)
    return;

  const std::vector<Glib::ustring> module_names = document->get_library_module_names();
  
}



} //namespace Glom




