/*
 * Copyright 2002 Murray Cumming
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

#ifndef GLOM_BAKERY_APPINSTANCEMANAGER_H
#define GLOM_BAKERY_APPINSTANCEMANAGER_H

#include <sigc++/sigc++.h>
#include <list>

namespace GlomBakery
{

class App;

/** Contains a list of App instances.
 * Each App registers itself with the one AppInstanceManager,
 * and the AppInstanceManager will then delete the App when
 * it has been closed, by catching the "hide" signal.
 * You should not need to use this class directly.
 */
class AppInstanceManager : public sigc::trackable
{
public:
  AppInstanceManager();
  virtual ~AppInstanceManager();

  virtual void add_app(App* pApp);
  virtual void close_all();
  virtual void cancel_close_all();

  virtual unsigned int get_app_count() const;

  typedef std::list<App*> type_listAppInstances;
  virtual type_listAppInstances get_instances() const; //Used by App_WithDoc to get associated Documents.

protected:
  //Signal handler:
  virtual void on_app_hide(App* pApp);

  //Instances:
  type_listAppInstances m_listAppInstances;

  bool m_bExiting;
};

} //namespace

#endif //BAKERY_APPINSTANCEMANAGER_H
