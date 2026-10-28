# The one place for version numbers. Building after an edit reconfigures on its own.
# Each pg_app_version entry gives its target a PG_AppVersion.h (APP_VERSION, APP_NAME) and a VERSIONINFO resource.

# Suite version, used for release packaging
set(PROJECT_GLOBAL_NUMBER 0.7.1)

#              target                  version   display name
pg_app_version(DisaFileManager         0.7.1     "Disa File Manager")
pg_app_version(DisaMapEditor           0.5.0     "Disa Map Editor")
pg_app_version(DisaModManager          0.3.7     "Disa Mod Manager")
pg_app_version(DisaDataEditor          0.1.0     "Disa Data Editor")
pg_app_version(DisaTx2Editor           0.1.0     "Disa TX2 Editor")
pg_app_version(DisaScriptTest          0.1.0     "Disa Script Test")

# j.dll. The Disa module uses it as install version and checks mods' min_supported_version against major.minor.patch
pg_app_version(DisaReverseEngineering  0.4.8.0   "Disa mod runtime")
