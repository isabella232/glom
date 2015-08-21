#include <gtkmm/window.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <libglom/avahi_publisher.h>
#include <libglom/utils.h>
#include <iostream>

class TestWindow : public Gtk::Window
{

public:
  TestWindow();
  virtual ~TestWindow();

private:
  //Signal handlers:
  virtual void on_button_start();
  virtual void on_button_stop();

  //Member widgets:
  Gtk::Box m_box;
  Gtk::Button m_button_start;
  Gtk::Button m_button_stop;

  Glom::AvahiPublisher* m_avahi_publisher;
};



TestWindow::TestWindow()
: m_box(Gtk::ORIENTATION_VERTICAL, static_cast<int>(Glom::UiUtils::DefaultSpacings::SMALL)),
  m_button_start("Start"),
  m_button_stop("Stop"),
  m_avahi_publisher(nullptr)
{
  set_border_width(10);
  add(m_box);
  m_box.pack_start(m_button_start);
  m_box.pack_start(m_button_stop);

  m_button_start.signal_clicked().connect(sigc::mem_fun(*this, &TestWindow::on_button_start));
  m_button_stop.signal_clicked().connect(sigc::mem_fun(*this, &TestWindow::on_button_stop));

  show_all_children();
}

TestWindow::~TestWindow()
{
  delete m_avahi_publisher;
}

void TestWindow::on_button_start()
{
  if(m_avahi_publisher)
    return;

  std::cout << "Starting" << std::endl;
 
  m_avahi_publisher = new Glom::AvahiPublisher("testservice", "_testthing._tcp", 1234 /* port */);
}

void TestWindow::on_button_stop()
{
  if(!m_avahi_publisher)
    return;

  std::cout << "Stopping" << std::endl;

  delete m_avahi_publisher;
  m_avahi_publisher = nullptr;
}


int main (int argc, char *argv[])
{
  Gtk::Main kit(argc, argv);

  TestWindow helloworld;
  Gtk::Main::run(helloworld); //Shows the window and returns when it is closed.

  return 0;
}


