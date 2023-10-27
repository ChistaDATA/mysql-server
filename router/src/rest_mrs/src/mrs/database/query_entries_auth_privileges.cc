/*
  Copyright (c) 2022, Oracle and/or its affiliates.

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

#include "mrs/database/query_entries_auth_privileges.h"
#include "helper/mysql_row.h"

namespace mrs {
namespace database {

void QueryEntriesAuthPrivileges::query_user(MySQLSession *session,
                                            uint64_t user_id,
                                            Privileges *out_privileges) {
  privileges_ = out_privileges;
  privileges_->clear();
  query_ =
      "SELECT p.service_id, p.db_schema_id, p.db_object_id, "
      "BIT_OR(p.crud_operations) as crud FROM "
      "mysql_rest_service_metadata.auth_privilege as p "
      "  WHERE p.auth_role_id in ( "
      "    WITH recursive cte As "
      "        ( "
      "      SELECT r.id AS id, r.derived_from_role_id FROM "
      "mysql_rest_service_metadata.auth_role r WHERE "
      "r.id IN (SELECT auth_role_id FROM "
      "mysql_rest_service_metadata.user_has_role WHERE auth_user_id=?) "
      "      UNION ALL "
      "      SELECT h.id AS id, h.derived_from_role_id "
      "        FROM mysql_rest_service_metadata.auth_role AS h "
      "        JOIN cte c ON c.derived_from_role_id=h.id "
      "        ) "
      "        SELECT id FROM cte) "
      " OR p.auth_role_id in ( SELECT auth_role_id FROM "
      "mysql_rest_service_metadata.user_group_has_role as ughr WHERE "
      "ughr.user_group_id in ( "
      "    WITH recursive cte_group_ids As "
      "        ( "
      "        SELECT user_group_id as id FROM "
      "          mysql_rest_service_metadata.user_has_group as uhg WHERE "
      "uhg.auth_user_id = ? "
      "        UNION ALL "
      "            SELECT h.user_group_id "
      "              FROM mysql_rest_service_metadata.user_group_hierarchy AS "
      "h "
      "              JOIN cte_group_ids c ON c.id=h.parent_group_id ) "
      "          SELECT id FROM cte_group_ids"
      "          )) "
      "GROUP BY p.service_id, p.db_schema_id, p.db_object_id";

  query_ << user_id << user_id;

  query(session);
}

void QueryEntriesAuthPrivileges::on_row(const Row &r) {
  helper::MySQLRow row{r};
  entry::AuthPrivilege entry;

  row.unserialize(&entry.service_id);
  row.unserialize(&entry.schema_id);
  row.unserialize(&entry.object_id);
  row.unserialize(&entry.crud);

  privileges_->push_back(entry);
}

}  // namespace database
}  // namespace mrs
