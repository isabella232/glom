#include <tests/import/utils.h>

namespace ImportTests
{

bool check(const std::string& name, bool test, std::stringstream& report)
{
  if(!test)
    report << name << ": FAILED" << std::endl;

  return test;
}

// Returns the file name of the temporary created file, which will contain the buffer's contents.
static std::string create_file_from_buffer(const char* input, guint size)
{
  // Use Glib's file utilities to get a unique temporary filename:
  std::string tmp_filename;
  const int tmp_file_handle = Glib::file_open_tmp(tmp_filename, "glom_testdata");

  if(0 < tmp_file_handle)
  {
    ssize_t result = write(tmp_file_handle, input, size);
    g_assert(-1 != result); // g_return_with_val would still be wrong here, I think?

    close(tmp_file_handle);
  }

  return Glib::filename_to_uri(tmp_filename);
}

static Glib::RefPtr<Glib::MainLoop>& get_mainloop_instance()
{
  static Glib::RefPtr<Glib::MainLoop> mainloop(0);
  return mainloop;
}

static bool& get_result_instance()
{
  static bool result = true;
  return result;
}

static void on_mainloop_killed_by_watchdog()
{
  get_result_instance() = false; // Comment out if you want to run tests and get useful results even with a non-working finished_parsing signal.
  get_mainloop_instance()->quit();
}

static void on_parser_finished()
{
  //Quit the mainloop that we ran because the parser uses an idle handler.
  get_mainloop_instance()->quit();
}

bool run_parser_from_buffer(const FuncConnectParserSignals& connect_parser_signals, const char* input, guint size)
{
  get_result_instance() = true;

  //Start a mainloop because the parser uses an idle handler.
  //TODO: Stop the parser from doing that.
  get_mainloop_instance().reset();
  get_mainloop_instance() = Glib::MainLoop::create();

  Glom::CsvParser parser("UTF-8");

  parser.signal_finished_parsing().connect(&on_parser_finished);

  // Install a watchdog for the mainloop. No test should need longer than 3
  // seconds. Also, we need to avoid being stuck in the mainloop.
  // Infinitely running tests are useless.
  get_mainloop_instance()->get_context()->signal_timeout().connect_seconds_once(
    sigc::ptr_fun(&on_mainloop_killed_by_watchdog), 3);

  connect_parser_signals(parser);

  const std::string file_uri = create_file_from_buffer(input, size);
  parser.set_file_and_start_parsing(file_uri);

  get_mainloop_instance()->run();

  Glib::RefPtr<Gio::File> file = Gio::File::create_for_uri(file_uri);
  const bool removed = file->remove();
  g_assert(removed);

  return get_result_instance();
}

} //namespace ImportTests

