# Template for a custom hunter configuration Useful when there is a need for a
# non-default version or arguments of a dependency, or when a project not
# registered in soramitsu-hunter should be added.
#
# hunter_config(
#     package-name
#     VERSION 0.0.0-package-version
#     CMAKE_ARGS
#      CMAKE_VARIABLE=value
# )
#
# hunter_config(
#     package-name
#     URL https://github.com/organization/repository/archive/hash.tar.gz
#     SHA1 1234567890abcdef1234567890abcdef12345678
#     CMAKE_ARGS
#       CMAKE_VARIABLE=value
# )

hunter_config(
    qtils
    URL  https://github.com/qdrvm/qtils/archive/f5013e52027f6b3014c2ae902a94903583898639.tar.gz
    SHA1 ebc7ae5215bada4316ebf2917cfd61bd4a398b39
)
