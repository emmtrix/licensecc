#include <array>
#include "identification_strategy.hpp"
#include "default_strategy.hpp"
#include "ethernet.hpp"
#include "disk_strategy.hpp"
namespace license {
namespace hw_identifier {

using namespace std;
LCC_EVENT_TYPE IdentificationStrategy::validate_identifier(const HwIdentifier& identifier) const {
	LCC_EVENT_TYPE result = IDENTIFIERS_MISMATCH;

	if (identifier.get_identification_strategy() == identification_strategy()) {
		const vector<HwIdentifier> available_ids = alternative_ids();
		for (const auto& it : available_ids) {
			if (it == identifier) {
				result = LICENSE_OK;
				break;
			}
		}
	}
	return result;
}

FUNCTION_RETURN IdentificationStrategy::generate_pc_id(HwIdentifier& pc_id) const {
	vector<array<uint8_t, HW_IDENTIFIER_PROPRIETARY_DATA>> data;
	const vector<HwIdentifier> available_ids = alternative_ids();
	FUNCTION_RETURN result = FUNC_RET_NOT_AVAIL;
	if (available_ids.size() > 0) {
		pc_id = available_ids[0];
		result = FUNC_RET_OK;
	}
	return result;
}

std::unique_ptr<IdentificationStrategy> IdentificationStrategy::get_strategy(LCC_API_HW_IDENTIFICATION_STRATEGY strategy) {
	unique_ptr<IdentificationStrategy> result;
	switch (strategy) {
		case STRATEGY_DEFAULT:
			result = unique_ptr<IdentificationStrategy>(dynamic_cast<IdentificationStrategy*>(new DefaultStrategy()));
			break;
		case STRATEGY_ETHERNET:
			result = unique_ptr<IdentificationStrategy>(dynamic_cast<IdentificationStrategy*>(new Ethernet(false)));
			break;
		case STRATEGY_IP_ADDRESS:
			result = unique_ptr<IdentificationStrategy>(dynamic_cast<IdentificationStrategy*>(new Ethernet(true)));
			break;
		case STRATEGY_DISK_NUM:
			result = unique_ptr<IdentificationStrategy>(dynamic_cast<IdentificationStrategy*>(new DiskStrategy(true)));
			break;
		case STRATEGY_DISK_LABEL:
			result = unique_ptr<IdentificationStrategy>(dynamic_cast<IdentificationStrategy*>(new DiskStrategy(false)));
			break;
		default:
			throw logic_error("strategy not supported");
	}
	return result;
}
}  // namespace hw_identifier
}  // namespace license
