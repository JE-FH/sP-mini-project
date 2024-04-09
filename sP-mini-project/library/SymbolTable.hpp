#pragma once
#include <unordered_map>

namespace stosim {
	struct SymbolDoesNotExistException : public std::exception {
		SymbolDoesNotExistException(const char* message)
			: std::exception(message) {}
	};

	struct SymbolAlreadyExistsException : public std::exception {
		SymbolAlreadyExistsException(const char* message)
			: std::exception(message) {}
	};

	template<typename K, typename V>
	class SymbolTable {
		std::unordered_map<K, V> _symbol_map;
	public:
		const V& lookup(const K& k) const {
			auto it = _symbol_map.find(k);
			if (it == std::end(_symbol_map)) {
				throw SymbolDoesNotExistException("lookup() The requested symbol does not exist");
			}
			return it->second;
		}

		void store(K k, V v) {
			auto [it, inserted] = _symbol_map.try_emplace(std::move(k), std::move(v));
			if (!inserted) {
				throw SymbolAlreadyExistsException("store() The symbol already has an assigned value");
			}
		}

		const std::unordered_map<K, V> get_map() const {
			return _symbol_map;
		}
	};
}