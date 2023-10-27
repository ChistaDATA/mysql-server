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

#ifndef ROUTER_SRC_REST_MRS_TESTS_MOCK_MOCK_ROUTE_SCHEMA_H_
#define ROUTER_SRC_REST_MRS_TESTS_MOCK_MOCK_ROUTE_SCHEMA_H_

#include "mrs/interface/route_schema.h"

class MockRouteSchema : public mrs::interface::RouteSchema {
 public:
  using State = mrs::State;
  using Route = mrs::interface::Route;

  MOCK_METHOD(void, turn, (const State state), (override));
  MOCK_METHOD(void, route_unregister, (Route * r), (override));
  MOCK_METHOD(void, route_register, (Route * r), (override));
  MOCK_METHOD(const std::string &, get_name, (), (const, override));
  MOCK_METHOD(const std::string &, get_url, (), (const, override));
  MOCK_METHOD(const std::string &, get_path, (), (const, override));
  MOCK_METHOD(const std::string &, get_options, (), (const, override));
  MOCK_METHOD(const std::string, get_full_path, (), (const, override));
  MOCK_METHOD(const VectorOfRoutes &, get_routes, (), (const, override));
  MOCK_METHOD(bool, requires_authentication, (), (const, override));
  MOCK_METHOD(uint64_t, get_service_id, (), (const, override));
  MOCK_METHOD(uint64_t, get_id, (), (const, override));

  MOCK_METHOD(void, destroy, (), ());
};

#endif  // ROUTER_SRC_REST_MRS_TESTS_MOCK_MOCK_ROUTE_SCHEMA_H_
