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

#ifndef BASE_DB_H
#define BASE_DB_H

#include "gtkmm.h"

#include "document/document_glom.h"
#include "connectionpool.h"
#include "appstate.h"
#include "bakery/View/View.h"
#include <bakery/Utilities/BusyCursor.h>

/** A base class that is a Bakery View with some database functionality.
*/
class Base_DB :
  public View_Composite_Glom
{
public:
  Base_DB();
  virtual ~Base_DB();
  
  virtual void init_db_details(const Glib::ustring& strDatabaseName);

  /** Returns whether we are in developer mode.
   * Some functionality will be deactivated when not in developer mode.
   */
  virtual AppState::userlevels get_userlevel() const;
  virtual void set_userlevel(AppState::userlevels value);
   
  static sharedptr<SharedConnection> connect_to_server();
  
  virtual Glib::ustring get_database_name();

  virtual void set_document(Document_Glom* pDocument); //View override
  virtual void load_from_document(); //View override

  typedef std::vector< Field > type_vecFields;

  static type_vecFields get_fields_for_table_from_database(const Glib::ustring& table_name);

  /** Create an appropriate title for an ID string.
   * For instance, date_of_birth would become Date Of Birth.
   */
  static Glib::ustring util_title_from_string(const Glib::ustring& text);

  virtual Glib::RefPtr<Gnome::Gda::DataModel> Query_execute(const Glib::ustring& strQuery);
    
protected:
  typedef std::vector<Glib::ustring> type_vecStrings;
  type_vecStrings get_table_names();

  type_vecFields get_fields_for_table(const Glib::ustring& table_name) const;

  virtual void fill_from_database();
  virtual void fill_end(); //Call this from the end of fill_from_database() overrides.

  virtual void on_userlevel_changed(AppState::userlevels userlevel);

  static Glib::ustring util_string_from_decimal(guint decimal);
  static guint util_decimal_from_string(const Glib::ustring& str);

  static bool util_string_has_whitespace(const Glib::ustring& text);
  
  static type_vecStrings util_vecStrings_from_Fields(const type_vecFields& fields);
  
  virtual void handle_error(const std::exception& ex); //TODO_port: This is probably useless now.
  virtual bool handle_error();


  //Member data:
  Glib::ustring m_strDatabaseName;
};

#endif //BASE_DB_H
