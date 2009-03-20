/*
 * Copyright 2000 Murray Cumming
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <glom/bakery/AppInstanceManager.h>
#include <glom/bakery/App.h>
#include <gtkmm/main.h>

namespace GlomBakery
{

AppInstanceManager::AppInstanceManager()
{
  m_bExiting = false;
}

AppInstanceManager::~AppInstanceManager()
{
}

void AppInstanceManager::on_app_hide(App* pApp)
{
  //If pApp is one of the remembered instances (it always should be):
  type_listAppInstances::iterator iterFind = std::find(m_listAppInstances.begin(), m_listAppInstances.end(), pApp);
  if(iterFind != m_listAppInstances.end())
  {
    m_listAppInstances.erase(iterFind);
    delete pApp;
    pApp = 0;
  }

  //When the last instance goes, the application closes.
  if(m_listAppInstances.empty())
  {
    Gtk::Main::quit();
  }
}

void AppInstanceManager::add_app(App* pApp)
{
  m_listAppInstances.push_back(pApp);

  //Allow the AppInstanceManager to respond when the Application is hidden (hidden == closed):
  pApp->ui_signal_hide().connect( sigc::bind<App*>(sigc::mem_fun(*this, &AppInstanceManager::on_app_hide), pApp) );
}

void AppInstanceManager::close_all()
{
  m_bExiting = true; //One of the instances could cancel this loop.

  type_listAppInstances::iterator i = m_listAppInstances.begin();
  while (m_bExiting && (i != m_listAppInstances.end()))
  {
    type_listAppInstances::iterator j = i;
    i++;

    App* pApp = (*j);
    if(pApp)
    {
      type_listAppInstances::size_type count = m_listAppInstances.size();
      pApp->on_menu_file_close();

      //The iterator is invalid if an element has been removed:
      if(count != m_listAppInstances.size())
      {
        i = m_listAppInstances.begin(); //There should not be a problem with asking again.
      }
    }
  }
}

void AppInstanceManager::cancel_close_all()
{
  m_bExiting = false;
}

unsigned int AppInstanceManager::get_app_count() const
{
  return m_listAppInstances.size();
}

AppInstanceManager::type_listAppInstances AppInstanceManager::get_instances() const
{
  return m_listAppInstances;
}



} //namespace
