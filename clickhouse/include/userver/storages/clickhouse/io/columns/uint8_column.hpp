#pragma once

#include <userver/storages/clickhouse/io/columns/column_includes.hpp>

USERVER_NAMESPACE_BEGIN

namespace storages::clickhouse::io::columns {

class UInt8Column final : public ClickhouseColumn<UInt8Column> {
 public:
  using cpp_type = std::uint8_t;
  using container_type = std::vector<cpp_type>;

  UInt8Column(ColumnRef column);

  static ColumnRef Serialize(const container_type& from);
};

}  // namespace storages::clickhouse::io::columns

USERVER_NAMESPACE_END
