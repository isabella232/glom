#include <glom/import_csv.h>
#include <glibmm/regex.h>
#include <iostream>
#include <cstdlib>

namespace
{

typedef std::vector<std::string> Tokens;

Tokens& get_tokens_instance()
{
  static Tokens tokens;
  return tokens;
}


void on_line_scanned(const Glib::ustring& line, guint line_number);

void print_tokens()
{
  for(Tokens::const_iterator iter = get_tokens_instance().begin();
       iter != get_tokens_instance().end();
       ++iter)
  {
    std::cout << " [" << *iter << "] ";
  }

  std::cout << std::endl;
}

bool check_tokens(Glib::RefPtr<Glib::Regex> check)
{
  for(Tokens::const_iterator iter = get_tokens_instance().begin();
       iter != get_tokens_instance().end();
       ++iter)
  {
    if(!check->match(*iter)) return false;
  }

  return true;
}

void set_parser_contents(Glom::CsvParser& parser, const char* input, guint size)
{

  parser.m_raw = std::vector<char>(input, input + size);
}

void on_line_scanned(const Glib::ustring& line, guint /*line_number*/)
{
  Glib::ustring field;
  Glib::ustring::const_iterator line_iter(line.begin());

  while (line_iter != line.end())
  {
    line_iter = Glom::CsvParser::advance_field(line_iter, line.end(), field);
    get_tokens_instance().push_back(field);

    // Manually have to skip separators.
    if (',' == *line_iter)
    {
      get_tokens_instance().push_back(",");
      ++line_iter;
    }
  }
}

} // namespace

// Testcases
int main()
{
  Glom::CsvParser parser("UTF-8");
  parser.signal_line_scanned().connect(sigc::ptr_fun(&on_line_scanned));

  bool test_dquoted_string = false;
  bool test_skip_on_no_ending_newline = false;
  bool test_skip_on_no_quotes_around_token = false;
  bool test_skip_spaces_around_separators = false;
  bool test_fail_on_non_comma_separators = false;
  bool test_parse_newline_inside_quotes = false;

  std::stringstream results;

  // test_dquoted_string
  {
    const char raw_line[] = "\"a \"\"quoted\"\" token\",\"sans quotes\"\n";
    set_parser_contents(parser, raw_line, sizeof(raw_line));

    while(parser.on_idle_parse())
    {}

    results << "test_dquoted_string: "
            << (test_dquoted_string = check_tokens(Glib::Regex::create("^(a \"quoted\" token|,|sans quotes)$")))
            << std::endl;

    get_tokens_instance().clear();
    parser.clear();
  }

  // test_skip_on_no_ending_newline
  {
    const char raw_line[] = "\"this\",\"line\",\"will\",\"be\",\"skipped\"";
    set_parser_contents(parser, raw_line, sizeof(raw_line));

    while(parser.on_idle_parse())
    {}

    results << "test_skip_on_no_ending_newline: "
            << (test_skip_on_no_ending_newline = (get_tokens_instance().size() == 0))
            << std::endl;

    get_tokens_instance().clear();
    parser.clear();
  }

  // test_skip_on_no_quotes_around_token
  {
    const char raw_line[] = "this,line,contains,only,empty,tokens\n";
    set_parser_contents(parser, raw_line, sizeof(raw_line));

    while(parser.on_idle_parse())
    {}

    results << "test_skip_on_no_quotes_around_token: "
            << (test_skip_on_no_quotes_around_token = check_tokens(Glib::Regex::create("^(|,)$")))
            << std::endl;

    get_tokens_instance().clear();
    parser.clear();
  }

  // test_skip_spaces_around_separators
  {
    const char raw_line[] = "\"spaces\" , \"around\", \"separators\"\n";
    set_parser_contents(parser, raw_line, sizeof(raw_line));

    while(parser.on_idle_parse())
    {}

    results << "test_skip_spaces_around_separators: "
            << (test_skip_spaces_around_separators = (get_tokens_instance().size() == 5))
            << std::endl;

    get_tokens_instance().clear();
    parser.clear();
  }

  // test_fail_on_non_comma_separators
  {
    const char raw_line[] = "\"cannot\"\t\"tokenize\"\t\"this\"\n";
    set_parser_contents(parser, raw_line, sizeof(raw_line));

    while(parser.on_idle_parse())
    {}

    results << "test_fail_on_non_comma_separators: "
            << (test_fail_on_non_comma_separators = check_tokens(Glib::Regex::create("^cannottokenizethis$")))
            << std::endl;

    get_tokens_instance().clear();
    parser.clear();
  }

  // test_parse_newline_inside_quotes
  {
    const char raw_line[] = "\"cell with\nnewline\"\n\"token on next line\"";
    set_parser_contents(parser, raw_line, sizeof(raw_line));

    while(parser.on_idle_parse())
    {}

    results << "test_parse_newline_inside_quotes: "
            << (test_parse_newline_inside_quotes = check_tokens(Glib::Regex::create("^(cell with\nnewline|token on next line)$")))
            << std::endl;

    get_tokens_instance().clear();
    parser.clear();
  }

  std::cout << results.rdbuf();

  return (test_dquoted_string &&
          test_skip_on_no_ending_newline &&
          test_skip_on_no_quotes_around_token &&
          test_skip_spaces_around_separators &&
          test_fail_on_non_comma_separators &&
          test_parse_newline_inside_quotes) ? EXIT_SUCCESS
                                            : EXIT_FAILURE;
}
