#include <gtkmm.h>
#include <glom/libglom/avahi_publisher.h>
#include <glom/libglom/utils.h>
#include <iostream>

class TestWindow : public Gtk::Window
{

public:
  TestWindow();
  virtual ~TestWindow();

protected:
  //Signal handlers:
  virtual void on_button_start();
  virtual void on_button_stop();

  //Member widgets:
  Gtk::VBox m_box;
  Gtk::Button m_button_start;
  Gtk::Button m_button_stop;

  Glom::AvahiPublisher* m_avahi_publisher;
};



TestWindow::TestWindow()
: m_box(false, Glom::Utils::DEFAULT_SPACING_SMALL),
  m_button_start("Start"),
  m_button_stop("Stop"),
  m_avahi_publisher(0)
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
  if(m_avahi_publisher)
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
  m_avahi_publisher = 0;
}


int main (int argc, char *argv[])
{
  Gtk::Main kit(argc, argv);

  TestWindow helloworld;
  Gtk::Main::run(helloworld); //Shows the window and returns when it is closed.

  return 0;
}


