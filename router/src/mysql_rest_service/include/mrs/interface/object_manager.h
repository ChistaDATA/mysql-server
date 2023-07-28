/*
  Copyright (c) 2021, 2023, Oracle and/or its affiliates.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2.0,
  as published by the Free Software Foundation.

  This program is also distributed with certain software (including
  but not limited to OpenSSL) that is licensed under separate terms,
  as designated in a particular file or component or in included license
  documentation.  The authors of MySQL hereby grant you an additional
  permission to link the program and your derivative works with the
  separately licensed software that they have included with MySQL.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef ROUTER_SRC_REST_MRS_SRC_MRS_INTERFACE_OBJECT_MANAGER_H_
#define ROUTER_SRC_REST_MRS_SRC_MRS_INTERFACE_OBJECT_MANAGER_H_

#include <vector>

#include "mrs/database/entry/content_file.h"
#include "mrs/database/entry/db_object.h"
#include "mrs/interface/object_schema.h"
#include "mrs/interface/state.h"

namespace mrs {
namespace interface {

class ObjectManager {
 public:
  using DbObject = database::entry::DbObject;
  using ContentFile = database::entry::ContentFile;
  using RouteSchema = mrs::interface::ObjectSchema;

 public:
  virtual ~ObjectManager() = default;

  virtual void turn(const State state) = 0;
  virtual void update(const std::vector<DbObject> &paths) = 0;
  virtual void update(const std::vector<ContentFile> &contents) = 0;
  virtual void schema_not_used(RouteSchema *route) = 0;
};

}  // namespace interface
}  // namespace mrs

#endif  // ROUTER_SRC_REST_MRS_SRC_MRS_INTERFACE_OBJECT_MANAGER_H_
