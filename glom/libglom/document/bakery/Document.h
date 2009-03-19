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

#include <glibmm.h>
#include <libglom/document/bakery/view/ViewBase.h>
#include <iostream>

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
  virtual ~Document();

  /* Saves the data to disk.
   * Asks the View to update this document before saving to disk,
   * but you should probably ensure that the document is updated more regularly than this,
   * so that different parts of the GUI are synchronized.
   * Only saves if the document has been modified.
   * bool indicates success.
   */
  bool save();

  /* Loads data from disk, using the URI (set with set_file_uri()) then asks the View to update itself.
   * bool indicates success.
   */
  bool load();

  //This can't be virtual because that would break ABI.
  //Hopefully it doesn't need to be.
  /* Loads data from disk, using the URI (set with set_file_uri()) then asks the View to update itself.
   * bool indicates success.
   */
  bool load_from_data(const guchar* data, std::size_t length);


  virtual bool get_modified() const;
  virtual void set_modified(bool bVal = true);

  ///Whether this just a default document.
  virtual bool get_is_new() const;
  ///Called by App_WithDoc::init_create_document().
  void set_is_new(bool bVal);

  virtual Glib::ustring get_contents() const;
  virtual void set_contents(const Glib::ustring& strVal);

  virtual Glib::ustring get_file_uri_with_extension(const Glib::ustring& uri);

  virtual Glib::ustring get_file_uri() const;
  virtual void set_file_uri(const Glib::ustring& file_uri, bool bEnforceFileExtension = false);

  ///Gets filename part of file_uri, or 'untitled'.
  virtual Glib::ustring get_name() const;
  static Glib::ustring util_file_uri_get_name(const Glib::ustring& file_uri, const Glib::ustring& file_extension);

  virtual bool get_read_only() const;
  virtual void set_read_only(bool bVal);

  ///If you don't want to use a View, then don't use set_view().
  virtual void set_view(ViewBase* pView);
  virtual ViewBase* get_view();

  virtual void set_file_extension(const Glib::ustring& strVal);
  virtual Glib::ustring get_file_extension() const;

  //Signals
  /** For instance, void on_document_modified(bool modified);
   */
  typedef sigc::signal<void, bool> type_signal_modified;

  /** This signal is emitted when the document has been modified.
   * It allows the view to update itself to show the new information.
   */
  type_signal_modified& signal_modified();

  typedef sigc::signal<void> type_signal_forget;

  /** This signal is emitted when the view should forget the document.
   * This is used internally, and you should not need to use it yourself.
   */
  type_signal_forget& signal_forget();

  ///Allow app to update icons/title bar.

protected:
  /** overrideable.
   * Does anything which should be done after the data has been loaded from disk, but before updating the View.
   */
  virtual bool load_after();

  /** overrideable.
   * Does anything which should be done before the view has saved its data, before writing to disk..
   */
  virtual bool save_before();

  virtual bool read_from_disk();
  virtual bool write_to_disk();

  Glib::ustring m_strContents;
  Glib::ustring m_file_uri;
  Glib::ustring m_file_extension;

  ViewBase* m_pView;

  type_signal_modified signal_modified_;
  type_signal_forget signal_forget_;
  
  bool m_bModified;
  bool m_bIsNew; //see get_is_new().
  bool m_bReadOnly;
};

} //namespace

#endif //GNOME_APPWITHDOCS_DOCUMENT_H
