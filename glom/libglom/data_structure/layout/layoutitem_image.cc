/* Glom
 *
 * Copyright (C) 2006 Murray Cumming
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

#include <libglom/data_structure/layout/layoutitem_image.h>
#include <libglom/utils.h>
#include <libglom/file_utils.h>
#include <glibmm/i18n-lib.h>

namespace Glom
{

LayoutItem_Image::LayoutItem_Image()
{
  m_translatable_item_type = enumTranslatableItemType::IMAGEOBJECT;
}

LayoutItem_Image::LayoutItem_Image(const LayoutItem_Image& src)
: LayoutItem(src),
  m_image(src.m_image)
{
}

LayoutItem* LayoutItem_Image::clone() const
{
  return new LayoutItem_Image(*this);
}

bool LayoutItem_Image::operator==(const LayoutItem_Image& src) const
{
  auto result = LayoutItem::operator==(src) &&
                (m_image == src.m_image);

  return result;
}

//Avoid using this, for performance:
LayoutItem_Image& LayoutItem_Image::operator=(const LayoutItem_Image& src)
{
  LayoutItem::operator=(src);

  m_image = src.m_image;

  return *this;
}

Glib::ustring LayoutItem_Image::get_part_type_name() const
{
  //Translators: This is the name of a UI element (a layout part name).
  return _("Image");
}

Glib::ustring LayoutItem_Image::get_report_part_id() const
{
  return "field"; //We reuse this for this node.
}

Gnome::Gda::Value LayoutItem_Image::get_image() const
{
  return m_image;
}

void LayoutItem_Image::set_image(const Gnome::Gda::Value& image)
{
  m_image = image;
}

Glib::ustring LayoutItem_Image::create_local_image_uri() const
{
  return FileUtils::create_local_image_uri(m_image);
}

} //namespace Glom

