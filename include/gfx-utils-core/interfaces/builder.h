#pragma once

#include <string>

namespace gfxutils {

template<typename Derived, typename BuildTarget>
class IBuilder {
protected:
	std::string _Name;

public:
	explicit IBuilder(const std::string &name)
	    : _Name(name) {
	}

	[[nodiscard]] std::string get_name() const { return _Name; }

	BuildTarget build() const {
		return static_cast<const Derived *>(this)->_build();
	}
};

}  // namespace gfxutils
