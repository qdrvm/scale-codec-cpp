/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Defines the Configurable class for managing custom configurations in
 * SCALE serialization.
 *
 * This class allows for optional configuration of encoding and decoding
 * streams. If custom configuration support is enabled, the class can store and
 * retrieve configuration objects dynamically.
 */

#pragma once

#include <scale/definitions.hpp>
#ifdef CUSTOM_CONFIG_ENABLED
#include <any>
#endif
#include <type_traits>
#include <typeindex>
#include <unordered_map>

namespace scale {

  /**
   * @concept MaybeConfig
   * @brief Concept ensuring a type is a class but not a union.
   */
  template <typename T>
  concept MaybeConfig = std::is_class_v<T> and not std::is_union_v<T>;

  /**
   * @class Configurable
   * @brief Base class providing optional configuration support for SCALE
   * serialization.
   *
   * This class enables the use of custom configurations when encoding or
   * decoding. If custom configuration support is disabled, it functions as a
   * no-op.
   */
  class Configurable {
   public:
    /// @brief Default constructor.
    Configurable() = default;
    /// @brief Default destructor.
    ~Configurable() = default;

#ifdef CUSTOM_CONFIG_ENABLED
    /**
     * @brief Constructs a Configurable instance with custom configurations.
     * @tparam ConfigTs Variadic template for configuration types.
     * @param configs Custom configuration objects.
     */
    template <typename... ConfigTs>
      requires(MaybeConfig<ConfigTs> and ...)
    explicit Configurable(const ConfigTs &...configs) {
      (addConfig(configs), ...);
    }
#else
    /**
     * @brief Constructs a Configurable instance when custom configurations are
     * disabled.
     * @tparam ConfigTs Variadic template for configuration types.
     * @param configs Custom configuration objects (unused).
     */
    template <typename... ConfigTs>
      requires(MaybeConfig<ConfigTs> and ...)
    explicit Configurable(const ConfigTs &...configs) {}
#endif

#ifdef CUSTOM_CONFIG_ENABLED
    /**
     * @brief Retrieves a stored configuration object.
     * @tparam T The type of the configuration object to retrieve.
     * @return Reference to the requested configuration object.
     * @throws std::runtime_error If the requested configuration type was not
     * provided.
     */
    template <typename T>
      requires MaybeConfig<T>
    const T &getConfig() const {
      const auto it = configs_.find(typeid(T));
      if (it == configs_.end()) {
        throw std::runtime_error(
            "Stream was not configured by such custom config type");
      }
      return std::any_cast<std::reference_wrapper<const T>>(it->second).get();
    }
#else
    /**
     * @brief Deleted method when custom configurations are disabled.
     * @tparam T The type of the configuration object to retrieve.
     */
    template <typename T>
    [[deprecated("Scale has compiled without custom config support")]]  //
    const T &
    getConfig() const = delete;
#endif

#ifdef CUSTOM_CONFIG_ENABLED
   private:
    /**
     * @brief Stores a configuration object.
     * @tparam ConfigT The type of the configuration object.
     * @param config The configuration object to store.
     * @throws std::runtime_error If a configuration of the same type is already
     * present.
     */
    template <typename ConfigT>
    void addConfig(const ConfigT &config) {
      auto [_, added] = configs_.emplace(typeid(ConfigT), std::cref(config));
      if (not added) {
        throw std::runtime_error(
            "Stream can be configured by different custom config types only");
      }
    }

    /// @brief Stores configuration objects indexed by type.
    std::unordered_map<std::type_index, std::any> configs_{};
#endif
  };

}  // namespace scale