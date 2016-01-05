#include <glom/import_csv/csv_parser.h>
#include <tests/import/utils.h>
#include <giomm/file.h>
#include <glibmm/init.h>
#include <giomm/init.h>
#include <iostream>
#include <stdexcept>
#include <cstdlib>

namespace {

typedef std::vector<std::string> type_encodings;

guint& get_line_scanned_count_instance()
{
  static guint line_scanned_count = 0;
  return line_scanned_count;
}

guint& get_encoding_error_count_instance()
{
  static guint encoding_error_count = 0;
  return encoding_error_count;
}

void on_line_scanned()
{
  ++(get_line_scanned_count_instance());
}

void on_encoding_error()
{
  ++(get_encoding_error_count_instance());
}

void reset_signal_counts()
{
  get_line_scanned_count_instance() = 0;
  get_encoding_error_count_instance() = 0;
}

/*
void print_signal_counts()
{
  std::cout << "lines scanned: " << get_line_scanned_count_instance() << std::endl;
  std::cout << "encoding errors: " << get_encoding_error_count_instance() << std::endl;
}
*/

void connect_signals(Glom::CsvParser& parser)
{
  parser.signal_line_scanned().connect(sigc::hide(sigc::hide(&on_line_scanned)));
  parser.signal_encoding_error().connect(sigc::ptr_fun(&on_encoding_error));
}

} // namespace

// Testcases
int main()
{
  //Threading is always enabled starting from GLib 2.31.0:
  //Glib::thread_init();

  Glib::init();
  Gio::init();

  bool result = true;
  std::stringstream report;

  // test_ignore_quoted_newlines
  {
    // 2 CSV lines, first one contains newlines inside quotes
    const char* raw = "\"some\n quoted\r\n newlines\n\", \"token2\"\n\"token3\"\n";
    const auto finished_parsing = ImportTests::run_parser_from_buffer(&connect_signals, raw);

    const bool passed = (finished_parsing &&
                   2 == get_line_scanned_count_instance() &&
                   0 == get_encoding_error_count_instance());

    if(!ImportTests::check("test_ignore_quoted_newlines", passed, report))
      result = false;

    reset_signal_counts();
  }

  // test_ignore_empty_lines
  {
    // 5 CSV lines, but only 2 contain data
    const char* raw = "token1\n\n\n\ntoken2, token3\n";
    const auto finished_parsing = ImportTests::run_parser_from_buffer(&connect_signals, raw);

    const bool passed = (finished_parsing &&
                   2 == get_line_scanned_count_instance() &&
                   0 == get_encoding_error_count_instance());

    if(!ImportTests::check("test_ignore_empty_lines", passed, report))
      result = false;

    reset_signal_counts();
  }

// TODO: Cannot currently run this test in a sane fashion, fix me!
/*
  {
    const char* const encoding_arr[] = {"UTF-8", "UCS-2"};
    type_encodings encodings(encoding_arr, encoding_arr + G_N_ELEMENTS(encoding_arr));

    // An invalid Unicode sequence.
    const char* raw = "\0xc0\0x00\n";
    ImportTests::set_parser_contents(parser, raw);

    for (auto iter = encodings.begin();
         iter != encodings.end();
         ++iter)
    {
      try
      {
        while(parser.on_idle_parse())
        {}

        parser.clear();
      }
      catch(const Glib::ConvertError& exception)
      {
        std::cout << exception.what() << std::endl;
      }

      parser.set_encoding((*iter));
    }


    const bool passed = (2 == get_encoding_error_count_instance() &&
                   0 == get_line_scanned_count_instance());

    if(!ImportTests::check("test_wrong_encoding", passed, report))
      result = false;

    reset_signal_counts();
    parser.clear();
  }
*/

  // test_incomplete_chars
  {
    // An incomplete Unicode sequence.
    const char raw[] = "\0xc0\n";
    const auto finished_parsing = ImportTests::run_parser_from_buffer(&connect_signals, raw, G_N_ELEMENTS(raw));

    const bool passed = (finished_parsing &&
                   1 == get_encoding_error_count_instance() &&
                   0 == get_line_scanned_count_instance());

    if(!ImportTests::check("test_incomplete_chars", passed, report))
      result = false;

    reset_signal_counts();
  }

  if(!result)
    std::cout << report.rdbuf() << std::endl;

  return result ? EXIT_SUCCESS : EXIT_FAILURE;
}


