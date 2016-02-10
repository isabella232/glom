/* Glom
 *
 * Copyright (C) 2015 Murray Cumming
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

#ifndef GLOM_ALGORITHMS_UTILS_H
#define GLOM_ALGORITHMS_UTILS_H

#include <algorithm>

namespace Glom
{

namespace Utils
{

template<typename T_container, typename T_element>
bool
find_exists(const T_container& container, const T_element& element)
{
  const auto end = std::end(container);
  return std::find(std::begin(container), end, element) != end;
}

template<typename T_container, typename T_callable>
bool
find_if_exists(const T_container& container, const T_callable& callable)
{
  const auto end = std::end(container);
  return std::find_if(std::begin(container), end, callable) != end;
}


template<typename T_container, typename T_element>
typename T_container::iterator
find(T_container& container, const T_element& element)
{
  return std::find(std::begin(container), std::end(container), element);
}

template<typename T_container, typename T_element>
typename T_container::const_iterator
find(const T_container& container, const T_element& element)
{
  return std::find(std::begin(container), std::end(container), element);
}


template<typename T_container, typename T_callable>
typename T_container::iterator
find_if(T_container& container, const T_callable& callable)
{
  return std::find_if(std::begin(container), std::end(container), callable);
}

template<typename T_container, typename T_callable>
typename T_container::const_iterator
find_if(const T_container& container, const T_callable& callable)
{
  return std::find_if(std::begin(container), std::end(container), callable);
}

template<typename T_container, typename T_element>
void
add_unique(T_container& container, T_element&& element)
{
  if(!find_exists(container, element))
    container.emplace_back(std::forward<T_element>(element));
}


} //namespace Utils

} //namespace Glom

#endif //GLOM_ALGORITHMS_UTILS_H
