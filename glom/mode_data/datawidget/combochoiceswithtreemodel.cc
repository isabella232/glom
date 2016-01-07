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

#include "combochoiceswithtreemodel.h"
#include <glom/mode_data/datawidget/treemodel_db_withextratext.h>
#include <libglom/data_structure/glomconversions.h>
#include <libglom/privs.h>
#include <glom/utils_ui.h>
#include <glom/appwindow.h>
#include <gtkmm/liststore.h>
#include <glibmm/i18n.h>
//#include <sstream> //For stringstream

#include <locale>     // for locale, time_put
#include <ctime>     // for struct tm
#include <iostream>   // for cout, endl


namespace Glom
{

namespace DataWidgetChildren
{

ComboChoicesWithTreeModel::ComboChoicesWithTreeModel()
: m_fixed_cell_height(0)
{
}

ComboChoicesWithTreeModel::~ComboChoicesWithTreeModel()
{
  delete_model();
}

int ComboChoicesWithTreeModel::get_fixed_model_text_column() const
{
  const auto count = m_refModel->get_n_columns();
  if(count > 0)
    return count -1;
  else
   return 0; //An error, but better than a negative number.
}

void ComboChoicesWithTreeModel::create_model_non_db(guint columns_count)
{
  delete_model();

  Gtk::TreeModel::ColumnRecord record;

  //Create the TreeModelColumns, adding them to the ColumnRecord:
    
  m_vec_model_columns_value_fixed.resize(columns_count, 0);
  for(guint i = 0; i < columns_count; ++i)
  {    
    //Create a value column for all columns
    //for instance for later value comparison.
    auto model_column = new type_model_column_value_fixed();
   
    //Store it so we can use it and delete it later:
    m_vec_model_columns_value_fixed[i] = model_column;

    record.add(*model_column);
  }
  
  //Create a text column, for use by a GtkComboBox with has-entry, which allows no other column type:
  //Note that get_fixed_model_text_column() assumes that this is the last column:
  m_vec_model_columns_string_fixed.resize(1, 0);
  if(columns_count > 0)
  {
    auto model_column = new type_model_column_string_fixed();
   
    //Store it so we can use it and delete it later:
    m_vec_model_columns_string_fixed.push_back(model_column);

    record.add(*model_column);
  }

  //Create the model:
  m_refModel = Gtk::ListStore::create(record);
}

void ComboChoicesWithTreeModel::delete_model()
{
  //Delete the vector's items:
  for(const auto& model_column : m_vec_model_columns_string_fixed)
  {
    delete model_column;
  }
  m_vec_model_columns_string_fixed.clear();
  
  //Delete the vector's items:
  for(const auto& model_column : m_vec_model_columns_value_fixed)
  {
    delete model_column;
  }
  m_vec_model_columns_value_fixed.clear();

  m_refModel.reset();
}

/* TODO: Remove this
void ComboChoicesWithTreeModel::set_choices_with_second(const type_list_values_with_second& list_values)
{
  //Recreate the entire model:
  guint columns_count = 1; //For the main field.
  if(!list_values.empty())
  {
    type_list_values_with_second::const_iterator iter= list_values.begin();
    if(iter != list_values.end())
    {
      const type_list_values& second = iter->second;
      columns_count += second.size();
    }
  }
  create_model(columns_count);

  //Fill the model with data:
  auto layout_item =
    std::dynamic_pointer_cast<LayoutItem_Field>(get_layout_item());
  const auto format = layout_item->get_formatting_used();
  std::shared_ptr<const Relationship> choice_relationship;
  std::shared_ptr<const LayoutItem_Field> layout_choice_first;
  std::shared_ptr<const LayoutGroup> layout_choice_extra;
  bool choice_show_all = false;
  format.get_choices_related(choice_relationship, layout_choice_first, layout_choice_extra, choice_show_all);

  LayoutGroup::type_list_const_items extra_fields;
  if(layout_choice_extra)
    extra_fields = layout_choice_extra->get_items_recursive();

  auto list_store = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(m_refModel);
  if(!list_store)
  {
    std::cerr << G_STRFUNC << ": list_store is null." << std::endl;
    return;
  }

  for(const auto& the_pair : list_values)
  {
    auto iterTree = list_store->append();
    Gtk::TreeModel::Row row = *iterTree;

    if(layout_choice_first)
    {
      const Glib::ustring text =
        Conversions::get_text_for_gda_value(layout_choice_first->get_glom_type(), the_pair->first, layout_choice_first->get_formatting_used().m_numeric_format);
      row.set_value(0, text);

      const type_list_values extra_values = the_pair->second;
      if(layout_choice_extra && !extra_values.empty())
      {
        guint model_index = 1; //0 is for the main field.
        const auto iterValues = extra_values.begin();
        for(const auto& extra_field : extra_fields)
        {
          if(model_index >= columns_count)
            break;

          if(iterValues == extra_values.end())
            break;

          const auto item = extra_field;
          const auto item_field = std::dynamic_pointer_cast<const LayoutItem_Field>(item);
          if(item_field)
          {
            const Gnome::Gda::Value value = *iterValues;
            const Glib::ustring text =
              Conversions::get_text_for_gda_value(item_field->get_glom_type(), value, item_field->get_formatting_used().m_numeric_format);
            row.set_value(model_index, text);
          }

          ++model_index;
          ++iterValues;
        }
      }
    }
  }
}
*/


void ComboChoicesWithTreeModel::set_choices_fixed(const Formatting::type_list_values& list_values, bool restricted)
{
  create_model_non_db(1); //Use a regular ListStore without a dynamic column?

  auto list_store = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(m_refModel);
  if(!list_store)
  {
    std::cerr << G_STRFUNC << ": list_store is null." << std::endl;
    return;
  }

  for(const auto& choicevalue : list_values)
  {
    auto iterTree = list_store->append();
    Gtk::TreeModel::Row row = *iterTree;

    auto layout_item = std::dynamic_pointer_cast<LayoutItem_Field>(get_layout_item());
    if(!layout_item)
      continue;
    
    if(!choicevalue)
      continue;

    //Note that this is never a translated version of the value.
    //This is the original value that will be stored in, or read form, the database.
    const auto value = choicevalue->get_value();
    row.set_value(0, value);

    //The text to show in the combo box for the item:
    Glib::ustring text;
    if(restricted && choicevalue->is_translatable())
    {
      //Show the translated text of the value:
      //This will never be stored in the database:
      text = item_get_title(choicevalue);
    }
    else
    {
      text = Conversions::get_text_for_gda_value(layout_item->get_glom_type(), 
        value, layout_item->get_formatting_used().m_numeric_format);
    }

    row.set_value(1, text);
  }

  //The derived class's (virtual) implementation calls this base method and
  //then sets up the view, using the model.
}

void ComboChoicesWithTreeModel::set_choices_related(const Document* document, const std::shared_ptr<const LayoutItem_Field>& layout_field, const Gnome::Gda::Value& foreign_key_value)
{
  if(!document)
  {
    std::cerr << G_STRFUNC << ": document is null." << std::endl;
    return;
  }

  const auto format = layout_field->get_formatting_used();
  std::shared_ptr<const Relationship> choice_relationship;
  std::shared_ptr<const LayoutItem_Field> layout_choice_first;
  std::shared_ptr<const LayoutGroup> layout_choice_extra;
  Formatting::type_list_sort_fields choice_sort_fields;
  bool choice_show_all = false;
  format.get_choices_related(choice_relationship, layout_choice_first, layout_choice_extra, choice_sort_fields, choice_show_all);
  if(layout_choice_first->get_glom_type() == Field::glom_field_type::INVALID)
    std::cerr << G_STRFUNC << ": layout_choice_first has invalid type. field name: " << layout_choice_first->get_name() << std::endl;

  //Set full field details, cloning the group to avoid the constness:
  auto layout_choice_extra_full = glom_sharedptr_clone(layout_choice_extra);
  const auto table_name = choice_relationship->get_to_table();
  document->fill_layout_field_details(table_name,  layout_choice_extra_full);

  //Get the list of fields to show:
  LayoutGroup::type_list_items extra_fields;
  if(layout_choice_extra_full)
    extra_fields = layout_choice_extra_full->get_items_recursive();

  LayoutGroup::type_list_const_items layout_items;
  layout_items.push_back(layout_choice_first);
  layout_items.insert(layout_items.end(), extra_fields.begin(), extra_fields.end());

  //Make sure that the primary key is also in the list, but hidden,
  //because TreeModel_DB needs it:
  layout_items = Utils::get_layout_items_plus_primary_key(layout_items, document, table_name);

  //Build the FoundSet:
  const auto to_table = choice_relationship->get_to_table();
  FoundSet found_set;
  found_set.m_table_name = to_table;

  if(!foreign_key_value.is_null())
  {
    const auto to_field = document->get_field(to_table, choice_relationship->get_to_field());

    found_set.m_where_clause = Utils::build_simple_where_expression(
      to_table, to_field, foreign_key_value);
  }

  found_set.m_sort_clause = choice_sort_fields;
  if(found_set.m_sort_clause.empty())
  {
    //Sort by the first field, because that is better than so sort at all.
    found_set.m_sort_clause.push_back( FoundSet::type_pair_sort_field(layout_choice_first, true /* ascending */) );
  }

  m_db_layout_items.clear();

  //We create DbTreeModelWithExtraText rather than just DbTreeModel, 
  //because Combo(has_entry) needs it.
  //TODO: Avoid getting the actual data if the user does not have view rights.
  const auto table_privs = Privs::get_current_privs(found_set.m_table_name);
  m_refModel = DbTreeModelWithExtraText::create(found_set, layout_items, table_privs.m_view, false /* find mode */, m_db_layout_items);
  if(!m_refModel)
  {
    std::cerr << G_STRFUNC << ": DbTreeModel::create() returned a null model." << std::endl;
  }

  //The derived class's (virtual) implementation calls this base method and
  //then sets up the view, using the model.
}

Glib::RefPtr<Gtk::TreeModel> ComboChoicesWithTreeModel::get_choices_model()
{
  return m_refModel;
}

void ComboChoicesWithTreeModel::set_cell_for_field_value(Gtk::CellRenderer* cell, const std::shared_ptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value)
{
  if(!field)
    return;

  if(!cell)
    return;

  const auto type = field->get_glom_type();
  switch(type)
  {
    case(Field::glom_field_type::BOOLEAN):
    {
      auto pDerived = dynamic_cast<Gtk::CellRendererToggle*>(cell);
      if(pDerived)
        pDerived->set_active( (value.get_value_type() == G_TYPE_BOOLEAN) && value.get_boolean() );

      break;
    }
    case(Field::glom_field_type::IMAGE):
    {
      auto pDerived = dynamic_cast<Gtk::CellRendererPixbuf*>(cell);
      if(pDerived)
      {
        const auto pixbuf = UiUtils::get_pixbuf_for_gda_value(value);

        //Scale it down to a sensible size.
        //TODO: if(pixbuf)
        //  pixbuf = UiUtils::image_scale_keeping_ratio(pixbuf,  get_fixed_cell_height(), pixbuf->get_width());
        pDerived->property_pixbuf() = pixbuf;
      }
      else
        std::cerr << G_STRFUNC << ": Field::sql(): glom_type is enumType::IMAGE but gda type is not VALUE_TYPE_BINARY" << std::endl;

      break;
    }
    default:
    {
      //TODO: Maybe we should have custom cellcells for time, date, and numbers.
      auto pDerived = dynamic_cast<Gtk::CellRendererText*>(cell);
      if(pDerived)
      {
        //std::cout << "debug: " << G_STRFUNC << ": field name=" << field->get_name() << ", glom type=" << field->get_glom_type() << std::endl;
        const auto text = Conversions::get_text_for_gda_value(field->get_glom_type(), value, field->get_formatting_used().m_numeric_format);
        pDerived->property_text() = text;
      }
      else
      {
         std::cerr << G_STRFUNC << ": cell has an unexpected type: " << typeid(cell).name() << std::endl;
      }

      //Show a different color if the value is numeric, if that's specified:
      if(type == Field::glom_field_type::NUMERIC)
      {
        const Glib::ustring fg_color =
        field->get_formatting_used().get_text_format_color_foreground_to_use(value);
        if(!fg_color.empty())
          pDerived->property_foreground() = fg_color;
        else
          pDerived->property_foreground().reset_value();
      }

      break;
    }
  }
}

void ComboChoicesWithTreeModel::on_cell_data(const Gtk::TreeModel::iterator& iter, Gtk::CellRenderer* cell, guint model_column_index)
{
  //std::cout << G_STRFUNC << ": DEBUG: model_column_index=" << model_column_index << std::endl;
  if(model_column_index >= m_db_layout_items.size())
  {
    std::cerr << G_STRFUNC << ": model_column_index (" << model_column_index << ") is out of range. size=" << m_db_layout_items.size() << std::endl;
    return;
  }
   
  if(!cell)
  {
    std::cerr << G_STRFUNC << ": cell is null." << std::endl;
    return;
  }

  if(!iter)
    return;

  const std::shared_ptr<const LayoutItem>& layout_item = m_db_layout_items[model_column_index];
  auto field = std::dynamic_pointer_cast<const LayoutItem_Field>(layout_item);
  if(!field)
    return;

  Gnome::Gda::Value value;
  Gtk::TreeModel::Row treerow = *iter;
  treerow->get_value(model_column_index, value);

  set_cell_for_field_value(cell, field, value);
}

void ComboChoicesWithTreeModel::cell_connect_cell_data_func(Gtk::CellLayout* celllayout, Gtk::CellRenderer* cell, guint model_column_index)
{
  if(model_column_index >= m_db_layout_items.size())
  {
    std::cerr << G_STRFUNC << ": model_column_index (" << model_column_index << ") is out of range. size=" << m_db_layout_items.size() << std::endl;
    return;
  }
  
  celllayout->set_cell_data_func(*cell,
    sigc::bind( sigc::mem_fun(*this, &ComboChoicesWithTreeModel::on_cell_data), cell, model_column_index));
}

int ComboChoicesWithTreeModel::get_fixed_cell_height(Gtk::Widget& widget)
{
  if(m_fixed_cell_height <= 0)
  {
    // Discover a suitable height, and cache it,
    // by looking at the heights of all columns:
    // Note that this is usually calculated during construct_specified_columns(),
    // when all columns are known.

    //Get a default:
    const Glib::RefPtr<const Pango::Layout> refLayoutDefault = widget.create_pango_layout("example");
    int width_default = 0;
    int height_default = 0;
    refLayoutDefault->get_pixel_size(width_default, height_default);
    m_fixed_cell_height = height_default;

    //Look at each column:
    for(const auto& item : m_db_layout_items)
    {
      Glib::ustring font_name;

      const auto item_withformatting = std::dynamic_pointer_cast<const LayoutItem_WithFormatting>(item);
      if(item_withformatting)
      {
         const auto formatting = item_withformatting->get_formatting_used();
         font_name = formatting.get_text_format_font();
      }

      if(font_name.empty())
        continue;

      // Translators: This is just some example text used to discover an appropriate height for user-entered text in the UI. This text itself is never shown to the user.
      auto refLayout = widget.create_pango_layout(_("Example"));
      const Pango::FontDescription font(font_name);
      refLayout->set_font_description(font);
      int width = 0;
      int height = 0;
      refLayout->get_pixel_size(width, height);

      if(height > m_fixed_cell_height)
        m_fixed_cell_height = height;
    }
  }

  return m_fixed_cell_height;
}

} //namespace DataWidetChildren
} //namespace Glom
