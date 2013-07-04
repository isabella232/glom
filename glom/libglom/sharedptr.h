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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef GLOM_SHAREDPTR_H
#define GLOM_SHAREDPTR_H

//#include <iostream> //Just for debugging.
#include <cstddef> // For size_t.
#include <memory> //For shared_ptr

namespace Glom
{

template <class T_obj>
std::shared_ptr<T_obj> glom_sharedptr_clone(const std::shared_ptr<T_obj>& src)
{
  if(src)
  {
    //std::cout << "glom_sharedptr_clone src.name=" << src->get_name() << std::endl;
    return std::shared_ptr<T_obj>(static_cast<T_obj*>(src->clone()));
  }
  else
    return std::shared_ptr<T_obj>();
}

template <class T_obj>
std::shared_ptr<T_obj> glom_sharedptr_clone(const std::shared_ptr<const T_obj>& src)
{
  if(src)
  {
    //std::cout << "glom_sharedptr_cloneconst src.name=" << src->get_name() << std::endl;
    return std::shared_ptr<T_obj>(static_cast<T_obj*>(src->clone()));
  }
  else
    return std::shared_ptr<T_obj>();
}

} //namespace Glom

#endif //GLOM_SHAREDPTR_H


