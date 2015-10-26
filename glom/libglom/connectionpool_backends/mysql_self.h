/* Glom
 *
 * Copyright (C) 2001-2013 Murray Cumming
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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#ifndef GLOM_BACKEND_MYSQL_SELF_H
#define GLOM_BACKEND_MYSQL_SELF_H

#include <libglom/libglom_config.h>

#include <libglom/connectionpool_backends/mysql.h>

namespace Glom
{

namespace ConnectionPoolBackends
{

class MySQLSelfHosted : public MySQL
{
public:
  MySQLSelfHosted();

  /** Return whether the self-hosted server is currently running.
   *
   * @result True if it is running, and false otherwise.
   */
  bool get_self_hosting_active() const;

  /** Returns the port number the local mysql server is running on.
   *
   * @result The port number of the self-hosted server, or 0 if it is not
   * running.
   */
  unsigned int get_port() const;

  /** Try to install mysql on the distro, though this will require a
   * distro-specific patch to the implementation.
   */
  static bool install_mysql(const SlotProgress& slot_progress);

private:
  std::string get_mysqladmin_command(const Glib::ustring& username, const Glib::ustring& password);

  InitErrors initialize(const SlotProgress& slot_progress, const Glib::ustring& initial_username, const Glib::ustring& password, bool network_shared = false) override;

  StartupErrors startup(const SlotProgress& slot_progress, bool network_shared = false) override;
  bool cleanup(const SlotProgress& slot_progress) override;
  bool set_network_shared(const SlotProgress& slot_progress, bool network_shared = true) override;

  Glib::RefPtr<Gnome::Gda::Connection> connect(const Glib::ustring& database, const Glib::ustring& username, const Glib::ustring& password, bool fake_connection = false) override;

  bool create_database(const SlotProgress& slot_progress, const Glib::ustring& database_name, const Glib::ustring& username, const Glib::ustring& password) override;

private:
  /** Examine ports one by one, starting at @a starting_port, in increasing
   * order, and return the first one that is available.
   */
  static unsigned int discover_first_free_port(unsigned int start_port, unsigned int end_port);

  /** Run the command-line with the --version option to discover what version
   * of MySQL is installed, so we can use the appropriate configuration
   * options when self-hosting.
   */
  Glib::ustring get_mysqlql_utils_version(const SlotProgress& slot_progress);

  float get_mysqlql_utils_version_as_number(const SlotProgress& slot_progress);
  
  void show_active_connections();

  bool m_network_shared;
  
  //These are remembered in order to use them to issue the shutdown command via mysqladmin:
  Glib::ustring m_saved_username, m_saved_password;

  bool m_temporary_password_active; //Whether the password is an initial temporary one.
  Glib::ustring m_initial_password_to_set, m_initial_username_to_set;
  Glib::ustring m_temporary_password;
};

} // namespace ConnectionPoolBackends

} //namespace Glom

#endif //GLOM_BACKEND_MYSQL_SELF_H
