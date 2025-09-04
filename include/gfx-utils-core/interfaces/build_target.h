#pragma once

#include <string>

namespace gfxutils {

template<typename Derived>
class IBuildTarget {
protected:
	std::string _Name;

private:
	bool _IsComplete = false;

public:
	[[nodiscard]] bool is_complete() const {
		return _IsComplete;
	}

	void _set_complete() {
		_IsComplete = true;
	}

	[[nodiscard]] std::string get_name() const {
		return _Name;
	}

	void _set_name(const std::string &name) {
		_Name = name;
	}
};

}  // namespace gfxutils
