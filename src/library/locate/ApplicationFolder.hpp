/*
 * ApplicationFolder.h
 *
 *  Created on: Oct 6, 2019
 *      Author: devel
 */

#ifndef SRC_LIBRARY_RETRIEVERS_APPLICATIONFOLDER_H_
#define SRC_LIBRARY_RETRIEVERS_APPLICATIONFOLDER_H_

#include <string>

#include "LocatorStrategy.hpp"

namespace license {
namespace locate {

class ApplicationFolder : public LocatorStrategy {
	void try_license_location(std::string file, EventRegistry& eventRegistry, std::vector<std::string>& diskFiles,
							  bool tryLowercase = false, const std::string& dir = "");

public:
	ApplicationFolder();
	const virtual std::vector<std::string> license_locations(EventRegistry& eventRegistry);
	virtual ~ApplicationFolder();
};

}  // namespace locate
} /* namespace license */

#endif /* SRC_LIBRARY_RETRIEVERS_APPLICATIONFOLDER_H_ */
