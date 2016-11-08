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

#ifndef GLOM_BAKERY_DOCUMENT_H
#define GLOM_BAKERY_DOCUMENT_H

#include <libglom/document/bakery/view/viewbase.h>
#include <glibmm/ustring.h>
#include <sigc++/sigc++.h>

namespace GlomBakery
{

/** The Document is like the 'Model' in the Model-View-Controller framework.
 * Each App should have a Document.
 * Each View gets and sets data in its document.
 */
class Document
{
public:
  Document();
  virtual ~Document() = default;

  /* Saves the data to disk.
   * Asks the View to update this document before saving to disk,
   * but you should probably ensure that the document is updated more regularly than this,
   * so that different parts of the GUI are synchronized.
   * Only saves if the document has been modified.
   * bool indicates success.
   */
  bool save();

  enum class LoadFailureCodes
  {
    NONE = 0,
    NOT_FOUND = 1,
    LAST = 20 //arbitrary large number. Anything after this is for the application's custom codes.
  };

  /* Loads data from disk, using the URI (set with set_file_uri()) then asks the View to update itself.
   * bool indicates success.
   * @param failure_code Used to return an error code that is understood by your application.
   * Custom error codes should be greater than LoadFailureCodes::CODE_LAST.
   */
  bool load(int& failure_code);

  /* Loads data from disk, using the URI (set with set_file_uri()) then asks the View to update itself.
   * bool indicates success.
   * @param data The bytes.
   * @param length The number of bytes.
   * @param failure_code Used to return a custom error code that is understood by your application. This must be greater than zero.
   */
  bool load_from_data(const guchar* data, std::size_t length, int& failure_code);


  bool get_modified() const;
  virtual void set_modified(bool value = true);

  ///Whether this just a default document.
  bool get_is_new() const;
  ///Called by AppWindow_WithDoc::init_create_document().
  void set_is_new(bool value);

  Glib::ustring get_contents() const;

  Glib::ustring get_file_uri_with_extension(const Glib::ustring& uri);

  Glib::ustring get_file_uri() const;
  virtual void set_file_uri(const Glib::ustring& file_uri, bool bEnforceFileExtension = false);

  ///Gets filename part of file_uri, or 'untitled'.
  virtual Glib::ustring get_name() const;
  static Glib::ustring util_file_uri_get_name(const Glib::ustring& file_uri, const Glib::ustring& file_extension);

  bool get_read_only() const;
  void set_read_only(bool value);

  ///If you don't want to use a View, then don't use set_view().
  void set_view(ViewBase* pView);
  ViewBase* get_view();

  void set_file_extension(const Glib::ustring& strVal);
  Glib::ustring get_file_extension() const;

  //Signals
  /** For instance, void on_document_modified(bool modified);
   */
  typedef sigc::signal<void(bool)> type_signal_modified;

  /** This signal is emitted when the document has been modified.
   * It allows the view to update itself to show the new information.
   */
  type_signal_modified& signal_modified();

  ///Allow app to update icons/title bar.

protected:

  /** overrideable.
   * Does anything which should be done after the data has been loaded from disk, but before updating the View.
   * @param failure_code Used to return a custom error code that is understood by your application. This must be greater than zero.
   */
  virtual bool load_after(int& failure_code);

  /** overrideable.
   * Does anything which should be done before the view has saved its data, before writing to disk..
   */
  virtual bool save_before();

  bool read_from_disk(int& failure_code);
  bool write_to_disk();

  Glib::ustring m_strContents;
  Glib::ustring m_file_uri;
  Glib::ustring m_file_extension;

  ViewBase* m_view;

  type_signal_modified signal_modified_;

  bool m_modified;
  bool m_is_new; //see get_is_new().
  bool m_read_only;
};

} //namespace

#endif //GNOME_APPWITHDOCS_DOCUMENT_H
