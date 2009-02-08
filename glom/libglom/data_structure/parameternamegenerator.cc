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

#include "parameternamegenerator.h"

namespace Glom
{

ParameterNameGenerator::ParameterNameGenerator() :
    m_id(0)
{
    
}

ParameterNameGenerator::~ParameterNameGenerator()
{
    
}
    
Glib::ustring ParameterNameGenerator::get_next_name(unsigned int& id)
{
    m_id_table[m_id] = Glib::ustring::compose("glom_param%1", m_id);
    id = m_id++;
    return m_id_table[id];
}

Glib::ustring ParameterNameGenerator::get_name_from_id(unsigned int id)
{
    return m_id_table[id];
}

} // namespace Glom
