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

#ifndef GLOM_BACKEND_POSTGRES_SELF_H
#define GLOM_BACKEND_POSTGRES_SELF_H

#include <libglom/libglom_config.h>

#include <libglom/connectionpool_backends/postgres.h>

namespace Glom
{

namespace ConnectionPoolBackends
{

class PostgresSelfHosted : public Postgres
{
public:
  PostgresSelfHosted();

  /** This specifies that Glom should start its own database server instance
   * for this database, using the database files stored at the specified uri.
   */
  void set_self_hosting_data_uri(const std::string& data_uri);

  /** Return whether the self-hosted server is currently running.
   *
   * @result True if it is running, and false otherwise.
   */
  bool get_self_hosting_active() const;

  /** Returns the port number the local postgres server is running on.
   *
   * @result The port number of the self-hosted server, or 0 if it is not
   * running.
   */
  int get_port() const;

  /** Try to install postgres on the distro, though this will require a
   * distro-specific patch to the implementation.
   */
  static bool install_postgres(const SlotProgress& slot_progress);

  static std::string get_path_to_postgres_executable(const std::string& program);

private:
  virtual InitErrors initialize(const SlotProgress& slot_progress, const Glib::ustring& initial_username, const Glib::ustring& password, bool network_shared = false);

  virtual bool startup(const SlotProgress& slot_progress, bool network_shared = false);
  virtual void cleanup(const SlotProgress& slot_progress);
  virtual bool set_network_shared(const SlotProgress& slot_progress, bool network_shared = true);

  virtual Glib::RefPtr<Gnome::Gda::Connection> connect(const Glib::ustring& database, const Glib::ustring& username, const Glib::ustring& password, std::auto_ptr<ExceptionConnection>& error);

  virtual bool create_database(const Glib::ustring& database_name, const Glib::ustring& username, const Glib::ustring& password, std::auto_ptr<Glib::Error>& error);

private:
  /** Examine ports one by one, starting at @a starting_port, in increasing
   * order, and return the first one that is available.
   */
  static int discover_first_free_port(int start_port, int end_port);

  static bool create_text_file(const std::string& file_uri, const std::string& contents);

  //bool directory_exists_filepath(const std::string& filepath);
  bool directory_exists_uri(const std::string& uri);

  std::string m_self_hosting_data_uri;
  int m_port;
  bool m_network_shared;
};

} // namespace ConnectionPoolBackends

} //namespace Glom

#endif //GLOM_BACKEND_POSTGRES_SELF_H

