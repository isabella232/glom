#include <tests/import/utils.h>

namespace ImportTests
{

bool check(const std::string& name, bool test, std::stringstream& report)
{
  if(!test)
    report << name << ": FAILED" << std::endl;

  return test;
}

void set_parser_contents(Glom::CsvParser& /*parser*/, const char* /*input*/, guint /*size*/)
{
  // Do not read terminating null byte.
  //parser.m_raw = std::vector<char>(input, input + size -1);
}

} //namespace ImportTests

