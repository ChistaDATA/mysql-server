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

#include "mrs/database/query_changes_db_object.h"

#include <utility>
#include "helper/mysql_row.h"

#include "mrs/database/query_entries_audit_log.h"
#include "mrs/database/query_entry_fields.h"
#include "mrs/database/query_entry_group_row_security.h"

#include "mysql/harness/logging/logging.h"

IMPORT_LOG_FUNCTIONS()

namespace mrs {
namespace database {

const std::string kObjTableName = "object";
const std::string kObjRefTableName = "object_reference";
const std::string kObjFieldTableName = "object_field";

QueryChangesDbObject::QueryChangesDbObject(const uint64_t last_audit_id) {
  audit_log_id_ = last_audit_id;
  query_length_ = query_.str().length();
}

void QueryChangesDbObject::query_entries(MySQLSession *session) {
  path_entries_fetched.clear();
  query(session, "START TRANSACTION");

  QueryAuditLogEntries audit_entries;
  VectorOfPathEntries local_path_entries;
  uint64_t max_audit_log_id = audit_log_id_;
  audit_entries.query_entries(
      session,
      {"service", "db_schema", "db_object", "url_host", kObjTableName,
       kObjRefTableName, kObjFieldTableName},
      audit_log_id_);

  for (const auto &audit_entry : audit_entries.entries) {
    if (audit_entry.old_table_id.has_value())
      query_path_entries(session, &local_path_entries, audit_entry.table,
                         audit_entry.old_table_id.value());

    if (audit_entry.new_table_id.has_value())
      query_path_entries(session, &local_path_entries, audit_entry.table,
                         audit_entry.new_table_id.value());

    if (max_audit_log_id < audit_entry.id) max_audit_log_id = audit_entry.id;
  }

  QueryEntryGroupRowSecurity qg;
  QueryEntryFields qp;
  for (auto &e : local_path_entries) {
    qg.query_group_row_security(session, e.id);
    e.row_group_security = std::move(qg.get_result());
    qp.query_parameters(session, e.id);
    auto &r = qp.get_result();
    e.fields = std::move(r);
  }

  entries.swap(local_path_entries);

  query(session, "COMMIT");

  audit_log_id_ = max_audit_log_id;
}

void QueryChangesDbObject::query_path_entries(MySQLSession *session,
                                              VectorOfPathEntries *out,
                                              const std::string &table_name,
                                              const entry::UniversalId &id) {
  entries.clear();
  log_debug("Checking audit-log entry for table:%s, id:%s", table_name.c_str(),
            id.to_string().c_str());

  query(session, build_query(table_name, id));

  for (const auto &entry : entries) {
    if (path_entries_fetched.count(entry.id)) continue;

    out->push_back(entry);
    path_entries_fetched.insert(entry.id);
  }

  if (entries.empty() && table_name == "db_object") {
    DbObject pe;
    pe.id = id;
    pe.deleted = true;
    path_entries_fetched.insert(id);
    out->push_back(pe);
  }
}

std::string QueryChangesDbObject::build_query(const std::string &table_name,
                                              const entry::UniversalId &id) {
  mysqlrouter::sqlstring query = query_;

  if (kObjTableName == table_name) {
    mysqlrouter::sqlstring where =
        " WHERE id in (select db_object_id from "
        "mysql_rest_service_metadata.object as f where f.id=? GROUP BY "
        "db_object_id)";
    where << id;
    query << mysqlrouter::sqlstring{""};
    return query.str() + where.str();
  } else if (kObjRefTableName == table_name) {
    mysqlrouter::sqlstring where =
        " WHERE id in (SELECT o.db_object_id FROM "
        "mysql_rest_service_metadata.object_field AS f JOIN "
        "mysql_rest_service_metadata.object AS o ON o.id=f.object_id WHERE "
        "(f.parent_reference_id=? or f.represents_reference_id=?) GROUP BY "
        "db_object_id)";
    where << id << id;
    query << mysqlrouter::sqlstring{""};
    return query.str() + where.str();
  } else if (kObjFieldTableName == table_name) {
    mysqlrouter::sqlstring where =
        " WHERE id in (SELECT o.db_object_id FROM "
        "mysql_rest_service_metadata.object_field AS f  JOIN "
        "mysql_rest_service_metadata.object AS o ON o.id=f.object_id WHERE "
        "f.id=? GROUP BY db_object_id)";
    where << id;
    query << mysqlrouter::sqlstring{""};
    return query.str() + where.str();
  }

  mysqlrouter::sqlstring where = " WHERE !=? ";
  where << (table_name + "_id") << id;
  query << mysqlrouter::sqlstring{""};

  return query.str() + where.str();
}

}  // namespace database
}  // namespace mrs
