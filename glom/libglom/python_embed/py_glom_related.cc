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

#include <glom/libglom/python_embed/py_glom_related.h>
//#include <glom/libglom/python_embed/py_glom_record.h>
#include <glom/libglom/python_embed/py_glom_relatedrecord.h>

#include <glom/libglom/data_structure/field.h>
#include <glom/libglom/data_structure/glomconversions.h>
#include <glibmm/ustring.h>


namespace Glom
{

//Allocate a new object:
//TODO: Why not parse the args here as well as in Related_init()?
static PyObject *
Related_new(PyTypeObject *type, PyObject * /* args */, PyObject * /* kwds */)
{
  PyGlomRelated *self  = (PyGlomRelated*)type->tp_alloc(type, 0);
  if(self)
  {
    self->m_record = 0;

    self->m_pMap_relationships = new PyGlomRelated::type_map_relationships();
    self->m_pMap_relatedrecords = new PyGlomRelated::type_map_relatedrecords();
  }

  return (PyObject*)self;
}

//Set the object's member data, from the parameters supplied when creating the object:
static int
Related_init(PyObject *self, PyObject* /* args */, PyObject* /* kwds */)
{
  PyGlomRelated *self_related = (PyGlomRelated*)self;

  if(self_related)
  {
    self_related->m_record = 0;

    if(self_related->m_pMap_relationships == 0)
      self_related->m_pMap_relationships = new PyGlomRelated::type_map_relationships();

    if(self_related->m_pMap_relatedrecords == 0)
      self_related->m_pMap_relatedrecords = new PyGlomRelated::type_map_relatedrecords();
  }

  return 0;
}

static void
Related_dealloc(PyObject* self)
{
  PyGlomRelated *self_related = (PyGlomRelated*)self;

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

  self_related->ob_type->tp_free((PyObject*)self);
}


//Adapt to API changes in Python 2.5:
#if defined(PY_VERSION_HEX) && (PY_VERSION_HEX >= 0x02050000) /* Python 2.5 */
static Py_ssize_t
Related_tp_as_mapping_length(PyObject *self)
{
  PyGlomRelated *self_related = (PyGlomRelated*)self;
  return self_related->m_pMap_relationships->size();
}
#else
static int
Related_tp_as_mapping_length(PyObject *self)
{
  PyGlomRelated *self_related = (PyGlomRelated*)self;
  return (int)(self_related->m_pMap_relationships->size());
}
#endif

static PyObject *
Related_tp_as_mapping_getitem(PyObject *self, PyObject *item)
{
  PyGlomRelated *self_related = (PyGlomRelated*)self;

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
        Py_INCREF((PyObject*)pyRelatedRecord);
        return (PyObject*)pyRelatedRecord;
      }
      else
      {
        //If the relationship exists:
        PyGlomRelated::type_map_relationships::const_iterator iterFind = self_related->m_pMap_relationships->find(key);
        if(iterFind != self_related->m_pMap_relationships->end())
        {
          //Return a new RelatedRecord:
          PyObject* new_args = PyTuple_New(0);
          PyGlomRelatedRecord* pyRelatedRecord = (PyGlomRelatedRecord*)PyObject_Call((PyObject*)PyGlomRelatedRecord_GetPyType(), new_args, 0);
          Py_DECREF(new_args);

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

              return (PyObject*)pyRelatedRecord; //TODO: pygda_value_as_pyobject(iterFind->second.gobj(), true /* copy */);
            }
          }
        }
      }
    }
  }

  PyErr_SetString(PyExc_IndexError, "relationship not found");
  return NULL;
}

/*
static int
Related_tp_as_mapping_setitem(PyGObject *self, PyObject *item, PyObject *value)
{
  Py_INCREF(Py_None);
  return Py_None;
}
*/

static PyMappingMethods Related_tp_as_mapping = {
    Related_tp_as_mapping_length,
    Related_tp_as_mapping_getitem,
    (objobjargproc)0 /* Related_tp_as_mapping_setitem */
};


static PyTypeObject pyglom_RelatedType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    (char*)"glom.Related",             /*tp_name*/
    sizeof(PyGlomRelated), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)Related_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    &Related_tp_as_mapping,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    (char*)"Glom objects",           /* tp_doc */
    0,                  /* tp_traverse */
    0,                   /* tp_clear */
    0,                   /* tp_richcompare */
    0,                   /* tp_weaklistoffset */
    0,                   /* tp_iter */
    0,                   /* tp_iternext */
    0 /* Related_methods */,             /* tp_methods */
    0 /* Related_members */,             /* tp_members */
    0,                   /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)Related_init,      /* tp_init */
    0,                         /* tp_alloc */
    Related_new,                 /* tp_new */
    0, 0, 0, 0, 0, 0, 0, 0,
};

PyTypeObject* PyGlomRelated_GetPyType()
{
  return &pyglom_RelatedType;
}

/*
static void Related_HandlePythonError()
{
  if(PyErr_Occurred())
    PyErr_Print();
}
*/


void PyGlomRelated_SetRelationships(PyGlomRelated* self, const PyGlomRelated::type_map_relationships& relationships)
{
  *(self->m_pMap_relationships) = relationships;
}

} //namespace Glom

