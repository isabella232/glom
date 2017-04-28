#include <glom/bakery/busy_cursor.h>
#include <gtkmm/main.h>

namespace Glom
{

//Intialize static member variable:
BusyCursor::type_map_cursors BusyCursor::m_map_cursors;

BusyCursor::BusyCursor(Gtk::Window& window, Gdk::Cursor::Type cursor_type)
: BusyCursor(&window, cursor_type)
{
}

BusyCursor::BusyCursor(Gtk::Window* window, Gdk::Cursor::Type cursor_type)
: m_window(window) //If this is a nested cursor then remember the previously-set cursor, so we can restore it.
{
  if(!m_window)
    return;

  m_cursor = Gdk::Cursor::create(m_window->get_display(), cursor_type);

  m_gdk_window = m_window->get_window();
  if(!m_gdk_window)
    return;

  auto iter = m_map_cursors.find(m_window);
  if(iter != m_map_cursors.end())
  {
    m_old_cursor = iter->second; //Remember the existing cursor.
  }

  m_map_cursors[m_window] = m_cursor; //Let a further nested cursor know about the new cursor in use.


  //Change the cursor:
  if(m_gdk_window)
    m_gdk_window->set_cursor(m_cursor);

  force_gui_update();
}

BusyCursor::~BusyCursor()
{
  //Restore the old cursor:
  if(m_old_cursor)
  {
    if(m_gdk_window)
      m_gdk_window->set_cursor(m_old_cursor);
  }
  else
  {
    if(m_gdk_window)
      m_gdk_window->set_cursor();

    auto iter = m_map_cursors.find(m_window);
    if(iter != m_map_cursors.end())
      m_map_cursors.erase(iter);
  }

  force_gui_update();
}


void BusyCursor::force_gui_update()
{
  if(m_gdk_window)
  {
    //Force the GUI to update:
    //TODO: Make sure that gtkmm has some non-Gtk::Main API for this:
    while(gtk_events_pending())
      gtk_main_iteration_do(true);
  }
}


} //namespace Glom
