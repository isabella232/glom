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

#include "dialog_import_csv.h"
#include <glom/import_csv/file_encodings.h>
#include <libglom/libglom_config.h>

#include <libglom/data_structure/glomconversions.h>

#include <gtkmm/messagedialog.h>
#include <gtkmm/cellrenderercombo.h>
#include <glom/utils_ui.h>
#include <gtkmm/main.h>
#include <glibmm/convert.h>
#include <glibmm/markup.h>
#include <glibmm/i18n.h>
#include <cerrno>
#include <iostream>

namespace
{

// When auto-detecting the encoding, we try to read the file in these
// encodings, in order:
const char* AUTODETECT_ENCODINGS_CHARSETS[] = {
  "UTF-8",
  "ISO-8859-1",
  "ISO-8859-15",
  "UTF-16",
  "UCS-2",
  "UCS-4"
};

const auto N_AUTODETECT_ENCODINGS_CHARSETS = sizeof(AUTODETECT_ENCODINGS_CHARSETS)/sizeof(AUTODETECT_ENCODINGS_CHARSETS[0]);


Glib::ustring encoding_display(const Glib::ustring& name, const Glib::ustring& charset)
{
  if(charset.empty())
    return name;
  else
    return name + " (" + charset + ')';
}

} //anonymous namespace

namespace Glom
{

const char* Dialog_Import_CSV::glade_id("dialog_import_csv");
const bool Dialog_Import_CSV::glade_developer(false);

Dialog_Import_CSV::Dialog_Import_CSV(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Dialog(cobject),
  m_auto_detect_encoding(),
  m_cols_count(-1)
{
  builder->get_widget("import_csv_fields", m_sample_view);
  builder->get_widget("import_csv_target_table", m_target_table);
  builder->get_widget("import_csv_encoding", m_encoding_combo);
  builder->get_widget("import_csv_encoding_info", m_encoding_info);
  builder->get_widget("import_csv_first_line_as_title", m_first_line_as_title);
  builder->get_widget("import_csv_sample_rows", m_sample_rows);
  builder->get_widget("import_csv_advice_label", m_advice_label);
  builder->get_widget("import_csv_error_label", m_error_label);

  if(!m_sample_view || !m_encoding_combo || !m_target_table || !m_encoding_info || !m_first_line_as_title || !m_sample_rows || !m_error_label)
  {
    std::cerr << G_STRFUNC << ": Missing widgets from glade file for Dialog_Import_CSV" << std::endl;
  }

  //Set the adjustment details, to avoid a useless 0-to-0 range and a 0 incremenet.
  //We don't do this the Glade file because GtkBuilder wouldn't find the
  //associated adjustment object unless we specified it explictly:
  //See http://bugzilla.gnome.org/show_bug.cgi?id=575714
  m_sample_rows->set_range(0, 100);
  m_sample_rows->set_increments(1, 10);
  m_sample_rows->set_value(2); //A sensible default.

  //Fill the list of encodings:
  m_encoding_model = Gtk::ListStore::create(m_encoding_columns);

  auto iter = m_encoding_model->append();
  (*iter)[m_encoding_columns.m_col_name] = _("Auto Detect");

  // Separator:
  m_encoding_model->append();

  for(const auto& encoding : FileEncodings::get_list_of_encodings())
  {
    if(encoding.get_name().empty())
      continue;

    iter = m_encoding_model->append();
    Gtk::TreeModel::Row row = *iter;
    row[m_encoding_columns.m_col_name] = encoding.get_name();
    row[m_encoding_columns.m_col_charset] = encoding.get_charset();
  }

  auto renderer = Gtk::manage(new Gtk::CellRendererText);
  m_encoding_combo->set_model(m_encoding_model);
  m_encoding_combo->pack_start(*renderer);
  m_encoding_combo->set_cell_data_func(*renderer, sigc::bind(sigc::mem_fun(*this, &Dialog_Import_CSV::encoding_data_func), sigc::ref(*renderer)));
  m_encoding_combo->set_row_separator_func(sigc::mem_fun(*this, &Dialog_Import_CSV::row_separator_func));
  m_encoding_combo->set_active(0);

  m_encoding_combo->signal_changed().connect(sigc::mem_fun(*this, &Dialog_Import_CSV::on_combo_encoding_changed));

  // TODO: Reset parser encoding on selection changed.
  m_parser = std::make_shared<CsvParser>(get_current_encoding().c_str());
  m_parser->signal_file_read_error().connect(sigc::mem_fun(*this, &Dialog_Import_CSV::on_parser_file_read_error));
  m_parser->signal_have_display_name().connect(sigc::mem_fun(*this, &Dialog_Import_CSV::on_parser_have_display_name));
  m_parser->signal_encoding_error().connect(sigc::mem_fun(*this, &Dialog_Import_CSV::on_parser_encoding_error));
  m_parser->signal_line_scanned().connect(sigc::mem_fun(*this, &Dialog_Import_CSV::on_parser_line_scanned));
  m_parser->signal_state_changed().connect(sigc::mem_fun(*this, &Dialog_Import_CSV::on_parser_state_changed));

  m_first_line_as_title->set_active(false);
  m_first_line_as_title->signal_toggled().connect(sigc::mem_fun(*this, &Dialog_Import_CSV::on_first_line_as_title_toggled));
  m_sample_rows->signal_changed().connect(sigc::mem_fun(*this, &Dialog_Import_CSV::on_sample_rows_changed));

  m_sample_view->set_headers_visible(m_first_line_as_title->get_active());


  //Warn the user about the numeric and date formats expected:

  //A date that is really really the date that we mean:
  tm the_c_time;
  memset(&the_c_time, 0, sizeof(the_c_time));

  //We mean 22nd November 2008:
  the_c_time.tm_year = 2008 - 1900; //C years start are the AD year - 1900. So, 01 is 1901.
  the_c_time.tm_mon = 11 - 1; //C months start at 0.
  the_c_time.tm_mday = 22; //starts at 1

  //Get the ISO (not current locale) text representation:
  const auto date_text = Glom::Conversions::format_date(the_c_time, std::locale::classic() /* ignored */, true /* iso_format */);
  const auto advice = Glib::ustring::compose(_("Note that the source file should contain numbers and dates in international ISO format. For instance, 22nd November 2008 should be %1."), date_text);
  m_advice_label->set_text(advice);
  //std::cout << "DEBUG: advice=" << advice << std::endl;

  clear();
}

CsvParser::State Dialog_Import_CSV::get_parser_state() const
{
  return m_parser->get_state();
}

Glib::ustring Dialog_Import_CSV::get_target_table_name() const
{
  return m_target_table->get_text();
}

const Glib::ustring& Dialog_Import_CSV::get_file_uri() const
{
  return m_file_uri;
}

void Dialog_Import_CSV::import(const Glib::ustring& uri, const Glib::ustring& into_table)
{
  clear();

  auto document = get_document();
  if(!document)
  {
    show_error_dialog(_("No Document Available"), _("You need to open a document to import the data into a table."));
  }
  else
  {
    // Make the relevant widgets sensitive. We will make them insensitive
    // again when a (non-recoverable) error occurs.
    m_sample_view->set_sensitive();
    m_encoding_combo->set_sensitive();
    m_first_line_as_title->set_sensitive();
    m_sample_rows->set_sensitive();

    set_title(_("Import From CSV File"));
    m_target_table->set_markup("<b>" + Glib::Markup::escape_text(into_table) + "</b>");

    m_field_model = Gtk::ListStore::create(m_field_columns);
    auto tree_iter_none = m_field_model->append();
    (*tree_iter_none)[m_field_columns.m_col_field_name] = _("<None>");

    const Document::type_vec_fields fields(document->get_table_fields(into_table));
    for (const auto& field : fields)
    {
      if(!field)
        continue;

      // Don't allow the primary key to be selected when it is an auto
      // increment key, since the value for the primary key is chosen
      // automatically anyway.
      if(!field->get_primary_key() || !field->get_auto_increment())
      {
        auto tree_iter = m_field_model->append();
        (*tree_iter)[m_field_columns.m_col_field] = field;
        (*tree_iter)[m_field_columns.m_col_field_name] = field->get_name();
      }
    }

    // Create the sorted version of this model,
    // so the user sees the fields in alphabetical order:
    m_field_model_sorted = Gtk::TreeModelSort::create(m_field_model);
    m_field_model_sorted->set_sort_column(m_field_columns.m_col_field_name, Gtk::SORT_ASCENDING);

    m_file_uri = uri;
    m_parser->set_file_and_start_parsing(uri);
  }
}

std::shared_ptr<const Field> Dialog_Import_CSV::get_field_for_column(guint col) const
{
  if(col >= m_fields.size())
    return std::shared_ptr<const Field>();

  return m_fields[col];
}

const Glib::ustring& Dialog_Import_CSV::get_data(guint row, guint col)
{
  if(m_first_line_as_title->get_active())
    ++row;

  return m_parser->get_data(row, col);
}

CsvParser& Dialog_Import_CSV::get_parser()
{
  return *(m_parser.get());
}

void Dialog_Import_CSV::clear()
{
  // TODO: Do we explicitely need to cancel async operations?
  // TODO: Disconnect idle handlers
  m_sample_model.reset();
  UiUtils::treeview_delete_all_columns(m_sample_view);
  m_sample_view->set_model(m_sample_model);
  m_field_model.reset();
  m_field_model_sorted.reset();
  m_fields.clear();
  m_file_uri.clear();
  m_parser->clear();
  //m_parser.reset(0);
  m_encoding_info->set_text("");
  m_sample_view->set_sensitive(false);
  m_encoding_combo->set_sensitive(false);
  m_first_line_as_title->set_sensitive(false);
  m_sample_rows->set_sensitive(false);

  validate_primary_key();
}

void Dialog_Import_CSV::show_error_dialog(const Glib::ustring&, const Glib::ustring& secondary)
{
  UiUtils::show_ok_dialog(_("Error Importing CSV File"),
     secondary, *this, Gtk::MESSAGE_ERROR);
}

void Dialog_Import_CSV::encoding_data_func(const Gtk::TreeModel::iterator& iter, Gtk::CellRendererText& renderer)
{
  const auto name = (*iter)[m_encoding_columns.m_col_name];
  const auto charset = (*iter)[m_encoding_columns.m_col_charset];

  renderer.set_property("text", encoding_display(name, charset));
}

bool Dialog_Import_CSV::row_separator_func(const Glib::RefPtr<Gtk::TreeModel>& /* model */, const Gtk::TreeModel::iterator& iter) const
{
  return (*iter)[m_encoding_columns.m_col_name] == "";
}


void Dialog_Import_CSV::on_combo_encoding_changed()
{
  const auto active = m_encoding_combo->get_active_row_number();

  switch(active)
  {
  case -1: // No active item
    g_assert_not_reached();
    break;
  case 0: // Auto-Detect
    // Begin with first encoding
    m_auto_detect_encoding = 0;
    break;
  default: // Some specific encoding
    m_auto_detect_encoding = -1;
    break;
  }

  // Parse from beginning with new encoding:
  m_parser->set_encoding(get_current_encoding());
  m_parser->set_file_and_start_parsing(m_file_uri);
}

void Dialog_Import_CSV::on_first_line_as_title_toggled()
{
  // Ignore if we don't have a model yet, we will take care of the option
  // later when inserting items into it.
  if(!m_sample_model)
    return;

  if(m_first_line_as_title->get_active())
  {
    m_sample_view->set_headers_visible(true);
    Gtk::TreeModel::Path path("1");
    auto iter = m_sample_model->get_iter(path);

    // Remove the first row from the view
    if(iter && (*iter)[m_sample_columns.m_col_row] == 0)
    {
      m_sample_model->erase(iter);

      // Add another row to the end, if one is loaded.
      const auto last_index = m_sample_model->children().size();
      iter = m_sample_model->append();
      (*iter)[m_sample_columns.m_col_row] = last_index;
    }
  }
  else
  {
    m_sample_view->set_headers_visible(false);

    // Check whether row 0 is displayed
    Gtk::TreeModel::Path path("1");
    auto iter = m_sample_model->get_iter(path);

    //if((!iter || (*iter)[m_sample_columns.m_col_row] != 0) && !m_parser->get_rows_empty() && m_sample_rows->get_value_as_int() > 0)
    if((!iter || (*iter)[m_sample_columns.m_col_row] != 0) &&
        m_sample_rows->get_value_as_int() > 0)
    {
      // Add first row to model
      if(!iter)
        iter = m_sample_model->append();
      else
        iter = m_sample_model->insert(iter);

      (*iter)[m_sample_columns.m_col_row] = 0;

      // Remove last row if we hit the limit
      const auto sample_rows = m_sample_model->children().size() - 1;
      if(sample_rows > static_cast<guint>(m_sample_rows->get_value_as_int()))
      {
        //m_sample_model->erase(m_sample_model->children().rbegin());
        path[0] = sample_rows;
        iter = m_sample_model->get_iter(path);
        m_sample_model->erase(iter);
      }
    }
  }
}

void Dialog_Import_CSV::on_sample_rows_changed()
{
  // Ignore if we don't have a model yet, we will take care of the option
  // later when inserting items into it.
  if(!m_sample_model)
    return;

  const auto children_size = m_sample_model->children().size();
  const guint current_sample_rows = (children_size == 0 ? 0 : children_size - 1);
  const guint new_sample_rows = m_sample_rows->get_value_as_int();

  if(current_sample_rows > new_sample_rows)
  {
    // +1 for the "target field" row
    Gtk::TreeModel::Path path(1);
    path[0] = new_sample_rows + 1;
    auto iter = m_sample_model->get_iter(path);

    while(iter != m_sample_model->children().end())
      iter = m_sample_model->erase(iter);
  }
  else
  {
    // Find index of first row to add
    guint row_index = current_sample_rows;
    if(m_first_line_as_title->get_active())
      ++row_index;

    for(guint i = current_sample_rows; i < new_sample_rows; ++i, ++row_index)
    {
      auto iter = m_sample_model->append();
      (*iter)[m_sample_columns.m_col_row] = row_index;
    }
  }
}

Glib::ustring Dialog_Import_CSV::get_current_encoding() const
{
  auto iter = m_encoding_combo->get_active();
  const Glib::ustring encoding = (*iter)[m_encoding_columns.m_col_charset];

  if(encoding.empty())
  {
    // Auto-Detect:
    g_assert(m_auto_detect_encoding != -1);
    return AUTODETECT_ENCODINGS_CHARSETS[m_auto_detect_encoding];
  }

  return encoding;
}

void Dialog_Import_CSV::begin_parse()
{
  if(m_auto_detect_encoding != -1)
  {
    const char* encoding_charset = AUTODETECT_ENCODINGS_CHARSETS[m_auto_detect_encoding];
    const auto encoding_name = FileEncodings::get_name_of_charset(encoding_charset);
    m_encoding_info->set_text(Glib::ustring::compose(_("Encoding detected as: %1"), encoding_display(encoding_name, encoding_charset)));
  }
  else
    m_encoding_info->set_text("");

  // Clear sample preview since we reparse everything, perhaps with
  // another encoding.
  m_sample_model.reset();
  UiUtils::treeview_delete_all_columns(m_sample_view);
  m_sample_view->set_model(m_sample_model); // Empty model
  m_parser->clear();

  m_parser->set_encoding(get_current_encoding());

  // Allow the Import button to be pressed when a field for the primary key
  // field is set. When the import button is pressed without the file being
  // fully loaded, the import progress waits for us to load the rest.
  validate_primary_key();
}

void Dialog_Import_CSV::on_parser_encoding_error()
{
  m_parser->clear();
  // Clear sample preview (TODO: Let it visible, and only remove when reparsing?)
  m_sample_model.reset();
  UiUtils::treeview_delete_all_columns(m_sample_view);
  m_sample_view->set_model(m_sample_model); // Empty model

  // Don't allow the import button to be pressed when an error occured. This
  // would not make sense since we cleared all the parsed row data anyway.
  validate_primary_key();

  // If we are auto-detecting the encoding, then try the next one
  if(m_auto_detect_encoding != -1)
  {
    ++ m_auto_detect_encoding;
    if(static_cast<guint>(m_auto_detect_encoding) < N_AUTODETECT_ENCODINGS_CHARSETS)
      begin_parse();
    else
      m_encoding_info->set_text(_("Encoding detection failed. Please manually choose one from the box."));
  }
  else
  {
    m_encoding_info->set_text(_("The file contains data not in the specified encoding. Please choose another one, or try \"Auto Detect\"."));
  }
}

void Dialog_Import_CSV::on_parser_line_scanned(CsvParser::type_row_strings row, unsigned int row_number)
{
  // This is the first line read if there is no model yet:
  if(!m_sample_model)
  {
    setup_sample_model(row);
    auto iter = m_sample_model->append();
    // -1 means the row to select target fields (see special handling in cell data funcs)
    (*iter)[m_sample_columns.m_col_row] = -1;
  }

  // Add the row to the treeview if there are not yet as much sample rows
  // as the user has chosen (note the first row is to choose the target fields,
  // not a sample row, which is why we do -1 here).
  const auto sample_rows = m_sample_model->children().size() - 1;

  // Don't add if this is the first line and m_first_line_as_title is active:
  if(row_number > 1 || !m_first_line_as_title->get_active())
  {
    if(sample_rows < static_cast<guint>(m_sample_rows->get_value_as_int()))
    {
      auto tree_iter = m_sample_model->append();
      (*tree_iter)[m_sample_columns.m_col_row] = row_number;
    }
  }
}

void Dialog_Import_CSV::setup_sample_model(const CsvParser::type_row_strings& row)
{
  m_sample_model = Gtk::ListStore::create(m_sample_columns);
  m_sample_view->set_model(m_sample_model);

  // Create field vector that contains the fields into which to import
  // the data.
  m_fields.resize(row.size());

  // Start with a column showing the line number.
  auto text = Gtk::manage(new Gtk::CellRendererText);
  auto col = Gtk::manage(new Gtk::TreeViewColumn(_("Line")));
  col->pack_start(*text, false);
  col->set_cell_data_func(*text, sigc::mem_fun(*this, &Dialog_Import_CSV::line_data_func));
  m_sample_view->append_column(*col);

  m_cols_count = row.size();

  for(guint i = 0; i < m_cols_count; ++ i)
  {
    const Glib::ustring& data = row[i];
    m_sample_view->append_column(*Gtk::manage(create_sample_column(data, i)));
  }
}

Gtk::TreeViewColumn* Dialog_Import_CSV::create_sample_column(const Glib::ustring& title, guint index)
{
  Gtk::TreeViewColumn* col = new Gtk::TreeViewColumn(title);
  auto cell = create_sample_cell(index);
  col->pack_start(*Gtk::manage(cell), true);
  col->set_cell_data_func(*cell, sigc::bind(sigc::mem_fun(*this, &Dialog_Import_CSV::field_data_func), index));
  col->set_sizing(Gtk::TREE_VIEW_COLUMN_AUTOSIZE);
  return col;
}

Gtk::CellRendererCombo* Dialog_Import_CSV::create_sample_cell(guint index)
{
  Gtk::CellRendererCombo* cell = new Gtk::CellRendererCombo;
  cell->property_model() = m_field_model_sorted;
  cell->property_text_column() = 0;
  cell->property_has_entry() = false;
  cell->signal_edited().connect(sigc::bind(sigc::mem_fun(*this, &Dialog_Import_CSV::on_field_edited), index));

  return cell;
}

void Dialog_Import_CSV::line_data_func(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter)
{
  const auto row = (*iter)[m_sample_columns.m_col_row];
  auto renderer_text = dynamic_cast<Gtk::CellRendererText*>(renderer);
  if(!renderer_text)
    throw std::logic_error("CellRenderer is not a CellRendererText in line_data_func");

  if(row == -1)
    renderer_text->set_property("text", Glib::ustring(_("Target Field")));
  else
    renderer_text->set_property("text", Glib::ustring::compose("%1", row + 1));
}

void Dialog_Import_CSV::field_data_func(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter, guint column_number)
{
  const auto row = (*iter)[m_sample_columns.m_col_row];
  auto renderer_combo = dynamic_cast<Gtk::CellRendererCombo*>(renderer);
  if(!renderer_combo) throw std::logic_error("CellRenderer is not a CellRendererCombo in field_data_func");

  Glib::ustring text;
  bool editable = false;

  if(row == -1)
  {
    std::shared_ptr<Field> field = m_fields[column_number];
    if(field)
      text = field->get_name();
    else
      text = _("<None>");

    editable = true;
  }
  else
  {
    // Convert to currently chosen field, if any, and back, too see how it
    // looks like when imported:

    if(column_number < m_fields.size())
    {
      std::shared_ptr<Field> field = m_fields[column_number];

      if(row != -1) // && static_cast<unsigned int>(row) < m_parser->get_rows_count())
      {
          const auto orig_text = m_parser->get_data(row, column_number);

          if(field)
          {
            if(field->get_glom_type() != Field::glom_field_type::IMAGE)
            {
              /* Exported data is always stored in postgres format */
              bool success = false;
              const auto value = field->from_file_format(orig_text, success);
              if(!success)
                text = _("<Import Failure>");
              else
                text = Glom::Conversions::get_text_for_gda_value(field->get_glom_type(), value);
            }
            else
            {
              // TODO: It is too slow to create the picture here. Maybe we should
              // create it once and cache it. We could also think about using a
              // GtkCellRendererPixbuf to show it, then.
              if(!orig_text.empty() && orig_text != "NULL")
                text = _("<Picture>");
            }
          }
          else
          {
            // TODO: Should we unescape the field's content?
            text = orig_text;
          }

          if(text.length() > 32)
          {
            text.erase(32);
            text.append("…");
          }

          editable = false;
      }
    }
  }

  renderer_combo->set_property("text", text);
  renderer_combo->set_property("editable", editable);
}


/**  Parse a row from a .cvs file. Note that this "line" might have newline
  *  characters inside one field value, inside quotes.
  **/
void Dialog_Import_CSV::on_field_edited(const Glib::ustring& path, const Glib::ustring& new_text, guint column_number)
{
  Gtk::TreeModel::Path treepath(path);
  auto iter = m_sample_model->get_iter(treepath);

  // Lookup field indicated by new_text
  const auto children = m_field_model->children();
  for(auto field_iter = children.begin(); field_iter != children.end(); ++ field_iter)
  {
    if( (*field_iter)[m_field_columns.m_col_field_name] == new_text)
    {
      std::shared_ptr<Field> field = (*field_iter)[m_field_columns.m_col_field];
      // Check whether another column is already using that field
      auto vec_field_iter = std::find(m_fields.begin(), m_fields.end(), field);
      // Reset the old column since two different columns cannot be imported into the same field
      if(vec_field_iter != m_fields.end()) *vec_field_iter = std::shared_ptr<Field>();

      m_fields[column_number] = field;

      // Update the rows, so they are redrawn, doing a conversion to the new type.
      const auto sample_children = m_sample_model->children();
      // Create a TreeModel::Path with initial index 0. We need a TreeModel::Path for the row_changed() call
      Gtk::TreeModel::Path path_changed("0");

      for(auto sample_iter = sample_children.begin(); sample_iter != sample_children.end(); ++ sample_iter)
      {
        if(sample_iter != iter)
          m_sample_model->row_changed(path_changed, sample_iter);

        path_changed.next();
      }

      validate_primary_key();
      break;
    }
  }
}

void Dialog_Import_CSV::validate_primary_key()
{
  if(get_parser_state() == CsvParser::State::NONE
   || get_parser_state() == CsvParser::State::ENCODING_ERROR)
  {
    m_error_label->hide();
    set_response_sensitive(Gtk::RESPONSE_ACCEPT, false);
  }
  else
  {
    // Allow the import button to be pressed when the value for the primary key
    // has been chosen:
    auto primary_key = get_field_primary_key_for_table(get_target_table_name());
    bool primary_key_selected = false;

    if(primary_key && !primary_key->get_auto_increment())
    {
      // If m_rows is empty, then no single line was read from the file yet,
      // and the m_fields array is not up to date. It is set in handle_line()
      // when the first line is parsed.
      primary_key_selected = false;
      if(!m_parser->get_rows_empty())
      {
        for(const auto& field : m_fields)
        {
          if(field == primary_key) //TODO: Is this just comparing shared_ptr?
          {
            primary_key_selected = true;
            break;
          }
        }
      }

      if(!primary_key_selected)
        m_error_label->set_markup(Glib::ustring::compose(_("One column needs to be assigned the table's primary key (<b>%1</b>) as target field before importing"), Glib::Markup::escape_text(primary_key->get_name())));
    }
    else
    {
      // auto_increment primary keys always work since their value is
      // assigned automatically.
      primary_key_selected = true;
    }

    set_response_sensitive(Gtk::RESPONSE_ACCEPT, primary_key_selected);
    if(primary_key_selected)
      m_error_label->hide();
    else
      m_error_label->show();
  }
}

void Dialog_Import_CSV::on_parser_file_read_error(const Glib::ustring& error_message)
{
  std::string filename;
  try
  {
    filename = Glib::filename_from_uri(m_file_uri);
  }
  catch(const Glib::ConvertError& ex)
  {
    std::cerr << G_STRFUNC << "Glib::filename_from_uri() failed: " << ex.what() << std::endl;

    show_error_dialog(_("Could Not Open file"),
      Glib::ustring::compose(_("The file at \"%1\" could not be opened: %2"), filename, error_message) );
  }
}

void Dialog_Import_CSV::on_parser_have_display_name(const Glib::ustring& display_name)
{
  set_title( Glib::ustring::compose(_("Import From CSV File: %1"), display_name) );
}

void Dialog_Import_CSV::on_parser_state_changed()
{
  //Remit (via our similarly-named signal) this so that the progress dialog can respond:
  signal_state_changed().emit();
}

Dialog_Import_CSV::type_signal_state_changed Dialog_Import_CSV::signal_state_changed() const
{
  return m_signal_state_changed;
}

} //namespace Glom
