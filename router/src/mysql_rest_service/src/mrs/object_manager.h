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

#ifndef ROUTER_SRC_REST_MRS_SRC_MRS_OBJECT_MANAGER_H_
#define ROUTER_SRC_REST_MRS_SRC_MRS_OBJECT_MANAGER_H_

#include <map>
#include <memory>
#include <vector>

#include "collector/mysql_cache_manager.h"
#include "interface/object.h"
#include "interface/object_factory.h"
#include "mrs/database/entry/content_file.h"
#include "mrs/database/entry/db_object.h"
#include "mrs/interface/authorize_manager.h"
#include "mrs/interface/object_manager.h"
#include "mrs/interface/object_schema.h"
#include "mrs/state.h"

namespace mrs {

class ObjectManager : public mrs::interface::ObjectManager {
 public:
  using EntryKey = database::entry::EntryKey;
  using EntryType = database::entry::EntryType;
  using DbObject = database::entry::DbObject;
  using ContentFile = database::entry::ContentFile;
  using RoutePtr = std::shared_ptr<mrs::interface::Object>;
  using RouteSchema = mrs::interface::ObjectSchema;
  using RouteSchemaPtr = std::shared_ptr<mrs::interface::ObjectSchema>;

  // TODO(lkotula): This is prototype, thus not every solution done here is
  // thread safe, some workarounded for now by shared_ptr & shared_from_this
  // (Shouldn't be in review)
 public:
  ObjectManager(collector::MysqlCacheManager *cache, const bool is_ssl,
                mrs::interface::AuthorizeManager *auth_manager,
                std::shared_ptr<::mrs::interface::ObjectFactory> factory);
  ObjectManager(collector::MysqlCacheManager *cache, const bool is_ssl,
                mrs::interface::AuthorizeManager *auth_manager);
  ~ObjectManager() override;

  void turn(const State state) override;
  void update(const std::vector<DbObject> &paths) override;
  void update(const std::vector<ContentFile> &contents) override;
  void schema_not_used(RouteSchema *route) override;

 private:
  void handle_new_route(const DbObject &pe);
  void handle_new_route(const ContentFile &pe);
  void handle_existing_route(const DbObject &pe);
  void handle_existing_route(const ContentFile &pe);
  void handle_delete_route(const EntryKey &pe_id);

  RouteSchemaPtr handle_schema(const DbObject &pe);
  RouteSchemaPtr handle_schema(const ContentFile &pe);

  std::map<EntryKey, RoutePtr> routes_;
  std::map<std::string, RouteSchemaPtr> schemas_;
  collector::MysqlCacheManager *cache_;
  bool is_ssl_;
  State state_{stateOff};
  mrs::interface::AuthorizeManager *auth_manager_;
  std::shared_ptr<mrs::interface::ObjectFactory> factory_;
};

}  // namespace mrs

#endif  // ROUTER_SRC_REST_MRS_SRC_MRS_OBJECT_MANAGER_H_
