/* sharedptr.h
 *
 * Copyright (C) 2004 Glom developers
 *
 * Licensed under the GPL
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
 
#ifndef GLOM_SHAREDPTR_H
#define GLOM_SHAREDPTR_H

#include <iostream> //Just for debugging.



/**A ref-counting smart-pointer for the underlying C object.
 * You can copy these smarpointers-of-C-resources, and therefore the C++ classes can
 * have simple copy constructors which just share the underlying C resources.
 *
 */
template< typename T_obj >
class sharedptr
{
public:
  typedef size_t size_type;

  sharedptr();

  ///Take ownership
  explicit sharedptr(T_obj* pobj);

  ///Share ownership
  sharedptr(const sharedptr& src);

  ///Share ownership
  sharedptr& operator=(const sharedptr& src);

  virtual ~sharedptr();


  ///Forget the instance.
  virtual void clear();

  /** Dereferencing.
   */
  inline T_obj& operator*();

  /** Dereferencing.
   */
  inline const T_obj& operator*() const;

  /** Dereferencing.
   *
   * Use the methods of the underlying instance like so:
   * <code>refptr->memberfun()</code>.
   */
  inline T_obj* operator->() const;

  /** Test whether the RefPtr<> points to any underlying instance.
   *
   * Mimics usage of ordinary pointers:
   * @code
   *   if (ptr)
   *     do_something();
   * @endcode
   */
  inline operator bool() const;

  ///Get the underlying instance:
  inline T_obj* obj();

  ///Get the underlying instance:
  inline const T_obj* obj() const;


protected:
  inline void ref();
  inline void unref();

  void initialize();

  size_type* m_pRefCount; //Shared between instances, by copying.
  T_obj* m_pobj; //The underlying instance.
};

template< typename T_obj>
sharedptr<T_obj>::sharedptr()
: m_pRefCount(0), m_pobj(0)
{
  initialize();
}

template< typename T_obj>
sharedptr<T_obj>::sharedptr(T_obj* pobj)
: m_pRefCount(0), m_pobj(pobj)
{
    //Start refcounting:
    ref();
}

template< typename T_obj>
sharedptr<T_obj>::sharedptr(const sharedptr<T_obj>& src)
: m_pRefCount(src.m_pRefCount), m_pobj(src.m_pobj)
{
  ref();
}

template< typename T_obj>
sharedptr<T_obj>& sharedptr<T_obj>::operator=(const sharedptr<T_obj>& src)
{
  //std::cout << "sharedptr& operator=(const sharedptr& src)" << std::endl;
  if(&src != this)
  {
    //Unref any existing stuff.
    //operator= can never run before a constructor, so these values will be initialized already.
    if(m_pobj) //The if() might not be required.
    {
      unref(); //Could cause a deallocation.
    }

    //Copy:
    m_pobj = src.m_pobj;

    m_pRefCount = src.m_pRefCount;
    ref();
  }

  return *this;
}

template< typename T_obj>
sharedptr<T_obj>::~sharedptr()
{
   unref();
}

template< typename T_obj>
void sharedptr<T_obj>::clear()
{
  initialize();
}

template< typename T_obj>
inline
T_obj* sharedptr<T_obj>::obj()
{
  return m_pobj;
}

template< typename T_obj>
inline
const T_obj* sharedptr<T_obj>::obj() const
{
  return m_pobj;
}

template< typename T_obj>
inline
T_obj& sharedptr<T_obj>::operator*()
{
  return *m_pobj;
}

template< typename T_obj>
inline
const T_obj& sharedptr<T_obj>::operator*() const
{
  return *m_pobj;
}

template< typename T_obj>
inline
T_obj* sharedptr<T_obj>::operator->() const
{
  return m_pobj;
}

template <class T_obj>
inline
sharedptr<T_obj>::operator bool() const
{
  return (m_pobj != 0);
}


template <class T_obj>
inline
void sharedptr<T_obj>::ref()
{
  if(m_pobj) //Don't waste time on invalid instances. These would be very rare anyway, and intentionally created with (0,0) construction.
  {
    if(m_pRefCount == 0)
    {
      //std::cout << "sharedptr::ref(): first ref" << std::endl;
      //First ref, so allocate the shared count:
      m_pRefCount = new size_type();
      *m_pRefCount = 1;
    }
    else
    {
      //std::cout << "sharedptr::ref(): starting at" << *m_pRefCount << std::endl;
      (*m_pRefCount)++;
    }
  }
}

template <class T_obj>
inline
void sharedptr<T_obj>::unref()
{
  if(m_pRefCount)
  {
    //std::cout << "sharedptr::unref(): starting at " << *m_pRefCount << std::endl;

    if( (*m_pRefCount) > 0 )
       (*m_pRefCount)--;

    //Unalloc if this is the last user of the obj:
    if(*m_pRefCount == 0)
    {
      if(m_pobj)
      {
        delete m_pobj;
        m_pobj = 0;
      }

       //Clear ref count:
       delete m_pRefCount;
       m_pRefCount = 0;
    }
  }
  else
  {
    //std::cout << "sharedptr::unref(): ref not setup." << std::endl;
  }

}

template <class T_obj>
void sharedptr<T_obj>::initialize()
{
  //Forget any previous instance:
  if(m_pobj)
  {
    unref();
    m_pobj = 0;
    m_pRefCount = 0;
  }
}



#endif //GLOM_SHAREDPTR_H

