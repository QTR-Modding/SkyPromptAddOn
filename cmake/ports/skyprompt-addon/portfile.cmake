# header-only library
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO QTR-Modding/SkyPromptAddOn
    REF eb385bace4ac84f62eb01d8c13f43ca3f4b6b428
    SHA512 6f6a537142ad695a9450bff24bca3ccb4fc45857674e0a5ac72c14dce66743d7feaa99c228c85cfcc15ebb7c9e87243a5a221bb233731c464b51fcef6bb00314
    HEAD_REF main
)

# Install codes
set(SkyPromptAddOn_SOURCE	${SOURCE_PATH}/include/SkyPrompt)
file(INSTALL ${SkyPromptAddOn_SOURCE} DESTINATION ${CURRENT_PACKAGES_DIR}/include)
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
