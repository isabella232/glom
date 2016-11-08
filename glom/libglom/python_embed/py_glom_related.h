/* Glom
 *
 * Copyright (C) 2001-2005 Murray Cumming
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

#ifndef GLOM_PYTHON_GLOM_RELATED_H
#define GLOM_PYTHON_GLOM_RELATED_H

#include <libglom/python_embed/py_glom_record.h>
#include <libglom/data_structure/relationship.h>
#include <boost/python/object_fwd.hpp>

namespace Glom
{

class PyGlomRelatedRecord;
class PyGlomRecord;

class PyGlomRelated
{
public:
  PyGlomRelated();
  ~PyGlomRelated() = default;

  typedef std::unordered_map<Glib::ustring, std::shared_ptr<Relationship> , std::hash<std::string>> type_map_relationships;
  void set_relationships(const PyGlomRelated::type_map_relationships& relationships);


  //[] notation:
  type_map_relationships::size_type len() const;
  boost::python::object getitem(const boost::python::object& item);

  friend class PyGlomRecord;

private:
  typedef std::unordered_map<Glib::ustring, boost::python::object /* Actually PyGlomRelatedRecord* */, std::hash<std::string>> type_map_relatedrecords;

  boost::python::object m_record; //Actually PyGlomRecord. A reference to the parent record.


  type_map_relationships m_map_relationships;

  type_map_relatedrecords m_map_relatedrecords;
};



} //namespace Glom

#endif //GLOM_PYTHON_GLOM_RELATED_H
