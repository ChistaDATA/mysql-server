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

#include "mrs/object.h"

#include <stdexcept>
#include <string_view>

#include "mysql/harness/logging/logging.h"

#include "helper/container/any_of.h"
#include "helper/container/generic.h"
#include "mrs/database/helper/query_table_columns.h"

namespace mrs {

using namespace helper;

using Allowed = mrs::database::entry::DbObject::Format;

Object::Object(const EntryDbObject &pe, RouteSchemaPtr schema,
               collector::MysqlCacheManager *cache, const bool is_ssl,
               mrs::interface::AuthorizeManager *auth_manager,
               std::shared_ptr<HandlerFactory> handler_factory,
               std::shared_ptr<QueryFactory> query_factory)
    : schema_{schema},
      pe_{pe},
      cache_{cache},
      is_ssl_{is_ssl},
      auth_manager_{auth_manager},
      handler_factory_{handler_factory},
      query_factory_{query_factory} {
  schema_->route_register(this);
  update_variables();
}

Object::~Object() {
  if (schema_) schema_->route_unregister(this);
}

void Object::turn(const State state) {
  if (stateOff == state || !pe_.active) {
    handle_object_.reset();
    handle_metadata_.reset();

    return;
  }

  switch (pe_.type) {
    case EntryDbObject::typeTable:
      handlers_for_table();
      break;
    case EntryDbObject::typeProcedure:
      handlers_for_sp();
      break;
  }
}

void Object::handlers_for_table() {
  auto handler_obj =
      handler_factory_->create_object_handler(this, auth_manager_);
  auto handler_meta =
      handler_factory_->create_object_metadata_handler(this, auth_manager_);

  handle_object_ = std::move(handler_obj);
  handle_metadata_ = std::move(handler_meta);
}

void Object::handlers_for_sp() {
  auto handler_obj = handler_factory_->create_sp_handler(this, auth_manager_);
  auto handler_meta =
      handler_factory_->create_object_metadata_handler(this, auth_manager_);

  handle_object_ = std::move(handler_obj);
  handle_metadata_ = std::move(handler_meta);
}

bool Object::update(const void *pv, RouteSchemaPtr schema) {
  auto &pe = *reinterpret_cast<const EntryDbObject *>(pv);
  bool result = false;
  if (schema != schema_) {
    if (schema_) schema_->route_unregister(this);
    if (schema) schema->route_register(this);
    schema_ = schema;
    result = true;
  }

  if ((pe_.service_path != pe.service_path) ||
      (pe_.schema_path != pe.schema_path) ||
      (pe_.object_path != pe.object_path))
    result = true;

  pe_ = pe;
  cached_columns_.clear();
  update_variables();

  return result;
}

const std::string &Object::get_rest_canonical_url() {
  return url_rest_canonical_;
}

const std::string &Object::get_rest_url() { return url_route_; }

const std::string &Object::get_json_description() { return json_description_; }

const std::vector<std::string> Object::get_rest_path() { return {rest_path_}; }

const std::string &Object::get_rest_canonical_path() {
  return rest_canonical_path_;
}

void Object::update_variables() {
  const static std::string k_metadata = "/metadata-catalog";
  rest_path_ = "^" + pe_.service_path + pe_.schema_path + pe_.object_path +
               "(/[0-9]*/?)?$";
  rest_canonical_path_ = "^" + pe_.service_path + pe_.schema_path + k_metadata +
                         pe_.object_path + "/?$";
  rest_path_raw_ = pe_.service_path + pe_.schema_path + pe_.object_path;
  schema_name_ = extract_first_slash(pe_.db_schema);
  object_name_ = extract_first_slash(pe_.db_table);

  url_route_ = (is_ssl_ ? "https://" : "http://") + pe_.host + rest_path_raw_;
  url_rest_canonical_ = (is_ssl_ ? "https://" : "http://") + pe_.host +
                        pe_.service_path + pe_.schema_path + k_metadata +
                        pe_.object_path;
  json_description_ = "{\"name\":\"" + pe_.object_path +
                      "\", \"links\":[{\"rel\":\"describes\", \"href\": \"" +
                      get_rest_url() +
                      "\"},{\"rel\":\"canonical\", \"href\": \"" +
                      get_rest_canonical_url() + "\"}]}";

  using Op = database::entry::Operation::Values;
  static_assert(static_cast<int>(Op::valueCreate) == kCreate);
  static_assert(static_cast<int>(Op::valueRead) == kRead);
  static_assert(static_cast<int>(Op::valueUpdate) == kUpdate);
  static_assert(static_cast<int>(Op::valueDelete) == kDelete);

  access_flags_ = pe_.operation;
}

void Object::cache_columns() {
  cached_primary_column_ = {};
  auto table_columns = query_factory_->create_query_table_columns();
  auto session = cache_->get_instance(collector::kMySQLConnectionUserdata);
  table_columns->query_entries(session.get(), schema_name_, object_name_);

  cached_columns_ = table_columns->columns;

  std::vector<std::string_view> internal_columns;

  if (get_user_row_ownership().user_ownership_enforced)
    internal_columns.emplace_back(
        get_user_row_ownership().user_ownership_column);

  for (auto &group : get_group_row_ownership()) {
    internal_columns.emplace_back(group.row_group_ownership_column);
  }

  container::remove_if(cached_columns_, [&internal_columns](const Column &c) {
    if (c.is_primary) return false;
    return container::any_of(internal_columns, c.name);
  });

  for (auto &column : cached_columns_) {
    if (column.is_primary) {
      cached_primary_column_ = column;
      break;
    }
  }
}

const std::vector<Column> &Object::get_cached_columnes() {
  if (cached_columns_.empty()) {
    cache_columns();
  }

  return cached_columns_;
}

Object::RouteSchema *Object::get_schema() { return schema_.get(); }

const std::string &Object::get_object_path() { return pe_.object_path; }

const std::string &Object::get_object_name() { return object_name_; }
const std::string &Object::get_schema_name() { return schema_name_; }

const std::string &Object::get_options() { return pe_.options_json; }

bool Object::requires_authentication() const {
  return pe_.requires_authentication || pe_.schema_requires_authentication;
}

UniversalId Object::get_id() const { return pe_.id; }

UniversalId Object::get_service_id() const { return pe_.service_id; }

bool Object::has_access(const Access access) const {
  return access & pe_.operation;
}

const Object::Column &Object::get_cached_primary() {
  if (cached_columns_.empty()) {
    cache_columns();
  }

  return cached_primary_column_;
}

uint32_t Object::get_on_page() { return pe_.on_page; }

Object::Format Object::get_format() const {
  static_assert(static_cast<int>(EntryDbObject::formatFeed) == kFeed);
  static_assert(static_cast<int>(EntryDbObject::formatItem) == kItem);
  static_assert(static_cast<int>(EntryDbObject::formatMedia) == kMedia);

  return static_cast<Object::Format>(pe_.format);
}

Object::Media Object::get_media_type() const {
  return {!pe_.media_type.has_value() && pe_.autodetect_media_type,
          pe_.media_type};
}

collector::MysqlCacheManager *Object::get_cache() { return cache_; }

const std::string &Object::get_rest_path_raw() { return rest_path_raw_; }

std::string Object::extract_first_slash(const std::string &value) {
  if (value.length()) {
    if (value[0] == '/') return value.substr(1);
  }

  return value;
}

uint32_t Object::get_access() const { return access_flags_; }

const Object::RowUserOwnership &Object::get_user_row_ownership() const {
  return pe_.row_security;
}

const Object::VectorOfRowGroupOwnership &Object::get_group_row_ownership()
    const {
  return pe_.row_group_security;
}

const mrs::interface::Object::Fields &Object::get_parameters() {
  return pe_.fields;
}

}  // namespace mrs
