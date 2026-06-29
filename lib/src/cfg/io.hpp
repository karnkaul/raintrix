#pragma once
#include "cfg/variable.hpp"
#include "klib/ptr.hpp"
#include "klib/string/c_string.hpp"
#include <cstdint>
#include <istream>
#include <unordered_map>

namespace raintrix::cfg {
enum class WriteStatus : std::int8_t { Active, Commented };

class Reader {
  public:
	void track_variable(IVariable& variable);

	void parse_next(std::string_view line);
	void parse_stream(std::istream& is);
	auto parse_file(klib::CString path) -> bool;

  private:
	std::unordered_map<std::string_view, klib::Ptr<IVariable>> m_variables{};
};

class Writer {
  public:
	void write_variable(IVariable const& variable, std::string_view description = {}, WriteStatus status = WriteStatus::Active);

	[[nodiscard]] auto get_text() const -> std::string_view { return m_text; }
	[[nodiscard]] auto write_to(klib::CString path) const -> bool;

  private:
	std::string m_text{};
};
} // namespace raintrix::cfg
