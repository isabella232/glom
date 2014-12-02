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
}

PyGlomRelated::~PyGlomRelated()
{
}


PyGlomRelated::type_map_relationships::size_type PyGlomRelated::len() const
{
  return m_map_relationships.size();
}

boost::python::object PyGlomRelated::getitem(const boost::python::object& cppitem)
{
  boost::python::extract<std::string> extractor(cppitem);
  if(extractor.check())
  {
    const std::string key = extractor;
    if(!key.empty())
    {
      //Return a cached item if possible:
      PyGlomRelated::type_map_relatedrecords::iterator iterCacheFind = m_map_relatedrecords.find(key);
      if(iterCacheFind != m_map_relatedrecords.end())
      {
        //Return a reference to the cached item:
        boost::python::object objectRelatedRecord = iterCacheFind->second;
        return objectRelatedRecord;
      }
      else
      {
        //If the relationship exists:
        PyGlomRelated::type_map_relationships::const_iterator iterFind = m_map_relationships.find(key);
        if(iterFind != m_map_relationships.end())
        {
          //Get the value of the from_key in the parent record.
          sharedptr<Relationship> relationship = iterFind->second;
          const Glib::ustring from_key = relationship->get_from_field();

          boost::python::extract<PyGlomRecord*> extractor(m_record);
          if(extractor.check())
          {
            PyGlomRecord* record = extractor;
            PyGlomRecord::type_map_field_values::const_iterator iterFromKey = record->m_map_field_values.find(from_key);
            if(iterFromKey != record->m_map_field_values.end())
            {
              const Gnome::Gda::Value& from_key_value = iterFromKey->second;

              //TODO_Performance:
              //Get the full field details so we can sqlize its value:
              sharedptr<const Field> from_key_field = record->m_document->get_field(record->m_table_name, from_key);
              if(from_key_field)
              {
                //Return a new RelatedRecord:
                PyGlomRelatedRecord* pyRelatedRecord = new PyGlomRelatedRecord();
                pyRelatedRecord->set_relationship(iterFind->second, from_key_value, record->m_document);

                //Store it in the cache:
                boost::python::object objectRelatedRecord(pyRelatedRecord);
                m_map_relatedrecords[key] = objectRelatedRecord;

                return objectRelatedRecord;
              }
            }
          }
        }
      }
    }
  }

  PyErr_SetString(PyExc_IndexError, "relationship not found");
  boost::python::throw_error_already_set(); //TODO: Find a simpler way to throw a python exception/error.
  return boost::python::object();
}


/*
static void Related_HandlePythonError()
{
  if(PyErr_Occurred())
    PyErr_Print();
}
*/


void PyGlomRelated::set_relationships(const PyGlomRelated::type_map_relationships& relationships)
{
  m_map_relationships = relationships;
}

} //namespace Glom
