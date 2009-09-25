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
}

std::string create_file_from_buffer(const char* input, guint size)
{
  // use Glib's file utilities to get an unique tmp filename
  std::string tmp_filename;
  int tmp_file_handle = Glib::file_open_tmp(tmp_filename, "glom_testdata");

  if (0 < tmp_file_handle)
  {
    ssize_t result = write(tmp_file_handle, input, size);
    g_assert(-1 != result); // g_return_with_val would still be wrong here, I think?

    close(tmp_file_handle);
  }

  return Glib::filename_to_uri(tmp_filename);
}

Glib::RefPtr<Glib::MainLoop>& get_mainloop_instance()
{
  static Glib::RefPtr<Glib::MainLoop> mainloop(0);
  return mainloop;
}

bool& get_result_instance()
{
  static bool result = true;
  return result;
}

void on_mainloop_killed_by_watchdog()
{
  get_result_instance() = false; // Comment out if you want to run tests and get useful results even with a non-working finished_parsing signal.
  get_mainloop_instance()->quit();
}

typedef sigc::slot<void, Glom::CsvParser&> FuncConnectParserSignals;


bool run_parser_from_buffer(const FuncConnectParserSignals& connect_parser_signals, const char* input, guint size)
{
  get_result_instance() = true;

  get_mainloop_instance().reset();
  get_mainloop_instance() = Glib::MainLoop::create();

  Glom::CsvParser parser("UTF-8");

  parser.signal_finished_parsing().connect(sigc::mem_fun(*get_mainloop_instance().operator->(), &Glib::MainLoop::quit));

  // Install a watchdog for the mainloop, no test should need longer than 3
  // seconds. Also, we need to guard against being stuck in the mainloop.
  // Infinitely running tests are useless.
  get_mainloop_instance()->get_context()->signal_timeout().connect_seconds_once(sigc::ptr_fun(&on_mainloop_killed_by_watchdog), 3);

  connect_parser_signals(parser);

  const std::string file_name = create_file_from_buffer(input, size);
  parser.set_file_and_start_parsing(file_name);

  get_mainloop_instance()->run();

  int result = unlink(Glib::filename_from_uri(file_name).c_str());
  g_assert(-1 != result);

  return get_result_instance();
}

} //namespace ImportTests

