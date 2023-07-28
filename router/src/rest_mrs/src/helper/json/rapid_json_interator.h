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

#ifndef ROUTER_SRC_REST_MRS_SRC_HELPER_JSON_RAPID_JSON_INTERATOR_H_
#define ROUTER_SRC_REST_MRS_SRC_HELPER_JSON_RAPID_JSON_INTERATOR_H_

#include <utility>

#ifdef RAPIDJSON_NO_SIZETYPEDEFINE
#include "my_rapidjson_size_t.h"
#endif

#include <rapidjson/document.h>

namespace helper {

namespace json {

class Iterable {
 public:
  using Object = rapidjson::Document::ConstObject;
  using Value = rapidjson::Document::ValueType;
  using MemberIterator = Object::MemberIterator;
  using Pair = std::pair<const char *, const Value *>;

  class It {
   public:
    It(MemberIterator it) : it_{it} {}

    It &operator++() {
      ++it_;
      return *this;
    }
    auto operator*() { return Pair(it_->name.GetString(), &it_->value); }
    bool operator!=(const It &other) { return it_ != other.it_; }

   private:
    MemberIterator it_;
  };

 public:
  Iterable(Object &object) : obj_{object} {}

  It begin() { return It{obj_.MemberBegin()}; }
  It end() { return It{obj_.MemberEnd()}; }

  Object &obj_;
};

}  // namespace json

}  // namespace helper

#endif  // ROUTER_SRC_REST_MRS_SRC_HELPER_JSON_RAPID_JSON_INTERATOR_H_
