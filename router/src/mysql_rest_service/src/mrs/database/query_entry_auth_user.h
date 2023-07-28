/*
  Copyright (c) 2022, 2023, Oracle and/or its affiliates.

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

#ifndef ROUTER_SRC_REST_MRS_SRC_MRS_DATABASE_QUERY_ENTRY_AUTH_USER_H_
#define ROUTER_SRC_REST_MRS_SRC_MRS_DATABASE_QUERY_ENTRY_AUTH_USER_H_

#include "mrs/database/entry/auth_user.h"
#include "mrs/database/helper/query.h"

namespace mrs {
namespace database {

class QueryEntryAuthUser : private QueryLog {
 public:
  using AuthUser = entry::AuthUser;
  using UserId = AuthUser::UserId;
  using UniversalId = entry::UniversalId;

 public:
  virtual bool query_user(MySQLSession *session, const AuthUser *user);
  virtual AuthUser::UserId insert_user(
      MySQLSession *session, const AuthUser *user,
      const helper::Optional<UniversalId> &default_role_id);
  virtual bool update_user(MySQLSession *session, const AuthUser *user);

  virtual const AuthUser &get_user();

 private:
  void on_row(const ResultRow &r) override;

  int loaded_{0};
  AuthUser user_data_;
};

}  // namespace database
}  // namespace mrs

#endif  // ROUTER_SRC_REST_MRS_SRC_MRS_DATABASE_QUERY_ENTRY_AUTH_USER_H_
