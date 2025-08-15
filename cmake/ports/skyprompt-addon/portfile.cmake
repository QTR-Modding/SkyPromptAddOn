# header-only library
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO QTR-Modding/SkyPromptAddOn
    REF 0
    SHA512 0
    HEAD_REF main
)

# Install codes
set(SkyPromptAddOn_SOURCE	${SOURCE_PATH}/include/SkyPrompt)
file(INSTALL ${SkyPromptAddOn_SOURCE} DESTINATION ${CURRENT_PACKAGES_DIR}/include)
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")