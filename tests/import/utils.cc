#include <tests/import/utils.h>
#include <libglom/utils.h>
#include <glibmm/convert.h>
#include <glibmm/fileutils.h>
#include <glibmm/main.h>

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

// Returns the file URI of the temporary created file, which will contain the buffer's contents.
static Glib::ustring create_file_from_buffer(const char* input, guint input_size)
{
  const auto file_uri = Glom::Utils::get_temp_file_uri("glom_import_testdata");
  if(file_uri.empty())
  {
    std::cerr << G_STRFUNC << ": file_uri was empty." << std::endl;
    return std::string();
  }

  auto file = Gio::File::create_for_uri(file_uri);

  gssize result = 0;
 
  try
  {
    result = file->append_to()->write(input, input_size);
    g_return_val_if_fail(-1 < result, "");
  }
  catch(const Gio::Error& ex)
  {
    std::cerr << G_STRFUNC << ": Gio::File::write() failed: " << ex.what() << std::endl;
    return std::string();
  }

  return file_uri;
}

static void on_mainloop_killed_by_watchdog(const Glib::RefPtr<Glib::MainLoop>& mainloop)
{
  finished_parsing = false;
  //Quit the mainloop that we ran because the parser uses an idle handler.
  mainloop->quit();
}

static void on_parser_encoding_error(const Glib::RefPtr<Glib::MainLoop>& mainloop)
{
  finished_parsing = true;
  //Quit the mainloop that we ran because the parser uses an idle handler.
  mainloop->quit();
}

static void on_parser_finished(const Glib::RefPtr<Glib::MainLoop>& mainloop)
{
  finished_parsing = true;
  //Quit the mainloop that we ran because the parser uses an idle handler.
  mainloop->quit();
}

static void on_file_read_error(const std::string& /*unused*/, const Glib::RefPtr<Glib::MainLoop>& mainloop)
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
  auto mainloop = Glib::MainLoop::create();
  Glom::CsvParser parser("UTF-8");

  parser.signal_encoding_error().connect(sigc::bind(&on_parser_encoding_error, mainloop));
  parser.signal_finished_parsing().connect(sigc::bind(&on_parser_finished, mainloop));

  // Install a watchdog for the mainloop. No test should need longer than 300
  // seconds. Also, we need to avoid being stuck in the mainloop.
  // Infinitely running tests are useless.
  mainloop->get_context()->signal_timeout().connect_seconds_once(sigc::bind(&on_mainloop_killed_by_watchdog, mainloop), 300);

  connect_parser_signals(parser);

  const auto file_uri = create_file_from_buffer(input, input_size);
  parser.set_file_and_start_parsing(file_uri);
  if(Glom::CsvParser::State::PARSING != parser.get_state())
    return false;

  mainloop->run();

  auto file = Gio::File::create_for_uri(file_uri);

  try
  {
    const auto removed = file->remove();
    g_assert(removed);
  }
  catch(const Gio::Error& ex)
  {
    std::cerr << G_STRFUNC << ": Gio::File::remove() failed: " << ex.what() << std::endl;
    return false;
  }

  return finished_parsing;
}

bool run_parser_on_file(const FuncConnectParserSignals& connect_parser_signals, const std::string &uri)
{
  finished_parsing = true;

  //Start a mainloop because the parser uses an idle handler.
  //TODO: Stop the parser from doing that.
  auto mainloop = Glib::MainLoop::create();
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
  if(Glom::CsvParser::State::PARSING != parser.get_state())
    return false;

  mainloop->run();

  return finished_parsing;
}

} //namespace ImportTests
