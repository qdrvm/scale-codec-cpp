vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO qdrvm/qtils
  REF f5013e52027f6b3014c2ae902a94903583898639
  SHA512 6492edc93437b8edc254014b5face0521f0fd46fce6e6cab4e577fc00b6964b5930c75fd228fd8f3bb10bf57a6dd8722dcf2912a488b3f1174a99ed991ae4c54
)
vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "qtils")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
