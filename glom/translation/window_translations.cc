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
#include <bakery/App/App_Gtk.h> //For util_bold_message().

//#include <libgnome/gnome-i18n.h>
#include <glibmm/i18n.h>

Window_Translations::Window_Translations(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::Window(cobject),
  m_treeview(0),
  m_button_identify(0),
  m_combo_target_locale(0),
  m_label_source_locale(0)
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


    m_treeview->append_column_editable(_("Translation"), m_columns.m_col_translation);
  }

  refGlade->get_widget("button_identify", m_button_identify);
  m_button_identify->signal_clicked().connect( sigc::mem_fun(*this, &Window_Translations::on_button_identify) );

  refGlade->get_widget_derived("combobox_target_locale", m_combo_target_locale);

  show_all_children();
}

Window_Translations::~Window_Translations()
{
}


void Window_Translations::enable_buttons()
{

}

void Window_Translations::on_button_identify()
{

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

  const Glib::ustring translation_locale = "TODO";

  //Add tables:
  Document_Glom::type_listTableInfo tables = document->get_tables();
  for(Document_Glom::type_listTableInfo::const_iterator iter = tables.begin(); iter != tables.end(); ++iter)
  {
    const TableInfo& tableinfo = *iter;

    //Table title:
    Gtk::TreeModel::iterator iterTree = m_model->append();
    Gtk::TreeModel::Row row = *iterTree;
    row[m_columns.m_col_item] = sharedptr<TableInfo>(new TableInfo(tableinfo));
    row[m_columns.m_col_translation] = tableinfo.get_title(translation_locale);
    row[m_columns.m_col_parent_table] = Glib::ustring(); //Not used for tables.

    //The table's field titles:
    Document_Glom::type_vecFields fields = document->get_table_fields(tableinfo.get_name());
    for(Document_Glom::type_vecFields::iterator iter = fields.begin(); iter != fields.end(); ++iter)
    {
      Gtk::TreeModel::iterator iterTree = m_model->append();
      Gtk::TreeModel::Row row = *iterTree;

      sharedptr<Field> field = *iter;
      row[m_columns.m_col_item] = field;
      row[m_columns.m_col_translation] = field->get_title(translation_locale);
      row[m_columns.m_col_parent_table] = tableinfo.get_name();
    }

    //The table's report titles:
    //TODO: 
  }

}

void Window_Translations::save_to_document()
{
}




