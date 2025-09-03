#pragma once

#include <string>

namespace gfxutils {

template<typename Derived>
class IExportableResource {
public:
	void export_to_file(const std::string file_path) const {
		static_cast<const Derived *>(this)->_export_to_file(file_path);
	}

protected:
	IExportableResource() = default;
	~IExportableResource() = default;
};

}  // namespace gfxutils
