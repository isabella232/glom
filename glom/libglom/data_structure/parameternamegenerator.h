/*
 * glom
 * Copyright (C) Johannes Schmid 2009 <jhs@gnome.org>
 * 
 * glom is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * glom is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _PARAMETERNAMEGENERATOR_H_
#define _PARAMETERNAMEGENERATOR_H_

#include <map>
#include <glibmm/ustring.h>

namespace Glom
{

class ParameterNameGenerator
{
public:
    ParameterNameGenerator();
    ~ParameterNameGenerator();
    
    Glib::ustring get_next_name(unsigned int& id);
    Glib::ustring get_name_from_id(unsigned int id);
    
private:
    std::map<unsigned int, Glib::ustring> m_id_table;
    unsigned int m_id;
};

#endif // _PARAMETERNAMEGENERATOR_H_

}