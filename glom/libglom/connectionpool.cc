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

#include <libglom/libglom_config.h> // For GLOM_ENABLE_MAEMO
 
#include <libglom/connectionpool.h>
#include <libglom/document/document_glom.h>
#include <libglom/utils.h>
//#include <libgdamm/connectionevent.h>
#include <gtkmm/main.h>
#include <gtkmm/messagedialog.h>
#include <glibmm/i18n.h>

#ifndef G_OS_WIN32
# include <libepc/shell.h> //For epc_shell_set_progress_hooks().
# include <libepc/publisher.h>
#else
// objidl.h, included by windows.h, defines a type called DATADIR, so we need
// to undef it temporarily.
# define GLOM_SAVE_DATADIR DATADIR
# undef DATADIR
# include <windows.h>
# define DATADIR GLOM_SAVE_DATADIR
#endif

#ifdef GLOM_ENABLE_MAEMO
#include <hildonmm/note.h>
#endif

#include <signal.h> //To catch segfaults

#ifndef GLOM_ENABLE_CLIENT_ONLY
#ifndef G_OS_WIN32
static EpcProtocol publish_protocol = EPC_PROTOCOL_HTTPS;
#endif
#endif

// Uncomment to see debug messages
//#define GLOM_CONNECTION_DEBUG

namespace
{

#ifdef G_OS_WIN32
static BOOL
pgwin32_get_dynamic_tokeninfo(HANDLE token, TOKEN_INFORMATION_CLASS class_,
                char **InfoBuffer, char *errbuf, int errsize)
{
  DWORD    InfoBufferSize;

  if(GetTokenInformation(token, class_, NULL, 0, &InfoBufferSize))
  {
    snprintf(errbuf, errsize, "could not get token information: got zero size\n");
    return FALSE;
  }

  if(GetLastError() != ERROR_INSUFFICIENT_BUFFER)
  {
    snprintf(errbuf, errsize, "could not get token information: error code %d\n",
         (int) GetLastError());
    return FALSE;
  }

  *InfoBuffer = static_cast<char*>(malloc(InfoBufferSize));
  if(*InfoBuffer == NULL)
  {
    snprintf(errbuf, errsize, "could not allocate %d bytes for token information\n",
         (int) InfoBufferSize);
    return FALSE;
  }

  if(!GetTokenInformation(token, class_, *InfoBuffer,
               InfoBufferSize, &InfoBufferSize))
  {
    snprintf(errbuf, errsize, "could not get token information: error code %d\n",
         (int) GetLastError());
    return FALSE;
  }

  return TRUE;
}

int
pgwin32_is_admin(void)
{
  HANDLE    AccessToken;
  char     *InfoBuffer = NULL;
  char    errbuf[256];
  PTOKEN_GROUPS Groups;
  PSID    AdministratorsSid;
  PSID    PowerUsersSid;
  SID_IDENTIFIER_AUTHORITY NtAuthority = {SECURITY_NT_AUTHORITY};
  UINT    x;
  BOOL    success;

  if(!OpenProcessToken(GetCurrentProcess(), TOKEN_READ, &AccessToken))
  {
    throw std::runtime_error(Glib::ustring::compose("Could not open process token: error code %1", (int)GetLastError()));
  }

  if(!pgwin32_get_dynamic_tokeninfo(AccessToken, TokenGroups,
                     &InfoBuffer, errbuf, sizeof(errbuf)))
  {
    CloseHandle(AccessToken);
    throw std::runtime_error(errbuf);
  }

  Groups = (PTOKEN_GROUPS) InfoBuffer;

  CloseHandle(AccessToken);

  if(!AllocateAndInitializeSid(&NtAuthority, 2,
     SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0,
                  0, &AdministratorsSid))
  {
    free(InfoBuffer);
    throw std::runtime_error(Glib::ustring::compose("could not get SID for Administrators group: error code %1", (int)GetLastError()));
  }

  if(!AllocateAndInitializeSid(&NtAuthority, 2,
  SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_POWER_USERS, 0, 0, 0, 0, 0,
                  0, &PowerUsersSid))
  {
    free(InfoBuffer);
    FreeSid(AdministratorsSid);
    throw std::runtime_error(Glib::ustring::compose("could not get SID for PowerUsers group: error code %1", (int) GetLastError()));
  }

  success = FALSE;

  for (x = 0; x < Groups->GroupCount; x++)
  {
    if((EqualSid(AdministratorsSid, Groups->Groups[x].Sid) && (Groups->Groups[x].Attributes & SE_GROUP_ENABLED)) ||
      (EqualSid(PowerUsersSid, Groups->Groups[x].Sid) && (Groups->Groups[x].Attributes & SE_GROUP_ENABLED)))
    {
      success = TRUE;
      break;
    }
  }

  free(InfoBuffer);
  FreeSid(AdministratorsSid);
  FreeSid(PowerUsersSid);
  return success;
}

#endif // G_OS_WIN32
} // anonymous namespace

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
#ifndef GLOM_ENABLE_CLIENT_ONLY
  m_epc_publisher(0),
  m_dialog_epc_progress(0),
#endif // !GLOM_ENABLE_CLIENT_ONLY
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
  if(connection_pool)
  {
#ifdef GLIBMM_EXCEPTIONS_ENABLED
    result = connection_pool->connect();
#else
    result = connection_pool->connect(error);
#endif // GLIBMM_EXCEPTIONS_ENABLED
  }

  return result;
}



// Store the connection for a few seconds in case it 
// is immediately requested again, to avoid making a new connection 
// and introspecting again, which is slow. 
static sharedptr<SharedConnection> connection_cached;
static sigc::connection connection_cached_timeout_connection;

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
  if(get_ready_to_connect())
  {
    //If the connection is already open (because it is being used by somebody):
    if(m_refGdaConnection)
    {
      sharedptr<SharedConnection> sharedConnection( new SharedConnection(m_refGdaConnection) );

      //Ask for notification when the SharedConnection has been finished with:
      sharedConnection->signal_finished().connect( sigc::mem_fun(*this, &ConnectionPool::on_sharedconnection_finished) );

      //Remember that somebody is using it:
      m_sharedconnection_refcount++;

      //Store the connection in a cache for a few seconds to avoid unnecessary disconnections/reconnections:
      //std::cout << "DEBUG: Stored connection cache." << std::endl;
      connection_cached = sharedConnection;
      connection_cached_timeout_connection.disconnect(); //Stop the existing timeout if it has not run yet.
      connection_cached_timeout_connection = Glib::signal_timeout().connect_seconds(&on_connection_pool_cache_timeout, 30 /* seconds */);

      return sharedConnection;
    }
    else if(connection_cached)
    {
      //Avoid a reconnection immediately after disconnecting:
      return connection_cached;
    }
    else
    {
#ifdef GLIBMM_EXCEPTIONS_ENABLED
      std::auto_ptr<ExceptionConnection> error;
#endif
      if(m_backend.get())
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
        std::cout << "DEBUG: Calling update_meta_store_data_types() ..." << std::endl;
        m_refGdaConnection->update_meta_store_data_types();
        std::cout << "DEBUG: ... update_meta_store_data_types() has finished." << std::endl;
        std::cout << "DEBUG: Calling update_meta_store_table_names() ..." << std::endl;
        m_refGdaConnection->update_meta_store_table_names();
        std::cout << "DEBUG: ... update_meta_store_table_names() has finished." << std::endl;

        // Connection succeeded
        // Create the fieldtypes member if it has not already been done:
        if(!m_pFieldTypes)
          m_pFieldTypes = new FieldTypes(m_refGdaConnection);
          
#ifndef GLOM_ENABLE_CLIENT_ONLY
#ifndef G_OS_WIN32
        //Let other clients discover this server via avahi:
        //TODO: Only advertize if we are the first to open the document,
        //to avoid duplicates.
        //TODO: Only advertize if it makes sense for the backend,
        //it does not for sqlite
        avahi_start_publishing(); //Stopped in the signal_finished handler.
#endif // !G_OS_WIN32
#endif // !GLOM_ENABLE_CLIENT_ONLY

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

#ifndef GLOM_ENABLE_CLIENT_ONLY
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
#endif // !GLOM_ENABLE_CLIENT_ONLY

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
  connection_cached.clear();
}

void ConnectionPool::set_password(const Glib::ustring& value)
{
  m_password = value;
  
  //Make sure that connect() makes a new connection:
  connection_cached.clear();
}

void ConnectionPool::set_database(const Glib::ustring& value)
{
  m_database = value;

  //Make sure that connect() makes a new connection:
  connection_cached.clear();
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

#ifndef GLOM_ENABLE_CLIENT_ONLY
#ifndef G_OS_WIN32
    //TODO: this should only even be started if we are the first to open the .glom file:
    avahi_stop_publishing();
#endif
#endif

    //g_warning("ConnectionPool: connection closed");
  }
}

//static
bool ConnectionPool::handle_error(bool cerr_only)
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
            error_details += "\n"; //Add newline after each error.

          error_details += (*iter)->get_description();
          std::cerr << "Internal error (Database): " << error_details << std::endl;
        }
      }

      //For debugging only:
      //Gtk::Dialog* dialog = 0;
      //dialog->run(); //Force a crash.

      if(!cerr_only)
      {
#ifdef GLOM_ENABLE_MAEMO
        Hildon::Note dialog(Hildon::NOTE_TYPE_INFORMATION, error_details);
#else
        Gtk::MessageDialog dialog(Utils::bold_message(_("Internal error")), true, Gtk::MESSAGE_WARNING );
        dialog.set_secondary_text(error_details);
#endif
        //TODO: dialog.set_transient_for(*get_application());
        dialog.run(); //TODO: This segfaults in gtk_window_set_modal() when this method is run a second time, for instance if there are two database errors.
#ifdef GLOM_CONNECTION_DEBUG
        std::cout << "debug: after Internal Error dialog run()." << std::endl;
#endif
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

bool ConnectionPool::startup(const SlotProgress& slot_progress)
{
  if(!m_backend.get())
    return false;

  if(!m_backend->startup(slot_progress))
    return false;

#ifndef G_OS_WIN32
  //Let clients discover this server via avahi:
  //avahi_start_publishing();
#endif // !G_OS_WIN32

  //If we crash while running (unlikely, hopefully), then try to cleanup.
  //Comment this out if you want to see the backtrace in a debugger.
  previous_sig_handler = signal(SIGSEGV, &on_linux_signal);

  return true;
}

void ConnectionPool::cleanup(const SlotProgress& slot_progress)
{
  if(m_backend.get())
    m_backend->cleanup(slot_progress);

  //Make sure that connect() makes a new connection:
  connection_cached.clear();

#ifndef G_OS_WIN32
  /* Stop advertising the self-hosting database server via avahi: */
  //avahi_stop_publishing();
#endif // !G_OS_WIN32

  //We don't need the segfault handler anymore:
  signal(SIGSEGV, previous_sig_handler);
  previous_sig_handler = SIG_DFL; /* Arbitrary default */
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
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
  const bool result = m_backend->add_column(m_refGdaConnection, table_name, field, error);
  if(error.get()) throw *error;

  m_refGdaConnection->update_meta_store_table(table_name);
#else
  const bool result = m_backend->add_column(m_refGdaConnection, table_name, field, error);
  if(result)
    result = m_refGdaConnection->update_meta_store_table(table_name, error);
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
  const bool result = m_backend->drop_column(m_refGdaConnection, table_name, field_name, error);
  if(error.get()) throw *error;

  m_refGdaConnection->update_meta_store_table(table_name);
#else
  const bool result = m_backend->drop_column(m_refGdaConnection, table_name, field_name, error);
  if(result)
    result = m_refGdaConnection->update_meta_store_table(table_name, error);
#endif

  return result;
}

#ifdef GLIBMM_EXCEPTIONS_ENABLED
bool ConnectionPool::change_column(const Glib::ustring& table_name, const sharedptr<const Field>& field_old, const sharedptr<const Field>& field)
#else
bool ConnectionPool::change_column(const Glib::ustring& table_name, const sharedptr<const Field>& field_old, const sharedptr<const Field>& field, std::auto_ptr<Glib::Error>& error)
#endif
{
  type_vecConstFields old_fields(1, field_old);
  type_vecConstFields new_fields(1, field);

#ifdef GLIBMM_EXCEPTIONS_ENABLED
  return change_columns(table_name, old_fields, new_fields);
#else
  return change_columns(table_name, old_fields, new_fields, error);
#endif
}

#ifdef GLIBMM_EXCEPTIONS_ENABLED
bool ConnectionPool::change_columns(const Glib::ustring& table_name, const type_vecConstFields& old_fields, const type_vecConstFields& new_fields)
#else
bool ConnectionPool::change_columns(const Glib::ustring& table_name, const type_vecConstFields& old_fields, const type_vecConstFields& new_fields, std::auto_ptr<Glib::Error>& error)
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
  const bool result = m_backend->change_columns(m_refGdaConnection, table_name, old_fields, new_fields, error);
  if(error.get()) throw *error;

  m_refGdaConnection->update_meta_store_table(table_name);
#else
  const bool result = m_backend->change_columns(m_refGdaConnection, table_name, old_fields, new_fields, error);
  if(result)
    result = m_refGdaConnection->update_meta_store_table(table_name, error);
#endif

  return result;
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

bool ConnectionPool::initialize(const SlotProgress& slot_progress)
{
  if(m_backend.get())
    return m_backend->initialize(slot_progress, get_user(), get_password());
  else
    return false;
}

bool ConnectionPool::check_user_is_not_root()
{
  Glib::ustring message;
#ifdef G_OS_WIN32
  try
  {
    if(pgwin32_is_admin())
    {
      message = _("You seem to be running Glom as a user with administrator privileges. Glom may not be run with such privileges for security reasons.\nPlease login to your system as a normal user.");
    }
  }
  catch(const std::runtime_error& ex)
  {
    message = ex.what();
  }
#else
  //std::cout << "ConnectionPool::check_user_is_not_root(): geteuid()=" << geteuid() << ", getgid()=" << getgid() << std::endl;

  //This is very linux-specific. We should ifdef this out for other platforms.
  if(geteuid() == 0)
  {
    //Warn the user:
    message = _("You seem to be running Glom as root. Glom may not be run as root.\nPlease login to your system as a normal user.");
  }
#endif

  if(!message.empty())
  {
#ifndef GLOM_ENABLE_MAEMO
    Gtk::MessageDialog dialog(Utils::bold_message(_("Running As Root")), true /* use_markup */, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true /* modal */);
    dialog.set_secondary_text(message);
    dialog.run();
#else
    Hildon::Note note(Hildon::NOTE_TYPE_INFORMATION, message);
    note.run();
#endif

    return false; /* Is root. Bad. */
  }

  return true; /* Not root. It's OK. */
}


#ifndef GLOM_ENABLE_CLIENT_ONLY
Document_Glom* ConnectionPool::get_document()
{
  if(!m_slot_get_document)
  {
    std::cerr << "Glom ConnectionPool::get_document(): m_slot_get_document is null." << std::endl;
    return 0;
  }

  return m_slot_get_document();
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

#ifndef GLOM_ENABLE_CLIENT_ONLY
#ifndef G_OS_WIN32
//static
EpcContents* ConnectionPool::on_publisher_document_requested(EpcPublisher* /* publisher */, const gchar* /* key */, gpointer user_data)
{
  Glom::ConnectionPool* connection_pool = static_cast<Glom::ConnectionPool*>(user_data);
  if(!connection_pool)
    return 0;

  const Document_Glom* document = connection_pool->get_document();
  if(!document)
    return 0;

  const Glib::ustring contents = document->get_contents();
  //std::cout << "DEBUG: ConnectionPool::on_publisher_document_requested(): returning: " << std::endl << "  " << contents << std::endl;
  return epc_contents_new_dup ("text/plain", (void*)contents.c_str(), -1);
}




//static
gboolean ConnectionPool::on_publisher_document_authentication(EpcAuthContext* context, const gchar* user_name, gpointer user_data)
{
  g_return_val_if_fail(context, FALSE);

  ConnectionPool* connection_pool = (ConnectionPool*)(user_data);
  g_return_val_if_fail(connection_pool, FALSE);

  // Check if the username/password are correct:
  const gchar* password = epc_auth_context_get_password(context);
  g_return_val_if_fail(password, FALSE); //TODO: This seems to happen once before this callback is called again properly.

  //std::cout << "ConnectionPool::on_publisher_document_authentication(): username=" << user_name << ", password=" << password << std::endl;

  g_return_val_if_fail(connection_pool->m_backend.get(), FALSE);
 
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

  //Create the dialog:
  if(connection_pool->m_dialog_epc_progress)
  {
    delete connection_pool->m_dialog_epc_progress;
    connection_pool->m_dialog_epc_progress = 0;
  }

  Gtk::MessageDialog* message_dialog = new Gtk::MessageDialog(Utils::bold_message(_("Glom: Generating Encryption Certificates")), true, Gtk::MESSAGE_INFO);
  message_dialog->set_secondary_text(_("Please wait while Glom prepares your system for publishing over the network."));
  message_dialog->show();

  connection_pool->m_dialog_epc_progress = message_dialog; 
}

void ConnectionPool::on_epc_progress_update(gdouble /* progress */, const gchar* /* message */, gpointer /* user_data */)
{
  //We ignore the title parameter because there is no way that libepc could know what Glom wants to say.
  //TODO: Show the progress in a ProgressBar.

  //ConnectionPool* connection_pool = (ConnectionPool*)user_data;

  //Allow GTK+ to process events, so that the UI is responsive:
  while(Gtk::Main::events_pending())
   Gtk::Main::iteration();
}

void ConnectionPool::on_epc_progress_end(gpointer user_data)
{
  ConnectionPool* connection_pool = (ConnectionPool*)user_data;

  //Delete the dialog:
  if(connection_pool->m_dialog_epc_progress)
  {
    delete connection_pool->m_dialog_epc_progress;
    connection_pool->m_dialog_epc_progress = 0;
  }
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
  const Document_Glom* document = get_document();
  if(!document)
    return;

  m_epc_publisher = epc_publisher_new(document->get_database_title().c_str(), "glom", NULL);
  epc_publisher_set_protocol(m_epc_publisher, publish_protocol);
  
  epc_publisher_add_handler(m_epc_publisher, "document", on_publisher_document_requested, this /* user_data */, NULL);

  //Password-protect the document,
  //because the document's structure could be considered as a business secret:
  epc_publisher_set_auth_flags(m_epc_publisher, EPC_AUTH_PASSWORD_TEXT_NEEDED);
  epc_publisher_set_auth_handler(m_epc_publisher, "document", on_publisher_document_authentication, this /* user_data */, NULL);

  //Install progress callback, so we can keep the UI responsive while libepc is generating certificates for the first time:
  EpcShellProgressHooks callbacks;
  callbacks.begin = &ConnectionPool::on_epc_progress_begin;
  callbacks.update = &ConnectionPool::on_epc_progress_update;
  callbacks.end = &ConnectionPool::on_epc_progress_end;
  epc_shell_set_progress_hooks(&callbacks, this, NULL);
      
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
  m_epc_publisher = NULL;
}
#endif // !G_OS_WIN32

void ConnectionPool::set_get_document_func(const SlotGetDocument& slot)
{
  m_slot_get_document = slot;
}

#endif // !GLOM_ENABLE_CLIENT_ONLY


} //namespace Glom
