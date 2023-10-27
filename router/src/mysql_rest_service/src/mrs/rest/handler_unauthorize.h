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

#ifndef ROUTER_SRC_REST_MRS_SRC_MRS_REST_HANDLER_UNAUTHORIZED_H_
#define ROUTER_SRC_REST_MRS_SRC_MRS_REST_HANDLER_UNAUTHORIZED_H_

#include <optional>
#include <string>
#include <vector>

#include "helper/media_type.h"
#include "mrs/interface/authorize_manager.h"
#include "mrs/rest/handler.h"

namespace mrs {
namespace rest {

class HandlerUnauthorize : public Handler {
 public:
  HandlerUnauthorize(const UniversalId service_id, const std::string &url,
                     const std::string &rest_path_matcher,
                     const std::string &options,
                     interface::AuthorizeManager *auth_manager);

  bool may_check_access() const override;
  Authorization requires_authentication() const override;
  UniversalId get_service_id() const override;
  UniversalId get_db_object_id() const override;
  UniversalId get_schema_id() const override;
  uint32_t get_access_rights() const override;

  bool request_begin(RequestContext *ctxt) override;
  void request_end(RequestContext *ctxt) override;
  bool request_error(RequestContext *ctxt, const http::Error &e) override;

  HttpResult handle_get(RequestContext *ctxt) override;
  HttpResult handle_post(RequestContext *ctxt,
                         const std::vector<uint8_t> &document) override;
  HttpResult handle_delete(RequestContext *ctxt) override;
  HttpResult handle_put(RequestContext *ctxt) override;

 private:
  std::string append_status_parameters(const std::string &redirection_url,
                                       const http::Error &error);

  UniversalId service_id_;
  std::string copy_url_;
  std::string copy_path_;
  interface::AuthorizeManager *auth_manager_;
};

}  // namespace rest
}  // namespace mrs

#endif  // ROUTER_SRC_REST_MRS_SRC_MRS_REST_HANDLER_UNAUTHORIZED_H_
