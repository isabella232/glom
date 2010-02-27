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
//#include <Python.h>

//#define NO_IMPORT_PYGOBJECT //To avoid a multiple definition in pygtk.
#include <pygobject.h> //For the PyGObject and PyGBoxed struct definitions.

#include <libglom/python_embed/py_glom_record.h>
#include <libglom/python_embed/py_glom_related.h>
#include <libglom/python_embed/pygdavalue_conversions.h> //For pygda_value_as_pyobject().
#include <libglom/data_structure/glomconversions.h>

#include <libglom/data_structure/field.h>
#include <glibmm/ustring.h>

namespace Glom
{

//Set the object's member data, from the parameters supplied when creating the object:
PyGlomRecord::PyGlomRecord()
: m_document(0)
{
}

PyGlomRecord::~PyGlomRecord()
{
}

std::string PyGlomRecord::get_table_name() const
{
  return m_table_name;
}

boost::python::object PyGlomRecord::get_connection()
{
  boost::python::object result;

  if(m_connection)
  {
    //Ask pygobject to create a PyObject* that wraps our GObject,
    //presumably using something from pygda:
    PyObject* cobject = pygobject_new( G_OBJECT(m_connection->gobj()) );
    result = boost::python::object( boost::python::borrowed(cobject) );
  }

  return result;
}

boost::python::object PyGlomRecord::get_related()
{
  //We initialize it here, so that this work never happens if it's not needed:
  if(!m_related)
  {
    //Return a new RelatedRecord:
    m_related = boost::python::object(new PyGlomRelated()); //TODO_NotSure

    //Fill it:
    Document::type_vec_relationships vecRelationships = m_document->get_relationships(m_table_name);
    PyGlomRelated::type_map_relationships map_relationships;
    for(Document::type_vec_relationships::const_iterator iter = vecRelationships.begin(); iter != vecRelationships.end(); ++iter)
    {
      if(*iter)
        map_relationships[(*iter)->get_name()] = *iter;
    }

    boost::python::extract<PyGlomRelated*> extractor(m_related);
    if(extractor.check())
    {
      PyGlomRelated* related_cpp = extractor;
      PyGlomRelated_SetRelationships(related_cpp, map_relationships);
      related_cpp->m_record = boost::python::object(this); //TODO_NotSure
    }
  }

  return m_related;
}

long PyGlomRecord::len() const
{
  return m_map_field_values.size();
}

boost::python::object PyGlomRecord::getitem(const boost::python::object& cppitem)
{
  const std::string key = boost::python::extract<std::string>(cppitem);

  PyGlomRecord::type_map_field_values::const_iterator iterFind = m_map_field_values.find(key);
  if(iterFind != m_map_field_values.end())
  {
    return glom_pygda_value_as_boost_pyobject(iterFind->second);
  }

  return boost::python::object();
}

//TODO: Stop this from being used in field calculations, by making the record somehow read-only.
void PyGlomRecord::setitem(const boost::python::object& key, const boost::python::object& value)
{
  //Get the specificd field name (and details) and value:

  std::string field_name;
  boost::python::extract<std::string> extractor(key);
  if(extractor.check())
    field_name = extractor;

  sharedptr<const Field> field = m_document->get_field(m_table_name, field_name);
  if(!field)
  {
     std::cerr << "PyGlomRecord::setitem(): field=" << field_name << " not found in table=" << m_table_name << std::endl;
     //TODO: Throw python exception.
     return;
  }

  const Field::glom_field_type field_type = field->get_glom_type(); //TODO

  Gnome::Gda::Value field_value;
  GValue value_c = {0, {{0}}};
  bool test = glom_pygda_value_from_pyobject(&value_c, value);
  if(test && G_IS_VALUE(&value))
  {
    field_value = Gnome::Gda::Value(&value_c);

    //Make sure that the value is of the expected Gda type:
    field_value = Conversions::convert_value(field_value, field_type);

    g_value_unset(&value_c);
  }
  else
    field_value = Conversions::get_empty_value(field_type);

  //std::cout << "debug: PyGlomRecord::setitem(): field_name=" << field_name << ", field_type=" << field_type << ", field_value=" << field_value.to_string() << std::endl;


  //Set the value in the database:
  if(!m_key_field || Conversions::value_is_empty(m_key_field_value))
  {
    std::cerr << "PyGlomRecord::setitem(): The primary key name and value is not set. This would be a Glom bug." << std::endl;
    return;
  }

  if(!m_connection)
  {
    std::cerr << "PyGlomRecord::setitem(): The connection is null. This would be a Glom bug." << std::endl;
    return;
  }

  Glib::RefPtr<Gnome::Gda::Set> params = Gnome::Gda::Set::create();
  params->add_holder(field->get_holder(field_value));
  params->add_holder(m_key_field->get_holder(m_key_field_value));

  Glib::RefPtr<Gnome::Gda::SqlBuilder> builder = 
    Gnome::Gda::SqlBuilder::create(Gnome::Gda::SQL_STATEMENT_UPDATE);
  builder->set_table(m_table_name);
  builder->add_field_value_as_value(field->get_name(), field_value);
  builder->set_where(
    builder->add_cond(Gnome::Gda::SQL_OPERATOR_TYPE_EQ,
      builder->add_id(m_key_field->get_name()),
      builder->add_expr(m_key_field_value)));
 
  bool updated = false;
  try
  {
    updated = m_connection->statement_execute_non_select_builder(builder);
  }
  catch(const Glib::Exception& ex)
  {
    std::cerr << "PyGlomRecord::setitem(): exception while executing query: " << ex.what() << std::endl;
  }
  catch(const std::exception& ex)
  {
    std::cerr << "PyGlomRecord::setitem(): exception while executing query: " << ex.what() << std::endl;
  }

  if(!updated)
  {
    Glib::ustring failed_query;

    std::cerr << "PyGlomRecord::setitem(): UPDATE failed." << std::endl;
    /*
    if(stmt)
      failed_query = stmt->to_sql(params); //this throws too.
    else
      failed_query = strQuery;

    try
    {
      std::cerr << "  with SQL query: " << failed_query << std::endl;
    }
    catch(const Glib::Exception& ex)
    {
      std::cerr << "  query string could not be converted to std::cerr" << std::endl;
    }
    */

  }

  //TODO: Do dependent calculations and lookups. Or just do them for all fields for this record when finishing the script?
}

void PyGlomRecord_SetFields(PyGlomRecord* self, const PyGlomRecord::type_map_field_values& field_values, Document* document, const Glib::ustring& table_name, const sharedptr<const Field>& key_field, const Gnome::Gda::Value& key_field_value, const Glib::RefPtr<Gnome::Gda::Connection>& opened_connection)
{
  g_assert(self);

  self->m_map_field_values = field_values;

  self->m_table_name = table_name;
  self->m_key_field = key_field;
  self->m_key_field_value = key_field_value;

  if(self->m_document == 0)
    self->m_document = document;

  self->m_connection = opened_connection;

  /*
  if(self->m_fields_dict == 0)
    self->m_fields_dict = PyDict_New();

  PyDict_Clear( self->m_fields_dict );

  //TODO: Cache this in one place:
  PyObject* module_gda = PyImport_ImportModule("gda");
  if(!module_gda)
    g_warning("Could not import python gda module.");

  PyObject* module_gda_dict = PyModule_GetDict(module_gda);
  PyObject* pyTypeGdaValue = PyDict_GetItemString(module_gda_dict, "Value"); //TODO: Unref this?
  if(!pyTypeGdaValue || !PyType_Check(pyTypeGdaValue))
    g_warning("Could not get gda.Value from gda_module.");

  //Add the new pairs:
  for(type_map_fields::const_iterator iter = fields.begin(); iter != fields.end(); ++iter)
  {
    //Add each name/value pair:

    //PyObject* pyValue = _PyObject_New((PyTypeObject*)pyTypeGdaValue);
    //if(!pyValue)
    //  g_warning("_PyObject_New() failed.");

    //PyObject_New() does not call the derived constructor. Stupid PyObject_New().
    //PyObject* new_args = PyTuple_New(0);
    //pyValue->ob_type->tp_init(pyValue, new_args, 0);
    //Py_DECREF(new_args);

    //PyObject_Call() instantiates a type when passed that type as the object to call. Strange Python.
    PyObject* new_args = PyTuple_New(0);
    PyObject* pyValue = PyObject_Call(pyTypeGdaValue, new_args, 0);
    Py_DECREF(new_args);
    if(!pyValue)
    {
      g_warning("PyObject_Call() failed to instantiate the type.");
      Record_HandlePythonError();
    }

    //g_warning("pyValue->op_type->tp_name=%s", pyValue->ob_type->tp_name);

    PyGBoxed* pygBoxed = (PyGBoxed*)(pyValue);
    GdaValue* pGdaValue = (GdaValue*)pygBoxed->boxed;

    if(!pGdaValue)
      g_warning("pygBoxed->boxed is NULL");

    gda_value_set_from_value(pGdaValue, (iter->second).gobj());

    PyDict_SetItemString(self->m_fields_dict, iter->first.c_str(), pyValue);
  }
  */
}

} //namespace Glom
