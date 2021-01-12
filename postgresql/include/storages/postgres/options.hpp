#pragma once

/// @file storages/postgres/options.hpp
/// @brief Options

#include <chrono>
#include <iosfwd>
#include <optional>
#include <string>
#include <unordered_map>

#include <storages/postgres/postgres_fwd.hpp>

namespace storages::postgres {

/*! [Isolation levels] */
/// @brief SQL transaction isolation level
/// @see https://www.postgresql.org/docs/current/static/sql-set-transaction.html
enum class IsolationLevel {
  kReadCommitted,   //!< READ COMMITTED
  kRepeatableRead,  //!< REPEATABLE READ
  kSerializable,    //!< SERIALIZABLE
  kReadUncommitted  //!< READ UNCOMMITTED @warning In Postgres READ UNCOMMITTED
                    //!< is treated as READ COMMITTED
};
/*! [Isolation levels] */

std::ostream& operator<<(std::ostream&, IsolationLevel);

/// @brief PostgreSQL transaction options
///
/// A transaction can be started using all isolation levels and modes
/// supported by PostgreSQL server as specified in it's documentation.
///
/// Default isolation level is READ COMMITTED, default mode is READ WRITE.
/// @code
/// // Read-write read committed transaction.
/// TransactionOptions opts;
/// @endcode
///
/// Transaction class provides constants Transaction::RW, Transaction::RO and
/// Transaction::Deferrable for convenience.
///
/// Other variants can be created with TransactionOptions constructors
/// that are constexpr.
///
/// @see https://www.postgresql.org/docs/current/static/sql-set-transaction.html
struct TransactionOptions {
  /*! [Transaction modes] */
  enum Mode {
    kReadWrite = 0,
    kReadOnly = 1,
    kDeferrable = 3  //!< Deferrable transaction is read only
  };
  /*! [Transaction modes] */
  IsolationLevel isolation_level = IsolationLevel::kReadCommitted;
  Mode mode = kReadWrite;

  constexpr TransactionOptions() = default;
  constexpr explicit TransactionOptions(IsolationLevel lvl)
      : isolation_level{lvl} {}
  constexpr TransactionOptions(IsolationLevel lvl, Mode m)
      : isolation_level{lvl}, mode{m} {}
  constexpr explicit TransactionOptions(Mode m) : mode{m} {}

  bool IsReadOnly() const { return mode & kReadOnly; }

  /// The deferrable property has effect only if the transaction is also
  /// serializable and read only
  static constexpr TransactionOptions Deferrable() {
    return {IsolationLevel::kSerializable, kDeferrable};
  }
};

constexpr inline bool operator==(const TransactionOptions& lhs,
                                 const TransactionOptions& rhs) {
  return lhs.isolation_level == rhs.isolation_level && lhs.mode == rhs.mode;
}
const std::string& BeginStatement(const TransactionOptions&);

/// A structure to control timeouts for PosrgreSQL queries
///
/// There are two parameters, `execute` and `statement`.
///
/// `execute` parameter controls the overall time the driver spends executing a
/// query, that includes:
/// * connecting to PostgreSQL server, if there are no connections available and
///   connection pool still has space for new connections;
/// * waiting for a connection to become idle if there are no idle connections
///   and connection pool already has reached it's max size;
/// * preparing a statement if the statement is run for the first time on the
///   connection;
/// * binding parameters and executing the statement;
/// * waiting for the first results to arrive from the server. If the result set
///   is big, only time to the first data packet is taken into account.
///
/// `statement` is rather straightforward, it's the PostgreSQL server-side
/// parameter, and it controls the time the database backend can spend executing
/// a single statement. It is very costly to change the statement timeout
/// often, as it requires a roundtrip to the database to change the setting.
/// @see https://www.postgresql.org/docs/12/runtime-config-client.html
///
/// `execute` timeout should always be greater than the `statement` timeout!
///
/// In case of a timeout, either back-end or overall, the client gets an
/// exception and the driver tries to clean up the connection for further reuse.
struct CommandControl {
  /// Overall timeout for a command being executed
  TimeoutDuration execute;
  /// PostgreSQL server-side timeout
  TimeoutDuration statement;

  constexpr CommandControl() = default;

  constexpr CommandControl(TimeoutDuration execute, TimeoutDuration statement)
      : execute(execute), statement(statement) {}

  constexpr CommandControl WithExecuteTimeout(TimeoutDuration n) const
      noexcept {
    return {n, statement};
  }

  constexpr CommandControl WithStatementTimeout(TimeoutDuration s) const
      noexcept {
    return {execute, s};
  }

  bool operator==(const CommandControl& rhs) const {
    return execute == rhs.execute && statement == rhs.statement;
  }

  bool operator!=(const CommandControl& rhs) const { return !(*this == rhs); }
};

using OptionalCommandControl = std::optional<CommandControl>;

using CommandControlByMethodMap =
    std::unordered_map<std::string, CommandControl>;
using CommandControlByHandlerMap =
    std::unordered_map<std::string, CommandControlByMethodMap>;

OptionalCommandControl GetHandlerOptionalCommandControl(
    const CommandControlByHandlerMap& map, const std::string& path,
    const std::string& method);

struct TopologySettings {
  std::chrono::milliseconds max_replication_lag{0};
};

struct PoolSettings {
  size_t min_size = 0;
  size_t max_size = 0;
  size_t max_queue_size = 0;
  bool sync_start = false;
  std::string db_name{};
};

struct ConnectionSettings {
  enum PreparedStatementOptions {
    kNoPreparedStatements,
    kCachePreparedStatements,
  };
  PreparedStatementOptions prepared_statements = kCachePreparedStatements;
  std::optional<std::string> handlers_cmd_ctl_task_data_path_key{};
  std::optional<std::string> handlers_cmd_ctl_task_data_method_key{};
};

}  // namespace storages::postgres
