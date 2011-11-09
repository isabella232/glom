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

#include "config.h" // For POSTGRES_UTILS_PATH
#include <libglom/libglom_config.h>

#include <libglom/connectionpool_backends/postgres.h>
#include <libglom/spawn_with_feedback.h>
#include <libglom/utils.h>
#include <libgdamm/config.h>
#include <giomm/file.h>
#include <glibmm/convert.h>
#include <glibmm/miscutils.h>
#include <glibmm/shell.h>
#include <glib/gstdio.h> /* For g_rename(). TODO: Wrap this in glibmm? */
#include <glibmm/i18n.h>

#include <iostream>

// Uncomment to see debug messages
//#define GLOM_CONNECTION_DEBUG

namespace
{

static Glib::ustring create_auth_string(const Glib::ustring& username, const Glib::ustring& password)
{
  if(username.empty() and password.empty())
    return Glib::ustring();
  else
    return "USERNAME=" + username + ";PASSWORD=" + password; //TODO: How should we quote and escape these?
}

} //anonymous namespace

namespace Glom
{

namespace ConnectionPoolBackends
{

Postgres::Postgres()
: m_port(0),
  m_postgres_server_version(0.0f)
{
}

float Postgres::get_postgres_server_version() const
{
  return m_postgres_server_version;
}

Glib::RefPtr<Gnome::Gda::Connection> Postgres::attempt_connect(const Glib::ustring& port, const Glib::ustring& database, const Glib::ustring& username, const Glib::ustring& password, std::auto_ptr<ExceptionConnection>& error) throw()
{
  //We must specify _some_ database even when we just want to create a database.
  //This _might_ be different on some systems. I hope not. murrayc
  const Glib::ustring default_database = "template1";
  //const Glib::ustring& actual_database = (!database.empty()) ? database : default_database;;
  const Glib::ustring cnc_string_main = "HOST=" + m_host + ";PORT=" + port;
  Glib::ustring cnc_string = cnc_string_main + ";DB_NAME=" + database;

  Glib::RefPtr<Gnome::Gda::Connection> connection;
  Glib::RefPtr<Gnome::Gda::DataModel> data_model;

  const Glib::ustring auth_string = create_auth_string(username, password);

#ifdef GLOM_CONNECTION_DEBUG
  std::cout << std::endl << "DEBUG: Glom: trying to connect on port=" << port << std::endl;
  std::cout << "DEBUG: ConnectionPoolBackends::Postgres::attempt_connect(): cnc_string=" << cnc_string << std::endl;
  std::cout << "  DEBUG: auth_string=" << auth_string << std::endl;
#endif

#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
    connection = Gnome::Gda::Connection::open_from_string("PostgreSQL",
      cnc_string, auth_string,
      Gnome::Gda::CONNECTION_OPTIONS_SQL_IDENTIFIERS_CASE_SENSITIVE
      );
    connection->statement_execute_non_select("SET DATESTYLE = 'ISO'");
    data_model = connection->statement_execute_select("SELECT version()");
  }
  catch(const Gnome::Gda::ConfigError& ex)
  {
    //These errors are unusual.
    //For instance, the PostgreSQL libgda provider could be missing,
    //though we check for that at startup.
    std::cerr << G_STRFUNC << 
      ": ConfigError exception from Gnome::Gda::Connection::open_from_string(): " <<
      ex.what();
  }
  catch(const Glib::Error& ex)
  {
#else
  std::auto_ptr<Glib::Error> ex;
  connection = Gnome::Gda::Connection::open_from_string("PostgreSQL",
    cnc_string, auth_string,
    Gnome::Gda::CONNECTION_OPTIONS_SQL_IDENTIFIERS_CASE_SENSITIVE,
    ex);

  if(!ex.get())
    connection->statement_execute_non_select("SET DATESTYLE = 'ISO'", ex);

  if(!ex.get())
    data_model = connection->statement_execute_select("SELECT version()", Gnome::Gda::STATEMENT_MODEL_RANDOM_ACCESS, ex);

  if(!ex.get())
  {
#endif

#ifdef GLOM_CONNECTION_DEBUG
    std::cout << "ConnectionPoolBackends::Postgres::attempt_connect(): Attempt to connect to database failed on port=" << port << ", database=" << database << ": " << ex.what() << std::endl;
    std::cout << "ConnectionPoolBackends::Postgres::attempt_connect(): Attempting to connect without specifying the database." << std::endl;
#endif

    Glib::ustring cnc_string = cnc_string_main + ";DB_NAME=" + default_database;
    Glib::RefPtr<Gnome::Gda::Connection> temp_conn;
    Glib::ustring auth_string = create_auth_string(username, password);
#ifdef GLIBMM_EXCEPTIONS_ENABLED
    try
    {
      temp_conn = Gnome::Gda::Connection::open_from_string("PostgreSQL",
        cnc_string, auth_string,
        Gnome::Gda::CONNECTION_OPTIONS_SQL_IDENTIFIERS_CASE_SENSITIVE);
    }
    catch(const Gnome::Gda::ConfigError& ex)
    {
      //These errors are unusual.
      //For instance, the PostgreSQL libgda provider could be missing,
      //though we check for that at startup.
      std::cerr << G_STRFUNC << 
        ": ConfigError exception from Gnome::Gda::Connection::open_from_string(): " <<
        ex.what();
    }
    catch(const Glib::Error& ex)
    {}
#else
    temp_conn = Gnome::Gda::Connection::open_from_string("PostgreSQL",
      cnc_string, auth_string,
      Gnome::Gda::CONNECTION_OPTIONS_SQL_IDENTIFIERS_CASE_SENSITIVE, ex);
#endif

#ifdef GLOM_CONNECTION_DEBUG
    if(temp_conn)
      std::cout << "  (Connection succeeds, but not to the specific database,  database=" << database << std::endl;
    else
      std::cerr << "  (Could not connect even to the default database, database=" << database  << std::endl;
#endif

    error.reset(new ExceptionConnection(temp_conn ? ExceptionConnection::FAILURE_NO_DATABASE : ExceptionConnection::FAILURE_NO_SERVER));
    return Glib::RefPtr<Gnome::Gda::Connection>();
  }

  if(data_model && data_model->get_n_rows() && data_model->get_n_columns())
  {
#ifdef GLIBMM_EXCEPTIONS_ENABLED
    Gnome::Gda::Value value = data_model->get_value_at(0, 0);
#else
    Gnome::Gda::Value value = data_model->get_value_at(0, 0, ex);
#endif
    if(value.get_value_type() == G_TYPE_STRING)
    {
      const Glib::ustring version_text = value.get_string();
      //This seems to have the format "PostgreSQL 7.4.11 on i486-pc-linux"
      const Glib::ustring namePart = "PostgreSQL ";
      const Glib::ustring::size_type posName = version_text.find(namePart);
      if(posName != Glib::ustring::npos)
      {
        const Glib::ustring versionPart = version_text.substr(namePart.size());
        m_postgres_server_version = strtof(versionPart.c_str(), 0);

#ifdef GLOM_CONNECTION_DEBUG
        std::cout << "  Postgres Server version: " << m_postgres_server_version << std::endl;
#endif
      }
    }
  }

  return connection;
}

bool Postgres::change_columns(const Glib::RefPtr<Gnome::Gda::Connection>& connection, const Glib::ustring& table_name, const type_vec_const_fields& old_fields, const type_vec_const_fields& new_fields, std::auto_ptr<Glib::Error>& error)
{
  static const char TRANSACTION_NAME[] = "glom_change_columns_transaction";
  static const char TEMP_COLUMN_NAME[] = "glom_temp_column"; // TODO: Find a unique name.

  if(!begin_transaction(connection, TRANSACTION_NAME, Gnome::Gda::TRANSACTION_ISOLATION_UNKNOWN, error)) return false; // TODO: What does the transaction isolation do?

  for(unsigned int i = 0; i < old_fields.size(); ++ i)
  {
    // If the type did change, then we need to recreate the column. See
    // http://www.postgresql.org/docs/faqs.FAQ.html#item4.3
    if(old_fields[i]->get_field_info()->get_g_type() != new_fields[i]->get_field_info()->get_g_type())
    {
      // Create a temporary column
      sharedptr<Field> temp_field = glom_sharedptr_clone(new_fields[i]);
      temp_field->set_name(TEMP_COLUMN_NAME);
      // The temporary column must not be primary key as long as the original
      // (primary key) column is still present, because there cannot be two
      // primary key columns.
      temp_field->set_primary_key(false);

      if(!add_column(connection, table_name, temp_field, error)) break;

      Glib::ustring conversion_command;
      const Glib::ustring field_name_old_quoted = "\"" + old_fields[i]->get_name() + "\"";
      const Field::glom_field_type old_field_type = old_fields[i]->get_glom_type();

      if(Field::get_conversion_possible(old_fields[i]->get_glom_type(), new_fields[i]->get_glom_type()))
      {
        //TODO: postgres seems to give an error if the data cannot be converted (for instance if the text is not a numeric digit when converting to numeric) instead of using 0.
        /*
        Maybe, for instance:
        http://groups.google.de/groups?hl=en&lr=&ie=UTF-8&frame=right&th=a7a62337ad5a8f13&seekm=23739.1073660245%40sss.pgh.pa.us#link5
        UPDATE _table
        SET _bbb = to_number(substring(_aaa from 1 for 5), '99999')
        WHERE _aaa <> '     ';
        */

        switch(new_fields[i]->get_glom_type())
        {
          case Field::TYPE_BOOLEAN:
          {
            if(old_field_type == Field::TYPE_NUMERIC)
            {
              conversion_command = "(CASE WHEN " + field_name_old_quoted + " > 0 THEN true "
                                         "WHEN " + field_name_old_quoted + " = 0 THEN false "
                                         "WHEN " + field_name_old_quoted + " IS NULL THEN false END)";
            }
            else if(old_field_type == Field::TYPE_TEXT)
              conversion_command = '(' + field_name_old_quoted + " !~~* \'false\')"; // !~~* means ! ILIKE
            else // Dates and Times:
              conversion_command = '(' + field_name_old_quoted + " IS NOT NULL)";
            break;
          }

          case Field::TYPE_NUMERIC: // CAST does not work if the destination type is numeric
          {
            if(old_field_type == Field::TYPE_BOOLEAN)
            {
              conversion_command = "(CASE WHEN " + field_name_old_quoted + " = true THEN 1 "
                                         "WHEN " + field_name_old_quoted + " = false THEN 0 "
                                         "WHEN " + field_name_old_quoted + " IS NULL THEN 0 END)";
            }
            else
            {
              //We use to_number, with textcat() so that to_number always has usable data.
              //Otherwise, it says
              //invalid input syntax for type numeric: " "
              //
              //We must use single quotes with the 0, otherwise it says "column 0 does not exist.".
              conversion_command = "to_number( textcat(\'0\', " + field_name_old_quoted + "), '999999999.99999999' )";
            }

            break;
          }

          case Field::TYPE_DATE: // CAST does not work if the destination type is date.
          {
            conversion_command = "to_date( " + field_name_old_quoted + ", 'YYYYMMDD' )"; // TODO: Standardise date storage format.
            break;
          }
          case Field::TYPE_TIME: // CAST does not work if the destination type is timestamp.
          {
            conversion_command = "to_timestamp( " + field_name_old_quoted + ", 'HHMMSS' )"; // TODO: Standardise time storage format.
            break;
          }

          default:
          {
            // To Text:

            // bool to text:
            if(old_field_type == Field::TYPE_BOOLEAN)
            {
              conversion_command = "(CASE WHEN " + field_name_old_quoted + " = true THEN \'true\' "
                                         "WHEN " + field_name_old_quoted + " = false THEN \'false\' "
                                         "WHEN " + field_name_old_quoted + " IS NULL THEN \'false\' END)";
            }
            else
            {
              // This works for most to-text conversions:
              conversion_command = "CAST(" + field_name_old_quoted + " AS " + new_fields[i]->get_sql_type() + ")";
            }

            break;
          }
        }

        if(!query_execute(connection, "UPDATE \"" + table_name + "\" SET \"" + TEMP_COLUMN_NAME + "\" = " + conversion_command, error))
          break;
      }
      else
      {
        // The conversion is not possible, so drop data in that column
      }

      if(!drop_column(connection, table_name, old_fields[i]->get_name(), error))
        break;

      if(!query_execute(connection, "ALTER TABLE \"" + table_name + "\" RENAME COLUMN \"" + TEMP_COLUMN_NAME + "\" TO \"" + new_fields[i]->get_name() + "\"", error))
        break;

      // Readd primary key constraint
      if(new_fields[i]->get_primary_key())
      {
        if(!query_execute(connection, "ALTER TABLE \"" + table_name + "\" ADD PRIMARY KEY (\"" + new_fields[i]->get_name() + "\")", error))
          break;
      }
    }
    else
    {
      // The type did not change. What could have changed: The field being a
      // unique key, primary key, its name or its default value.

      // Primary key
      // TODO: Test whether this is able to remove unique key constraints
      // added via libgda's DDL API in add_column(). Maybe override
      // add_column() if we can't.
      bool primary_key_was_set = false;
      bool primary_key_was_unset = false;
      if(old_fields[i]->get_primary_key() != new_fields[i]->get_primary_key())
      {
        if(new_fields[i]->get_primary_key())
        {
          primary_key_was_set = true;

          // Primary key was added
          if(!query_execute(connection, "ALTER TABLE \"" + table_name + "\" ADD PRIMARY KEY (\"" + old_fields[i]->get_name() + "\")", error))
            break;

          // Remove unique key constraint, because this is already implied in
          // the field being primary key.
          //TODO: This depends on knowledge of the automatically-created 
          //(during SERVER_OPERATION_CREATE_TABLE) constraint name.
          //TODO: Find out how the table name and field name are escaped/quoted.
          if(old_fields[i]->get_unique_key())
            if(!query_execute(connection, "ALTER TABLE \"" + table_name + "\" DROP CONSTRAINT \"" + table_name + "_" + old_fields[i]->get_name() + "_key", error))
              break;
        }
        else
        {
          primary_key_was_unset = true;

          // Primary key was removed
          if(!query_execute(connection, "ALTER TABLE \"" + table_name + "\" DROP CONSTRAINT \"" + table_name + "_pkey\"", error))
            break;
        }
      }

      // Uniqueness
      if(old_fields[i]->get_unique_key() != new_fields[i]->get_unique_key())
      {
        // Postgres automatically makes primary keys unique, so we do not need
        // to do that separately if we already made it a primary key
        if(!primary_key_was_set && new_fields[i]->get_unique_key())
        {
          //TODO: This depends on knowledge of the automatically-created 
          //(during SERVER_OPERATION_CREATE_TABLE) constraint name.
          //TODO: Find out how the table name and field name are escaped/quoted.
          if(!query_execute(connection, "ALTER TABLE \"" + table_name + "\" ADD CONSTRAINT \"" + table_name  + "_" + old_fields[i]->get_name() + "_key\" UNIQUE (\"" + old_fields[i]->get_name() + "\")", error))
            break;
        }
        else if(!primary_key_was_unset && !new_fields[i]->get_unique_key() && !new_fields[i]->get_primary_key())
        {
          //TODO: This depends on knowledge of the automatically-created 
          //(during SERVER_OPERATION_CREATE_TABLE) constraint name.
          //TODO: Find out how the table name and field name are escaped/quoted.
          if(!query_execute(connection, "ALTER TABLE \"" + table_name + "\" DROP CONSTRAINT \"" + table_name + "_" +old_fields[i]->get_name() + "_key\"", error))
            break;
        }
      }

      if(!new_fields[i]->get_auto_increment()) // Auto-increment fields have special code as their default values.
      {
        if(old_fields[i]->get_default_value() != new_fields[i]->get_default_value())
        {
          if(!query_execute(connection, "ALTER TABLE \"" + table_name + "\" ALTER COLUMN \"" + old_fields[i]->get_name() + "\" SET DEFAULT " + new_fields[i]->sql(new_fields[i]->get_default_value(), connection), error))
            break;
        }
      }

      if(old_fields[i]->get_name() != new_fields[i]->get_name())
      {
        if(!query_execute(connection, "ALTER TABLE \"" + table_name + "\" RENAME COLUMN \"" + old_fields[i]->get_name() + "\" TO \"" + new_fields[i]->get_name() + "\"", error))
          break;
      }
    }
  }

  if(error.get() || !commit_transaction(connection, TRANSACTION_NAME, error))
  {
    std::auto_ptr<Glib::Error> rollback_error;
    rollback_transaction(connection, TRANSACTION_NAME, rollback_error);
    return false;
  }

  return true;
}

bool Postgres::attempt_create_database(const Glib::ustring& database_name, const Glib::ustring& host, const Glib::ustring& port, const Glib::ustring& username, const Glib::ustring& password, std::auto_ptr<Glib::Error>& error)
{
  Glib::RefPtr<Gnome::Gda::ServerOperation> op;
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
    op = Gnome::Gda::ServerOperation::prepare_create_database("PostgreSQL",
                                                              database_name);
  }
  catch(const Glib::Error& ex)
  {
    error.reset(new Glib::Error(ex));
    return false;
  }
#else
  std::auto_ptr<Glib::Error> ex;
  op = Gnome::Gda::ServerOperation::prepare_create_database("PostgreSQL",
                                                            database_name,
                                                            ex);
  if(ex.get())
    return false;
#endif
  g_assert(op);
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
    op->set_value_at("/SERVER_CNX_P/HOST", host);
    op->set_value_at("/SERVER_CNX_P/PORT", port);
    op->set_value_at("/SERVER_CNX_P/ADM_LOGIN", username);
    op->set_value_at("/SERVER_CNX_P/ADM_PASSWORD", password);
    op->perform_create_database("PostgreSQL");
  }
  catch(const Glib::Error& ex)
  {
    error.reset(new Glib::Error(ex));
    return false;
  }
#else //GLIBMM_EXCEPTIONS_ENABLED
  op->set_value_at("/SERVER_CNX_P/HOST", host, error);
  op->set_value_at("/SERVER_CNX_P/PORT", port, error);
  op->set_value_at("/SERVER_CNX_P/ADM_LOGIN", username, error);
  op->set_value_at("/SERVER_CNX_P/ADM_PASSWORD", password, error);

  if(error.get() == 0)
    op->perform_create_database("PostgreSQL", ex);
  else
    return false;
  if (error.get())
    return false;
#endif //GLIBMM_EXCEPTIONS_ENABLED

  return true;
}

bool Postgres::check_postgres_gda_client_is_available()
{
  //This API is horrible.
  //See libgda bug http://bugzilla.gnome.org/show_bug.cgi?id=575754
  Glib::RefPtr<Gnome::Gda::DataModel> model = Gnome::Gda::Config::list_providers();
  if(model && model->get_n_columns() && model->get_n_rows())
  {
    Glib::RefPtr<Gnome::Gda::DataModelIter> iter = model->create_iter();

    do
    {
      //See http://library.gnome.org/devel/libgda/unstable/libgda-40-Configuration.html#gda-config-list-providers
      //about the columns of this DataModel:
      const Gnome::Gda::Value name = iter->get_value_at(0);
      if(name.get_value_type() != G_TYPE_STRING)
        continue;

      const Glib::ustring name_as_string = name.get_string();
      //std::cout << "DEBUG: Provider name:" << name_as_string << std::endl;
      if(name_as_string == "PostgreSQL")
        return true;
    }
    while(iter->move_next());
  }

  return false;
}

std::string Postgres::get_path_to_postgres_executable(const std::string& program, bool quoted)
{
#ifdef G_OS_WIN32
  // Add the .exe extension on Windows:
  std::string real_program = program + EXEEXT;

  // Have a look at the bin directory of the application executable first.
  // The installer installs postgres there. postgres needs to be installed
  // in a directory called bin for its relocation stuff to work, so that
  // it finds the share data in share. Unfortunately it does not look into
  // share/postgresql which would be nice to separate the postgres stuff
  // from the other shared data. We can perhaps still change this later by
  // building postgres with another prefix than /local/pgsql.
  gchar* installation_directory = g_win32_get_package_installation_directory_of_module(0);
  std::string test;

  try
  {
    test = Glib::build_filename(installation_directory, Glib::build_filename("bin", real_program));
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << ": exception from Glib::build_filename(): " << ex.what() << std::endl;
    return std::string();
  }

  g_free(installation_directory);

  if(Glib::file_test(test, Glib::FILE_TEST_IS_EXECUTABLE))
  {
    if(quoted)
      test = Glib::shell_quote(path);
    return test;
  }

  // Look in PATH otherwise
  std::string path = Glib::find_program_in_path(real_program);
  if(quoted)
    path = Glib::shell_quote(path);
  return path;
#else // G_OS_WIN32
  // POSTGRES_UTILS_PATH is defined in config.h, based on the configure.
  try
  {
    std::string path = Glib::build_filename(POSTGRES_UTILS_PATH, program + EXEEXT);
    if(quoted)
      path = Glib::shell_quote(path);
    return path;
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << ": exception from Glib::build_filename(): " << ex.what() << std::endl;
    return std::string();
  }
#endif // !G_OS_WIN32
}


Glib::ustring Postgres::port_as_string(unsigned int port_num)
{
  Glib::ustring result;
  char* cresult = g_strdup_printf("%u", port_num);
  if(cresult)
    result = cresult;
  g_free(cresult);

  return result;
}

//Because ~/.pgpass is not an absolute path.
static std::string get_absolute_pgpass_filepath()
{
  return Glib::build_filename(
    Glib::get_home_dir(), ".pgpass");
}

bool Postgres::save_password_to_pgpass(const Glib::ustring username, const Glib::ustring& password, std::string& filepath_previous, std::string& filepath_original)
{
  //Initialize output variables:
  filepath_previous.clear();
  filepath_original.clear();

  const std::string filepath_pgpass = get_absolute_pgpass_filepath();
  filepath_original = filepath_pgpass;

  //Move any existing file out of the way:
  if(file_exists_filepath(filepath_pgpass))
  {
    //std::cout << "DEBUG: File exists: " << filepath_pgpass << std::endl;
    filepath_previous = filepath_pgpass + ".glombackup";
    if(g_rename(filepath_pgpass.c_str(), filepath_previous.c_str()) != 0)
    {
      std::cerr << G_STRFUNC << "Could not rename file: from=" << filepath_pgpass << ", to=" << filepath_previous << std::endl;
      return false;
    }
  }

  //See http://www.postgresql.org/docs/8.4/static/libpq-pgpass.html
  //TODO: Escape \ and : characters.
  const Glib::ustring contents =
    m_host + ":" + port_as_string(m_port) + ":*:" + username + ":" + password;

  std::string uri;
  try
  {
    uri = Glib::filename_to_uri(filepath_pgpass);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << ": exception from Glib::filename_from_uri(): " << ex.what() << std::endl;
    g_rename(filepath_previous.c_str(), filepath_pgpass.c_str());
    return false;
  }

  const bool result = create_text_file(uri, contents, true /* current user only */);
  if(!result)
  {
    std::cerr << G_STRFUNC << ": create_text_file() failed." << std::endl;
    g_rename(filepath_previous.c_str(), filepath_pgpass.c_str());
    return false;
  }

  return result;
}

bool Postgres::save_backup(const SlotProgress& slot_progress, const Glib::ustring& username, const Glib::ustring& password, const Glib::ustring& database_name)
{
/* TODO:
  if(m_network_shared && !running)
  {
    std::cerr << G_STRFUNC << ": The self-hosted database is not running." << std::endl;
    return;
  }
*/

  if(m_host.empty())
  {
    std::cerr << G_STRFUNC << ": m_host is empty." << std::endl;
    return false;
  }

  if(m_port == 0)
  {
    std::cerr << G_STRFUNC << ": m_port is empty." << std::endl;
    return false;
  }

  //TODO: Remember the existing username and password?
  if(username.empty())
  {
    std::cerr << G_STRFUNC << ": username is empty." << std::endl;
    return false;
  }

  if(password.empty())
  {
    std::cerr << G_STRFUNC << ": password is empty." << std::endl;
    return false;
  }

  // Save the password to ~/.pgpass, because this is the only way to use
  // pg_dump without it asking for the password:
  std::string pgpass_backup, pgpass_original;
  const bool pgpass_created = save_password_to_pgpass(username, password, pgpass_backup, pgpass_original);
  if(!pgpass_created)
  {
    std::cerr << G_STRFUNC << ": save_password_to_pgpass() failed." << std::endl;
    return false;
  }

  const std::string path_backup = get_self_hosting_backup_path(std::string(), true /* create parent directory if necessary */);
  if(path_backup.empty())
    return false;

  // Make sure to use double quotes for the executable path, because the
  // CreateProcess() API used on Windows does not support single quotes.
  const std::string command_dump = get_path_to_postgres_executable("pg_dump") +
    " --format=c " + // The default (plain) format cannot be used with pg_restore.
    " --create --file=" + Glib::shell_quote(path_backup) +
    " --host=" + Glib::shell_quote(m_host) +
    " --port=" + port_as_string(m_port) +
    " --username=" + Glib::shell_quote(username) +
    " " + database_name; //TODO: Quote database_name?


  //std::cout << "DEBUG: command_dump=" << command_dump << std::endl;

  const bool result = Glom::Spawn::execute_command_line_and_wait(command_dump, slot_progress);

  //Move the previously-existing .pgpass file back:
  //TODO: Really, we should just edit the file instead of completely replacing it,
  //      because another application might try to edit it in the meantime.
  if(!pgpass_backup.empty())
  {
    g_rename(pgpass_backup.c_str(), pgpass_original.c_str());
  }

  if(!result)
  {
    std::cerr << "Error while attempting to call pg_dump." << std::endl;
  }

  return result;
}

bool Postgres::convert_backup(const SlotProgress& slot_progress, const std::string& base_directory, const Glib::ustring& username, const Glib::ustring& password, const Glib::ustring& database_name)
{
/* TODO:
  if(m_network_shared && !running)
  {
    std::cerr << G_STRFUNC << ": The self-hosted database is not running." << std::endl;
    return;
  }
*/

  if(m_host.empty())
  {
    std::cerr << G_STRFUNC << ": m_host is empty." << std::endl;
    return false;
  }

  if(m_port == 0)
  {
    std::cerr << G_STRFUNC << ": m_port is empty." << std::endl;
    return false;
  }

  //TODO: Remember the existing username and password?
  if(username.empty())
  {
    std::cerr << G_STRFUNC << ": username is empty." << std::endl;
    return false;
  }

  if(password.empty())
  {
    std::cerr << G_STRFUNC << ": password is empty." << std::endl;
    return false;
  }

  //Make sure the path exists:
  const std::string path_backup = get_self_hosting_backup_path(base_directory);
  if(path_backup.empty() || !file_exists_filepath(path_backup))
  {
    std::cerr << G_STRFUNC << ": Backup file not found: " << path_backup << std::endl;
    return false;
  }

  // Save the password to ~/.pgpass, because this is the only way to use
  // pg_dump without it asking for the password:
  std::string pgpass_backup, pgpass_original;
  const bool pgpass_created = save_password_to_pgpass(username, password, pgpass_backup, pgpass_original);
  if(!pgpass_created)
  {
    std::cerr << G_STRFUNC << ": save_password_to_pgpass() failed." << std::endl;
    return false;
  }

  // Make sure to use double quotes for the executable path, because the
  // CreateProcess() API used on Windows does not support single quotes.
  const std::string command_restore = get_path_to_postgres_executable("pg_restore") +
    " -d " + database_name + //TODO: Quote database name?
    " --host=" + Glib::shell_quote(m_host) +
    " --port=" + port_as_string(m_port) +
    " --username=" + Glib::shell_quote(username) +
    " " + path_backup;

  std::cout << "DEBUG: command_restore=" << command_restore << std::endl;

  //TODO: Put the password in .pgpass

  const bool result = Glom::Spawn::execute_command_line_and_wait(command_restore, slot_progress);

  //Move the previously-existing .pgpass file back:
  //TODO: Really, we should just edit the file instead of completely replacing it,
  //      because another application might try to edit it in the meantime.
  if(!pgpass_backup.empty())
  {
    g_rename(pgpass_backup.c_str(), pgpass_original.c_str());
  }

  if(!result)
  {
    std::cerr << "Error while attempting to call pg_restore." << std::endl;
  }

  return result;
}

std::string Postgres::get_self_hosting_path(bool create, const std::string& child_directory)
{
  //Get the filepath of the directory that we should create:
  const std::string dbdir_uri = m_database_directory_uri;
  //std::cout << "debug: dbdir_uri=" << dbdir_uri << std::endl;

  std::string dbdir;
  try
  {
    dbdir = Glib::build_filename(
      Glib::filename_from_uri(dbdir_uri), child_directory);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << ": exception from Glib::build_filename(): " << ex.what() << std::endl;
  }

  if(file_exists_filepath(dbdir))
    return dbdir;
  else if(!create)
    return std::string();

  //Create the directory:

  //std::cout << "debug: dbdir=" << dbdir << std::endl;
  g_assert(!dbdir.empty());

  if(create_directory_filepath(dbdir))
    return dbdir;
  else
    return std::string();
}

std::string Postgres::get_self_hosting_config_path(bool create)
{
  return get_self_hosting_path(create, "config");
}

std::string Postgres::get_self_hosting_data_path(bool create)
{
  return get_self_hosting_path(create, "data");
}

std::string Postgres::get_self_hosting_backup_path(const std::string& base_directory, bool create_parent_dir)
{
  //This is a file, not a directory, so we don't use get_self_hosting_path("backup");
  std::string dbdir;
  if(base_directory.empty())
    dbdir = get_self_hosting_path(create_parent_dir);
  else
  {
    dbdir = base_directory;
  }

  if(dbdir.empty())
    return std::string();

  try
  {
    return Glib::build_filename(dbdir, "backup");
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << G_STRFUNC << ": exception from Glib::build_filename(): " << ex.what() << std::endl;
    return std::string();
  }
}

bool Postgres::create_directory_filepath(const std::string& filepath)
{
  if(filepath.empty())
    return false;

  const int mkdir_succeeded = g_mkdir_with_parents(filepath.c_str(), 0770);
  if(mkdir_succeeded == -1)
  {
    std::cerr << G_STRFUNC << "Error from g_mkdir_with_parents() while trying to create directory: " << filepath << std::endl;
    perror("Error from g_mkdir_with_parents");

    return false;
  }

  return true;
}

bool Postgres::file_exists_filepath(const std::string& filepath)
{
  if(filepath.empty())
    return false;

  const Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(filepath);
  return file && file->query_exists();
}

bool Postgres::file_exists_uri(const std::string& uri) const
{
  if(uri.empty())
    return false;

  const Glib::RefPtr<Gio::File> file = Gio::File::create_for_uri(uri);
  return file && file->query_exists();
}



bool Postgres::create_text_file(const std::string& file_uri, const std::string& contents, bool current_user_only)
{
  if(file_uri.empty())
    return false;

  Glib::RefPtr<Gio::File> file = Gio::File::create_for_uri(file_uri);
  Glib::RefPtr<Gio::FileOutputStream> stream;

  //Create the file if it does not already exist:
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
    if(file->query_exists())
    {
      if(current_user_only)
      {
        stream = file->replace(std::string() /* etag */, false /* make_backup */, Gio::FILE_CREATE_PRIVATE); //Instead of append_to().
      }
      else
      {
         stream = file->replace(); //Instead of append_to().
      }
    }
    else
    {
      //By default files created are generally readable by everyone, but if we pass FILE_CREATE_PRIVATE in flags the file will be made readable only to the current user, to the level that is supported on the target filesystem.
      if(current_user_only)
      {
      //TODO: Do we want to specify 0660 exactly? (means "this user and his group can read and write this non-executable file".)
        stream = file->create_file(Gio::FILE_CREATE_PRIVATE);
      }
      else
      {
        stream = file->create_file();
      }
    }
  }
  catch(const Gio::Error& ex)
  {
#else
  std::auto_ptr<Gio::Error> error;
  stream.create(error);
  if(error.get())
  {
    const Gio::Error& ex = *error.get();
#endif
    // If the operation was not successful, print the error and abort
    std::cerr << "ConnectionPool::create_text_file(): exception while creating file." << std::endl
      << "  file uri:" << file_uri << std::endl
      << "  error:" << ex.what() << std::endl;
    return false; // print_error(ex, output_uri_string);
  }


  if(!stream)
    return false;


  gsize bytes_written = 0;
  const std::string::size_type contents_size = contents.size();
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
    //Write the data to the output uri
    bytes_written = stream->write(contents.data(), contents_size);
  }
  catch(const Gio::Error& ex)
  {
#else
  bytes_written = stream->write(contents.data(), contents_size, error);
  if(error.get())
  {
    Gio::Error& ex = *error.get();
#endif
    // If the operation was not successful, print the error and abort
    std::cerr << "ConnectionPool::create_text_file(): exception while writing to file." << std::endl
      << "  file uri:" << file_uri << std::endl
      << "  error:" << ex.what() << std::endl;
    return false; //print_error(ex, output_uri_string);
  }

  if(bytes_written != contents_size)
  {
    std::cerr << "ConnectionPool::create_text_file(): not all bytes written when writing to file." << std::endl
      << "  file uri:" << file_uri << std::endl;
    return false;
  }

  return true; //Success.
}

} //namespace ConnectionPoolBackends

} //namespace Glom
