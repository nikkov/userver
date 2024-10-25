#pragma once

/// @file userver/ugrpc/field_mask.hpp
/// @brief @copybrief ugrpc::FieldMask

#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <userver/utils/fast_pimpl.hpp>
#include <userver/utils/impl/projecting_view.hpp>
#include <userver/utils/impl/transparent_hash.hpp>
#include <userver/utils/optional_ref.hpp>
#include <userver/utils/str_icase.hpp>

namespace google::protobuf {

class Descriptor;
class FieldMask;
class Message;

}  // namespace google::protobuf

USERVER_NAMESPACE_BEGIN

namespace ugrpc {

/// @brief Utilities to process the field masks. Respects Google's AIP-161: https://google.aip.dev/161.
///
/// 1. An empty mask is treated as a mask with all fields.
/// 2. Map masks (i.e. `reviews` and `reviews.smith` for `map<string, string> reviews`).
/// 3. Wildcard masks for repeated and map fields (i.e. `authors`, `authors.*`, `authors.*.name`).
/// 4. Backticks (`) are separation charaters for problematic keys and may not appear in paths.
class FieldMask {
public:
    class BadPathError : public std::runtime_error {
    public:
        explicit BadPathError(const std::string& msg) : std::runtime_error(msg) {}
    };

    /// @brief Construct an empty field-mask
    FieldMask() = default;

    FieldMask(FieldMask&&) = default;
    FieldMask(const FieldMask&) = default;

    FieldMask& operator=(FieldMask& other) = default;
    FieldMask& operator=(FieldMask&& other) = default;

    /// @brief Constructs the field-mask from a raw gRPC field-mask
    explicit FieldMask(const google::protobuf::FieldMask& field_mask);

    /// @brief Adds a dot-separated path to the field mask.
    /// Backtick (`) is treated as a separation character according to AIP-161
    /// and may not appear in the path.
    ///
    /// @throws BadPathError if the path is malformed.
    /// In this case, the state of the field mask is undefined.
    /// You must not continue using the instance after encountering the exception.
    void AddPath(std::string_view path);

    /// @brief Converts the field-mask back to a google field-mask
    google::protobuf::FieldMask ToGoogleMask() const;

    /// @brief Check if the field mask is valid for this message.
    ///
    /// @throws BadPathError if the field mask contains invalid paths.
    void CheckValidity(const google::protobuf::Descriptor* descriptor) const;

    /// @brief Does this field-mask fully contain the given path.
    /// @throws BadPathError if the path is malformed.
    bool IsPathFullyIn(std::string_view path) const;

    /// @brief Does this field-mask contain the given path or any of its child paths.
    /// @throws BadPathError if the path is malformed.
    bool IsPathPartiallyIn(std::string_view path) const;

    /// @brief Remove all fields not present in the field-mask from the message.
    /// The mask must be valid for this to work. Use @ref IsValid to check.
    ///
    /// @throws BadPathError if the field mask contains invalid paths.
    ///
    /// @warning This causes a segmentation fault for messages that contain
    /// optional fields in protobuf versions prior to 3.13.
    /// See https://github.com/protocolbuffers/protobuf/issues/7801
    void Trim(google::protobuf::Message& message) const;

    /// @brief Same as @ref Trim but does not perform pre-validation of the mask.
    /// You should not catch any errors generated by this method.
    /// Use this only if you are absolutely sure the mask is valid.
    ///
    /// @warning This causes a segmentation fault for messages that contain
    /// optional fields in protobuf versions prior to 3.13.
    /// See https://github.com/protocolbuffers/protobuf/issues/7801
    void TrimNoValidate(google::protobuf::Message& message) const;

    /// @brief Checks if there are any nested masks inside this mask
    bool IsLeaf() const;

    /// @brief Gets the names of all masked fields inside this mask.
    /// Returns an std::ranges::transform_view containing std::string_view.
    auto GetFieldNames() const {
        return utils::impl::ProjectingView(*children_, [](const auto& e) -> std::string_view { return e.first; });
    }

    /// @brief Gets the names of all masked fields inside this mask. Returns an std::vector.
    std::vector<std::string_view> GetFieldNamesList() const;

    /// @brief Checks if the specified field is in the mask
    bool HasFieldName(std::string_view field) const;

    /// @brief Gets the nested mask or returns nullopt if the field is not in the mask
    utils::OptionalRef<const FieldMask> GetMaskForField(std::string_view field) const;

private:
    void ToGoogleMaskImpl(std::vector<std::string>& stack, google::protobuf::FieldMask& out) const;

    utils::FastPimpl<utils::impl::TransparentMap<std::string, FieldMask, utils::StrCaseHash>, 96, 8> children_;
    bool is_leaf_{false};
};

}  // namespace ugrpc

USERVER_NAMESPACE_END
