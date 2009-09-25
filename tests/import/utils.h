#ifndef TEST_IMPORT_UTILS_H
#define TEST_IMPORT_UTILS_H

#include <glom/import_csv/csv_parser.h>
#include <iostream>
#include <unistd.h>
#include <errno.h>

namespace ImportTests
{

bool check(const std::string& name, bool test, std::stringstream& report);

/// This takes a @a size argument so we can test parsing of null bytes.
void set_parser_contents(Glom::CsvParser& parser, const char* input, guint size);

// Returns the file name of the temporary created file, which will contain the buffer's contents.
std::string create_file_from_buffer(const char* input, guint size);

bool run_parser_from_buffer(void (*connect_parser_signals)(Glom::CsvParser& parser), const char* input, guint size);

void on_mainloop_killed_by_watchdog();
Glib::RefPtr<Glib::MainLoop>& get_mainloop_instance();
bool& get_result_instance();

} //namespace ImportTests
#endif
