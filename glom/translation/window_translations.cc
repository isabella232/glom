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

#include "window_translations.h"
#include "combobox_locale.h"
#include "dialog_identify_original.h"
#include "dialog_copy_translation.h"
#include <bakery/App/App_Gtk.h> //For util_bold_message().

//#include <libgnome/gnome-i18n.h>
#include <glibmm/i18n.h>

Window_Translations::Window_Translations(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::Window(cobject),
  m_treeview(0),
  m_button_identify(0),
  m_combo_target_locale(0),
  m_label_source_locale(0),
  m_button_ok(0),
  m_button_cancel(0),
  m_treeview_modified(false)
{
  refGlade->get_widget("label_source_locale", m_label_source_locale);

  refGlade->get_widget("treeview", m_treeview);
  if(m_treeview)
  {
    m_model = Gtk::ListStore::create(m_columns);
    m_treeview->set_model(m_model);

    // Append the View columns:
    Gtk::TreeView::Column* column_original = Gtk::manage( new Gtk::TreeView::Column(_("Original")) );
    m_treeview->append_column(*column_original);

    Gtk::CellRendererText* renderer_name = Gtk::manage(new Gtk::CellRendererText);
    column_original->pack_start(*renderer_name);
    column_original->set_cell_data_func(*renderer_name, sigc::mem_fun(*this, &Window_Translations::on_cell_data_original));


    Gtk::TreeView::Column* column_item_typename = Gtk::manage( new Gtk::TreeView::Column(_("Item")) );
    m_treeview->append_column(*column_item_typename);

    Gtk::CellRendererText* renderer_item_typename = Gtk::manage(new Gtk::CellRendererText);
    column_item_typename->pack_start(*renderer_item_typename);
    column_item_typename->set_cell_data_func(*renderer_item_typename, sigc::mem_fun(*this, &Window_Translations::on_cell_data_item_typename));


    const int col = m_treeview->append_column_editable(_("Translation"), m_columns.m_col_translation);
    Gtk::CellRendererText* renderer = dynamic_cast<Gtk::CellRendererText*>(m_treeview->get_column_cell_renderer(col - 1));
    if(renderer)
      renderer->signal_edited().connect(sigc::mem_fun(*this, &Window_Translations::on_treeview_edited));
  }

  refGlade->get_widget("button_identify", m_button_identify);
  m_button_identify->signal_clicked().connect( sigc::mem_fun(*this, &Window_Translations::on_button_identify) );

  refGlade->get_widget_derived("combobox_target_locale", m_combo_target_locale);
  m_combo_target_locale->signal_changed().connect(sigc::mem_fun(*this, &Window_Translations::on_combo_target_locale_changed));

  refGlade->get_widget("button_ok", m_button_ok);
  m_button_ok->signal_clicked().connect( sigc::mem_fun(*this, &Window_Translations::on_button_ok) );

  refGlade->get_widget("button_cancel", m_button_cancel);
  m_button_cancel->signal_clicked().connect( sigc::mem_fun(*this, &Window_Translations::on_button_cancel) );

  refGlade->get_widget("button_copy_translation", m_button_copy_translation);
  m_button_copy_translation->signal_clicked().connect( sigc::mem_fun(*this, &Window_Translations::on_button_copy_translation) );


  show_all_children();

  //Start with the currently-used/tested translation, if appropriate:
  if(TranslatableItem::get_current_locale_not_original())
  {
    m_translation_locale = TranslatableItem::get_current_locale();
    m_combo_target_locale->set_selected_locale(m_translation_locale);
    //The translations will be shown in the treeview when load_from_document() is called.
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
  Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_translation_identify_original");
  if(refXml)
  {
    Dialog_IdentifyOriginal* dialog = 0;
    refXml->get_widget_derived("dialog_translation_identify_original", dialog);
    if(dialog)
    {
      add_view(dialog);
      dialog->load_from_document(); //Doesn't seem to happen otherwise.
      dialog->set_transient_for(*this);
      const int response = dialog->run();
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
  }
}


void Window_Translations::on_cell_data_original(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter)
{
  //Set the view's cell properties depending on the model's data:
  Gtk::CellRendererText* renderer_text = dynamic_cast<Gtk::CellRendererText*>(renderer);
  if(renderer_text)
  {
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;

      Glib::ustring text;
      sharedptr<TranslatableItem> item = row[m_columns.m_col_item];
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

void Window_Translations::on_cell_data_item_typename(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter)
{
  //Set the view's cell properties depending on the model's data:
  Gtk::CellRendererText* renderer_text = dynamic_cast<Gtk::CellRendererText*>(renderer);
  if(renderer_text)
  {
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;

      Glib::ustring item_type_name;
      sharedptr<TranslatableItem> item = row[m_columns.m_col_item];
      if(item)
        item_type_name = TranslatableItem::get_translatable_type_name(item->get_translatable_item_type());

      renderer_text->property_text() = item_type_name;
      renderer_text->property_editable() = false; //Names can never be edited.
    }
  }
}

void Window_Translations::load_from_document()
{
  m_model->clear(); //Remove all rows.

  Document_Glom* document = get_document();
  if(!document)
    return;

  //std::cout << "document->get_translation_original_locale()=" << document->get_translation_original_locale() << std::endl;
  Glib::ustring original_locale_name = IsoCodes::get_locale_name(document->get_translation_original_locale());
  if(original_locale_name.empty())
    original_locale_name = _("Unknown");
  m_label_source_locale->set_text(original_locale_name);

  //Add tables:
  Document_Glom::type_listTableInfo tables = document->get_tables();
  for(Document_Glom::type_listTableInfo::const_iterator iter = tables.begin(); iter != tables.end(); ++iter)
  {
    sharedptr<TableInfo> tableinfo = *iter;
    const Glib::ustring table_name = tableinfo->get_name();

    //Table title:
    Gtk::TreeModel::iterator iterTree = m_model->append();
    Gtk::TreeModel::Row row = *iterTree;
    row[m_columns.m_col_item] = tableinfo;
    row[m_columns.m_col_translation] = tableinfo->get_title(m_translation_locale);
    row[m_columns.m_col_parent_table] = Glib::ustring(); //Not used for tables.

    //The table's field titles:
    Document_Glom::type_vecFields fields = document->get_table_fields(table_name);
    for(Document_Glom::type_vecFields::iterator iter = fields.begin(); iter != fields.end(); ++iter)
    {
      Gtk::TreeModel::iterator iterTree = m_model->append();
      Gtk::TreeModel::Row row = *iterTree;

      sharedptr<Field> field = *iter;
      row[m_columns.m_col_item] = field;
      row[m_columns.m_col_translation] = field->get_title(m_translation_locale);
      row[m_columns.m_col_parent_table] = table_name;

    }

    //The table's relationships:
    Document_Glom::type_vecRelationships relationships = document->get_relationships(table_name);
    for(Document_Glom::type_vecRelationships::iterator iter = relationships.begin(); iter != relationships.end(); ++iter)
    {
      sharedptr<Relationship> relationship = *iter;
      if(relationship)
      {
        Gtk::TreeModel::iterator iterTree = m_model->append();
        Gtk::TreeModel::Row row = *iterTree;

        row[m_columns.m_col_item] = relationship;
        row[m_columns.m_col_translation] = relationship->get_title(m_translation_locale);
        row[m_columns.m_col_parent_table] = table_name;
      }
    }

    //The table's report titles:
    Document_Glom::type_listReports listReports = document->get_report_names(table_name);
    for(Document_Glom::type_listReports::iterator iter = listReports.begin(); iter != listReports.end(); ++iter)
    {
      sharedptr<Report> report = document->get_report(table_name, *iter);
      if(report)
      {
        Gtk::TreeModel::iterator iterTree = m_model->append();
        Gtk::TreeModel::Row row = *iterTree;

        row[m_columns.m_col_item] = report;
        row[m_columns.m_col_translation] = report->get_title(m_translation_locale);
        row[m_columns.m_col_parent_table] = table_name;
      }
    }

    //The table's translatable layout items:
    Document_Glom::type_list_translatables list_layout_items = document->get_translatable_layout_items(table_name);
    for(Document_Glom::type_list_translatables::iterator iter = list_layout_items.begin(); iter != list_layout_items.end(); ++iter)
    {
      sharedptr<TranslatableItem> item = *iter;
      if(item)
      {
        if(!(item->get_title_original().empty()))
        {
          Gtk::TreeModel::iterator iterTree = m_model->append();
          Gtk::TreeModel::Row row = *iterTree;

          row[m_columns.m_col_item] = item;
          row[m_columns.m_col_translation] = item->get_title(m_translation_locale);
          row[m_columns.m_col_parent_table] = table_name;
        }

        sharedptr<LayoutItem_Field> layout_field = sharedptr<LayoutItem_Field>::cast_dynamic(item);
        if(layout_field)
        {
          sharedptr<CustomTitle> custom_title = layout_field->get_title_custom();
          if(custom_title && !(custom_title->get_title_original().empty()))
          {
            Gtk::TreeModel::iterator iterTree = m_model->append();
            Gtk::TreeModel::Row row = *iterTree;

            row[m_columns.m_col_item] = custom_title;
            row[m_columns.m_col_translation] = custom_title->get_title(m_translation_locale);
            row[m_columns.m_col_parent_table] = table_name;
          }
        }

      }
    }

  } //for

  m_treeview_modified = false;
}

void Window_Translations::save_to_document()
{
  if(!m_treeview_modified || m_translation_locale.empty())
    return;

  //Look at every item in the treeview and apply its translation:
  for(Gtk::TreeModel::iterator iter = m_model->children().begin(); iter != m_model->children().end(); ++iter)
  {
    Gtk::TreeModel::Row row = *iter;

    //We have stored a sharedptr to the original item, so we can just change it directly:
    sharedptr<TranslatableItem> item = row[m_columns.m_col_item];
    if(item)
    {
      const Glib::ustring translation = row[m_columns.m_col_translation];
      item->set_title(m_translation_locale, translation);
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
   Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_translation_copy");
  if(refXml)
  {
    Dialog_CopyTranslation* dialog = 0;
    refXml->get_widget_derived("dialog_translation_copy", dialog);
    if(dialog)
    {
      dialog->set_transient_for(*this);
      const int response = dialog->run();
      dialog->hide();

      if(response == Gtk::RESPONSE_OK)
      {
        const Glib::ustring copy_source_locale = dialog->get_locale();
        if(!copy_source_locale.empty())
        {
          //Save and update:
          on_combo_target_locale_changed();

          for(Gtk::TreeModel::iterator iter = m_model->children().begin(); iter != m_model->children().end(); ++iter)
          {
            Gtk::TreeModel::Row row = *iter;

            sharedptr<TranslatableItem> item = row[m_columns.m_col_item];
            if(item)
            {
              //Copy the translation from the chosen locale to the current locale:
              const Glib::ustring translation = item->get_title(copy_source_locale);
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
  }
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

