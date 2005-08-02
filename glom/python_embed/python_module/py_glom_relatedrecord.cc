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

#include "py_glom_relatedrecord.h"
#include "py_glom_record.h"
#include "pygdavalue_conversions.h" //For pygda_value_as_pyobject().
#include "../../connectionpool.h"

#include "../../data_structure/field.h"
#include <glibmm/ustring.h>


//Allocate a new object:
//TODO: Why not parse the args here as well as in RelatedRecord_init()?
static PyObject *
RelatedRecord_new(PyTypeObject *type, PyObject * /* args */, PyObject * /* kwds */)
{
  PyGlomRelatedRecord *self  = (PyGlomRelatedRecord*)type->tp_alloc(type, 0);
  if(self)
  {
    self->m_py_gda_connection = 0;
    self->m_document = 0;
    self->m_relationship = 0;
    self->m_from_key_value_sqlized = 0;

    //self->m_record_parent = 0;

    self->m_pMap_field_values = new PyGlomRelatedRecord::type_map_field_values();
  }

  return (PyObject*)self;
}

//Set the object's member data, from the parameters supplied when creating the object:
static int
RelatedRecord_init(PyGlomRelatedRecord *self, PyObject * /* args */, PyObject * /* kwds */)
{
  //static char *kwlist[] = {"test", NULL};

  //if(!PyArg_ParseTupleAndKeywords(args, kwds, "|i", kwlist,
   //                                 &self->m_test))
   // return -1;

  if(self)
  {
    self->m_py_gda_connection = 0;
    self->m_document = 0;
    self->m_relationship = 0;
    self->m_from_key_value_sqlized = 0;

    //self->m_record_parent = 0;

    if(self->m_pMap_field_values == 0)
      self->m_pMap_field_values = new PyGlomRelatedRecord::type_map_field_values();
  }

  return 0;
}

static void
RelatedRecord_dealloc(PyGlomRelatedRecord* self)
{
  if(self->m_pMap_field_values)
  {
    delete self->m_pMap_field_values;
    self->m_pMap_field_values = 0;
  }

  if(self->m_relationship)
  {
    delete self->m_relationship;
    self->m_relationship = 0;
  }

  if(self->m_from_key_value_sqlized)
  {
    delete self->m_from_key_value_sqlized;
    self->m_from_key_value_sqlized = 0;
  }

  if(self->m_py_gda_connection)
  {
    Py_XDECREF( (PyObject*)(self->m_py_gda_connection));
    self->m_py_gda_connection = 0;
  }

  self->ob_type->tp_free((PyObject*)self);
}

/*
static PyObject *
RelatedRecord__get_fields(PyGlomRelatedRecord *self, void * closure )
{
  if(self->m_fields_dict)
  {
    Py_INCREF(self->m_fields_dict); //TODO: Should we do this?
    return self->m_fields_dict;
  }
  else
  {
    Py_INCREF(Py_None);
    return Py_None;
  }
}
*/

/*
static PyGetSetDef RelatedRecord_getseters[] = {
    {"fields",
     (getter)RelatedRecord__get_fields, (setter)0, 0, 0
    },
    {NULL, 0, 0, 0, 0, }  // Sentinel
};
*/


static int
RelatedRecord_tp_as_mapping_length(PyGlomRelatedRecord *self)
{
  return self->m_pMap_field_values->size();
}

static void RelatedRecord_HandlePythonError()
{
  if(PyErr_Occurred())
    PyErr_Print();
}

static PyObject *
RelatedRecord_tp_as_mapping_getitem(PyGlomRelatedRecord *self, PyObject *item)
{
  if(PyString_Check(item))
  {
    const char* pchKey = PyString_AsString(item);
    if(pchKey)
    {
      const Glib::ustring field_name(pchKey);
      PyGlomRelatedRecord::type_map_field_values::const_iterator iterFind = self->m_pMap_field_values->find(field_name);
      if(iterFind != self->m_pMap_field_values->end())
      {
        //If the value has already been stored, then just return it again:
        return pygda_value_as_pyobject(iterFind->second.gobj(), true /* copy */);
      }
      else
      {
         const Glib::ustring related_table = self->m_relationship->get_to_table();

        //Check whether the field exists in the table.
        //TODO_Performance: Do this without the useless Field information?
        Field field;
        bool exists = self->m_document->get_field(self->m_relationship->get_to_table(), field_name, field);
        if(!exists)
          g_warning("RelatedRecord_tp_as_mapping_getitem: field %s not found in table %s", field_name.c_str(), self->m_relationship->get_to_table().c_str());
        else
        {
          //Try to get the value from the database:
          //const Glib::ustring parent_key_name;
          sharedptr<SharedConnection> sharedconnection = ConnectionPool::get_instance()->connect();
          if(sharedconnection)
          {
            Glib::RefPtr<Gnome::Gda::Connection> gda_connection = sharedconnection->get_gda_connection();

            const Glib::ustring related_key_name = self->m_relationship->get_to_field();

            Glib::ustring sql_query = "SELECT " + related_table + "." + field_name + " FROM " + related_table
              + " WHERE " + related_table + "." + related_key_name + " = " + *(self->m_from_key_value_sqlized);

            std::cout << "PyGlomRelatedRecord: Executing:  " << sql_query << std::endl;
            Glib::RefPtr<Gnome::Gda::DataModel> datamodel = gda_connection->execute_single_command(sql_query);
            if(datamodel && datamodel->get_n_rows())
            {
              Gnome::Gda::Value value = datamodel->get_value_at(0, 0);
              g_warning("RelatedRecord_tp_as_mapping_getitem(): value from datamodel = %s", value.to_string().c_str());

              //Cache it, in case it's asked-for again.
              (*(self->m_pMap_field_values))[field_name] = value;
              return pygda_value_as_pyobject(value.gobj(), true /* copy */);
            }
            else if(!datamodel)
            {
              g_warning("RelatedRecord_tp_as_mapping_getitem(): The datamodel was null.");
              ConnectionPool::handle_error(true /* cerr only */);
              RelatedRecord_HandlePythonError();
            }
            else
            {
              g_warning("RelatedRecord_tp_as_mapping_getitem(): No related records exist yet for relationship %s.",  self->m_relationship->get_name().c_str());
            }
          }
        }
      }
    }
  }

  g_warning("RelatedRecord_tp_as_mapping_getitem(): return null.");
  PyErr_SetString(PyExc_IndexError, "field not found");
  return NULL;
}

/*
static int
RelatedRecord_tp_as_mapping_setitem(PyGObject *self, PyObject *item, PyObject *value)
{
  Py_INCREF(Py_None);
  return Py_None;
}
*/

static PyMappingMethods RelatedRecord_tp_as_mapping = {
    (inquiry)RelatedRecord_tp_as_mapping_length,
    (binaryfunc)RelatedRecord_tp_as_mapping_getitem,
    (objobjargproc)0 /* RelatedRecord_tp_as_mapping_setitem */
};

static PyObject *
RelatedRecord_generic_aggregate(PyGlomRelatedRecord* self, PyObject *args, PyObject *kwargs, const Glib::ustring& aggregate)
{
  static char *kwlist[] = { "field_name", 0 };
  PyObject* py_field_name = 0;

  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:RelatedRecord.sum", kwlist, &py_field_name))
    return NULL;

  if(!(PyString_Check(py_field_name)))
    return NULL;

  const char* pchKey = PyString_AsString(py_field_name);
  if(pchKey)
  {
    const Glib::ustring field_name(pchKey);
    const Glib::ustring related_table = self->m_relationship->get_to_table();

    //Check whether the field exists in the table.
    //TODO_Performance: Do this without the useless Field information?
    Field field;
    bool exists = self->m_document->get_field(self->m_relationship->get_to_table(), field_name, field);
    if(!exists)
      g_warning("RelatedRecord_sum: field %s not found in table %s", field_name.c_str(), self->m_relationship->get_to_table().c_str());
    else
    {
      //Try to get the value from the database:
      //const Glib::ustring parent_key_name;
      sharedptr<SharedConnection> sharedconnection = ConnectionPool::get_instance()->connect();
      if(sharedconnection)
      {
        Glib::RefPtr<Gnome::Gda::Connection> gda_connection = sharedconnection->get_gda_connection();

        const Glib::ustring related_key_name = self->m_relationship->get_to_field();

        Glib::ustring sql_query = "SELECT " + aggregate + "(" + related_table + "." + field_name + ") FROM " + related_table
          + " WHERE " + related_table + "." + related_key_name + " = " + *(self->m_from_key_value_sqlized);

        //std::cout << "PyGlomRelatedRecord: Executing:  " << sql_query << std::endl;
        Glib::RefPtr<Gnome::Gda::DataModel> datamodel = gda_connection->execute_single_command(sql_query);
        if(datamodel && datamodel->get_n_rows())
        {
          Gnome::Gda::Value value = datamodel->get_value_at(0, 0);
          //g_warning("RelatedRecord_generic_aggregate(): value from datamodel = %s", value.to_string().c_str());

          //Cache it, in case it's asked-for again.
          (*(self->m_pMap_field_values))[field_name] = value;
          return pygda_value_as_pyobject(value.gobj(), true /* copy */);
        }
        else if(!datamodel)
        {
          g_warning("RelatedRecord_generic_aggregate(): The datamodel was null.");
          ConnectionPool::handle_error(true /* cerr only */);
          RelatedRecord_HandlePythonError();
        }
        else
        {
          g_warning("RelatedRecord_generic_aggregate(): No related records exist yet for relationship %s.",  self->m_relationship->get_name().c_str());
        }
      }
    }
  }

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *
RelatedRecord_sum(PyGlomRelatedRecord* self, PyObject *args, PyObject *kwargs)
{
  return RelatedRecord_generic_aggregate(self, args, kwargs, "sum");
}

static PyObject *
RelatedRecord_count(PyGlomRelatedRecord* self, PyObject *args, PyObject *kwargs)
{
  return RelatedRecord_generic_aggregate(self, args, kwargs, "count");
}

static PyObject *
RelatedRecord_min(PyGlomRelatedRecord* self, PyObject *args, PyObject *kwargs)
{
  return RelatedRecord_generic_aggregate(self, args, kwargs, "min");
}

static PyObject *
RelatedRecord_max(PyGlomRelatedRecord* self, PyObject *args, PyObject *kwargs)
{
  return RelatedRecord_generic_aggregate(self, args, kwargs, "max");
}

static PyMethodDef RelatedRecord_methods[] = {
    {"sum", (PyCFunction)RelatedRecord_sum, METH_VARARGS | METH_KEYWORDS,
     "Add all values of the field in the related records."
    },
    {"count", (PyCFunction)RelatedRecord_count, METH_VARARGS | METH_KEYWORDS,
     "Count all values in the field in the related records."
    },
    {"min", (PyCFunction)RelatedRecord_min, METH_VARARGS | METH_KEYWORDS,
     "Minimum of all values of the field in the related records."
    },
    {"max", (PyCFunction)RelatedRecord_max, METH_VARARGS | METH_KEYWORDS,
     "Maximum of all values of the field in the related records."
    },
    {NULL, 0, 0, 0}  /* Sentinel */
};




static PyTypeObject pyglom_RelatedRecordType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "glom.RelatedRecord",             /*tp_name*/
    sizeof(PyGlomRelatedRecord), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)RelatedRecord_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    &RelatedRecord_tp_as_mapping,      /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    "Glom objects",           /* tp_doc */
    0,                  /* tp_traverse */
    0,                   /* tp_clear */
    0,                   /* tp_richcompare */
    0,                   /* tp_weaklistoffset */
    0,                   /* tp_iter */
    0,                   /* tp_iternext */
    RelatedRecord_methods,             /* tp_methods */
    0 /* RelatedRecord_members */,             /* tp_members */
    0, /* RelatedRecord_getseters, */                   /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)RelatedRecord_init,      /* tp_init */
    0,                         /* tp_alloc */
    RelatedRecord_new,                 /* tp_new */
    0, 0, 0, 0, 0, 0, 0, 0,
};

PyTypeObject* PyGlomRelatedRecord_GetPyType()
{
  return &pyglom_RelatedRecordType;
}


void PyGlomRelatedRecord_SetRelationship(PyGlomRelatedRecord* self, const Relationship& relationship, const Glib::ustring& from_key_value_sqlized,  Document_Glom* document)
{
  self->m_relationship = new Relationship(relationship);
  self->m_from_key_value_sqlized = new Glib::ustring(from_key_value_sqlized);
  self->m_document = document;
}



