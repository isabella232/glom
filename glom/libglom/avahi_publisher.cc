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
 
#include <glom/libglom/avahi_publisher.h>

#include <avahi-client/client.h>
#include <avahi-client/publish.h>
#include <avahi-common/error.h>
#include <avahi-common/timeval.h>
#include <avahi-common/alternative.h>
#include <avahi-glib/glib-watch.h>
#include <avahi-glib/glib-malloc.h>


#include "config.h"

/* Callback for Avahi API Timeout Event */
static void
avahi_timeout_event (AVAHI_GCC_UNUSED AvahiTimeout *timeout, AVAHI_GCC_UNUSED void *userdata)
{
    g_message ("Avahi API Timeout reached!");
}

/* Callback for GLIB API Timeout Event */
static gboolean
avahi_timeout_event_glib (void *userdata)
{
    Glom::AvahiPublisher* self = static_cast<Glom::AvahiPublisher*>(userdata);

    g_message ("GLIB API Timeout reached, quitting main loop!");
    
    /* Quit the application */
    if(self->m_avahi_mainloop)
    {
      g_main_loop_quit(self->m_avahi_mainloop);
    }

    self->m_timeout_id = 0;

    return FALSE; /* Don't re-schedule timeout event */
}

/* Callback for state changes on the Client */
static void
avahi_client_callback (AvahiClient *client, AvahiClientState state, void *userdata)
{
  Glom::AvahiPublisher* self = static_cast<Glom::AvahiPublisher*>(userdata);

  g_message ("Avahi Client State Change: %d", state);

   /* Called whenever the client or server state changes */
   switch (state) {
        case AVAHI_CLIENT_S_RUNNING:
        
            /* The server has startup successfully and registered its host
             * name on the network, so it's time to create our services */
            if (!self->m_avahi_group)
                self->avahi_create_services(client);
            break;

        case AVAHI_CLIENT_FAILURE:
            
            fprintf(stderr, "Client failure: %s\n", avahi_strerror(avahi_client_errno(client)));
            g_main_loop_quit (self->m_avahi_mainloop);
            
            break;

        case AVAHI_CLIENT_S_COLLISION:
        
            /* Let's drop our registered services. When the server is back
             * in AVAHI_SERVER_RUNNING state we will register them
             * again with the new host name. */
            
        case AVAHI_CLIENT_S_REGISTERING:

            /* The server records are now being established. This
             * might be caused by a host name change. We need to wait
             * for our own records to register until the host name is
             * properly esatblished. */
            
            if (self->m_avahi_group)
                avahi_entry_group_reset(self->m_avahi_group);
            
            break;

        case AVAHI_CLIENT_CONNECTING:
            ;
    }
}

namespace Glom
{


AvahiPublisher::AvahiPublisher(const Glib::ustring& service_name, const Glib::ustring& service_type, int port)
: m_avahi_service_name(service_name),
  m_avahi_service_type(service_type),
  m_port(port),
  m_avahi_group(0),
  m_avahi_client(0),
  m_avahi_mainloop(0),
  m_timeout_id(0)
{
  /* Optional: Tell avahi to use g_malloc and g_free */
  avahi_set_allocator(avahi_glib_allocator ());

  /* Create the GLIB main loop */
  m_avahi_mainloop = g_main_loop_new (NULL, FALSE);

  /* Create the GLIB Adaptor */
  AvahiGLibPoll* glib_poll = avahi_glib_poll_new(NULL, G_PRIORITY_DEFAULT);
  const AvahiPoll* poll_api = avahi_glib_poll_get(glib_poll);

  /* Example, schedule a timeout event with the Avahi API */
  struct timeval tv;
  avahi_elapse_time (&tv,                         /* timeval structure */
            1000,                                   /* 1 second */
            0);                                     /* "jitter" - Random additional delay from 0 to this value */

  poll_api->timeout_new (poll_api,                /* The AvahiPoll object */
                      &tv,                          /* struct timeval indicating when to go activate */
                      avahi_timeout_event,          /* Pointer to function to call */
                      NULL);                        /* User data to pass to function */

  /* Schedule a timeout event with the glib api */
  m_timeout_id = g_timeout_add (5000,                            /* 5 seconds */
            avahi_timeout_event_glib,               /* Pointer to function callback */
            this);                                  /* User data to pass to function */

  /* Create a new AvahiClient instance */
  int error = 0;
  m_avahi_client = avahi_client_new (poll_api,            /* AvahiPoll object from above */
                               (AvahiClientFlags)0,
            avahi_client_callback,                  /* Callback function for Client state changes */
            this,                                   /* User data */
            &error);                                /* Error return */

  bool failed = false;
  const char* version = 0;

  /* Check the error return code */
  if (m_avahi_client == NULL)
  {
    /* Print out the error string */
    g_warning ("Error initializing Avahi: %s", avahi_strerror (error));

    failed = true;
  }
  else
  {
    /* Make a call to get the version string from the daemon */
    const char* version = avahi_client_get_version_string (m_avahi_client);

    /* Check if the call suceeded */
    if (version == NULL)
    {
      g_warning ("Error getting version string: %s", avahi_strerror (avahi_client_errno (m_avahi_client)));

      failed = true;
    }
  }
    
  if(!failed)
  {
    if(version)
      g_message ("Avahi Server Version: %s", version);

    /* Start the GLIB Main Loop */
    g_main_loop_run (m_avahi_mainloop);
  }
  else
  {
    /* Clean up */
    if(m_avahi_mainloop)
    {
      g_main_loop_unref (m_avahi_mainloop);
      m_avahi_mainloop = 0;
    }

    if(m_avahi_client)
    {
      avahi_client_free (m_avahi_client);
      m_avahi_client = 0;
    }

    if(glib_poll)
      avahi_glib_poll_free (glib_poll);
  }
}

AvahiPublisher::~AvahiPublisher()
{
  //Stop the timeout callback, because it uses instance data, which will soon be invalid:
  if(m_timeout_id)
  {
    g_source_remove(m_timeout_id);
  }

  if(m_avahi_client)
  {
    avahi_client_free (m_avahi_client);
    m_avahi_client = 0;
  }

  /* Clean up */
  if(m_avahi_mainloop)
  {
    g_main_loop_unref (m_avahi_mainloop);
    m_avahi_mainloop = 0;
  }
}

static void entry_group_callback(AvahiEntryGroup *g, AvahiEntryGroupState state, void *userdata)
{
    AvahiPublisher* self = (AvahiPublisher*)userdata;

    assert(g == self->m_avahi_group || self->m_avahi_group == NULL);

    /* Called whenever the entry group state changes */

    switch (state) {
        case AVAHI_ENTRY_GROUP_ESTABLISHED :
            /* The entry group has been established successfully */
            fprintf(stderr, "Service '%s' successfully established.\n", self->m_avahi_service_name.c_str());
            break;

        case AVAHI_ENTRY_GROUP_COLLISION : {
   
            /* A service name collision happened. Let's pick a new name */
            char* n = avahi_alternative_service_name(self->m_avahi_service_name.c_str());
            self->m_avahi_service_name = n;
            avahi_free(n);
            n = 0;
            
            fprintf(stderr, "Service name collision, renaming service to '%s'\n", self->m_avahi_service_name.c_str());
            
            /* And recreate the services */
            self->avahi_create_services(avahi_entry_group_get_client(g));
            break;
        }

        case AVAHI_ENTRY_GROUP_FAILURE :

            fprintf(stderr, "Entry group failure: %s\n", avahi_strerror(avahi_client_errno(avahi_entry_group_get_client(g))));

            /* Some kind of failure happened while we were registering our services */
            g_main_loop_quit (self->m_avahi_mainloop);
            break;

        case AVAHI_ENTRY_GROUP_UNCOMMITED:
        case AVAHI_ENTRY_GROUP_REGISTERING:
            ;
    }
}

void AvahiPublisher::avahi_create_services(AvahiClient *c)
{
    int ret;
    assert(c);

    bool failed = false;

    /* If this is the first time we're called, let's create a new entry group */
    if (!m_avahi_group)
        if (!(m_avahi_group = avahi_entry_group_new(c, entry_group_callback, this /* user_data */))) {
            fprintf(stderr, "avahi_entry_group_new() failed: %s\n", avahi_strerror(avahi_client_errno(c)));
            failed = true;
        }
    
    if(!failed)
    {
      fprintf(stderr, "Adding service '%s'\n", m_avahi_service_name.c_str());
    }

    const int port = m_port;

    /* Add the service for Glom: */
    if (!failed && (ret = avahi_entry_group_add_service(m_avahi_group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, (AvahiPublishFlags)0, m_avahi_service_name.c_str(), m_avahi_service_type.c_str(), NULL, NULL, port, NULL)) < 0) {
        fprintf(stderr, "Failed to add _ipp._tcp service: %s\n", avahi_strerror(ret));
        failed = true;
    }
    
    /* Tell the server to register the service */
    if (!failed && (ret = avahi_entry_group_commit(m_avahi_group)) < 0) {
        fprintf(stderr, "Failed to commit entry_group: %s\n", avahi_strerror(ret));
        failed = true;
    }

    if(failed)
    {
      g_main_loop_quit (m_avahi_mainloop);
    }
}









} //namespace Glom
