#include <userver/storages/clickhouse/io/columns/uint64_column.hpp>

#include <storages/clickhouse/io/columns/impl/numeric_column.hpp>

USERVER_NAMESPACE_BEGIN

namespace storages::clickhouse::io::columns {

namespace {
using NativeType = clickhouse::impl::clickhouse_cpp::ColumnUInt64;
}

UInt64Column::UInt64Column(ColumnRef column)
    : ClickhouseColumn{impl::GetTypedColumn<UInt64Column, NativeType>(column)} {
}

template <>
UInt64Column::cpp_type BaseIterator<UInt64Column>::DataHolder::Get() const {
  // We know the type, see ctor
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
  return static_cast<NativeType*>(column_.get())->At(ind_);
}

ColumnRef UInt64Column::Serialize(const container_type& from) {
  return impl::NumericColumn<UInt64Column>::Serialize(from);
}

}  // namespace storages::clickhouse::io::columns

USERVER_NAMESPACE_END
