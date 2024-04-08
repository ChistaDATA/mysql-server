/*
  Copyright (c) 2015, 2024, Oracle and/or its affiliates.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2.0,
  as published by the Free Software Foundation.

  This program is designed to work with certain software (including
  but not limited to OpenSSL) that is licensed under separate terms,
  as designated in a particular file or component or in included license
  documentation.  The authors of MySQL hereby grant you an additional
  permission to link the program and your derivative works with the
  separately licensed software that they have either included with
  the program or referenced in the documentation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef ROUTING_DESTINATION_INCLUDED
#define ROUTING_DESTINATION_INCLUDED

#include <atomic>
#include <cstdint>
#include <list>
#include <mutex>
#include <optional>
#include <string>
#include <system_error>
#include <vector>

#include "my_compiler.h"  // MY_ATTRIBUTE
#include "mysql/harness/net_ts/io_context.h"
#include "mysqlrouter/destination.h"
#include "mysqlrouter/destination_nodes_state_notifier.h"
#include "mysqlrouter/routing.h"
#include "protocol/protocol.h"
#include "tcp_address.h"

namespace mysql_harness {
class PluginFuncEnv;
}

/** @class RouteDestination
 * @brief Manage destinations for a Connection Routing
 *
 * This class manages destinations which are used in Connection Routing.
 * A destination is usually a MySQL Server and is stored using the IP
 * or hostname together with the TCP port (defaulting to 3306 for classic
 * protocol or to 33060 for x protocol).
 *
 * RouteDestination is meant to be a base class and used to inherite and
 * create class which change the behavior. For example, the `get_next()`
 * method is usually changed to get the next server in the list.
 */
class RouteDestination : public DestinationNodesStateNotifier {
 public:
  using AddrVector = std::vector<mysql_harness::TCPAddress>;

  /** @brief Default constructor
   *
   * @param io_ctx context for IO operations
   * @param protocol Protocol for the destination, defaults to value returned
   *        by Protocol::get_default()
   */
  RouteDestination(net::io_context &io_ctx,
                   Protocol::Type protocol = Protocol::get_default())
      : io_ctx_(io_ctx), protocol_(protocol) {}

  /** @brief Destructor */
  virtual ~RouteDestination() = default;

  RouteDestination(const RouteDestination &other) = delete;
  RouteDestination(RouteDestination &&other) = delete;
  RouteDestination &operator=(const RouteDestination &other) = delete;
  RouteDestination &operator=(RouteDestination &&other) = delete;

  /** @brief Return our routing strategy
   */
  virtual routing::RoutingStrategy get_strategy() = 0;

  /** @brief Adds a destination
   *
   * Adds a destination using the given address and port number.
   *
   * @param dest destination address
   */
  virtual void add(const mysql_harness::TCPAddress dest);

  /** @overload */
  virtual void add(const std::string &address, uint16_t port);

  /** @brief Removes a destination
   *
   * Removes a destination using the given address and port number.
   *
   * @param address IP or name
   * @param port Port number
   */
  virtual void remove(const std::string &address, uint16_t port);

  /** @brief Gets destination based on address and port
   *
   * Gets destination base on given address and port and returns a pair
   * with the information.
   *
   * Raises std::out_of_range when the combination of address and port
   * is not in the list of destinations.
   *
   * This function can be used to check whether given destination is in
   * the list.
   *
   * @param address IP or name
   * @param port Port number
   * @return an instance of mysql_harness::TCPAddress
   */
  virtual mysql_harness::TCPAddress get(const std::string &address,
                                        uint16_t port);

  /** @brief Removes all destinations
   *
   * Removes all destinations from the list.
   */
  virtual void clear();

  /** @brief Gets the number of destinations
   *
   * Gets the number of destinations currently in the list.
   *
   * @return Number of destinations as size_t
   */
  size_t size() noexcept;

  /** @brief Returns whether there are destinations
   *
   * @return whether the destination is empty
   */
  virtual bool empty() const noexcept { return destinations_.empty(); }

  /** @brief Start the destination threads (if any)
   *
   * @param env pointer to the PluginFuncEnv object
   */
  virtual void start(const mysql_harness::PluginFuncEnv *env);

  AddrVector::iterator begin() { return destinations_.begin(); }

  AddrVector::const_iterator begin() const { return destinations_.begin(); }

  AddrVector::iterator end() { return destinations_.end(); }

  AddrVector::const_iterator end() const { return destinations_.end(); }

  virtual AddrVector get_destinations() const;

  /**
   * get destinations to connect() to.
   *
   * destinations are in order of preference.
   */
  virtual Destinations destinations() = 0;

  /**
   * refresh destinations.
   *
   * should be called after connecting to all destinations failed.
   *
   * @param dests previous destinations.
   *
   * @returns new destinations, if there are any.
   */
  virtual std::optional<Destinations> refresh_destinations(
      const Destinations &dests);

  /**
   * Trigger listening socket acceptors state handler based on the destination
   * type.
   */
  virtual void handle_sockets_acceptors() {}

 protected:
  /** @brief List of destinations */
  AddrVector destinations_;

  /** @brief Mutex for updating destinations and iterator */
  std::mutex mutex_update_;

  net::io_context &io_ctx_;

  /** @brief Protocol for the destination */
  Protocol::Type protocol_;
};

#endif  // ROUTING_DESTINATION_INCLUDED
