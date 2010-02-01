#ifndef GLOM_PYGDA_VALUE_CONVERSIONS_H
#define GLOM_PYGDA_VALUE_CONVERSIONS_H

//We need to include this before anything else, to avoid redefinitions:
#include <Python.h>

#include <glibmm/value.h>
#include <libgda/libgda.h>
#include <boost/python.hpp>

int
glom_pygda_value_from_pyobject(GValue *boxed, PyObject *input);

//PyObject *
//glom_pygda_value_as_pyobject(const GValue *value, gboolean copy_boxed);

boost::python::object glom_pygda_value_as_boost_pyobject(const Glib::ValueBase& value);

#endif //GLOM_PYGDA_VALUE_CONVERSIONS_H
