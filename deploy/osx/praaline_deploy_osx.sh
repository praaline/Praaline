#!/bin/sh

# DEFINITIONS
QT_BIN_DIRECTORY="/Users/george/Qt/5.11.2/clang_64/bin"
BUILD_DIRECTORY="/Users/george/Develop/build/Praaline-Desktop_Qt_5_11_2_clang_64bit-Release"
DEPLOY_DIRECTORY="/Users/george/Documents/praaline-mac"
declare -a PLUGINS=("libDisMo" "libProsogram" "libPromise")

# Collect the executable and all libraries in the deploy directory
cp -R $BUILD_DIRECTORY/app/build/Praaline.app $DEPLOY_DIRECTORY 
mkdir $DEPLOY_DIRECTORY/libs
cp $BUILD_DIRECTORY/pncore/build/release/libpncore.1.0.0.dylib $DEPLOY_DIRECTORY/libs/libpncore.1.dylib
cp $BUILD_DIRECTORY/dependencies/qtpropertybrowser/build/release/libqtpropertybrowser.1.0.0.dylib $DEPLOY_DIRECTORY/libs/libqtpropertybrowser.1.dylib
cp $BUILD_DIRECTORY/dependencies/qtilities/build/release/libQtilitiesProjectManagement-1.5.1.0.0.dylib $DEPLOY_DIRECTORY/libs/libQtilitiesProjectManagement-1.5.1.dylib
cp $BUILD_DIRECTORY/dependencies/qtilities/build/release/libQtilitiesExtensionSystem-1.5.1.0.0.dylib $DEPLOY_DIRECTORY/libs/libQtilitiesExtensionSystem-1.5.1.dylib
cp $BUILD_DIRECTORY/dependencies/qtilities/build/release/libQtilitiesCoreGui-1.5.1.0.0.dylib $DEPLOY_DIRECTORY/libs/libQtilitiesCoreGui-1.5.1.dylib
cp $BUILD_DIRECTORY/dependencies/qtilities/build/release/libQtilitiesCore-1.5.1.0.0.dylib $DEPLOY_DIRECTORY/libs/libQtilitiesCore-1.5.1.dylib
cp $BUILD_DIRECTORY/dependencies/qtilities/build/release/libQtilitiesLogging-1.5.1.0.0.dylib $DEPLOY_DIRECTORY/libs/libQtilitiesLogging-1.5.1.dylib
cp $BUILD_DIRECTORY/dependencies/qscintilla/Qt4Qt5/build/release/libqscintilla2_qt5.13.1.0.dylib $DEPLOY_DIRECTORY/libs/libqscintilla2_qt5.13.dylib

# Run MacDeployQt
cd $QT_BIN_DIRECTORY 
./macdeployqt $DEPLOY_DIRECTORY/Praaline.app -always-overwrite -libpath=/Users/george/Documents/praaline-mac/libs 

# PLUGINS
# Copy over plugins
mkdir $DEPLOY_DIRECTORY/plugins
cd $DEPLOY_DIRECTORY/plugins/

for PLUGIN in "${PLUGINS[@]}"
do
	cp $BUILD_DIRECTORY/app/build/plugins/$PLUGIN.dylib $DEPLOY_DIRECTORY/plugins/
	# Each plugin must be made to look for the shared libraries inside Praaline's app bundle
	install_name_tool -change libpncore.1.dylib @rpath/libpncore.1.dylib $PLUGIN.dylib
	install_name_tool -change libqtpropertybrowser.1.dylib @rpath/libqtpropertybrowser.1.dylib $PLUGIN.dylib
	install_name_tool -change libQtilitiesExtensionSystem-1.5.1.dylib @rpath/libQtilitiesExtensionSystem-1.5.1.dylib $PLUGIN.dylib
	install_name_tool -change libQtilitiesCoreGui-1.5.1.dylib @rpath/libQtilitiesCoreGui-1.5.1.dylib $PLUGIN.dylib
	install_name_tool -change libQtilitiesCore-1.5.1.dylib @rpath/libQtilitiesCore-1.5.1.dylib $PLUGIN.dylib
	install_name_tool -change libQtilitiesLogging-1.5.1.dylib @rpath/libQtilitiesLogging-1.5.1.dylib $PLUGIN.dylib
	otool -L $PLUGIN.dylib 
done

