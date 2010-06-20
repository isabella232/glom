#include <tests/import/utils.h>

namespace ImportTests
{

//The result just shows whether it finished before the timeout.
static bool finished_parsing = true;

bool check(const std::string& name, bool test, std::stringstream& report)
{
  if(!test)
    report << name << ": FAILED" << std::endl;

  return test;
}

// Returns the file name of the temporary created file, which will contain the buffer's contents.
static std::string create_file_from_buffer(const char* input, guint input_size)
{
  // Use Glib's file utilities to get a unique temporary filename:
  std::string tmp_filename;
  const int tmp_file_handle = Glib::file_open_tmp(tmp_filename, "glom_testdata");
  if(-1 < tmp_file_handle)
    close(tmp_file_handle);

  std::string file_uri;
  //TODO: Catch exception.
  file_uri = Glib::filename_to_uri(tmp_filename);
  
  Glib::RefPtr<Gio::File> file = Gio::File::create_for_uri(file_uri);
  
  gssize result = 0;
  //TODO: Catch exception.
  result = file->append_to()->write(input, input_size);
  g_return_val_if_fail(-1 < result, "");

  return file_uri;
}

static void on_mainloop_killed_by_watchdog(MainLoopRp mainloop)
{
  finished_parsing = false;
  //Quit the mainloop that we ran because the parser uses an idle handler.
  mainloop->quit();
}

static void on_parser_encoding_error(MainLoopRp mainloop)
{
  finished_parsing = true;
  //Quit the mainloop that we ran because the parser uses an idle handler.
  mainloop->quit();
}

static void on_parser_finished(MainLoopRp mainloop)
{
  finished_parsing = true;
  //Quit the mainloop that we ran because the parser uses an idle handler.
  mainloop->quit();
}

static void on_file_read_error(const std::string& /*unused*/, MainLoopRp mainloop)
{
  finished_parsing = true;
  //Quit the mainloop that we ran because the parser uses an idle handler.
  mainloop->quit();
}

bool run_parser_from_buffer(const FuncConnectParserSignals& connect_parser_signals, const std::string& input)
{
  return run_parser_from_buffer(connect_parser_signals, input.data(), input.size());
}

bool run_parser_from_buffer(const FuncConnectParserSignals& connect_parser_signals, const char* input, guint input_size)
{
  finished_parsing = true;

  //Start a mainloop because the parser uses an idle handler.
  //TODO: Stop the parser from doing that.
  MainLoopRp mainloop = Glib::MainLoop::create();
  Glom::CsvParser parser("UTF-8");

  parser.signal_encoding_error().connect(sigc::bind(&on_parser_encoding_error, mainloop));
  parser.signal_finished_parsing().connect(sigc::bind(&on_parser_finished, mainloop));

  // Install a watchdog for the mainloop. No test should need longer than 300
  // seconds. Also, we need to avoid being stuck in the mainloop.
  // Infinitely running tests are useless.
  mainloop->get_context()->signal_timeout().connect_seconds_once(sigc::bind(&on_mainloop_killed_by_watchdog, mainloop), 300);

  connect_parser_signals(parser);

  const std::string file_uri = create_file_from_buffer(input, input_size);
  parser.set_file_and_start_parsing(file_uri);
  if (Glom::CsvParser::STATE_PARSING != parser.get_state())
    return false;

  mainloop->run();

  Glib::RefPtr<Gio::File> file = Gio::File::create_for_uri(file_uri);

  //TODO: Catch exception.
  const bool removed = file->remove();
  g_assert(removed);

  return finished_parsing;
}

bool run_parser_on_file(const FuncConnectParserSignals& connect_parser_signals, const std::string &uri)
{
  finished_parsing = true;

  //Start a mainloop because the parser uses an idle handler.
  //TODO: Stop the parser from doing that.
  MainLoopRp mainloop = Glib::MainLoop::create();
  Glom::CsvParser parser("UTF-8");

  parser.signal_encoding_error().connect(sigc::bind(&on_parser_encoding_error, mainloop));
  parser.signal_finished_parsing().connect(sigc::bind(&on_parser_finished, mainloop));
  parser.signal_file_read_error().connect(sigc::bind(&on_file_read_error, mainloop));

  // Install a watchdog for the mainloop. No test should need longer than 300
  // seconds. Also, we need to avoid being stuck in the mainloop.
  // Infinitely running tests are useless.
  mainloop->get_context()->signal_timeout().connect_seconds_once(sigc::bind(&on_mainloop_killed_by_watchdog, mainloop), 300);

  connect_parser_signals(parser);

  parser.set_file_and_start_parsing(uri);
  if (Glom::CsvParser::STATE_PARSING != parser.get_state())
    return false;

  mainloop->run();

  return finished_parsing;
}

} //namespace ImportTests
