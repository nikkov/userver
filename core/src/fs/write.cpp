#include <userver/fs/write.hpp>

#include <fmt/format.h>

#include <boost/filesystem/operations.hpp>

#include <userver/engine/async.hpp>
#include <userver/fs/blocking/write.hpp>

USERVER_NAMESPACE_BEGIN

namespace fs {

void CreateDirectories(engine::TaskProcessor& async_tp, std::string_view path, boost::filesystem::perms perms) {
    engine::AsyncNoSpan(async_tp, [path, perms] { fs::blocking::CreateDirectories(path, perms); }).Get();
}

void CreateDirectories(engine::TaskProcessor& async_tp, std::string_view path) {
    engine::AsyncNoSpan(async_tp, [path] { fs::blocking::CreateDirectories(path); }).Get();
}

void RewriteFileContents(engine::TaskProcessor& async_tp, const std::string& path, std::string_view contents) {
    engine::AsyncNoSpan(async_tp, &fs::blocking::RewriteFileContents, path, contents).Get();
}

void SyncDirectoryContents(engine::TaskProcessor& async_tp, const std::string& path) {
    engine::AsyncNoSpan(async_tp, &fs::blocking::SyncDirectoryContents, path).Get();
}

void Rename(engine::TaskProcessor& async_tp, const std::string& source, const std::string& destination) {
    engine::AsyncNoSpan(async_tp, &fs::blocking::Rename, source, destination).Get();
}

void Chmod(engine::TaskProcessor& async_tp, const std::string& path, boost::filesystem::perms perms) {
    engine::AsyncNoSpan(async_tp, &fs::blocking::Chmod, path, perms).Get();
}

void RewriteFileContentsAtomically(
    engine::TaskProcessor& async_tp,
    const std::string& path,
    std::string_view contents,
    boost::filesystem::perms perms
) {
    engine::AsyncNoSpan(async_tp, &fs::blocking::RewriteFileContentsAtomically, path, contents, perms).Get();
}

bool RemoveSingleFile(engine::TaskProcessor& async_tp, const std::string& path) {
    return engine::AsyncNoSpan(async_tp, &fs::blocking::RemoveSingleFile, path).Get();
}

}  // namespace fs

USERVER_NAMESPACE_END
