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

//We need to include this before anything else, to avoid redefinitions:
//#include <Python.h>

#define NO_IMPORT_PYGOBJECT //To avoid a multiple definition in pygtk.
#include <pygobject.h> //For the PyGObject and PyGBoxed struct definitions.

#include <libglom/python_embed/py_glom_record.h>
#include <libglom/python_embed/py_glom_related.h>
#include <libglom/python_embed/pygdavalue_conversions.h> //For pygda_value_as_pyobject().
#include <libglom/data_structure/glomconversions.h>

#include <libglom/data_structure/field.h>
#include <glibmm/ustring.h>

#include <iostream>

namespace Glom
{

//Set the object's member data, from the parameters supplied when creating the object:
PyGlomRecord::PyGlomRecord()
: m_document(0),
  m_read_only(false)
{
}

PyGlomRecord::~PyGlomRecord()
{
}

void PyGlomRecord::set_read_only()
{
  m_read_only = true;
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
    if(!_PyGObject_API)
    {
      std::cerr << G_STRFUNC << ": pyggobject does not seem to be initialized properly." << std::endl;
      return result;
    }

    PyObject* cobject = pygobject_new( G_OBJECT(m_connection->gobj()) );
    if(cobject)
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
    for(const auto& relationship : vecRelationships)
    {
      if(relationship)
        map_relationships[relationship->get_name()] = relationship;
    }

    boost::python::extract<PyGlomRelated*> extractor(m_related);
    if(extractor.check())
    {
      PyGlomRelated* related_cpp = extractor;
      related_cpp->set_relationships(map_relationships);
      related_cpp->m_record = boost::python::object(this); //TODO_NotSure
    }
  }

  return m_related;
}

PyGlomRecord::type_map_field_values::size_type PyGlomRecord::len() const
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
  if(m_read_only)
  {
    std::cerr << G_STRFUNC << ": PyGlomRecord::setitem(): Failed to set a value because the record object is read-only."<< std::endl;
    return;
  }
  //Get the specified field name (and details) and value:

  std::string field_name;
  boost::python::extract<std::string> extractor(key);
  if(extractor.check())
    field_name = extractor;

  std::shared_ptr<const Field> field = m_document->get_field(m_table_name, field_name);
  if(!field)
  {
     std::cerr << G_STRFUNC << ": field=" << field_name << " not found in table=" << m_table_name << std::endl;
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

  //std::cout << "debug: " << G_STRFUNC << ": field_name=" << field_name << ", field_type=" << field_type << ", field_value=" << field_value.to_string() << std::endl;


  //Set the value in the database:
  if(!m_key_field || Conversions::value_is_empty(m_key_field_value))
  {
    std::cerr << G_STRFUNC << ": The primary key name and value is not set. This would be a Glom bug." << std::endl;
    return;
  }

  if(!m_connection)
  {
    std::cerr << G_STRFUNC << ": The connection is null. This would be a Glom bug." << std::endl;
    return;
  }

  Glib::RefPtr<Gnome::Gda::SqlBuilder> builder =
    Gnome::Gda::SqlBuilder::create(Gnome::Gda::SQL_STATEMENT_UPDATE);
  builder->set_table(m_table_name);
  builder->add_field_value_as_value(field->get_name(), field_value);
  builder->set_where(
    builder->add_cond(Gnome::Gda::SQL_OPERATOR_TYPE_EQ,
      builder->add_field_id(m_key_field->get_name(), m_table_name),
      builder->add_expr(m_key_field_value)));

  bool updated = false;
  try
  {
    updated = m_connection->statement_execute_non_select_builder(builder);
  }
  catch(const Glib::Exception& ex)
  {
    std::cerr << G_STRFUNC << ": exception while executing query: " << ex.what() << std::endl;
  }
  catch(const std::exception& ex)
  {
    std::cerr << G_STRFUNC << ": exception while executing query: " << ex.what() << std::endl;
  }

  if(!updated)
  {
    std::cerr << G_STRFUNC << ": UPDATE failed." << std::endl;
  }

  //TODO: Do dependent calculations and lookups. Or just do them for all fields for this record when finishing the script?
}

void PyGlomRecord::set_fields(const PyGlomRecord::type_map_field_values& field_values, const Document* document, const Glib::ustring& table_name, const std::shared_ptr<const Field>& key_field, const Gnome::Gda::Value& key_field_value, const Glib::RefPtr<Gnome::Gda::Connection>& opened_connection)
{
  m_map_field_values = field_values;
  /* Just for debugging:
  for(const auto& the_pair : field_values)
  {
    const Gnome::Gda::Value value =the_pair.second;
    std::cout << "debug: " << G_STRFUNC << ": field name=" << the_pair.first << ", type=" << g_type_name(value.get_value_type()) << std::endl;
  }
  */

  m_table_name = table_name;
  m_key_field = key_field;
  m_key_field_value = key_field_value;

  if(m_document == 0)
    m_document = document;

  m_connection = opened_connection;
}

} //namespace Glom
