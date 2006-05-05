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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef GLOM_PYTHON_GLOM_RELATED_H
#define GLOM_PYTHON_GLOM_RELATED_H

#include <glom/libglom/python_embed/py_glom_record.h>
#include <glom/libglom/data_structure/relationship.h>

class PyGlomRelatedRecord;

class PyGlomRelated
{
public:
  PyObject_HEAD

  PyGlomRecord* m_record; //A reference to the parent record.

  typedef std::map<Glib::ustring, sharedptr<Relationship> > type_map_relationships;
  type_map_relationships* m_pMap_relationships;

  typedef std::map<Glib::ustring, PyGlomRelatedRecord*> type_map_relatedrecords;
  type_map_relatedrecords* m_pMap_relatedrecords;
};

PyTypeObject* PyGlomRelated_GetPyType();


void PyGlomRelated_SetRelationships(PyGlomRelated* self, const PyGlomRelated::type_map_relationships& relationships);




#endif //GLOM_PYTHON_GLOM_RELATED_H
