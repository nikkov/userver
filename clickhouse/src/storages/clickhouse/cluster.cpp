#include <userver/storages/clickhouse/cluster.hpp>

#include <userver/storages/clickhouse/settings.hpp>

#include <userver/components/component_config.hpp>
#include <userver/engine/task/task_with_result.hpp>
#include <userver/utils/async.hpp>

#include <fmt/format.h>

USERVER_NAMESPACE_BEGIN

namespace storages::clickhouse {

namespace {

size_t WrappingIncrement(std::atomic<size_t>& value, size_t mod) {
  // we don't actually care about order being broken once in 2^64 iterations
  return (value++) % mod;
}

}  // namespace

Cluster::Cluster(clients::dns::Resolver* resolver,
                 const ClickhouseSettings& settings,
                 const components::ComponentConfig& config) {
  const auto& endpoints = settings.endpoints;
  const auto& auth_settings = settings.auth_settings;

  std::vector<engine::TaskWithResult<Pool>> init_tasks;
  init_tasks.reserve(endpoints.size());
  for (const auto& endpoint : endpoints) {
    init_tasks.emplace_back(USERVER_NAMESPACE::utils::Async(
        fmt::format("create_pool_{}", endpoint.host),
        [resolver, &endpoint, &auth_settings, &config]() {
          return Pool{resolver, PoolSettings{config, endpoint, auth_settings}};
        }));
  }

  pools_.reserve(init_tasks.size());
  for (auto& init_task : init_tasks) {
    pools_.emplace_back(init_task.Get());
  }
}

Cluster::~Cluster() = default;

ExecutionResult Cluster::Execute(const Query& query) const {
  return Execute(OptionalCommandControl{}, query);
}

ExecutionResult Cluster::Execute(OptionalCommandControl optional_cc,
                                 const Query& query) const {
  return GetPool().Execute(optional_cc, query);
}

void Cluster::DoInsert(OptionalCommandControl optional_cc,
                       const InsertionRequest& request) const {
  GetPool().Insert(optional_cc, request);
}

const Pool& Cluster::GetPool() const {
  return pools_[WrappingIncrement(current_pool_ind_, pools_.size())];
}

formats::json::Value Cluster::GetStatistics() const {
  return pools_.back().GetStatistics();
}

}  // namespace storages::clickhouse

USERVER_NAMESPACE_END
