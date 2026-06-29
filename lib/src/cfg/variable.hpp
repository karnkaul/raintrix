#pragma once
#include "klib/base_types.hpp"
#include "klib/string/from_chars.hpp"
#include <format>
#include <string>

namespace raintrix::cfg {
template <typename Type>
concept ValueT = std::same_as<bool, Type> || klib::NumberT<Type> || std::same_as<std::string, Type>;

template <klib::NumberT T>
auto assign_if(std::string_view const input, T& out) -> bool {
	return klib::FromChars{.text = input}(out);
}

void assign_if(std::string_view input, bool& out);
void assign_if(std::string_view input, std::string& out);

class IVariable : public klib::Polymorphic {
  public:
	[[nodiscard]] virtual auto get_key() const -> std::string_view = 0;
	[[nodiscard]] virtual auto serialize_value() const -> std::string = 0;

	virtual void parse(std::string_view value) = 0;
};

template <ValueT Type>
class Variable : public IVariable {
  public:
	using value_type = Type;

	explicit Variable(std::string_view const key, Type value = {}) : value(value), m_key(key) {}

	[[nodiscard]] auto get_key() const -> std::string_view final { return m_key; }
	[[nodiscard]] auto serialize_value() const -> std::string final { return std::format("{}", value); }
	void parse(std::string_view const input) final { assign_if(input, value); }

	operator Type const&() const { return value; }

	Type value{};

  private:
	std::string_view m_key{};
};
} // namespace raintrix::cfg
