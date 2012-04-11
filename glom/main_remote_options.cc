/* Glom
 *
 * Copyright (C) 2012 Murray Cumming
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

#include <glom/main_remote_options.h>

#include <glibmm/optionentry.h>

#include <glibmm/i18n.h>

namespace Glom
{

RemoteOptionGroup::RemoteOptionGroup()
: Glib::OptionGroup("glom", _("Main Glom options"), _("Main command-line options for glom")),
  m_arg_restore(false),
  m_arg_stop_auto_server_shutdown(false),
  m_arg_debug_sql(false)
{
  Glib::OptionEntry entry;
  entry.set_long_name("file");
  entry.set_short_name('f');
  entry.set_description(_("The Filename"));
  add_entry_filename(entry, m_arg_filename);

  entry.set_long_name("restore");
  entry.set_short_name(0);
  entry.set_description(_("Whether the filename is a .tar.gz backup to be restored."));
  add_entry(entry, m_arg_restore);

  entry.set_long_name("stop-auto-server-shutdown");
  entry.set_short_name(0);
  entry.set_description(_("Do not automatically stop the database server if Glom quits. This is helpful for debugging with gdb."));
  add_entry(entry, m_arg_stop_auto_server_shutdown);

  entry.set_long_name("debug_sql");
  entry.set_short_name(0);
  entry.set_description(_("Show the generated SQL queries on stdout, for debugging."));
  add_entry(entry, m_arg_debug_sql);
}

} //namespace Glom
