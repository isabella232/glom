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

#include "dialog_import_csv.h"
#include <libglom/libglom_config.h>

#include <libglom/data_structure/glomconversions.h>

#include <gtkmm/messagedialog.h>
#include <gtkmm/cellrenderercombo.h>
#include <glom/utils_ui.h>
#include <glibmm/i18n.h>
#include <cerrno>

// On Windows, "iconv" seems to be a define for "libiconv", breaking the Glib::IConv::iconv() call.
#ifdef iconv
#undef iconv
#endif

namespace
{

const gunichar DELIMITER = ',';
const gunichar QUOTE = '\"';

static bool next_char_is_quote(const Glib::ustring::const_iterator& iter, const Glib::ustring::const_iterator& end)
{
  if(iter == end)
    return false;
  
  //Look at the next character to see if it's really "" (an escaped "):
  Glib::ustring::const_iterator iter_next = iter;
  ++iter_next;
  if(iter_next != end)
  {
    const gunichar c_next = *iter_next;
    if(c_next == QUOTE)
    {
      return true;
    }
  }
  
  return false;
}

//Parse the field in a comma-separated line, returning the field including the quotes:
static Glib::ustring::const_iterator advance_field(const Glib::ustring::const_iterator& iter, const Glib::ustring::const_iterator& end, Glib::ustring& field)
{
  bool inside_quotes = false;
  //bool string_finished = false; //Ignore anything after "something", such as "something"else,

  field.clear();

  Glib::ustring::const_iterator walk;
  for(walk = iter; walk != end; ++walk)
  {
    const gunichar c = *walk;
    
    //if(string_finished)
    //  continue;

    if(inside_quotes)
    {
      // End of quoted string?
      if(c == QUOTE)
      {
        if(next_char_is_quote(walk, end))
        {
          //This is "" so it's not an end quote. Just add one quote:
          field += c;
          ++walk; //Skip the second ".
        }
        else
        {
          inside_quotes = false;
          //string_finished = true; //Ignore anything else before the next comma.
        }
        
        continue;
      }
    }
    else
    {
      // Start of quoted string:
      if((c == QUOTE))
      {
        inside_quotes = true;
        continue;
      }
      // End of field:
      else if(!inside_quotes && c == DELIMITER)
      {
        break;
      }
      
      continue;
    }

    field += c; // Just so that we don't need to iterate through the field again, since there is no Glib::ustring::substr(iter, iter)
  }

  // TODO: Throw error if still inside a quoted string?
  std::cout << "debug: field=" << field << std::endl;
  return walk;
}

struct Encoding {
  const char* name;
  const char* charset;
};

//TODO: Can we get this from anywhere else, such as iso-codes? murrayc
const Encoding ENCODINGS[] = {
  { N_("Unicode"), "UTF-8" },
  { N_("Unicode"), "UTF-16" },
  { N_("Unicode"), "UTF-16BE" },
  { N_("Unicode"), "UTF-16LE" },
  { N_("Unicode"), "UTF-32" },
  { N_("Unicode"), "UTF-7" },
  { N_("Unicode"), "UCS-2" },
  { N_("Unicode"), "UCS-4" },
  { NULL, NULL }, // This just adds a separator in the combo box
  { N_("Western"), "ISO-8859-1" },
  { N_("Central European"), "ISO-8859-2" },
  { N_("South European"), "ISO-8859-3" },
  { N_("Baltic"), "ISO-8859-4" },
  { N_("Cyrillic"), "ISO-8859-5" },
  { N_("Arabic"), "ISO-8859-6" },
  { N_("Greek"), "ISO-8859-7" },
  { N_("Hebrew Visual"), "ISO-8859-8" },
  { N_("Hebrew"), "ISO-8859-8-I" },
  { N_("Turkish"), "ISO-8859-9" },
  { N_("Nordic"), "ISO-8859-10" },
  { N_("Baltic"), "ISO-8859-13" },
  { N_("Celtic"), "ISO-8859-14" },
  { N_("Western"), "ISO-8859-15" },
  { N_("Romanian"), "ISO-8859-16" },
  { NULL, NULL },
  { N_("Central European"), "WINDOWS-1250" },
  { N_("Cyrillic"), "WINDOWS-1251" },
  { N_("Western"), "WINDOWS-1252" },
  { N_("Greek"), "WINDOWS-1253" },
  { N_("Turkish"), "WINDOWS-1254" },
  { N_("Hebrew"), "WINDOWS-1255" },
  { N_("Arabic"), "WINDOWS-1256" },
  { N_("Baltic"), "WINDOWS-1257" },
  { N_("Vietnamese"), "WINDOWS-1258" }
};

// When auto-detecting the encoding, we try to read the file in these
// encodings, in order:
const Encoding AUTODETECT_ENCODINGS[] = {
  { N_("Unicode"), "UTF-8" },
  { N_("Western"), "ISO-8859-1" },
  { N_("Western"), "ISO-8859-15" },
  { N_("Unicode"), "UTF-16" },
  { N_("Unicode"), "UCS-2" },
  { N_("Unicode"), "UCS-4" }
};

const guint N_ENCODINGS = sizeof(ENCODINGS)/sizeof(ENCODINGS[0]);
const guint N_AUTODETECT_ENCODINGS = sizeof(AUTODETECT_ENCODINGS)/sizeof(AUTODETECT_ENCODINGS[0]);

Glib::ustring encoding_display(const Glib::ustring& name, const Glib::ustring& charset)
{
  if(charset.empty())
    return name;
  else
    return name + " (" + charset + ")";
}

} //anonymous namespace

namespace Glom
{

Dialog_Import_CSV::Dialog_Import_CSV(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Dialog(cobject), m_auto_detect_encoding(0), m_state(NONE)
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
    throw std::runtime_error("Missing widgets from glade file for Dialog_Import_CSV");

  m_encoding_model = Gtk::ListStore::create(m_encoding_columns);

  Gtk::TreeModel::iterator iter = m_encoding_model->append();
  (*iter)[m_encoding_columns.m_col_name] = _("Auto Detect");

  // Separator:
  m_encoding_model->append();

  for(guint i = 0; i < N_ENCODINGS; ++ i)
  {
    iter = m_encoding_model->append();
    if(ENCODINGS[i].name != NULL)
    {
      (*iter)[m_encoding_columns.m_col_name] = _(ENCODINGS[i].name);
      (*iter)[m_encoding_columns.m_col_charset] = ENCODINGS[i].charset;
    }
  }

  Gtk::CellRendererText* renderer = Gtk::manage(new Gtk::CellRendererText);
  m_encoding_combo->set_model(m_encoding_model);
  m_encoding_combo->pack_start(*renderer);
  m_encoding_combo->set_cell_data_func(*renderer, sigc::bind(sigc::mem_fun(*this, &Dialog_Import_CSV::encoding_data_func), sigc::ref(*renderer)));
  m_encoding_combo->set_row_separator_func(sigc::mem_fun(*this, &Dialog_Import_CSV::row_separator_func));
  m_encoding_combo->set_active(0);
  m_encoding_combo->signal_changed().connect(sigc::mem_fun(*this, &Dialog_Import_CSV::on_encoding_changed));
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
  const Glib::ustring date_text = Glom::Conversions::format_date(the_c_time, std::locale() /* ignored */, true /* iso_format */);
  const Glib::ustring advice = Glib::ustring::compose(_("Note that the source file should contain numbers and dates in international ISO format. For instance, 22nd November 2008 should be %1."), date_text);
  m_advice_label->set_text(advice);
  std::cout << "DEBUG: advice=" << advice << std::endl;

  clear();
}

void Dialog_Import_CSV::import(const Glib::ustring& uri, const Glib::ustring& into_table)
{
  clear();

  Document* document = get_document();
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
    Gtk::TreeModel::iterator tree_iter = m_field_model->append();
    (*tree_iter)[m_field_columns.m_col_field_name] = _("<None>");

    const Document::type_vec_fields fields(document->get_table_fields(into_table));
    for(Document::type_vec_fields::const_iterator iter = fields.begin(); iter != fields.end(); ++ iter)
    {
      sharedptr<Field> field = *iter;
      if(!field)
        continue;

      // Don't allow the primary key to be selected when it is an auto
      // increment key, since the value for the primary key is chosen
      // automatically anyway.
      if(!field->get_primary_key() || !field->get_auto_increment())
      {
        Gtk::TreeModel::iterator tree_iter = m_field_model->append();
        (*tree_iter)[m_field_columns.m_col_field] = *iter;
        (*tree_iter)[m_field_columns.m_col_field_name] = (*iter)->get_name();
      }
    }

    // Create the sorted version of this model, 
    // so the user sees the fields in alphabetical order:
    m_field_model_sorted = Gtk::TreeModelSort::create(m_field_model);
    m_field_model_sorted->set_sort_column(m_field_columns.m_col_field_name, Gtk::SORT_ASCENDING);


    m_file = Gio::File::create_for_uri(uri);
    m_file->read_async(sigc::mem_fun(*this, &Dialog_Import_CSV::on_file_read));

    // Query the display name of the file to set in the title:
    m_file->query_info_async(sigc::mem_fun(*this, &Dialog_Import_CSV::on_query_info), G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME);

    set_state(PARSING);
  }
}

guint Dialog_Import_CSV::get_row_count() const
{
  if(m_first_line_as_title->get_active() && m_rows.size() > 1)
    return m_rows.size() - 1;

  return m_rows.size();
}

guint Dialog_Import_CSV::get_column_count() const
{
  return m_fields.size();
}

const sharedptr<Field>& Dialog_Import_CSV::get_field_for_column(guint col)
{
  return m_fields[col];
}

const Glib::ustring& Dialog_Import_CSV::get_data(guint row, guint col)
{
  if(m_first_line_as_title->get_active())
    ++row;

  return m_rows[row][col];
}

void Dialog_Import_CSV::clear()
{
  // TODO: Do we explicitely need to cancel async operations?
  // TODO: Disconnect idle handlers
  m_sample_model.reset();
  m_sample_view->remove_all_columns();
  m_sample_view->set_model(m_sample_model);
  m_field_model.reset();
  m_field_model_sorted.reset();
  m_rows.clear();
  m_fields.clear();
  m_file.reset();
  m_stream.reset();
  m_filename.clear();
  m_buffer.reset(NULL);
  m_raw.clear();
  m_parser.reset(NULL);

  m_encoding_info->set_text("");
  m_sample_view->set_sensitive(false);
  m_encoding_combo->set_sensitive(false);
  m_first_line_as_title->set_sensitive(false);
  m_sample_rows->set_sensitive(false);

  set_state(NONE);
  validate_primary_key();
}

void Dialog_Import_CSV::show_error_dialog(const Glib::ustring&, const Glib::ustring& secondary)
{
  Utils::show_ok_dialog(_("Error Importing CSV File"),
     secondary, *this, Gtk::MESSAGE_ERROR);
}

void Dialog_Import_CSV::encoding_data_func(const Gtk::TreeModel::iterator& iter, Gtk::CellRendererText& renderer)
{
  const Glib::ustring name = (*iter)[m_encoding_columns.m_col_name];
  const Glib::ustring charset = (*iter)[m_encoding_columns.m_col_charset];

  renderer.set_property("text", encoding_display(name, charset));
}

bool Dialog_Import_CSV::row_separator_func(const Glib::RefPtr<Gtk::TreeModel>& /* model */, const Gtk::TreeModel::iterator& iter) const
{
  return (*iter)[m_encoding_columns.m_col_name] == "";
}

void Dialog_Import_CSV::on_query_info(const Glib::RefPtr<Gio::AsyncResult>& result)
{
  try
  {
    Glib::RefPtr<Gio::FileInfo> info = m_file->query_info_finish(result);
    m_filename = info->get_display_name();
    set_title(m_filename + _(" - Import From CSV File"));
  }
  catch(const Glib::Exception& ex)
  {
    std::cerr << "Failed to fetch display name of uri " << m_file->get_uri() << ": " << ex.what() << std::endl;
  }
}

void Dialog_Import_CSV::on_file_read(const Glib::RefPtr<Gio::AsyncResult>& result)
{
  try
  {
    m_stream = m_file->read_finish(result);

    m_buffer.reset(new Buffer);
    m_stream->read_async(m_buffer->buf, sizeof(m_buffer->buf), sigc::mem_fun(*this, &Dialog_Import_CSV::on_stream_read));
  }
  catch(const Glib::Exception& error)
  {
    show_error_dialog(_("Could Not Open file"), Glib::ustring::compose(_("The file at \"%1\" could not be opened: %2"), m_file->get_uri(), error.what()));
    clear();
    // TODO: Response?
  }
}

void Dialog_Import_CSV::on_stream_read(const Glib::RefPtr<Gio::AsyncResult>& result)
{
  try
  {
    const gssize size = m_stream->read_finish(result);
    m_raw.insert(m_raw.end(), m_buffer->buf, m_buffer->buf + size);

    // If the parser already exists, but it is currently not parsing because it waits
    // for new input, then continue parsing.
    if(m_parser.get() && !m_parser->idle_connection.connected())
    {
      m_parser->idle_connection = Glib::signal_idle().connect(sigc::mem_fun(*this, &Dialog_Import_CSV::on_idle_parse));
    }
    // If the parser does not exist yet, then create a new parser, except when the
    // current encoding does not work for the file ,in which case the user must first
    // choose another encoding.
    else if(!m_parser.get() && m_state != ENCODING_ERROR)
    {
      begin_parse();
    }

    if(size > 0)
    {
      // Read the next few bytes
      m_stream->read_async(m_buffer->buf, sizeof(m_buffer->buf), sigc::mem_fun(*this, &Dialog_Import_CSV::on_stream_read));
    }
    else
    {
      // Finished reading
      m_buffer.reset(NULL);
      m_stream.reset();
      m_file.reset();
    }
  }
  catch(const Glib::Exception& error)
  {
    show_error_dialog(_("Could Not Read File"), Glib::ustring::compose(_("The file at \"%1\" could not be read: %2"), m_file->get_uri(), error.what()));
    clear();
    // TODO: Response?
  }
}

void Dialog_Import_CSV::on_encoding_changed()
{
  // Reset current parsing process
  m_parser.reset(NULL);

  int active = m_encoding_combo->get_active_row_number();
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

  // Parse from beginning with new encoding if we have already some data to
  // parse.
  if(!m_raw.empty())
    begin_parse();
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
    Gtk::TreeModel::iterator iter = m_sample_model->get_iter(path);

    // Remove the first row from the view
    if(iter && (*iter)[m_sample_columns.m_col_row] == 0)
    {
      m_sample_model->erase(iter);

      // Add another row to the end, if one is loaded.
      const guint last_index = m_sample_model->children().size();
      if(last_index < m_rows.size())
      {
        iter = m_sample_model->append();
        (*iter)[m_sample_columns.m_col_row] = last_index;
      }
    }
  }
  else
  {
    m_sample_view->set_headers_visible(false);

    // Check whether row 0 is displayed
    Gtk::TreeModel::Path path("1");
    Gtk::TreeModel::iterator iter = m_sample_model->get_iter(path);

    if((!iter || (*iter)[m_sample_columns.m_col_row] != 0) && !m_rows.empty() && m_sample_rows->get_value_as_int() > 0)
    {
      // Add first row to model
      if(!iter)
        iter = m_sample_model->append();
      else
        iter = m_sample_model->insert(iter);

      (*iter)[m_sample_columns.m_col_row] = 0;

      // Remove last row if we hit the limit
      const guint sample_rows = m_sample_model->children().size() - 1;
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

  const guint current_sample_rows = m_sample_model->children().size() - 1;
  const guint new_sample_rows = m_sample_rows->get_value_as_int();

  if(current_sample_rows > new_sample_rows)
  {
    // +1 for the "target field" row
    Gtk::TreeModel::Path path(1);
    path[0] = new_sample_rows + 1;
    Gtk::TreeModel::iterator iter = m_sample_model->get_iter(path);

    while(iter != m_sample_model->children().end())
      iter = m_sample_model->erase(iter);
  }
  else
  {
    // Find index of first row to add
    guint row_index = current_sample_rows;
    if(m_first_line_as_title->get_active())
      ++row_index;

    for(guint i = current_sample_rows; i < new_sample_rows && row_index < m_rows.size(); ++i, ++row_index)
    {
      Gtk::TreeModel::iterator iter = m_sample_model->append();
      (*iter)[m_sample_columns.m_col_row] = row_index;
    }
  }
}

Glib::ustring Dialog_Import_CSV::get_current_encoding() const
{
  Gtk::TreeModel::iterator iter = m_encoding_combo->get_active();
  const Glib::ustring encoding = (*iter)[m_encoding_columns.m_col_charset];

  if(encoding.empty())
  {
    // Auto-Detect:
    g_assert(m_auto_detect_encoding != -1);
    return AUTODETECT_ENCODINGS[m_auto_detect_encoding].charset;
  }

  return encoding.c_str();
}

void Dialog_Import_CSV::begin_parse()
{
  if(m_auto_detect_encoding != -1)
    m_encoding_info->set_text(Glib::ustring::compose(_("Encoding detected as: %1"), encoding_display(gettext(AUTODETECT_ENCODINGS[m_auto_detect_encoding].name), AUTODETECT_ENCODINGS[m_auto_detect_encoding].charset)));
  else
    m_encoding_info->set_text("");

  // Clear sample preview since we reparse everything, perhaps with
  // another encoding.
  m_sample_model.reset();
  m_sample_view->remove_all_columns();
  m_sample_view->set_model(m_sample_model); // Empty model
  m_rows.clear();

  m_parser.reset(new Parser(get_current_encoding().c_str()));
  set_state(PARSING);

  // Allow the Import button to be pressed when a field for the primary key
  // field is set. When the import button is pressed without the file being
  // fully loaded, the import progress waits for us to load the rest.
  validate_primary_key();

  m_parser->idle_connection = Glib::signal_idle().connect(sigc::mem_fun(*this, &Dialog_Import_CSV::on_idle_parse));
}

void Dialog_Import_CSV::encoding_error()
{
  m_parser.reset(NULL);
  // Clear sample preview (TODO: Let it visible, and only remove when reparsing?)
  m_sample_model.reset();
  m_sample_view->remove_all_columns();
  m_sample_view->set_model(m_sample_model); // Empty model
  m_rows.clear();

  set_state(ENCODING_ERROR);

  // Don't allow the import button to be pressed when an error occured. This
  // would not make sense since we cleared all the parsed row data anyway.
  validate_primary_key();

  // If we are auto-detecting the encoding, then try the next one
  if(m_auto_detect_encoding != -1)
  {
    ++ m_auto_detect_encoding;
    if(static_cast<guint>(m_auto_detect_encoding) < N_AUTODETECT_ENCODINGS)
      begin_parse();
    else
      m_encoding_info->set_text(_("Encoding detection failed. Please manually choose one from the box."));
  }
  else
  {
    m_encoding_info->set_text(_("The file contains data not in the specified encoding. Please choose another one, or try \"Auto Detect\"."));
  }
}

bool Dialog_Import_CSV::on_idle_parse()
{
  // The amount of bytes to process in one pass of the idle handler:
  static const guint CONVERT_BUFFER_SIZE = 1024;

  const char* inbuffer = &m_raw[m_parser->input_position];
  char* inbuf = const_cast<char*>(inbuffer);
  gsize inbytes = m_raw.size() - m_parser->input_position;
  char outbuffer[CONVERT_BUFFER_SIZE];
  char* outbuf = outbuffer;
  gsize outbytes = CONVERT_BUFFER_SIZE;

  const std::size_t result = m_parser->conv.iconv(&inbuf, &inbytes, &outbuf, &outbytes);
  bool more_to_process = (inbytes != 0);

  if(result == static_cast<size_t>(-1))
  {
    if(errno == EILSEQ)
    {
      // Invalid text in the current encoding.
      encoding_error();
      return false;
    }

    // If EINVAL is set, this means that an incomplete multibyte sequence was at
    // the end of the input. We might have some more bytes, but those do not make
    // up a whole character, so we need to wait for more input.
    if(errno == EINVAL)
    {
      if(!m_stream)
      {
        // This means that we already reached the end of the file. The file
        // should not end with an incomplete multibyte sequence.
        encoding_error();
        return false;
      }
      else
      {
        more_to_process = false;
      }
    }
  }

  m_parser->input_position += (inbuf - inbuffer);

  // We now have outbuf - outbuffer bytes of valid UTF-8 in outbuffer.
  const char* prev_line_end = outbuffer;
  const char* prev = prev_line_end;

  //Identify the record rows in the .cvs file.
  //We can't just search for newlines because they may be inside quotes too. 
  //TODO: Use a regex instead, to more easily handle quotes?
  bool in_quotes = false;
  while(true)
  {
    //Note that, unlike std::string::find*, std::find* returns an iterator (char*), not a position.
    //It returns outbuf if none is found.
    const char newline_to_find[] = { '\r', '\n', '\0' };
    const char* pos_newline = std::find_first_of<const char*>(prev, outbuf, newline_to_find, newline_to_find + sizeof(newline_to_find));
    
    const char quote_to_find[] = {(char)QUOTE};
    const char* pos_quote = std::find_first_of<const char*>(prev, outbuf, quote_to_find, quote_to_find + sizeof(quote_to_find));
    
    //Examine the first character (quote or newline) that was found:
    const char* pos = pos_newline;
    if((pos_quote != outbuf) && pos_quote < pos)
      pos = pos_quote;
      
    if(pos == outbuf)
      break;
    
    char ch = *pos;   
    
    if(ch == '\0')
    {
      // There is a null byte in the conversion. Because normal text files don't
      // contain null bytes this only occurs when converting, for example, a UTF-16
      // file from ISO-8859-1 to UTF-8 (note that the UTF-16 file is valid ISO-8859-1 - 
      // it just contains lots of nullbytes). We therefore produce an error here.
      encoding_error();
      return false;
    }
    else if(in_quotes)
    {
      //Ignore newlines inside quotes.
      
      //End quote:
      if(ch == (char)QUOTE)
        in_quotes = false;
        
      prev = pos + 1;
      continue;
    }
    else
    {
      //Start quote:
      if(ch == (char)QUOTE)
      {
        in_quotes = true;
        prev = pos + 1;
        continue;
      }
    
      //Found a newline (outside of quotes) that marks the end of the line:
      m_parser->current_line.append(prev_line_end, pos - prev_line_end);
      ++(m_parser->line_number);
      
      if(!m_parser->current_line.empty())
        handle_line(m_parser->current_line, m_parser->line_number);

      m_parser->current_line.clear();

      // Skip linebreak
      prev = pos + 1;

      // Skip DOS-style linebreak (\r\n)
      if(ch == '\r' 
         && prev != outbuf && *prev == '\n')
      {
         ++prev;
      }
      
      prev_line_end = prev;
    }
  }

  // Append last chunk of this line
  m_parser->current_line.append(prev, outbuf - prev);
  if(!m_stream && m_raw.size() == m_parser->input_position)
  {
    ++(m_parser->line_number);
    
    // Handle last line, if nonempty
    if(m_parser->current_line.empty())
      handle_line(m_parser->current_line, m_parser->line_number);

    // Parsed whole file, done
    m_parser.reset(NULL);
    set_state(PARSED);
  }

  // Continue if there are more bytes to process
  return more_to_process;
}

void Dialog_Import_CSV::handle_line(const Glib::ustring& line, guint line_number)
{
  std::cout << "debug: handle_line=" << line << std::endl;
  if(line.empty())
   return;

  m_rows.push_back(type_row_strings());
  type_row_strings& row = m_rows.back();

  Glib::ustring field;
  //Gtk::TreeModelColumnRecord record;

  // Parse first field:
  Glib::ustring::const_iterator iter = advance_field(line.begin(), line.end(), field);
  row.push_back(field);

  // Parse more fields:
  while(iter != line.end())
  {
    // Skip delimiter:
    ++ iter;

    // Read field:
    iter = advance_field(iter, line.end(), field);

    // Add field to current row:
    row.push_back(field);
  }

  if(!m_sample_model)
  {
    // This is the first line read if there is no model yet:
    m_sample_model = Gtk::ListStore::create(m_sample_columns);
    m_sample_view->set_model(m_sample_model);

    // Create field vector that contains the fields into which to import
    // the data.
    m_fields.resize(row.size());

    Gtk::CellRendererText* text = Gtk::manage(new Gtk::CellRendererText);
    Gtk::TreeViewColumn* col = Gtk::manage(new Gtk::TreeViewColumn(_("Line")));
    col->pack_start(*text, false);
    col->set_cell_data_func(*text, sigc::mem_fun(*this, &Dialog_Import_CSV::line_data_func));
    m_sample_view->append_column(*col);

    for(guint i = 0; i < row.size(); ++ i)
    {
      Gtk::CellRendererCombo* cell = Gtk::manage(new Gtk::CellRendererCombo);
      Gtk::TreeViewColumn* col = Gtk::manage(new Gtk::TreeViewColumn(row[i]));
      col->pack_start(*cell, true);
      col->set_cell_data_func(*cell, sigc::bind(sigc::mem_fun(*this, &Dialog_Import_CSV::field_data_func), i));
      col->set_sizing(Gtk::TREE_VIEW_COLUMN_AUTOSIZE);
      cell->property_model() = m_field_model_sorted;
      cell->property_text_column() = 0;
      cell->property_has_entry() = false;
      cell->signal_edited().connect(sigc::bind(sigc::mem_fun(*this, &Dialog_Import_CSV::on_field_edited), i));
      m_sample_view->append_column(*col);
    }

    Gtk::TreeModel::iterator iter = m_sample_model->append();
    // -1 means the row to select target fields (see special handling in cell data funcs)
    (*iter)[m_sample_columns.m_col_row] = -1;
  }

  // Add the row to the treeview if there are not yet as much sample rows
  // as the user has chosen (note the first row is to choose the target fields,
  // not a sample row, which is why we do -1 here).
  guint sample_rows = m_sample_model->children().size() - 1;

  // Don't add if this is the first line and m_first_line_as_title is active:
  if(line_number > 1 || !m_first_line_as_title->get_active())
  {
    if(sample_rows < static_cast<guint>(m_sample_rows->get_value_as_int()))
    {
      Gtk::TreeModel::iterator iter = m_sample_model->append();
      (*iter)[m_sample_columns.m_col_row] = m_rows.size() - 1;
    }
  }
}

void Dialog_Import_CSV::line_data_func(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter)
{
  const int row = (*iter)[m_sample_columns.m_col_row];
  Gtk::CellRendererText* renderer_text = dynamic_cast<Gtk::CellRendererText*>(renderer);
  if(!renderer_text)
    throw std::logic_error("CellRenderer is not a CellRendererText in line_data_func");

  if(row == -1)
    renderer_text->set_property("text", Glib::ustring(_("Target Field")));
  else
    renderer_text->set_property("text", Glib::ustring::compose("%1", row + 1));
}

void Dialog_Import_CSV::field_data_func(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter, guint column_number)
{
  const int row = (*iter)[m_sample_columns.m_col_row];
  Gtk::CellRendererCombo* renderer_combo = dynamic_cast<Gtk::CellRendererCombo*>(renderer);
  if(!renderer_combo) throw std::logic_error("CellRenderer is not a CellRendererCombo in field_data_func");

  Glib::ustring text;
  bool editable = false;
  
  if(row == -1)
  {
    sharedptr<Field> field = m_fields[column_number];
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
      sharedptr<Field> field = m_fields[column_number];
 
      if(row != -1 && (unsigned int)row < m_rows.size())
      {
        const type_row_strings& row_strings = m_rows[row];
        if(column_number < row_strings.size())
        {
      
          const Glib::ustring& orig_text = row_strings[column_number];

          if(field)
          {
            if(field->get_glom_type() != Field::TYPE_IMAGE)
            {
              /* Exported data is always stored in postgres format */
              bool success = false;
              const Gnome::Gda::Value value = field->from_file_format(orig_text, success);
        
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
  }
  
  renderer_combo->set_property("text", text);
  renderer_combo->set_property("editable", editable);
}

void Dialog_Import_CSV::on_field_edited(const Glib::ustring& path, const Glib::ustring& new_text, guint column_number)
{
  Gtk::TreeModel::Path treepath(path);
  Gtk::TreeModel::iterator iter = m_sample_model->get_iter(treepath);

  // Lookup field indicated by new_text
  const Gtk::TreeNodeChildren& children = m_field_model->children();
  for(Gtk::TreeModel::iterator field_iter = children.begin(); field_iter != children.end(); ++ field_iter)
  {
    if( (*field_iter)[m_field_columns.m_col_field_name] == new_text)
    {
      sharedptr<Field> field = (*field_iter)[m_field_columns.m_col_field];
      // Check whether another column is already using that field
      type_vec_fields::iterator vec_field_iter = std::find(m_fields.begin(), m_fields.end(), field);
      // Reset the old column since two different columns cannot be imported into the same field
      if(vec_field_iter != m_fields.end()) *vec_field_iter = sharedptr<Field>();

      m_fields[column_number] = field;
      
      // Update the rows, so they are redrawn, doing a conversion to the new type.
      const Gtk::TreeNodeChildren& sample_children = m_sample_model->children();
      // Create a TreeModel::Path with initial index 0. We need a TreeModel::Path for the row_changed() call
      Gtk::TreeModel::Path path("0");

      for(Gtk::TreeModel::iterator sample_iter = sample_children.begin(); sample_iter != sample_children.end(); ++ sample_iter)
      {
        if(sample_iter != iter)
          m_sample_model->row_changed(path, sample_iter);

        path.next();
      }

      validate_primary_key();
      break;
    }
  }
}

void Dialog_Import_CSV::set_state(State state)
{
  if(m_state != state)
  {
    m_state = state;
    m_signal_state_changed.emit();
  }
}

void Dialog_Import_CSV::validate_primary_key()
{
  if(m_state == NONE || m_state == ENCODING_ERROR)
  {
    m_error_label->hide();
    set_response_sensitive(Gtk::RESPONSE_ACCEPT, false);
  }
  else
  {
    // Allow the import button to be pressed when the value for the primary key
    // has been chosen:
    sharedptr<Field> primary_key = get_field_primary_key_for_table(get_target_table_name());
    bool primary_key_selected = false;

    if(primary_key && !primary_key->get_auto_increment())
    {
      // If m_rows is empty, then no single line was read from the file yet,
      // and the m_fields array is not up to date. It is set in handle_line()
      // when the first line is parsed.
      primary_key_selected = false;
      if(!m_rows.empty())
      {
        for(type_vec_fields::iterator iter = m_fields.begin(); iter != m_fields.end(); ++ iter)
        {
          if(*iter == primary_key)
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

} //namespace Glom

