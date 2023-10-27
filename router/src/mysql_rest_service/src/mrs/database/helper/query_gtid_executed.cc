/* Copyright (c) 2023, Oracle and/or its affiliates.

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
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "mrs/database/helper/query_gtid_executed.h"

#include "mrs/database/helper/query_faults.h"

#include "mysql/harness/logging/logging.h"

IMPORT_LOG_FUNCTIONS()

namespace mrs {
namespace database {

static bool expect_single_bool_value(
    std::unique_ptr<mysqlrouter::MySQLSession::ResultRow> row,
    const bool default_value = false) {
  if (row->size()) {
    return atoi((*row)[0]) > 0;
  }

  return default_value;
}

bool wait_gtid_executed(mysqlrouter::MySQLSession *session,
                        const mysqlrouter::sqlstring &gtid, uint64_t timeout) {
  mysqlrouter::sqlstring check_gtid{
      "SELECT 0=WAIT_FOR_EXECUTED_GTID_SET(?, ?)"};
  check_gtid << gtid << timeout;
  log_debug("query: %s", check_gtid.str().c_str());
  return expect_single_bool_value(session->query_one(check_gtid));
}

bool is_gtid_executed(mysqlrouter::MySQLSession *session,
                      const mysqlrouter::sqlstring &gtid) {
  mysqlrouter::sqlstring check_gtid{
      "SELECT GTID_SUBSET(?, @@GLOBAL.gtid_executed)"};
  check_gtid << gtid;
  log_debug("query: %s", check_gtid.str().c_str());
  return expect_single_bool_value(session->query_one(check_gtid));
}

void throw_if_not_gtid_executed(mysqlrouter::MySQLSession *session,
                                const mysqlrouter::sqlstring &gtid) {
  if (!is_gtid_executed(session, gtid)) {
    throw_rest_error_asof_timeout(gtid.str());
  }
}

}  // namespace database
}  // namespace mrs
