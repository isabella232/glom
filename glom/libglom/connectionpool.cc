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

#include <libglom/libglom_config.h>

#include <libglom/connectionpool.h>
#include <libglom/document/document.h>
#include <libglom/utils.h>
//#include <libgdamm/connectionevent.h>

#include <libglom/connectionpool_backends/postgres_central.h>
#include <libglom/connectionpool_backends/postgres_self.h>
# include <libglom/connectionpool_backends/sqlite.h>

#ifdef G_OS_WIN32
# include <windows.h>
#else
# include <libepc/shell.h> //For epc_shell_set_progress_hooks().
# include <libepc/publisher.h>
#endif

#include <signal.h> //To catch segfaults

#include <glibmm/i18n.h>


#ifndef G_OS_WIN32
static EpcProtocol publish_protocol = EPC_PROTOCOL_HTTPS;
#endif

// Uncomment to see debug messages
//#define GLOM_CONNECTION_DEBUG

namespace Glom
{

SharedConnection::SharedConnection()
{
}

SharedConnection::SharedConnection(const Glib::RefPtr<Gnome::Gda::Connection>& gda_connection)
: m_gda_connection(gda_connection)
{
}

SharedConnection::~SharedConnection()
{
  if(m_gda_connection)
    m_signal_finished.emit();
}

Glib::RefPtr<Gnome::Gda::Connection> SharedConnection::get_gda_connection()
{
  return m_gda_connection;
}

Glib::RefPtr<const Gnome::Gda::Connection> SharedConnection::get_gda_connection() const
{
  return m_gda_connection;
}

SharedConnection::type_signal_finished SharedConnection::signal_finished()
{
  return m_signal_finished;
}

void SharedConnection::close()
{
  if(m_gda_connection)
    m_gda_connection.reset();


  //Tell the connection pool that we have finished with this connection.
  //It might want to close it, or keep it open if somebody else is using it.
  //It might even give it to someone else while it is waiting for that other person to finish with it.
  m_signal_finished.emit();
}

//init_db_details static data:
ConnectionPool* ConnectionPool::m_instance = 0;

ConnectionPool::ConnectionPool()
:
  m_epc_publisher(0),
  m_dialog_epc_progress(0),
  m_backend(0),
  m_sharedconnection_refcount(0),
  m_ready_to_connect(false),
  m_pFieldTypes(0)
{
}

ConnectionPool::~ConnectionPool()
{
  if(m_pFieldTypes)
  {
    delete m_pFieldTypes;
    m_pFieldTypes = 0;
  }
}

//static
ConnectionPool* ConnectionPool::get_instance()
{
  //TODO: Synchronize this for threads?
  if(m_instance)
    return m_instance;
  else
  {
    m_instance = new ConnectionPool();
    return m_instance;
  }
}

void ConnectionPool::setup_from_document(const Document* document)
{
  switch(document->get_hosting_mode())
  {
  case Document::HOSTING_MODE_POSTGRES_SELF:
    {
      ConnectionPoolBackends::PostgresSelfHosted* backend = new ConnectionPoolBackends::PostgresSelfHosted;
      backend->set_database_directory_uri(document->get_connection_self_hosted_directory_uri());
      set_backend(std::auto_ptr<ConnectionPool::Backend>(backend));
    }
    break;
  case Document::HOSTING_MODE_POSTGRES_CENTRAL:
    {
      ConnectionPoolBackends::PostgresCentralHosted* backend = new ConnectionPoolBackends::PostgresCentralHosted;
      backend->set_host(document->get_connection_server());
      backend->set_port(document->get_connection_port());
      backend->set_try_other_ports(document->get_connection_try_other_ports());
      set_backend(std::auto_ptr<ConnectionPool::Backend>(backend));
    }
    break;
  case Document::HOSTING_MODE_SQLITE:
    {
      ConnectionPoolBackends::Sqlite* backend = new ConnectionPoolBackends::Sqlite;
      backend->set_database_directory_uri(document->get_connection_self_hosted_directory_uri());
      set_backend(std::auto_ptr<ConnectionPool::Backend>(backend));
    }
    break;

  default:
    //on_document_load() should have checked for this already, informing the user.
    std::cerr << "Glom: setup_connection_pool_from_document(): Unhandled hosting mode: " << document->get_hosting_mode() << std::endl;
    g_assert_not_reached();
    break;
  }

  // Might be overwritten later when actually attempting a connection:
  set_user(document->get_connection_user());
  set_database(document->get_connection_database());

  set_ready_to_connect();
}

void ConnectionPool::delete_instance()
{
  if(m_instance)
  {
    delete m_instance;
    m_instance = 0;
  }
}

bool ConnectionPool::get_ready_to_connect() const
{
  return m_ready_to_connect;
}

void ConnectionPool::set_ready_to_connect(bool val)
{
  m_ready_to_connect = val;
}

void ConnectionPool::set_backend(std::auto_ptr<Backend> backend)
{
  m_backend = backend;
}

ConnectionPool::Backend* ConnectionPool::get_backend()
{
  return m_backend.get();
}

const ConnectionPool::Backend* ConnectionPool::get_backend() const
{
  return m_backend.get();
}



//static:
#ifdef GLIBMM_EXCEPTIONS_ENABLED
sharedptr<SharedConnection> ConnectionPool::get_and_connect()
#else
sharedptr<SharedConnection> ConnectionPool::get_and_connect(std::auto_ptr<ExceptionConnection>& error)
#endif // GLIBMM_EXCEPTIONS_ENABLED
{
  sharedptr<SharedConnection> result(0);

  ConnectionPool* connection_pool = ConnectionPool::get_instance();
  if(!connection_pool)
    return result;

  if(!(connection_pool->m_backend.get()))
  {
    std::cerr << "ConnectionPool::get_and_connect(): m_backend is null." << std::endl;
    return result; //TODO: Return a FAILURE_NO_BACKEND error?, though that would be tedious.
  }

#ifdef GLIBMM_EXCEPTIONS_ENABLED
  result = connection_pool->connect();
#else
  result = connection_pool->connect(error);
#endif // GLIBMM_EXCEPTIONS_ENABLED

  return result;
}



// Store the connection for a few seconds in case it
// is immediately requested again, to avoid making a new connection
// and introspecting again, which is slow.
// TODO: Why aren't these member variables?
static sharedptr<SharedConnection> connection_cached;
static sigc::connection connection_cached_timeout_connection;
static sigc::connection connection_cached_finished_connection;

static bool on_connection_pool_cache_timeout()
{
  //std::cout << "DEBUG: Clearing connection cache." << std::endl;

  //Forget the cached connection after a few seconds:
  connection_cached.clear();

  return false; //Don't call this again.
}


#ifdef GLIBMM_EXCEPTIONS_ENABLED
sharedptr<SharedConnection> ConnectionPool::connect()
#else
sharedptr<SharedConnection> ConnectionPool::connect(std::auto_ptr<ExceptionConnection>& error)
#endif
{
  //Don't try to connect if we don't have a backend to connect to.
  g_return_val_if_fail(m_backend.get(), sharedptr<SharedConnection>(0));

  if(get_ready_to_connect())
  {
    if(connection_cached)
    {
      //Avoid a reconnection immediately after disconnecting:
      return connection_cached;
    }
    //If the connection is already open (because it is being used by somebody):
    else if(m_refGdaConnection)
    {
      sharedptr<SharedConnection> sharedConnection( new SharedConnection(m_refGdaConnection) );

      //Ask for notification when the SharedConnection has been finished with:
      //TODO: Note that we are overwriting the connection to a signal of a
      //previous sharedconnection here. This can be problematic when
      //invalidate_connection() is called, because then we don't disconnect
      //from the signal of the previous instance, and when the signal
      //handler is called later we might decrement the reference count for
      //a completely different shared connection.
      connection_cached_finished_connection = sharedConnection->signal_finished().connect( sigc::mem_fun(*this, &ConnectionPool::on_sharedconnection_finished) );

      //Remember that somebody is using it:
      m_sharedconnection_refcount++;

      //Store the connection in a cache for a few seconds to avoid unnecessary disconnections/reconnections:
      //std::cout << "DEBUG: Stored connection cache." << std::endl;
      connection_cached = sharedConnection;
      connection_cached_timeout_connection.disconnect(); //Stop the existing timeout if it has not run yet.
      connection_cached_timeout_connection = Glib::signal_timeout().connect_seconds(&on_connection_pool_cache_timeout, 30 /* seconds */);

      return sharedConnection;
    }
    else
    {
#ifdef GLIBMM_EXCEPTIONS_ENABLED
      std::auto_ptr<ExceptionConnection> error;
#endif
      m_refGdaConnection = m_backend->connect(m_database, get_user(), get_password(), error);

      if(!m_refGdaConnection)
      {
#ifdef GLIBMM_EXCEPTIONS_ENABLED
        throw *error;
#endif
        return sharedptr<SharedConnection>(0);
      }
      else
      {
        //Allow get_meta_store_data() to succeed:
        //Hopefully this (and the update_meta_store_for_table() calls) is all we need.
        //std::cout << "DEBUG: Calling update_meta_store_data_types() ..." << std::endl;
#ifdef GLIBMM_EXCEPTIONS_ENABLED
        try
        {
          m_refGdaConnection->update_meta_store_data_types();
        }
        catch(const Glib::Error& ex)
        {
          std::cerr << "ConnectionPool::connect(): update_meta_store_data_types() failed: " << ex.what() << std::endl;
        }
        //std::cout << "DEBUG: ... update_meta_store_data_types() has finished." << std::endl;
#else
        std::auto_ptr<Glib::Error> ex;
        m_refGdaConnection->update_meta_store_data_types(ex);
        if (ex.get())
          std::cerr << "ConnectionPool::connect(): update_meta_store_data_types() failed: " << ex->what() << std::endl;
#endif
        //std::cout << "DEBUG: Calling update_meta_store_table_names() ..." << std::endl;
#ifdef GLIBMM_EXCEPTIONS_ENABLED
        try
        {
          //update_meta_store_table_names() has been known to throw an exception.
          //Glom is mostly unusable when it fails, but that's still better than a crash.
          m_refGdaConnection->update_meta_store_table_names(m_backend->get_public_schema_name());
        }
        catch(const Glib::Error& ex)
        {
          std::cerr << "ConnectionPool::connect(): update_meta_store_table_names() failed: " << ex.what() << std::endl;
        }
        //std::cout << "DEBUG: ... update_meta_store_table_names() has finished." << std::endl;
#else
        m_refGdaConnection->update_meta_store_table_names(m_backend->get_public_schema_name(), ex);
        if (ex.get())
          std::cerr << "ConnectionPool::connect(): update_meta_store_data_types() failed: " << ex->what() << std::endl;
#endif

        // Connection succeeded
        // Create the fieldtypes member if it has not already been done:
        if(!m_pFieldTypes)
          m_pFieldTypes = new FieldTypes(m_refGdaConnection);

#ifndef G_OS_WIN32
        //Let other clients discover this server via avahi:
        //TODO: Only advertize if we are the first to open the document,
        //to avoid duplicates.
        //TODO: Only advertize if it makes sense for the backend,
        //it does not for sqlite
        Document* document = get_document();
        if(document && document->get_network_shared())
          avahi_start_publishing(); //Stopped in the signal_finished handler.
#endif // !G_OS_WIN32

#ifdef GLIBMM_EXCEPTIONS_ENABLED
        return connect(); //Call this method recursively. This time m_refGdaConnection exists.
#else
        return connect(error); //Call this method recursively. This time m_refGdaConnection exists.
#endif // GLIBMM_EXCEPTIONS_ENABLED
      }
    }
  }
  else
  {
    //g_warning("ConnectionPool::connect(): not ready to connect.");
  }

  return sharedptr<SharedConnection>(0);
}

#ifdef GLIBMM_EXCEPTIONS_ENABLED
void ConnectionPool::create_database(const Glib::ustring& database_name)
#else
void ConnectionPool::create_database(const Glib::ustring& database_name, std::auto_ptr<Glib::Error>& error)
#endif
{
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  std::auto_ptr<Glib::Error> error;
#endif
  if(m_backend.get())
    m_backend->create_database(database_name, get_user(), get_password(), error);

#ifdef GLIBMM_EXCEPTIONS_ENABLED
  if(error.get()) throw *error;
#endif
}

void ConnectionPool::set_user(const Glib::ustring& value)
{
  if(value.empty())
  {
#ifdef GLOM_CONNECTION_DEBUG
    std::cout << "debug: ConnectionPool::set_user(): user is empty." << std::endl;
#endif
  }

  m_user = value;

  //Make sure that connect() makes a new connection:
  invalidate_connection();
}

bool ConnectionPool::save_backup(const SlotProgress& slot_progress, const std::string& path_dir)
{
  g_assert(m_backend.get());

  const std::string old_uri = m_backend->get_database_directory_uri();

  std::string uri;
  try
  {
    //TODO: Avoid the copy/paste of glom_postgres_data and make it work for sqlite too.
    const std::string subdir = Glib::build_filename(path_dir, "glom_postgres_data");
    uri = Glib::filename_to_uri(subdir);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << ": exception from Glib::build_filename(): " << ex.what() << std::endl;
    return false;
  }

  m_backend->set_database_directory_uri(uri);
  const bool result = m_backend->save_backup(slot_progress, m_user, m_password, m_database);
  m_backend->set_database_directory_uri(old_uri);
  return result;
}

bool ConnectionPool::convert_backup(const SlotProgress& slot_progress, const std::string& path_dir)
{
  g_assert(m_backend.get());

  //TODO: Avoid this copy/paste of the directory name:
  std::string path_dir_to_use = path_dir;
  if(!path_dir_to_use.empty())
  {
    path_dir_to_use = Glib::build_filename(path_dir, "glom_postgres_data");
  }

  const bool result = m_backend->convert_backup(slot_progress, path_dir_to_use, m_user, m_password, m_database);
  if(!result)
    return false;

  try
  {
    //update_meta_store_table_names() has been known to throw an exception.
    //Glom is mostly unusable when it fails, but that's still better than a crash.
    m_refGdaConnection->update_meta_store_table_names(m_backend->get_public_schema_name());
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << "ConnectionPool::connect(): update_meta_store_table_names() failed: " << ex.what() << std::endl;
  }

  return result;
}

void ConnectionPool::set_password(const Glib::ustring& value)
{
  m_password = value;

  //Make sure that connect() makes a new connection:
  invalidate_connection();
}

void ConnectionPool::set_database(const Glib::ustring& value)
{
  m_database = value;

  //Make sure that connect() makes a new connection:
  invalidate_connection();
}

Glib::ustring ConnectionPool::get_user() const
{
  return m_user;
}

Glib::ustring ConnectionPool::get_password() const
{
  return m_password;
}

Glib::ustring ConnectionPool::get_database() const
{
  return m_database;
}

Field::sql_format ConnectionPool::get_sql_format() const
{
  g_assert(m_backend.get());
  return m_backend->get_sql_format();
}

const FieldTypes* ConnectionPool::get_field_types() const
{
  return m_pFieldTypes;
}

Glib::ustring ConnectionPool::get_string_find_operator() const
{
  g_assert(m_backend.get());
  return m_backend->get_string_find_operator();
}

void ConnectionPool::invalidate_connection()
{
  connection_cached.clear();
  connection_cached_timeout_connection.disconnect();
  connection_cached_finished_connection.disconnect();
  m_refGdaConnection.reset();
  m_sharedconnection_refcount = 0;
}

void ConnectionPool::on_sharedconnection_finished()
{
  //g_warning("ConnectionPool::on_sharedconnection_finished().");

  //One SharedConnection is no longer being used:
  m_sharedconnection_refcount--;

  //If this was the last user of SharedConnection then we can close the connection.
  if(m_sharedconnection_refcount == 0)
  {
    //There should be no copies of the m_refConnection, so the Gnome::Gda::Connection destructor should
    //run when we clear this last RefPtr of it, but we will explicitly close it just in case.
    //g_warning("ConnectionPool::on_sharedconnection_finished(): closing GdaConnection");
    m_refGdaConnection->close();

    m_refGdaConnection.reset();

#ifndef G_OS_WIN32
    //TODO: this should only even be started if we are the first to open the .glom file:
    avahi_stop_publishing();
#endif

    //g_warning("ConnectionPool: connection closed");
  }
}

//static
bool ConnectionPool::handle_error_cerr_only()
{
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  sharedptr<SharedConnection> sharedconnection = get_and_connect();
#else
  std::auto_ptr<ExceptionConnection> conn_error;
  sharedptr<SharedConnection> sharedconnection = get_and_connect(conn_error);
  // Ignore error, sharedconnection presence is checked below
#endif
  if(sharedconnection)
  {
    Glib::RefPtr<Gnome::Gda::Connection> gda_connection = sharedconnection->get_gda_connection();

    typedef std::list< Glib::RefPtr<Gnome::Gda::ConnectionEvent> > type_list_errors;
    type_list_errors list_errors = gda_connection->get_events();

    if(!list_errors.empty())
    {
      Glib::ustring error_details;
      for(type_list_errors::iterator iter = list_errors.begin(); iter != list_errors.end(); ++iter)
      {
        Glib::RefPtr<Gnome::Gda::ConnectionEvent> event = *iter;
        if(event && (event->get_event_type() == Gnome::Gda::CONNECTION_EVENT_ERROR))
        {
          if(!error_details.empty())
            error_details += '\n'; //Add newline after each error.

          error_details += (*iter)->get_description();
          std::cerr << "Internal error (Database): " << error_details << std::endl;
        }
      }

      return true; //There really was an error.
    }
  }

  //There was no error. libgda just did not return any data, and has no concept of an empty datamodel.
  return false;
}

#ifdef G_OS_WIN32
// TODO: This is probably mingw specific
static __p_sig_fn_t previous_sig_handler = SIG_DFL;
#else
static sighandler_t previous_sig_handler = SIG_DFL; /* Arbitrary default */
#endif

/* This is a Linux/Unix signal handler,
* so we can respond to a crash.
*/
static void on_linux_signal(int signum)
{
  ConnectionPool* connection_pool = ConnectionPool::get_instance();
  if(!connection_pool)
  return;

  if(signum == SIGSEGV)
  {
    ConnectionPool::SlotProgress slot_ignored;
    connection_pool->cleanup(slot_ignored);

    //Let GNOME/Ubuntu's crash handler still handle this?
    if(previous_sig_handler)
      (*previous_sig_handler)(signum);
    else
      exit(1);
  }
}

ConnectionPool::StartupErrors ConnectionPool::startup(const SlotProgress& slot_progress, bool network_shared)
{
  if(!m_backend.get())
    return Backend::STARTUPERROR_FAILED_UNKNOWN_REASON;

  const Backend::StartupErrors started = m_backend->startup(slot_progress, network_shared);
  if(started != Backend::STARTUPERROR_NONE)
    return started;

#ifndef G_OS_WIN32
  //Let clients discover this server via avahi:
  //avahi_start_publishing();
#endif // !G_OS_WIN32

  //If we crash while running (unlikely, hopefully), then try to cleanup.
  //Comment this out if you want to see the backtrace in a debugger.
  previous_sig_handler = signal(SIGSEGV, &on_linux_signal);

  return started;
}

bool ConnectionPool::cleanup(const SlotProgress& slot_progress)
{
  // Without a valid backend instance we should not state that we are ready to
  // connect. Fixes crash described in #577821.

  // TODO: Implement checks in set_ready_to_connect() to only set the flag to
  // "true" when the ConnectionPool instance is in a consistent state
  // afterwards. Or, have a private set_ready_to_connect() and a public
  // set_not_ready_to_connect()?
  set_ready_to_connect(false);

  bool result = false;

  if(m_backend.get())
    result = m_backend->cleanup(slot_progress);

  //Make sure that connect() tries to make a new connection:
  invalidate_connection();


#ifndef G_OS_WIN32
  /* Stop advertising the self-hosting database server via avahi: */
  //avahi_stop_publishing();
#endif // !G_OS_WIN32

  //We don't need the segfault handler anymore:
  signal(SIGSEGV, previous_sig_handler);
  previous_sig_handler = SIG_DFL; /* Arbitrary default */

  return result;
}


bool ConnectionPool::set_network_shared(const SlotProgress& slot_progress, bool network_shared)
{
  if(m_backend.get())
    return m_backend->set_network_shared(slot_progress, network_shared);
  else
    return false;
}

#ifdef GLIBMM_EXCEPTIONS_ENABLED
bool ConnectionPool::add_column(const Glib::ustring& table_name, const sharedptr<const Field>& field)
#else
bool ConnectionPool::add_column(const Glib::ustring& table_name, const sharedptr<const Field>& field, std::auto_ptr<Glib::Error>& error)
#endif
{
  sharedptr<SharedConnection> conn;
  if(!m_refGdaConnection)
  {
#ifdef GLIBMM_EXCEPTIONS_ENABLED
    conn = connect();
#else
    std::auto_ptr<ExceptionConnection> local_error;
    // TODO: We can't rethrow local_error here since ExceptionConnection does
    // not derive from Glib::Error
    conn = connect(local_error);
#endif
  }

  if(!m_refGdaConnection)
    return false;
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  std::auto_ptr<Glib::Error> error;
#endif
  const bool result = m_backend->add_column(m_refGdaConnection, table_name, field, error);
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  if(error.get()) throw *error;
  m_refGdaConnection->update_meta_store_table(table_name, m_backend->get_public_schema_name());
#else
  if(error.get())
    std::cerr << "Error: " << error->what() << std::endl;
  m_refGdaConnection->update_meta_store_table(table_name, m_backend->get_public_schema_name(), error);
  if(error.get())
    std::cerr << "Error: " << error->what() << std::endl;
#endif
  return result;
}

#ifdef GLIBMM_EXCEPTIONS_ENABLED
bool ConnectionPool::drop_column(const Glib::ustring& table_name, const Glib::ustring& field_name)
#else
bool ConnectionPool::drop_column(const Glib::ustring& table_name, const Glib::ustring& field_name, std::auto_ptr<Glib::Error>& error)
#endif
{
  sharedptr<SharedConnection> conn;
  if(!m_refGdaConnection)
  {
#ifdef GLIBMM_EXCEPTIONS_ENABLED
    conn = connect();
#else
    std::auto_ptr<ExceptionConnection> local_error;
    // TODO: We can't rethrow local_error here since ExceptionConnection does
    // not derive from Glib::Error
    conn = connect(local_error);
#endif
  }

  if(!m_refGdaConnection)
    return false;

#ifdef GLIBMM_EXCEPTIONS_ENABLED
  std::auto_ptr<Glib::Error> error;
#endif
  const bool result = m_backend->drop_column(m_refGdaConnection, table_name, field_name, error);
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  if(error.get()) throw *error;
  m_refGdaConnection->update_meta_store_table(table_name, m_backend->get_public_schema_name());
#else
  if(error.get())
    std::cerr << "Error: " << error->what() << std::endl;
  m_refGdaConnection->update_meta_store_table(table_name, m_backend->get_public_schema_name(), error);
  if(error.get())
    std::cerr << "Error: " << error->what() << std::endl;
#endif
  return result;
}

#ifdef GLIBMM_EXCEPTIONS_ENABLED
bool ConnectionPool::change_column(const Glib::ustring& table_name, const sharedptr<const Field>& field_old, const sharedptr<const Field>& field)
#else
bool ConnectionPool::change_column(const Glib::ustring& table_name, const sharedptr<const Field>& field_old, const sharedptr<const Field>& field, std::auto_ptr<Glib::Error>& error)
#endif
{
  type_vec_const_fields old_fields(1, field_old);
  type_vec_const_fields new_fields(1, field);

#ifdef GLIBMM_EXCEPTIONS_ENABLED
  return change_columns(table_name, old_fields, new_fields);
#else
  return change_columns(table_name, old_fields, new_fields, error);
#endif
}

#ifdef GLIBMM_EXCEPTIONS_ENABLED
bool ConnectionPool::change_columns(const Glib::ustring& table_name, const type_vec_const_fields& old_fields, const type_vec_const_fields& new_fields)
#else
bool ConnectionPool::change_columns(const Glib::ustring& table_name, const type_vec_const_fields& old_fields, const type_vec_const_fields& new_fields, std::auto_ptr<Glib::Error>& error)
#endif
{
  sharedptr<SharedConnection> conn;
  if(!m_refGdaConnection)
  {
#ifdef GLIBMM_EXCEPTIONS_ENABLED
    conn = connect();
#else
    std::auto_ptr<ExceptionConnection> local_error;
    // TODO: We can't rethrow local_error here since ExceptionConnection does
    // not derive from Glib::Error
    conn = connect(local_error);
#endif
  }

  if(!m_refGdaConnection)
    return false;

#ifdef GLIBMM_EXCEPTIONS_ENABLED
  std::auto_ptr<Glib::Error> error;
#endif
  const bool result = m_backend->change_columns(m_refGdaConnection, table_name, old_fields, new_fields, error);
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  if(error.get()) throw *error;
  m_refGdaConnection->update_meta_store_table(table_name, m_backend->get_public_schema_name());

#else
  if(error.get())
    std::cerr << "Error: " << error->what() << std::endl;
  m_refGdaConnection->update_meta_store_table(table_name, m_backend->get_public_schema_name(), error);
#endif
  return result;
}

ConnectionPool::InitErrors ConnectionPool::initialize(const SlotProgress& slot_progress, bool network_shared)
{
  if(m_backend.get())
    return m_backend->initialize(slot_progress, get_user(), get_password(), network_shared);
  else
    return Backend::INITERROR_OTHER;
}

Document* ConnectionPool::get_document()
{
  if(!m_slot_get_document)
  {
    std::cerr << "Glom ConnectionPool::get_document(): m_slot_get_document is null." << std::endl;
    return 0;
  }

  return m_slot_get_document();
}

#ifndef G_OS_WIN32
//static
EpcContents* ConnectionPool::on_publisher_document_requested(EpcPublisher* /* publisher */, const gchar* /* key */, gpointer user_data)
{
  Glom::ConnectionPool* connection_pool = static_cast<Glom::ConnectionPool*>(user_data);
  if(!connection_pool)
    return 0;

  const Document* document = connection_pool->get_document();
  if(!document)
    return 0;

  const Glib::ustring contents = document->get_contents();
  //std::cout << "DEBUG: ConnectionPool::on_publisher_document_requested(): returning: " << std::endl << "  " << contents << std::endl;
  return epc_contents_new_dup ("text/plain", (void*)contents.c_str(), -1);
}




//static
gboolean ConnectionPool::on_publisher_document_authentication(EpcAuthContext* context, const gchar* user_name, gpointer user_data)
{
  g_return_val_if_fail(context, false);

  ConnectionPool* connection_pool = (ConnectionPool*)(user_data);
  g_return_val_if_fail(connection_pool, false);

  // Check if the username/password are correct:
  const gchar* password = epc_auth_context_get_password(context);
  g_return_val_if_fail(password, false); //TODO: This seems to happen once before this callback is called again properly.

  //std::cout << "ConnectionPool::on_publisher_document_authentication(): username=" << user_name << ", password=" << password << std::endl;

  g_return_val_if_fail(connection_pool->m_backend.get(), false);

  //Attempt a connection with this username/password:
  std::auto_ptr<ExceptionConnection> error;
  Glib::RefPtr<Gnome::Gda::Connection> connection = connection_pool->m_backend->connect(connection_pool->get_database(), user_name, password, error);

  if(connection)
  {
    //std::cout << "ConnectionPool::on_publisher_document_authentication(): succeeded." << std::endl;
    return true; //Succeeded.
  }
  else
  {
    //std::cout << "ConnectionPool::on_publisher_document_authentication(): failed." << std::endl;
    return false; //Failed.
  }
}


void ConnectionPool::on_epc_progress_begin(const gchar* /* title */, gpointer user_data)
{
  //We ignore the title parameter because there is no way that libepc could know what Glom wants to say.

  ConnectionPool* connection_pool = (ConnectionPool*)user_data;
  if(connection_pool)
    connection_pool->m_epc_slot_begin();
}

void ConnectionPool::on_epc_progress_update(gdouble /* progress */, const gchar* /* message */, gpointer user_data)
{
  //We ignore the title parameter because there is no way that libepc could know what Glom wants to say.
  //TODO: Show the progress in a ProgressBar.

  ConnectionPool* connection_pool = (ConnectionPool*)user_data;
  if(connection_pool)
    connection_pool->m_epc_slot_progress();
}

void ConnectionPool::on_epc_progress_end(gpointer user_data)
{
  ConnectionPool* connection_pool = (ConnectionPool*)user_data;
  if(connection_pool)
    connection_pool->m_epc_slot_done();
}

void ConnectionPool::set_avahi_publish_callbacks(const type_void_slot& slot_begin, const type_void_slot& slot_progress, const type_void_slot& slot_done)
{
  m_epc_slot_begin = slot_begin;
  m_epc_slot_progress = slot_progress;
  m_epc_slot_done = slot_done;
}


/** Advertise self-hosting via avahi:
 */
void ConnectionPool::avahi_start_publishing()
{
  // Don't advertize if the database cannot be accessed remotely anyway
  if(!m_backend->supports_remote_access())
    return;

  if(m_epc_publisher)
    return;
#ifdef GLOM_CONNECTION_DEBUG
  std::cout << "debug: ConnectionPool::avahi_start_publishing" << std::endl;
#endif

  //Publish the document contents over HTTPS (discoverable via avahi):
  const Document* document = get_document();
  if(!document)
    return;

  m_epc_publisher = epc_publisher_new(document->get_database_title().c_str(), "glom", 0);
  epc_publisher_set_protocol(m_epc_publisher, publish_protocol);

  epc_publisher_add_handler(m_epc_publisher, "document", on_publisher_document_requested, this /* user_data */, 0);

  //Password-protect the document,
  //because the document's structure could be considered as a business secret:
  epc_publisher_set_auth_flags(m_epc_publisher, EPC_AUTH_PASSWORD_TEXT_NEEDED);
  epc_publisher_set_auth_handler(m_epc_publisher, "document", on_publisher_document_authentication, this /* user_data */, 0);

  //Install progress callback, so we can keep the UI responsive while libepc is generating certificates for the first time:
  EpcShellProgressHooks callbacks;
  callbacks.begin = &ConnectionPool::on_epc_progress_begin;
  callbacks.update = &ConnectionPool::on_epc_progress_update;
  callbacks.end = &ConnectionPool::on_epc_progress_end;
  epc_shell_set_progress_hooks(&callbacks, this, 0);

  //Prevent the consumer from seeing duplicates,
  //if multiple client computers advertize the same document:
  //
  //Defer announcement until a duplicate disappears:
  epc_publisher_set_collision_handling(m_epc_publisher, EPC_COLLISIONS_UNIQUE_SERVICE);
  //
  // How to uniquely-identify the service. TODO: Use some additional criteria, such as the real host name.
  if(!m_database.empty())
    epc_publisher_set_service_cookie(m_epc_publisher, m_database.c_str());

  //Start the publisher, serving HTTPS:
  GError* error = 0;
  epc_publisher_run_async(m_epc_publisher, &error);
  if(error)
  {
#ifdef GLOM_CONNECTION_DEBUG
    std::cout << "Glom: ConnectionPool::avahi_start_publishing(): Error while running epc_publisher_run_async: " << error->message << std::endl;
#endif
    g_clear_error(&error);
  }
}

void ConnectionPool::avahi_stop_publishing()
{
  if(!m_backend->supports_remote_access())
    return;

  if(!m_epc_publisher)
    return;
#ifdef GLOM_CONNECTION_DEBUG
  std::cout << "debug: ConnectionPool::avahi_stop_publishing" << std::endl;
#endif

#ifndef G_OS_WIN32
  epc_publisher_quit(m_epc_publisher);
#endif // !G_OS_WIN32
  g_object_unref(m_epc_publisher);
  m_epc_publisher = 0;
}
#endif // !G_OS_WIN32

void ConnectionPool::set_get_document_func(const SlotGetDocument& slot)
{
  m_slot_get_document = slot;
}


} //namespace Glom
