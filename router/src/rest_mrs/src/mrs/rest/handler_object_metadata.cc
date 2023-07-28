/*
  Copyright (c) 2021, 2022, Oracle and/or its affiliates.

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

#include "mrs/rest/handler_object_metadata.h"

#ifdef RAPIDJSON_NO_SIZETYPEDEFINE
#include "my_rapidjson_size_t.h"
#endif

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#include "mrs/rest/request_context.h"

namespace mrs {
namespace rest {

using Result = Handler::Result;

HandlerMetadata::HandlerMetadata(Route *route,
                                 mrs::interface::AuthorizeManager *auth_manager)
    : Handler(route->get_rest_canonical_url(),
              {route->get_rest_canonical_path()}, route->get_options(),
              auth_manager),
      route_{route} {}

// TODO(lkotula): remove or finish (Shouldn't be in review)
// class JsonDocument {
// public:
//  using Document = rapidjson::Document;
//
// public:
// private:
//  Document json_doc_{rapidjson::kObjectType};
//  Document::AllocatorType &allocator = json_doc_.GetAllocator();
//};
void HandlerMetadata::authorization(rest::RequestContext *ctxt) {
  throw_unauthorize_when_check_auth_fails(ctxt);
}

Result HandlerMetadata::handle_get(rest::RequestContext *) {
  rapidjson::Document json_doc;
  {
    rapidjson::Document::AllocatorType &allocator = json_doc.GetAllocator();
    rapidjson::Value primary_key(rapidjson::kArrayType);
    rapidjson::Value links(rapidjson::kArrayType);
    rapidjson::Value members(rapidjson::kArrayType);
    auto &columns = route_->get_cached_columnes();
    const std::string *primary_column{nullptr};

    for (auto &c : columns) {
      rapidjson::Value json_column(rapidjson::kObjectType);
      json_column.AddMember("name", rapidjson::Value(c.name.c_str(), allocator),
                            allocator);
      json_column.AddMember(
          "type",
          rapidjson::Value(helper::to_string(c.type_json).c_str(), allocator),
          allocator);
      members.PushBack(json_column, allocator);

      if (c.is_primary) {
        primary_column = &c.name;
      }
    }

    // TODO(lkotula): Use JsonSerializer from helpers (Shouldn't be in review)
    rapidjson::Value json_link_coll(rapidjson::kObjectType);
    rapidjson::Value json_link_can(rapidjson::kObjectType);
    rapidjson::Value json_link_desc(rapidjson::kObjectType);

    json_link_coll.AddMember("rel", "collection", allocator);
    json_link_coll.AddMember(
        "href",
        rapidjson::Value(route_->get_schema()->get_url().c_str(), allocator),
        allocator);
    json_link_coll.AddMember("mediaType", "application/json", allocator);

    json_link_can.AddMember("rel", "canonical", allocator);
    json_link_can.AddMember(
        "href",
        rapidjson::Value(route_->get_rest_canonical_url().c_str(), allocator),
        allocator);

    json_link_desc.AddMember("rel", "describes", allocator);
    json_link_desc.AddMember(
        "href", rapidjson::Value(route_->get_rest_url().c_str(), allocator),
        allocator);

    links.PushBack(json_link_coll, allocator);
    links.PushBack(json_link_can, allocator);
    links.PushBack(json_link_desc, allocator);

    if (primary_column)
      primary_key.PushBack(rapidjson::Value(primary_column->c_str(), allocator),
                           allocator);

    json_doc.SetObject()
        .AddMember(
            "name",
            rapidjson::Value(route_->get_object_path().c_str(), allocator),
            allocator)
        .AddMember("primaryKey", primary_key, allocator)
        .AddMember("members", members, allocator)
        .AddMember("links", links, allocator);
  }
  rapidjson::StringBuffer json_buf;
  {
    rapidjson::Writer<rapidjson::StringBuffer> json_writer(json_buf);

    json_doc.Accept(json_writer);
  }

  return std::string(json_buf.GetString(), json_buf.GetLength());
}

Result HandlerMetadata::handle_post(
    [[maybe_unused]] rest::RequestContext *ctxt,
    [[maybe_unused]] const std::vector<uint8_t> &document) {
  throw http::Error(HttpStatusCode::Forbidden);
}

Result HandlerMetadata::handle_delete([
    [maybe_unused]] rest::RequestContext *ctxt) {
  throw http::Error(HttpStatusCode::Forbidden);
}

Result HandlerMetadata::handle_put([
    [maybe_unused]] rest::RequestContext *ctxt) {
  throw http::Error(HttpStatusCode::Forbidden);
}

Handler::Authorization HandlerMetadata::requires_authentication() const {
  return (route_->requires_authentication() ||
          route_->get_schema()->requires_authentication())
             ? Authorization::kCheck
             : Authorization::kNotNeeded;
}

UniversalId HandlerMetadata::get_service_id() const {
  return route_->get_service_id();
}

UniversalId HandlerMetadata::get_db_object_id() const {
  return route_->get_id();
}

UniversalId HandlerMetadata::get_schema_id() const {
  return route_->get_schema()->get_id();
}

uint32_t HandlerMetadata::get_access_rights() const { return Route::kRead; }

}  // namespace rest
}  // namespace mrs
