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

#include "mrs/rest/handler_file.h"

#include <map>
#include <vector>

#include "mysql/harness/filesystem.h"
#include "mysql/harness/logging/logging.h"
#include "mysql/harness/string_utils.h"
#include "mysqlrouter/http_request.h"

#include "mrs/database/query_entry_content_file.h"
#include "mrs/database/query_factory.h"
#include "mrs/http/error.h"
#include "mrs/rest/request_context.h"

IMPORT_LOG_FUNCTIONS()

using MySQLSession = collector::MysqlCacheManager::Object;
using CachedObject = collector::MysqlCacheManager::CachedObject;
using Type = mrs::interface::RestHandler::HttpResult::Type;
using HttpResult = mrs::rest::HandlerFile::HttpResult;
using MysqlCacheManager = collector::MysqlCacheManager;
using MySQLConnection = collector::MySQLConnection;

static CachedObject get_session(
    MySQLSession session, MysqlCacheManager *cache_manager,
    MySQLConnection type = MySQLConnection::kMySQLConnectionMetadataRO) {
  if (session) return CachedObject(nullptr, session);

  return cache_manager->get_instance(type, false);
}

static Type get_result_type_from_extension(const std::string &ext) {
  static std::map<std::string, Type> map{
      {".gif", Type::typeGif},  {".jpg", Type::typeJpg},
      {".png", Type::typePng},  {".js", Type::typeJs},
      {".mjs", Type::typeJs},   {".html", Type::typeHtml},
      {".htm", Type::typeHtml}, {".css", Type::typeCss},
      {".svg", Type::typeSvg},  {".map", Type::typePlain}};

  log_debug("ext:'%s'", ext.c_str());
  auto i = map.find(ext);

  if (i == map.end()) return Type::typePlain;

  return i->second;
}

namespace mrs {
namespace rest {

HandlerFile::HandlerFile(Route *route,
                         mrs::interface::AuthorizeManager *auth_manager,
                         std::shared_ptr<QueryFactory> factory)
    : Handler(route->get_rest_url(), route->get_rest_path(),
              route->get_options(), auth_manager),
      route_{route},
      factory_{factory} {
  if (!factory_) {
    factory_ = std::make_shared<mrs::database::QueryFactory>();
  }
}

UniversalId HandlerFile::get_service_id() const {
  return route_->get_service_id();
}

UniversalId HandlerFile::get_db_object_id() const {
  // TODO(lkotula): id of file is not the same as db_object.
  // It should be considered to return (Shouldn't be in
  // review)
  return {};
}

UniversalId HandlerFile::get_schema_id() const {
  // TODO(lkotula): id of file is not the same as db_object.
  // It should be considered to return (Shouldn't be in
  // review)
  return {};
}

Handler::Authorization HandlerFile::requires_authentication() const {
  return route_->requires_authentication() ? Authorization::kRequires
                                           : Authorization::kNotNeeded;
}

bool HandlerFile::is_json_response() const { return false; }

uint32_t HandlerFile::get_access_rights() const { return Route::kRead; }

void HandlerFile::authorization(rest::RequestContext *ctxt) {
  throw_unauthorize_when_check_auth_fails(ctxt);
}

HttpResult HandlerFile::handle_get(rest::RequestContext *ctxt) {
  auto file = factory_->create_query_content_file();
  mysql_harness::Path path{route_->get_object_path()};
  auto if_not_matched = ctxt->request->get_input_headers().get("If-None-Match");

  if (if_not_matched && route_->get_version() == if_not_matched) {
    throw http::Error(HttpStatusCode::NotModified);
  }

  auto session = get_session(ctxt->sql_session_cache.get(), route_->get_cache(),
                             MySQLConnection::kMySQLConnectionMetadataRO);
  auto result_type = get_result_type_from_extension(
      mysql_harness::make_lower(path.extension()));

  if (nullptr == session.get())
    throw http::Error(HttpStatusCode::InternalError);

  file->query_file(session.get(), route_->get_id());

  return {std::move(file->result), result_type, route_->get_version()};
}

HttpResult HandlerFile::handle_delete(rest::RequestContext *) {
  throw http::Error(HttpStatusCode::NotImplemented);
}

HttpResult HandlerFile::handle_post(rest::RequestContext *,
                                    const std::vector<uint8_t> &) {
  throw http::Error(HttpStatusCode::NotImplemented);
}

HttpResult HandlerFile::handle_put(rest::RequestContext *) {
  throw http::Error(HttpStatusCode::NotImplemented);
}

}  // namespace rest
}  // namespace mrs
