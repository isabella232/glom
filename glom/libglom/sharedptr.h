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

  /** Swap the contents of two sharedptr<>.
   * This method swaps the internal pointers.  This can be
   * done safely without involving a reference/unreference cycle and is
   * therefore highly efficient.
   */
  inline void swap(sharedptr<T_obj>& other);

  /** Copy constructor (from different, but castable type).
   *
   * Increments the reference count.
   */
  template <class T_CastFrom>
  inline sharedptr(const sharedptr<T_CastFrom>& src);

  ///Share ownership
  sharedptr& operator=(const sharedptr& src);

  /** Copy from different, but castable type).
   *
   * Increments the reference count.
   */
  template <class T_CastFrom>
  inline sharedptr<T_obj>& operator=(const sharedptr<T_CastFrom>& src);

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
   * <code>sharedptr->memberfun()</code>.
   */
  inline T_obj* operator->() const;

  /** Test whether the sharedptr<> points to any underlying instance.
   *
   * Mimics usage of ordinary pointers:
   * @code
   *   if (ptr)
   *     do_something();
   * @endcode
   */
  inline operator bool() const;

    /** Dynamic cast to derived class.
   *
   * The sharedptr can't be cast with the usual notation so instead you can use
   * @code
   *   ptr_derived = sharedptr<Derived>::cast_dynamic(ptr_base);
   * @endcode
   */
  template <class T_CastFrom>
  static inline sharedptr<T_obj> cast_dynamic(const sharedptr<T_CastFrom>& src);

  /** Static cast to derived class.
   *
   * Like the dynamic cast; the notation is 
   * @code
   *   ptr_derived = sharedptr<Derived>::cast_static(ptr_base);
   * @endcode
   */
  template <class T_CastFrom>
  static inline sharedptr<T_obj> cast_static(const sharedptr<T_CastFrom>& src);

  /** Cast to non-const.
   *
   * The sharedptr can't be cast with the usual notation so instead you can use
   * @code
   *   ptr_unconst = sharedptr<UnConstType>::cast_const(ptr_const);
   * @endcode
   */
  template <class T_CastFrom>
  static inline sharedptr<T_obj> cast_const(const sharedptr<T_CastFrom>& src);

  static inline sharedptr<T_obj>  create()
  {
    return sharedptr<T_obj>(new T_obj());
  }

  ///Get the underlying instance:
  inline T_obj* obj();

  ///Get the underlying instance:
  inline const T_obj* obj() const;

  ///This is for internal use. You never need to use it.
  inline size_type* _get_refcount() const
  { return m_pRefCount; }

protected:
  inline void ref();
  inline void unref();

  void init();

  mutable size_type* m_pRefCount; //Shared between instances, by copying.
  T_obj* m_pobj; //The underlying instance.
};

template< typename T_obj>
sharedptr<T_obj>::sharedptr()
: m_pRefCount(0), m_pobj(0)
{
  init();
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

// The templated ctor allows copy construction from any object that's
// castable.  Thus, it does downcasts:
//   base_ref = derived_ref
template <class T_obj>
  template <class T_CastFrom>
inline
sharedptr<T_obj>::sharedptr(const sharedptr<T_CastFrom>& src)
:
  // A different sharedptr<> will not allow us access to pCppObject_.  We need
  // to add a get_underlying() for this, but that would encourage incorrect
  // use, so we use the less well-known operator->() accessor:
  m_pRefCount(src._get_refcount()), m_pobj(src.operator->())
{
  if(m_pobj)
    ref();
}

template <class T_obj> inline
void sharedptr<T_obj>::swap(sharedptr<T_obj>& other)
{
  T_obj *const obj_temp = m_pobj;
  size_type* const count_temp = m_pRefCount;

  m_pobj = other.m_pobj;
  m_pRefCount = other.m_pRefCount;

  other.m_pobj = obj_temp;
  other.m_pRefCount = count_temp;
}

template< typename T_obj>
sharedptr<T_obj>& sharedptr<T_obj>::operator=(const sharedptr<T_obj>& src)
{
 sharedptr<T_obj> temp(src); //Increases ref
 this->swap(temp); //temp forgets everything and gives it to this.
 return *this;
}

template <class T_obj>
  template <class T_CastFrom>
inline
sharedptr<T_obj>& sharedptr<T_obj>::operator=(const sharedptr<T_CastFrom>& src)
{
  sharedptr<T_obj> temp(src); //Increases ref
  this->swap(temp); //temp forgets everything and gives it to this.
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
  sharedptr<T_obj> temp; // swap with an empty sharedptr<> to clear *this
  this->swap(temp);
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
void sharedptr<T_obj>::init()
{
  //Forget any previous instance:
  if(m_pobj)
  {
    unref();
  }

  m_pobj = 0;
  m_pRefCount = 0;
}


template <class T_obj>
  template <class T_CastFrom>
inline
sharedptr<T_obj> sharedptr<T_obj>::cast_dynamic(const sharedptr<T_CastFrom>& src)
{
  T_obj *const pCppObject = dynamic_cast<T_obj*>(src.operator->());

  sharedptr<T_obj> result(pCppObject);
  if(pCppObject)
    result.ref();

  return result;
}

template <class T_obj>
  template <class T_CastFrom>
inline
sharedptr<T_obj> sharedptr<T_obj>::cast_static(const sharedptr<T_CastFrom>& src)
{
  T_obj *const pCppObject = static_cast<T_obj*>(src.operator->());

  sharedptr<T_obj> result(pCppObject);
  if(pCppObject)
    result.ref();

  return result;
}

template <class T_obj>
  template <class T_CastFrom>
inline
sharedptr<T_obj> sharedptr<T_obj>::cast_const(const sharedptr<T_CastFrom>& src)
{
  T_obj *const pCppObject = const_cast<T_obj*>(src.operator->());

  sharedptr<T_obj> result(pCppObject);
  if(pCppObject)
    result.ref();

  return result;
}

template <class T_obj>
sharedptr<T_obj> glom_sharedptr_clone(const sharedptr<T_obj>& src)
{
  if(src)
  {
    //std::cout << "glom_sharedptr_clone src.name=" << src->get_name() << std::endl;
    return sharedptr<T_obj>(static_cast<T_obj*>(src->clone()));
  }
  else
    return sharedptr<T_obj>();
}

template <class T_obj>
sharedptr<T_obj> glom_sharedptr_clone(const sharedptr<const T_obj>& src)
{
  if(src)
  {
    //std::cout << "glom_sharedptr_cloneconst src.name=" << src->get_name() << std::endl;
    return sharedptr<T_obj>(static_cast<T_obj*>(src->clone()));
  }
  else
    return sharedptr<T_obj>();
}


#endif //GLOM_SHAREDPTR_H

