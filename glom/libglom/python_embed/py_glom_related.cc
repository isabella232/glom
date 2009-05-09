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

//We need to include this before anything else, to avoid redefinitions:
#include <Python.h>
#include <compile.h> /* for the PyCodeObject */
#include <eval.h> /* for PyEval_EvalCode */
#include <objimpl.h> /* for PyObject_New() */

#include <libglom/python_embed/py_glom_related.h>
//#include <libglom/python_embed/py_glom_record.h>
#include <libglom/python_embed/py_glom_relatedrecord.h>

#include <libglom/data_structure/field.h>
#include <libglom/data_structure/glomconversions.h>
#include <glibmm/ustring.h>


namespace Glom
{

PyGlomRelated::PyGlomRelated()
{
  PyGlomRelated *self  = this;
  if(self)
  {
    self->m_record = 0;

    self->m_pMap_relationships = new PyGlomRelated::type_map_relationships();
    self->m_pMap_relatedrecords = new PyGlomRelated::type_map_relatedrecords();
  }
}

PyGlomRelated::~PyGlomRelated()
{
  PyGlomRelated *self_related = this;

  if(self_related->m_pMap_relationships)
  {
    delete self_related->m_pMap_relationships;
    self_related->m_pMap_relationships = 0;
  }

  if(self_related->m_record)
  {
    Py_XDECREF( (PyObject*)self_related->m_record );
    self_related->m_record = 0;
  }

  if(self_related->m_pMap_relatedrecords)
  {
    //Unref each item:
    for(PyGlomRelated::type_map_relatedrecords::iterator iter = self_related->m_pMap_relatedrecords->begin(); iter != self_related->m_pMap_relatedrecords->end(); ++iter)
    {
      Py_XDECREF( (PyObject*)(iter->second) );
    }

    delete self_related->m_pMap_relatedrecords;
    self_related->m_pMap_relatedrecords = 0;
  }
}


long PyGlomRelated::length() const
{
  const PyGlomRelated *self_related = this;
  return self_related->m_pMap_relationships->size();
}

boost::python::object PyGlomRelated::getitem(boost::python::object cppitem)
{
  PyGlomRelated *self_related = this;

  PyObject* item = cppitem.ptr(); //TODO: Just use the C++ object.

  if(PyString_Check(item))
  {
    const char* pchKey = PyString_AsString(item);
    if(pchKey)
    {
      const Glib::ustring key(pchKey);

      //Return a cached item if possible:
      PyGlomRelated::type_map_relatedrecords::iterator iterCacheFind = self_related->m_pMap_relatedrecords->find(key);
      if(iterCacheFind != self_related->m_pMap_relatedrecords->end())
      {
        //Return a reference to the cached item:
        PyGlomRelatedRecord* pyRelatedRecord = iterCacheFind->second;

        PyObject* cobject = (PyObject*)pyRelatedRecord;
        Py_INCREF((PyObject*)pyRelatedRecord);
        return boost::python::object(boost::python::borrowed(cobject));
      }
      else
      {
        //If the relationship exists:
        PyGlomRelated::type_map_relationships::const_iterator iterFind = self_related->m_pMap_relationships->find(key);
        if(iterFind != self_related->m_pMap_relationships->end())
        {
          //Return a new RelatedRecord:
          PyGlomRelatedRecord* pyRelatedRecord = new PyGlomRelatedRecord();

          //Fill it.

          //Get the value of the from_key in the parent record.
          sharedptr<Relationship> relationship = iterFind->second;
          const Glib::ustring from_key = relationship->get_from_field();
          PyGlomRecord::type_map_field_values::const_iterator iterFromKey = self_related->m_record->m_pMap_field_values->find(from_key);
          if(iterFromKey != self_related->m_record->m_pMap_field_values->end())
          {
            const Gnome::Gda::Value from_key_value = iterFromKey->second;

            //TODO_Performance:
            //Get the full field details so we can sqlize its value:
            sharedptr<Field> from_key_field;
            from_key_field = self_related->m_record->m_document->get_field(*(self_related->m_record->m_table_name), from_key);
            if(from_key_field)
            {
              Glib::ustring key_value_sqlized;
              //std::cout << "from_key_field=" << from_key_field->get_name() << ", from_key_value=" << from_key_value.to_string() << std::endl;

              if(!Conversions::value_is_empty(from_key_value)) //Do not link on null-values. That would cause us to link on 0, or "0".
                key_value_sqlized = from_key_field->sql(from_key_value);

              PyGlomRelatedRecord_SetRelationship(pyRelatedRecord, iterFind->second, key_value_sqlized, self_related->m_record->m_document);

              //Store it in the cache:
              Py_INCREF((PyObject*)pyRelatedRecord); //Dereferenced in _dealloc().
              (*(self_related->m_pMap_relatedrecords))[key] = pyRelatedRecord;

              PyObject* cobject = (PyObject*)pyRelatedRecord;
              Py_INCREF((PyObject*)cobject);
              return boost::python::object(boost::python::borrowed(cobject));
            }
          }
        }
      }
    }
  }

  PyErr_SetString(PyExc_IndexError, "relationship not found");
  return boost::python::object(); //Is this the same as returning a NULL PyObject*, meaning an error occurred?
}

void PyGlomRelated_SetRelationships(PyGlomRelated* self, const PyGlomRelated::type_map_relationships& relationships)
{
  *(self->m_pMap_relationships) = relationships;
}

} //namespace Glom

