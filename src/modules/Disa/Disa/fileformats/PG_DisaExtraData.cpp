#include "PG_DisaExtraData.h"

bool nameRelevantChanged(const PG_DisaExtraFields& fields, const PG_Variety& before, const PG_Variety& after) {
	return std::ranges::any_of(fields, [&before, &after](const PG_DisaExtraField& field) {
		return field.nameRelevant && field.getValue && field.getValue(before) != field.getValue(after);
		});
}
