#ifndef TEST_IMPORT_UTILS_H
#define TEST_IMPORT_UTILS_H

#include <glom/import_csv.h>
#include <iostream>

namespace ImportTests
{

bool check(const std::string& name, bool test, std::stringstream& report);

/// This takes a @a size argument so we can test parsing of null bytes.
void set_parser_contents(Glom::CsvParser& parser, const char* input, guint size);

} //namespace ImportTests
#endif
