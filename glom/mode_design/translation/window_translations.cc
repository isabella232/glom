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

#include "window_translations.h"
#include "combobox_locale.h"
#include "dialog_identify_original.h"
#include "dialog_copy_translation.h"
#include <glom/utils_ui.h> //For bold_message()).
#include <libglom/utils.h>
#include <libglom/translations_po.h>
#include <glom/glade_utils.h>
#include <glom/appwindow.h>
#include <gtkmm/filefilter.h>
#include <gtkmm/filechooserdialog.h>

#include <glibmm/i18n.h>
#include <string.h> // for memset

#include <sstream>

namespace Glom
{

const char* Window_Translations::glade_id("window_translations");
const bool Window_Translations::glade_developer(true);

Window_Translations::Window_Translations(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Window(cobject),
  m_treeview(nullptr),
  m_button_identify(nullptr),
  m_combo_target_locale(nullptr),
  m_label_source_locale(nullptr),
  m_button_ok(nullptr),
  m_button_cancel(nullptr),
  m_button_import(nullptr),
  m_button_export(nullptr),
  m_treeview_modified(false)
{
  builder->get_widget("label_source_locale", m_label_source_locale);

  builder->get_widget("treeview", m_treeview);
  if(m_treeview)
  {
    m_model = Gtk::ListStore::create(m_columns);
    m_treeview->set_model(m_model);

    // Append the View columns:
    auto column_original = Gtk::manage( new Gtk::TreeView::Column(_("Original")) );
    m_treeview->append_column(*column_original);

    auto renderer_name = Gtk::manage(new Gtk::CellRendererText);
    column_original->pack_start(*renderer_name);
    column_original->set_cell_data_func(*renderer_name, sigc::mem_fun(*this, &Window_Translations::on_cell_data_original));

    const auto col = m_treeview->append_column_editable(_("Translation"), m_columns.m_col_translation);
    auto renderer = dynamic_cast<Gtk::CellRendererText*>(m_treeview->get_column_cell_renderer(col - 1));
    if(renderer)
      renderer->signal_edited().connect(sigc::mem_fun(*this, &Window_Translations::on_treeview_edited));

    //This is at the end, because it can contain a long description of the item's context.
    auto column_item_typename = Gtk::manage( new Gtk::TreeView::Column(_("Item")) );
    m_treeview->append_column(*column_item_typename);

    auto renderer_item_typename = Gtk::manage(new Gtk::CellRendererText);
    column_item_typename->pack_start(*renderer_item_typename);
    column_item_typename->set_cell_data_func(*renderer_item_typename, sigc::mem_fun(*this, &Window_Translations::on_cell_data_item_itemhint));
  }

  builder->get_widget("button_identify", m_button_identify);
  if(m_button_identify)
  {
    m_button_identify->signal_clicked().connect( sigc::mem_fun(*this, &Window_Translations::on_button_identify) );
  }

  builder->get_widget_derived("combobox_target_locale", m_combo_target_locale);
  if(m_combo_target_locale)
  {
    m_combo_target_locale->signal_changed().connect(sigc::mem_fun(*this, &Window_Translations::on_combo_target_locale_changed));
  }

  builder->get_widget("button_ok", m_button_ok);
  if(m_button_ok)
  {
    m_button_ok->signal_clicked().connect( sigc::mem_fun(*this, &Window_Translations::on_button_ok) );
  }

  builder->get_widget("button_cancel", m_button_cancel);
  if(m_button_cancel)
  {
    m_button_cancel->signal_clicked().connect( sigc::mem_fun(*this, &Window_Translations::on_button_cancel) );
  }

  builder->get_widget("button_copy_translation", m_button_copy_translation);
  if(m_button_copy_translation)
  {
    m_button_copy_translation->signal_clicked().connect( sigc::mem_fun(*this, &Window_Translations::on_button_copy_translation) );
  }

  builder->get_widget("button_import", m_button_import);
  if(m_button_import)
  {
    m_button_import->signal_clicked().connect( sigc::mem_fun(*this, &Window_Translations::on_button_import) );
  }

  builder->get_widget("button_export", m_button_export);
  if(m_button_export)
  {
    m_button_export->signal_clicked().connect( sigc::mem_fun(*this, &Window_Translations::on_button_export) );
  }

  show_all_children();

  //Start with the currently-used/tested translation, if appropriate:
  if(AppWindow::get_current_locale_not_original())
  {
    m_translation_locale = AppWindow::get_current_locale();
    m_combo_target_locale->set_selected_locale(m_translation_locale);
    //The translations will be shown in the treeview when load_from_document() is called.
  }
  else
  {
    //Start with _some_ locale, rather than no locale:
    m_combo_target_locale->set_active(0);
  }
}

Window_Translations::~Window_Translations()
{
}


void Window_Translations::enable_buttons()
{

}

void Window_Translations::on_button_identify()
{
  Dialog_IdentifyOriginal* dialog = nullptr;
  Utils::get_glade_widget_derived_with_warning(dialog);
  if(!dialog) //Unlikely and it already warns on stderr.
    return;

  add_view(dialog);
  dialog->load_from_document(); //Doesn't seem to happen otherwise.
  dialog->set_transient_for(*this);
  const auto response = Glom::UiUtils::dialog_run_with_help(dialog);
  dialog->hide();

  if(response == Gtk::RESPONSE_OK)
  {
    get_document()->set_translation_original_locale(dialog->get_locale());

    //Save and update:
    on_combo_target_locale_changed();
  }

  remove_view(dialog);
  delete dialog;
}


void Window_Translations::on_cell_data_original(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter)
{
  //Set the view's cell properties depending on the model's data:
  auto renderer_text = dynamic_cast<Gtk::CellRendererText*>(renderer);
  if(renderer_text)
  {
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;

      Glib::ustring text;
      std::shared_ptr<TranslatableItem> item = row[m_columns.m_col_item];
      if(item)
        text = item->get_title_original();

      //Use the name if there is no title:
      if(text.empty())
        text = item->get_name(); 

      //TODO: Mark non-English originals.
      renderer_text->property_text() = text;
      renderer_text->property_editable() = false; //Names can never be edited.
    }
  }
}

void Window_Translations::on_cell_data_item_itemhint(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter)
{
  //Set the view's cell properties depending on the model's data:
  auto renderer_text = dynamic_cast<Gtk::CellRendererText*>(renderer);
  if(!renderer_text)
    return;

  if(!iter)
    return;

  Gtk::TreeModel::Row row = *iter;

  Glib::ustring item_type_name;
  std::shared_ptr<TranslatableItem> item = row[m_columns.m_col_item];
  const Glib::ustring hint = row[m_columns.m_col_hint];

  renderer_text->property_text() = get_po_context_for_item(item, hint);
  renderer_text->property_editable() = false; //Names can never be edited.
}

void Window_Translations::load_from_document()
{
  m_model->clear(); //Remove all rows.

  auto document = get_document();
  if(!document)
    return;

  //std::cout << "document->get_translation_original_locale()=" << document->get_translation_original_locale() << std::endl;
  Glib::ustring original_locale_name = IsoCodes::get_locale_name(document->get_translation_original_locale());
  if(original_locale_name.empty())
    original_locale_name = _("Unknown");
  m_label_source_locale->set_text(original_locale_name);

  for(const auto& the_pair : document->get_translatable_items())
  {
    const auto& item = the_pair.first;
    if(!item)
      continue;

    if(item->get_title_original().empty())
      continue;
      
    auto iterTree = m_model->append();
    Gtk::TreeModel::Row row = *iterTree;

    row[m_columns.m_col_item] = item;
    row[m_columns.m_col_translation] = item->get_title_translation(m_translation_locale, false);
    row[m_columns.m_col_hint] = the_pair.second;
  }

  m_treeview_modified = false;
}

void Window_Translations::save_to_document()
{
  if(!m_treeview_modified || m_translation_locale.empty())
    return;

  //Look at every item in the treeview and apply its translation:
  for(const auto& row : m_model->children())
  {
    //We have stored a std::shared_ptr to the original item, so we can just change it directly:
    std::shared_ptr<TranslatableItem> item = row[m_columns.m_col_item];
    if(item)
    {
      const Glib::ustring translation = row[m_columns.m_col_translation];
      item->set_title(translation, m_translation_locale);
    }
  }

  m_treeview_modified = false;
  set_modified(); //Save to the document.
}

void Window_Translations::on_button_cancel()
{
  hide();
}

void Window_Translations::on_button_ok()
{
  save_to_document();
  hide();
}

void Window_Translations::on_button_copy_translation()
{
  Dialog_CopyTranslation* dialog = nullptr;
  Utils::get_glade_widget_derived_with_warning(dialog);
  if(!dialog) //Unlikely and it already warns on stderr.
    return;

  dialog->set_transient_for(*this);
  const auto response = Glom::UiUtils::dialog_run_with_help(dialog);
  dialog->hide();

  if(response == Gtk::RESPONSE_OK)
  {
    const auto copy_source_locale = dialog->get_locale();
    if(!copy_source_locale.empty())
    {
      //Save and update:
      on_combo_target_locale_changed();

      for(const auto& row : m_model->children())
      {
        std::shared_ptr<TranslatableItem> item = row[m_columns.m_col_item];
        if(item)
        {
          //Copy the translation from the chosen locale to the current locale:
          const auto translation = item->get_title_translation(copy_source_locale);
          row[m_columns.m_col_translation] = translation;
        }
      }

      //Save and update:
      m_treeview_modified = true;
      save_to_document();
    }
  }

  delete dialog;
}

void Window_Translations::on_combo_target_locale_changed()
{
  save_to_document();

  m_translation_locale = m_combo_target_locale->get_selected_locale();
  load_from_document();
}

void Window_Translations::on_treeview_edited(const Glib::ustring& /* path */, const Glib::ustring& /* new_text */)
{
  m_treeview_modified = true;
}

void Window_Translations::on_button_export()
{ 
  //Show the file-chooser dialog, to select an output .po file:
  Gtk::FileChooserDialog file_dlg(_("Choose .po File Name"), Gtk::FILE_CHOOSER_ACTION_SAVE);
  file_dlg.set_transient_for(*this);
  file_dlg.set_do_overwrite_confirmation();
  
  // Only po files
  Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
  filter->set_name(_("Po files"));
  filter->add_pattern("*.po");
  file_dlg.add_filter(filter);

  file_dlg.add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
  file_dlg.add_button(_("Export"), Gtk::RESPONSE_OK); 
  
  const auto result = file_dlg.run();
  if(result != Gtk::RESPONSE_OK)
    return;

  Glib::ustring uri = file_dlg.get_uri();
  if(uri.empty())
    return;

  save_to_document();
  
  //Enforce the file extension:
  const Glib::ustring extension = ".po";
  bool add_extension = false;
  if(uri.size() <= extension.size())
    add_extension = true;
  else if(uri.substr(uri.size() - extension.size()) != extension)
    add_extension = true;

  if(add_extension)
    uri += extension;

  Glom::write_translations_to_po_file(get_document(), uri, m_translation_locale,
    IsoCodes::get_locale_name(m_translation_locale));
}

void Window_Translations::on_button_import()
{
  Gtk::FileChooserDialog file_dlg(_("Choose .po File Name"), Gtk::FILE_CHOOSER_ACTION_OPEN);
  file_dlg.set_transient_for(*this);

  // Only po files
  Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
  filter->set_name(_("Po files"));
  filter->add_pattern("*.po");
  file_dlg.add_filter(filter);

  file_dlg.add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);

  //Note to translators: "Import" here is an action verb - it's a button. 
  file_dlg.add_button(_("Import"), Gtk::RESPONSE_OK);
  
  const auto result = file_dlg.run();
  if(result != Gtk::RESPONSE_OK)
    return;

  const auto uri = file_dlg.get_uri();
  if(uri.empty())
    return;

  Glom::import_translations_from_po_file(get_document(), uri, m_translation_locale);
  
  //Show the changed document in the UI:
  load_from_document();
}

} //namespace Glom
