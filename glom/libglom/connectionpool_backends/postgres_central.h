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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#ifndef GLOM_BACKEND_POSTGRES_CENTRAL_H
#define GLOM_BACKEND_POSTGRES_CENTRAL_H

#include <libglom/connectionpool_backends/postgres.h>

#include <libglom/libglom_config.h>

namespace Glom
{

namespace ConnectionPoolBackends
{

class PostgresCentralHosted : public Postgres
{
public:
  PostgresCentralHosted();

  /** 0 means any port
   * Other ports will be tried if the specified port fails.
   */
  void set_host(const Glib::ustring& value);
  void set_port(unsigned int port);
  void set_try_other_ports(bool val);

  Glib::ustring get_host() const;
  unsigned int get_port() const;
  bool get_try_other_ports() const;

private:
  Glib::RefPtr<Gnome::Gda::Connection> connect(const Glib::ustring& database, const Glib::ustring& username, const Glib::ustring& password, bool fake_connection = false) override;

  bool create_database(const SlotProgress& slot_progress, const Glib::ustring& database_name, const Glib::ustring& username, const Glib::ustring& password) override;

private:
  std::vector<Glib::ustring> m_list_ports;

  bool m_try_other_ports;
};

} //namespace ConnectionPoolBackends

} //namespace Glom

#endif //GLOM_BACKEND_POSTGRES_CENTRAL_H
