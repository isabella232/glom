#include <glom/import_csv.h>
#include <glibmm/regex.h>
#include <iostream>
#include <stdexcept>
#include <cstdlib>

namespace {

typedef std::vector<std::string> Encodings;

/// This takes a @a size argument so we can test parsing of null bytes.
void set_parser_contents(Glom::CsvParser& parser, const char* input, guint size)
{
  // Do not read terminating null byte.
  parser.m_raw = std::vector<char>(input, input + size -1);
}

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

void print_signal_counts()
{
  std::cout << "lines scanned: " << get_line_scanned_count_instance() << std::endl;
  std::cout << "encoding errors: " << get_encoding_error_count_instance() << std::endl;
}

} // namespace

// Testcases
int main()
{
  Glom::CsvParser parser("UTF-8");
  parser.signal_line_scanned().connect(sigc::hide(sigc::hide(&on_line_scanned)));
  parser.signal_encoding_error().connect(sigc::ptr_fun(&on_encoding_error));

  bool test_ignore_quoted_newlines = false;
  bool test_ignore_empty_lines = false;
  bool test_wrong_encoding = false;
  bool test_incomplete_chars = false;

  std::stringstream results;

  // test_ignore_quoted_newlines
  {
    // 2 CSV lines, first one contains newlines inside quotes
    const char raw[] = "\"some\n quoted\r\n newlines\n\", \"token2\"\n\"token3\"\n";
    set_parser_contents(parser, raw, sizeof(raw));

    while(parser.on_idle_parse())
    {}

    results << "test_ignore_quoted_newlines: "
            << (test_ignore_quoted_newlines = (2 == get_line_scanned_count_instance()))
            << std::endl;

    reset_signal_counts();
    parser.clear();
  }

  // test_ignore_empty_lines
  {
    // 5 CSV lines, but only 2 contain data
    const char raw[] = "token1\n\n\n\ntoken2, token3\n";
    set_parser_contents(parser, raw, sizeof(raw));

    while(parser.on_idle_parse())
    {}

    results << "test_ignore_empty_lines: "
            << (test_ignore_empty_lines = (2 == get_line_scanned_count_instance() &&
                                           0 == get_encoding_error_count_instance()))
            << std::endl;

    reset_signal_counts();
    parser.clear();
  }

  // test_wrong_encoding
  {
    const char* const encoding_arr[] = {"UTF-8", "UCS-2"};
    Encodings encodings(encoding_arr, encoding_arr + G_N_ELEMENTS(encoding_arr));

    // An invalid Unicode sequence.
    const char raw[] = "\0xc0\0x00\n";
    set_parser_contents(parser, raw, sizeof(raw));

    for (Encodings::const_iterator iter = encodings.begin();
         iter != encodings.end();
         ++iter)
    {
      try
      {
        while(parser.on_idle_parse())
        {}

        parser.clear();
      }
      catch (Glib::ConvertError& exception)
      {
        std::cout << exception.what() << std::endl;
      }

      parser.set_encoding((*iter).c_str());
    }

    results << "test_wrong_encoding: "
            << (test_wrong_encoding = (2 == get_encoding_error_count_instance() &&
                                       0 == get_line_scanned_count_instance()))
            << std::endl;

    reset_signal_counts();
    parser.clear();
  }

  // test_incomplete_chars
  {
    // An incomplete Unicode sequence.
    const char raw[] = "\0xc0\n";
    set_parser_contents(parser, raw, sizeof(raw));

    while(parser.on_idle_parse())
    {}

    parser.clear();

    results << "test_incomplete_chars: "
            << (test_incomplete_chars = (1 == get_encoding_error_count_instance() &&
                                         0 == get_line_scanned_count_instance()))
            << std::endl;

    reset_signal_counts();
    parser.clear();
  }

  std::cout << results.rdbuf() << std::endl;
  return (test_ignore_quoted_newlines &&
          test_ignore_empty_lines &&
          test_wrong_encoding &&
          test_incomplete_chars
         ) ? EXIT_SUCCESS : EXIT_FAILURE;
}
