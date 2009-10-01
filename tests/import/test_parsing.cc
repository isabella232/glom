#include <glom/import_csv/csv_parser.h>
#include <tests/import/utils.h>
//#include <glibmm/regex.h>
#include <gtkmm.h>
#include <iostream>
#include <cstdlib>

namespace
{

typedef std::vector<std::string> type_tokens;

type_tokens& get_tokens_instance()
{
  static type_tokens type_tokens;
  return type_tokens;
}


void on_line_scanned(const std::vector<Glib::ustring>& row, guint /*line_number*/)
{
  //std::cout << "debug: on_line_scanned(): row.size()=" << row.size() << std::endl;

  for(std::vector<Glib::ustring>::const_iterator iter = row.begin();
      iter != row.end();
      ++iter)
  {
    //std::cout << "  debug: on_line_scanned(): item=" << *iter << std::endl;

    get_tokens_instance().push_back(*iter);
  }
}

void print_tokens()
{
  for(type_tokens::const_iterator iter = get_tokens_instance().begin();
      iter != get_tokens_instance().end();
      ++iter)
  {
    std::cout << " [" << *iter << "] ";
  }

  std::cout << std::endl;
}

bool check_tokens(const std::string& regex)
{
  Glib::RefPtr<Glib::Regex> check;

  #ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
    check = Glib::Regex::create(regex);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << "Glib::Regex::create() failed: " << ex.what() << std::endl;
    return false;
  }
  #else
  std::auto_ptr<Glib::Error> ex;
  check = Glib::Regex::create(regex, static_cast<Glib::RegexCompileFlags>(0), static_cast<Glib::RegexMatchFlags>(0), ex);
  if(ex.get())
  {
    std::cerr << "Glib::Regex::create() failed: " << ex->what() << std::endl;
    return false;
  }
  #endif

  if(!check && 0 == get_tokens_instance().size())
    return false;

  for(type_tokens::const_iterator iter = get_tokens_instance().begin();
       iter != get_tokens_instance().end();
       ++iter)
  {
    if(!check->match(*iter))
      return false;
  }

  return true;
}

void connect_signals(Glom::CsvParser& parser)
{
  parser.signal_line_scanned().connect(sigc::ptr_fun(&on_line_scanned));
  //parser.signal_encoding_error().connect(sigc::ptr_fun(&on_encoding_error));
}

} // namespace

// Testcases
int main(int argc, char* argv[])
{
  Glib::thread_init();
  Gtk::Main gtk(argc, argv);

  bool result = true;
  std::stringstream report;

  // test_dquoted_string
  {
    const char* raw = "\"a \"\"quoted\"\" token\",\"sans quotes\"\n";
    const bool finished_parsing = ImportTests::run_parser_from_buffer(&connect_signals, raw);

    const bool passed = (finished_parsing &&
                         check_tokens("^(a \"quoted\" token|sans quotes)$") &&
                         2 == get_tokens_instance().size());
    get_tokens_instance().clear();

    if(!ImportTests::check("test_dquoted_string", passed, report))
      result = false;
  }

  // test_skip_on_no_ending_newline
  {
    const char* raw = "\"token in first line\"\n\"2nd token\", \"but\", \"this\",\"line\",\"will\",\"be\",\"skipped\"";
    const bool finished_parsing = ImportTests::run_parser_from_buffer(&connect_signals, raw);

    const bool passed = (finished_parsing &&
                         check_tokens("token in first line") &&
                         1 == get_tokens_instance().size());
    get_tokens_instance().clear();

    if(!ImportTests::check("test_skip_on_no_ending_newline", passed, report))
      result = false;
  }

  // test_skip_on_no_quotes_around_token
  {
    const char* raw = "this,line,contains,only,empty,tokens\n";
    const bool finished_parsing = ImportTests::run_parser_from_buffer(&connect_signals, raw);

    const bool passed = (finished_parsing &&
                         check_tokens("^$") &&
                         6 == get_tokens_instance().size());
    get_tokens_instance().clear();

    if(!ImportTests::check("test_skip_on_no_quotes_around_token", passed, report))
      result = false;
  }

  // test_skip_spaces_around_separators
  {
    const char* raw = "\"spaces\" , \"around\", \"separators\"\n";
    const bool finished_parsing = ImportTests::run_parser_from_buffer(&connect_signals, raw);

    const bool passed = (finished_parsing &&
                         check_tokens("^(spaces|around|separators)$") &&
                         3 == get_tokens_instance().size());
    get_tokens_instance().clear();

    if(!ImportTests::check("test_skip_spaces_around_separators", passed, report))
      result = false;

  }

  // test_fail_on_non_comma_separators
  {
    const char* raw = "\"cannot\"\t\"tokenize\"\t\"this\"\n";
    const bool finished_parsing = ImportTests::run_parser_from_buffer(&connect_signals, raw);

    const bool passed = (finished_parsing &&
                         check_tokens("^cannottokenizethis$") &&
                         1 == get_tokens_instance().size());
    get_tokens_instance().clear();

    if(!ImportTests::check("test_fail_on_non_comma_separators", passed, report))
      result = false;
  }

  // test_parse_newline_inside_quotes
  {
    const char* raw = "\"cell with\nnewline\"\n\"token on next line\"\n";
    const bool finished_parsing = ImportTests::run_parser_from_buffer(&connect_signals, raw);

    const bool passed = (finished_parsing &&
                         check_tokens("^(cell with\nnewline|token on next line)$") &&
                         2 == get_tokens_instance().size());
    get_tokens_instance().clear();

    if(!ImportTests::check("test_parse_newline_inside_quotes", passed, report))
      result = false;
  }

  // test_fail_on_non_matching_quotes
  {
    const char* raw = "\"token\"\nthis quote has no partner\",\"token\"\n";
    const bool finished_parsing = ImportTests::run_parser_from_buffer(&connect_signals, raw);

    const bool passed = (finished_parsing &&
                         check_tokens("token") &&
                         1 == get_tokens_instance().size());
    get_tokens_instance().clear();

    if(!ImportTests::check("test_fail_on_non_matching_quotes", passed, report))
      result = false;
  }

  if(!result)
    std::cout << report.rdbuf() << std::endl;

  return result ? EXIT_SUCCESS : EXIT_FAILURE;
}

