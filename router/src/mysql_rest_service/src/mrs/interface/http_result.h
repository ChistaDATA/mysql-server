/*
  Copyright (c) 2023, Oracle and/or its affiliates.

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

#ifndef ROUTER_SRC_REST_MRS_SRC_MRS_INTERFACE_HTTPRESULT_H_
#define ROUTER_SRC_REST_MRS_SRC_MRS_INTERFACE_HTTPRESULT_H_

#include <optional>
#include <string>

#include "helper/media_type.h"
#include "mysqlrouter/http_request.h"

namespace mrs {
namespace interface {

struct HttpResult {
  using Type = helper::MediaType;

  HttpResult() {}
  HttpResult(std::string &&r) : response{r} {}
  HttpResult(std::string &&r, Type t, std::string e = {})
      : response{r}, type{t}, etag{std::move(e)} {}
  HttpResult(HttpStatusCode::key_type s, std::string &&r, Type t,
             std::string e = {})
      : response{r}, status{s}, type{t}, etag{std::move(e)} {}
  HttpResult(const std::string &r, Type t, std::string e = {})
      : response{r}, type{t}, etag{std::move(e)} {}
  HttpResult(std::string &&r, std::string t, std::string e = {})
      : response{r}, type_text{t}, etag{std::move(e)} {}

  std::string response;
  HttpStatusCode::key_type status{HttpStatusCode::Ok};
  Type type{Type::typeDefault};
  std::optional<std::string> type_text;
  std::string etag;
};

}  // namespace interface

}  // namespace mrs

#endif  // ROUTER_SRC_REST_MRS_SRC_MRS_INTERFACE_HTTPRESULT_H_
