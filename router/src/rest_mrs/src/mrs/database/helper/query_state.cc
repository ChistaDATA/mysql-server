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

#include "mrs/state.h"

#include "mrs/database/helper/query_state.h"

namespace mrs {
namespace database {

void QueryState::query_state(MySQLSession *session) {
  query_ = "SELECT service_enabled FROM mysql_rest_service_metadata.config;";
  execute(session);
}

void QueryState::on_row(const Row &r) {
  if (r.size() < 1) return;

  auto state_new = atoi(r[0]) ? stateOn : stateOff;

  if (state_ != state_new) {
    changed_ = true;
    state_ = state_new;
  }
}

bool QueryState::was_changed() const { return changed_; }

State QueryState::get_state() {
  changed_ = false;
  return state_;
}

}  // namespace database
}  // namespace mrs
