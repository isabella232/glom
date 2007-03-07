/* Glom
 *
 * Copyright (C) 2001-2004 Murray Cumming
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef GLOM_AVAHIPUBLISHER_H
#define GLOM_AVAHIPUBLISHER_H

#include <glibmm/ustring.h>
#include <glib/gmain.h>
//#include <glibmm/mainloop>

typedef struct AvahiEntryGroup AvahiEntryGroup;
typedef struct AvahiClient AvahiClient;


namespace Glom
{

/** The named service will be advertized during the liftetime of the object.
 * So create an instance of this class to advertize a service, 
 * and delete the instance when you want to stop advertizing the service.
 */
class AvahiPublisher
{
public:

  /** Instantiate an AvahiPublisher instance to advertize a service.
   *
   * @param service_name For instance, "fooserver"
   * @param service_type For instance, "_glom._tcp"
   * @param port The network port on which the service is available on this host.
   */
  AvahiPublisher(const Glib::ustring& service_name, const Glib::ustring& service_type, int port);

  /** Delete the AvahiPublisher instance to stop advertizing the service.
   */
  virtual ~AvahiPublisher();

protected:

  /** Advertize self-hosting via avahi:
   */
  void avahi_start_publishing();
  void avahi_stop_publishing();

public:
  void avahi_create_services(AvahiClient *c);

public:
  Glib::ustring m_avahi_service_name;
  Glib::ustring m_avahi_service_type;
  int m_port;

  AvahiEntryGroup* m_avahi_group;
  AvahiClient* m_avahi_client;
  GMainLoop* m_avahi_mainloop;

  guint m_timeout_id;

private:

  static AvahiPublisher* m_instance;
};

} //namespace Glom

#endif //GLOM_AVAHIPUBLISHER_H

