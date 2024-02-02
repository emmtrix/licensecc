#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <licensecc/licensecc.h>
#include <fstream>
#include <string.h>
#include <iomanip>
#include "../library/base/string_utils.h"
#include "../library/ini/SimpleIni.h"
#include "../library/os/dmi_info.hpp"
#include "../library/os/cpu_info.hpp"
#include "../library/os/dmi_info.hpp"
#include "../library/os/network.hpp"

using namespace std;
using namespace license::os;

const map<int, string> stringByStrategyId = {
	{STRATEGY_DEFAULT, "DEFAULT"}, {STRATEGY_ETHERNET, "MAC"}, {STRATEGY_IP_ADDRESS, "IP"}, {STRATEGY_DISK, "Disk"}};

const unordered_map<int, string> descByVirtDetail = {{BARE_TO_METAL, "No virtualization"},
													 {VMWARE, "VMWare"},
													 {VIRTUALBOX, "VirtualBox"},
													 {V_XEN, "XEN"},
													 {KVM, "KVM"},
													 {HV, "Microsoft Hypervisor"},
													 {PARALLELS, "Parallels Desktop"},
													 {V_OTHER, "Unknown/other"}};

const unordered_map<int, string> descByVirt = {{LCC_API_VIRTUALIZATION_SUMMARY::NONE, "No virtualization"},
											   {LCC_API_VIRTUALIZATION_SUMMARY::VM, "Virtual machine"},
											   {LCC_API_VIRTUALIZATION_SUMMARY::CONTAINER, "Container"}};

const unordered_map<int, string> descByCloudProvider = {{PROV_UNKNOWN, "None"},
														{ON_PREMISE, "On premise hardware (no cloud)"},
														{GOOGLE_CLOUD, "Google Cloud"},
														{AZURE_CLOUD, "Microsoft Azure"},
														{AWS, "Amazon AWS"},
														{ALI_CLOUD, "Alibaba Cloud"}};

const unordered_map<int, string> stringByEventType = {
	{LICENSE_OK, "OK"},
	{LICENSE_FILE_NOT_FOUND, "License file not found"},
	{LICENSE_SERVER_NOT_FOUND, "License server can't be contacted"},
	{ENVIRONMENT_VARIABLE_NOT_DEFINED, "Environment variable not defined"},
	{FILE_FORMAT_NOT_RECOGNIZED, "Invalid license file format (not .ini file)"},
	{LICENSE_MALFORMED, "Mandatory field(s) missing, or data can't be fully read"},
	{PRODUCT_NOT_LICENSED, "Product not licensed"},
	{PRODUCT_EXPIRED, "License expired"},
	{LICENSE_CORRUPTED, "License signature verification failed"},
	{IDENTIFIERS_MISMATCH, "License identifier verification failed"}};

template <unsigned int N>
ostream& col(ostream& os) {
	os << setw(N) << left;
	return os;
}

void printCheckResult(string product, LCC_EVENT_TYPE result, LicenseInfo& licenseInfo) {
	auto& out = result == LICENSE_OK ? cout : cerr;
	out << col<40> << product + ":" << stringByEventType.find(result)->second;
	if (result == LICENSE_OK && licenseInfo.has_expiry) {
		out << " (expires in " << licenseInfo.days_left << " days)";
	}
	out << endl;
}

static LCC_EVENT_TYPE verifyLicense(const string& fname) {
	LicenseInfo licenseInfo;
	LicenseLocation licLocation = {LICENSE_PATH};
	std::copy(fname.begin(), fname.end(), licLocation.licenseData);
	LCC_EVENT_TYPE result = acquire_license(nullptr, &licLocation, &licenseInfo);
	printCheckResult("Default product [" LCC_PROJECT_NAME "]", result, licenseInfo);

	CSimpleIniA ini;
	ini.LoadFile(fname.c_str());
	CSimpleIniA::TNamesDepend sections;
	ini.GetAllSections(sections);
	CallerInformations callerInformation;
	for (const CSimpleIniA::Entry& section : sections) {
		const string section_name(section.pItem, 15);
		if (section_name != LCC_PROJECT_NAME) {
			std::copy(section_name.begin(), section_name.end(), callerInformation.feature_name);
			LCC_EVENT_TYPE result = acquire_license(&callerInformation, &licLocation, &licenseInfo);
			printCheckResult(string("Product [") + section.pItem + "]", result, licenseInfo);
		}
	}
	return result;
}

template<typename T, size_t N>
void formatted_array(ostream& os, T (&array)[N], char C) {
	for (int i = 0; i < N; i++) {
		if (i != 0) {
			os << C;
		}
		os << to_string(array[i]);
	}
}

int main(int argc, char* argv[]) {
	char hw_identifier[LCC_API_PC_IDENTIFIER_SIZE + 1];
	size_t bufSize = LCC_API_PC_IDENTIFIER_SIZE + 1;
	ExecutionEnvironmentInfo exec_env_info = {};

	cout << "=== Strategy ===" << endl;
	for (const auto& x : stringByStrategyId) {
		if (identify_pc(static_cast<LCC_API_HW_IDENTIFICATION_STRATEGY>(x.first), hw_identifier, &bufSize,
						&exec_env_info)) {
			std::cout << col<23> << x.second + ":" << hw_identifier << std::endl;
		} else {
			std::cout << col<23> << x.second + ":" << "NA" << endl;
		}
	}

	cout << "\n=== System   ===" << endl;
	cout << col<23> << "Virtualization class:" << descByVirt.find(exec_env_info.virtualization)->second << endl;
	cout << col<23> << "Virtualization detail:" << descByVirtDetail.find(exec_env_info.virtualization_detail)->second << endl;
	cout << col<23> << "Cloud provider:" << descByCloudProvider.find(exec_env_info.cloud_provider)->second << endl;

	std::vector<license::os::OsAdapterInfo> adapterInfos;
	FUNCTION_RETURN ret = license::os::getAdapterInfos(adapterInfos);
	if (ret == FUNCTION_RETURN::FUNC_RET_OK) {
		for (const auto& osAdapter : adapterInfos) {
			cout << col<23> << "Network adapter [" + to_string(osAdapter.id) + "]: " << osAdapter.description << endl;
			cout << col<23> << " "
				 << "IP: ";
			formatted_array(cout, osAdapter.ipv4_address, '.');
			cout << endl;
			cout << col<23> << " " << "MAC: " << std::hex;
			formatted_array(cout, osAdapter.mac_address, ':');
			cout << std::dec << endl;
		}
	} else {
		cout << "Unable to obtain network adapter details: " << ret << endl;
	}

	license::os::CpuInfo cpu;
	cout << col<23> << "CPU vendor:" << cpu.vendor() << endl;
	cout << col<23> << "CPU brand:" << cpu.brand() << endl;
	cout << col<23> << "CPU hypervisor:" << cpu.is_hypervisor_set() << endl;
	cout << col<23> << "CPU model:"
		 << "0x" << std::hex << ((long)cpu.model()) << std::dec << endl;
	license::os::DmiInfo dmi_info;
	cout << col<23> << "BIOS vendor:" << dmi_info.bios_vendor() << endl;
	cout << col<23> << "BIOS description:" << dmi_info.bios_description() << endl;
	cout << col<23> << "System vendor:" << dmi_info.sys_vendor() << endl;
	cout << col<23> << "CPU vendor (DMI):" << dmi_info.cpu_manufacturer() << endl;
	cout << col<23> << "CPU cores  (DMI):" << dmi_info.cpu_cores() << endl;

	cout << "\n=== License  ===" << endl;
	if (argc == 2) {
		const string fname(argv[1]);
		ifstream license_file(fname);
		if (license_file.good()) {
			verifyLicense(fname);
		} else {
			cerr << "License file '" << fname << "' not found" << endl;
		}
	}

	bool find_license_with_env_var = FIND_LICENSE_WITH_ENV_VAR;
	if (find_license_with_env_var) {
		char* env_var_value = getenv(LCC_LICENSE_LOCATION_ENV_VAR);
		if (env_var_value != nullptr && env_var_value[0] != '\0') {
			cout << "Environment variable [" << LCC_LICENSE_LOCATION_ENV_VAR << "] value [" << env_var_value << "]"
				 << endl;
			const vector<string> declared_licenses = license::split_string(string(env_var_value), ';');
			for (string fname : declared_licenses) {
				ifstream license_file(fname);
				if (license_file.good()) {
					verifyLicense(fname);
				} else {
					cerr << "License file '" << fname << "' not found" << endl;
				}
			}
		} else {
			cout << "Environment variable [" << LCC_LICENSE_LOCATION_ENV_VAR << "] configured but not defined" << endl;
		}
	}
}
