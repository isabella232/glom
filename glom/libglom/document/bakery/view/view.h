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

#ifndef GLOM_BAKERY_VIEW_H
#define GLOM_BAKERY_VIEW_H

#include <libglom/document/bakery/view/viewbase.h>
#include <libglom/document/bakery/document.h>
#include <sigc++/sigc++.h>

namespace GlomBakery
{

/** This is a base class which should be multiple-inherited with gtkmm widgets.
  * You should override save_to_document() and load_from_document().
  */
template< class T_Document >
class View : public ViewBase
{
public: 
  View()
  : m_pDocument(0)
  {
  }

  typedef View<T_Document> type_self;

  //typedef typename T_Document type_document;

  virtual T_Document* get_document()
  {
    return m_pDocument;
  }

  virtual const T_Document* get_document() const
  {
    return m_pDocument;
  }

  virtual void set_document(T_Document* pDocument)
  {
    m_pDocument = pDocument;
    if(m_pDocument)
      m_pDocument->signal_forget().connect( sigc::mem_fun(*this, &type_self::on_document_forget) );
  }

  ///Just a convenience, instead of get_docuement()->set_modified().
  virtual void set_modified(bool val = true)
  {
    if(m_pDocument)
      m_pDocument->set_modified(val);
  }

protected:

  void on_document_forget()
  {
    //This should prevent some segfaults:
    m_pDocument = 0;
  }
  
  T_Document* m_pDocument;
};

} //namespace

#endif //BAKERY_VIEW_H

