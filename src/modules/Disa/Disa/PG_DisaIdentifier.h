#pragma once

#include <string>

struct PG_DisaIdentifier {
	constexpr static unsigned int customIdStart = 10001;

	PG_DisaIdentifier() = default;
	PG_DisaIdentifier(std::string&& v) : value(std::move(v)) {
		build({});
	}

	PG_DisaIdentifier(const std::string& v) : value(v) {
		build({});
	}
	PG_DisaIdentifier(const std::string& v, const std::string& baseNamespace) : value(v) {
		build(baseNamespace);
	}

	std::string getNamespace() const {
		if (!isValid()) return {};
		return std::string(value.begin(), value.begin() + splitAt - 2);
	}

	std::string getName() const {
		if (!isValid()) return {};
		return std::string(value.begin() + splitAt, value.end());
	}

	bool isCustom() const {
		return getNamespace() != "disa";
	}

	unsigned int getNameAsNumber() const;

	inline const std::string& getIdentifier() const {
		return value;
	}

	inline bool isValid() const noexcept {
		return splitAt > 3;
	}

	inline operator bool() const { return isValid(); }
	inline operator const std::string() const { return getIdentifier(); }

	void operator=(std::string&& v) {
		value = std::move(v);
		build({});
	}
	void operator=(const std::string& v) {
		value = v;
		build({});
	}

	bool build(const std::string& baseNamespace);

	static inline bool isCustomId(unsigned int id) { return id >= customIdStart; }

	std::string value;
	unsigned int splitAt = 0;
};