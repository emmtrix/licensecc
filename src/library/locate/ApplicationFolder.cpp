/*
 * ApplicationFolder.cpp
 *
 *  Created on: Oct 12, 2019
 *      Author: Gabriele Contini
 */
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <algorithm>

#include <licensecc/datatypes.h>
#include <licensecc_properties.h>

#include "../base/logger.h"
#include "../base/base.h"
#include "../base/EventRegistry.h"
#include "../os/os.h"
#include "ApplicationFolder.hpp"
#include "../base/file_utils.hpp"

namespace license {
namespace locate {
using namespace std;

ApplicationFolder::ApplicationFolder() : LocatorStrategy("ApplicationFolder") {}

ApplicationFolder::~ApplicationFolder() {}

void ApplicationFolder::try_license_location(string file, EventRegistry& eventRegistry,
											 vector<string>& diskFiles,
											 bool tryLowercase, const string& dir) {
	string temptativeLicense = dir + file + LCC_LICENSE_FILE_EXTENSION;
	ifstream f(temptativeLicense.c_str());
	if (f.good()) {
		diskFiles.push_back(temptativeLicense);
		eventRegistry.addEvent(LICENSE_FOUND, temptativeLicense.c_str());
	} else if (tryLowercase) {
		eventRegistry.addEvent(LICENSE_FILE_NOT_FOUND, temptativeLicense.c_str());
		f.close();
		std::transform(file.begin(), file.end(), file.begin(),
					   [](unsigned char c) { return std::tolower(c); });
		temptativeLicense = dir + file + LCC_LICENSE_FILE_EXTENSION;
		ifstream f(temptativeLicense.c_str());
		if (f.good()) {
			diskFiles.push_back(temptativeLicense);
			eventRegistry.addEvent(LICENSE_FOUND, temptativeLicense.c_str());
		} else {
			eventRegistry.addEvent(LICENSE_FILE_NOT_FOUND, temptativeLicense.c_str());
		}
		f.close();
	}
	f.close();
}

const vector<string> ApplicationFolder::license_locations(EventRegistry &eventRegistry) {
	vector<string> diskFiles;
	char fname[MAX_PATH] = {0};
	const FUNCTION_RETURN fret = getModuleName(fname);
	if (fret == FUNC_RET_OK) {
		const string module_name = get_file(fname);
		const string project_name = LCC_PROJECT_NAME;
		const string module_dir = remove_file(fname);

		// Look for a license with the project name in the working directory
		try_license_location(project_name, eventRegistry, diskFiles, true);

		// Look for a license with the project name in the module directory
		try_license_location(project_name, eventRegistry, diskFiles, true, module_dir);

		// Look for a license with the module name in the working directory
		try_license_location(module_name, eventRegistry, diskFiles);

		// Look for a license with the module name in the module directory
		try_license_location(module_name, eventRegistry, diskFiles, false, module_dir);
	} else {
		LOG_WARN("Error determining module name.");
	}
	return diskFiles;
}

}  // namespace locate
} /* namespace license */
