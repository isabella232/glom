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

#ifndef GLOM_PYTHON_GLOM_RECORD_H
#define GLOM_PYTHON_GLOM_RECORD_H

#include "boost/python.hpp"
#include "../data_structure/field.h"
#include <glibmm/ustring.h>


class PyGlomRecord
{
public:
  int get_test() const;

protected:
  int m_test;
};

BOOST_PYTHON_MODULE(PyGlom)
{
    boost::python::class_<PyGlomRecord>("Record")
        .def("get_test", &PyGlomRecord::get_test)
    ;
}

#endif //GLOM_PYTHON_GLOM_RECORD_H
