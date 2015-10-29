/*
 * Copyright 2000 Murray Cumming
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef GLOM_BAKERY_APP_WITHDOC_H
#define GLOM_BAKERY_APP_WITHDOC_H

#include <glom/bakery/appwindow.h>
#include <libglom/document/bakery/document.h>

namespace GlomBakery
{

/** Main Window which supports documents.
 *
 * This is an abstract class. You must use a class such as AppWindow_WithDoc_Gtk, which implements
 * the ui_* methods for a particular GUI toolkit.
 
 * Features:
 * - 1 document per application instance. Uses Document-derived class polymorphically.
 * - Override init_create_document() to create new blank document.
 * - Appropriate Default handling of document open, save, save as.
 * - Appropriate checking of document 'modified' status - asks user about unsaved changes.
 * - Asks user about overwriting existing documents.
 * - Override methods to add/change menus/toolbars/statusbar.
 *   - Default is basic File, Edit, Help menus and toolbar icons.
 * - Shows document name (or 'untitled') in window title.
 * - Shows * in title bar for unsaved docs. Overridable to e.g. shade a Save icon.
 * - Enforces a file extension.
 * - Recent Documents menu item - if you use add_mime_type().
 *
 *
 * TODO:
 * - Printing -?
 *  - Print Setup
 *  - Print Preview
 *  - Multiple document-types:
 *  - File/New sub menu
 *  - Some way to associate a view with a document type: class factory.
 */
class AppWindow_WithDoc : public AppWindow
{
public: 
  ///Don't forget to call init() too.
  AppWindow_WithDoc(const Glib::ustring& appname = ""); //TODO: appname when using get_derived_widget()

  virtual ~AppWindow_WithDoc();

  void init() override; //overridden to create document.

  enum class enumSaveChanges
  {
    Save,
    Cancel,
    Discard
  };

  static bool file_exists(const Glib::ustring& uri);

protected:
  virtual void init_create_document(); //override this to new() the specific document type.

  /** Add a MIME-type that this application can support.
   * You should also register the MIME-type when the application is installed:
   * See http://freedesktop.org/Standards/AddingMIMETutor
   */
  static void add_mime_type(const Glib::ustring& mime_type);

  ///static_cast<> or dynamic_cast<> this pointer to the correct type.
  virtual Document* get_document();

  ///static_cast<> or dynamic_cast<> this pointer to the correct type.
  virtual const Document* get_document() const ;

  virtual void set_document_modified(bool bModified = true);

  /** Open the document from a file at a URI.
   * This will check whether the document is already open.
   * @result true indicates success.
   */
  virtual bool open_document(const Glib::ustring& file_uri);

  //This cannot be virtual, because that would break our ABI.
  //Hopefully that is not necessary.
  /** Open the document using the supplied document contents.
   * Unlike open_document(), this has no way to know whether the document is already open.
   * @param data A pointer to the bytes of the document contents.
   * @param length The number of bytes in the data.
   * @result true indicates success.
   */
  bool open_document_from_data(const guchar* data, std::size_t length);

  virtual void document_history_add(const Glib::ustring& file_uri);
  virtual void document_history_remove(const Glib::ustring& file_uri);

public:
  // We can not take function pointers of these methods in 
  // a derived class if they are protected - for instance, with sigc::mem_fun()
  //Signal handlers:

  //Menu items:
  virtual void on_menu_file_open();
  virtual void on_menu_file_saveas(); //signal handler.
  virtual void offer_saveas(); //For direct use.
  virtual void on_menu_file_save(); //signal handler.
  void on_menu_file_close() override;

  void on_menu_edit_copy() override;
  void on_menu_edit_paste() override;
  void on_menu_edit_clear() override;

protected:
  //Document:

  ///Update visual status.
  virtual void on_document_modified(bool modified);

  ///override this to show document contents.
  virtual bool on_document_load();
  
  ///override this to do extra cleanup.
  virtual void on_document_close();

  virtual void offer_to_save_changes();

  ///Stop the File|Close or the File|Exit.
  virtual void cancel_close_or_exit();

  virtual void update_window_title();

  virtual void after_successful_save(); //e.g. disable File|Save.

  virtual void ui_warning(const Glib::ustring& text, const Glib::ustring& secondary_text) = 0;

  /** Warn the user about a failure while loading a document.
   * Override this to show a specific message in response to your application's 
   * custom @a failure_code.
   */
  virtual void ui_warning_load_failed(int failure_code = 0);

  virtual Glib::ustring ui_file_select_open(const Glib::ustring& ui_file_select_open = Glib::ustring()) = 0;

  /** Present a user interface that allows the user to select a location to save the file.
   * @param old_file_uri The existing URI of the file, if any.
   * @result The URI of the file chosen by the user.
   */
  virtual Glib::ustring ui_file_select_save(const Glib::ustring& old_file_uri) = 0;

  virtual void ui_show_modification_status() = 0;

  virtual enumSaveChanges ui_offer_to_save_changes() = 0;

  static Glib::ustring get_conf_fullkey(const Glib::ustring& key);

  //Document:
  Document* m_pDocument; //An instance of a derived type.
  bool m_bCloseAfterSave;

  //Mime types which this application can load and save:
  typedef std::list<Glib::ustring> type_list_strings;
  static type_list_strings m_mime_types;
};

} //namespace

#endif //BAKERY_APP_WITHDOC_H
