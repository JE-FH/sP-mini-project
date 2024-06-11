#pragma once
#include <unordered_map>
#include <algorithm>
#include <iterator>
#include <coro/coro.hpp>
#include <concepts>

namespace stosim {
	struct SymbolDoesNotExistException : public std::exception {
		SymbolDoesNotExistException(const char* message)
			: std::exception(message) {}
	};

	struct SymbolAlreadyExistsException : public std::exception {
		SymbolAlreadyExistsException(const char* message)
			: std::exception(message) {}
	};

	template<typename T>
	concept MoveableAndOrdered = std::movable<T> && std::totally_ordered<T>;

	/* Requirement 3 states that a generic symbol table for keys and values should be craeted
	   We want to enforce that a token has only been used once and that a token name has only
	   been used once, therefore we use a sorted list and an index to check uniqueness
	   of both values
	   */
	template<MoveableAndOrdered K, MoveableAndOrdered V>
	class SymbolTable {
		std::vector<std::pair<K, V>> _key_sorted_values;
		std::vector<std::size_t> _value_index;

		static const K& key_proj(const std::pair<K, V>& a) {
			return a.first;
		}

		const V& value_index_proj(const std::size_t& v) const {
			return _key_sorted_values[v].second;
		}

	public:
		const V& lookup(const K& k) const {
			auto key_it = std::ranges::lower_bound(_key_sorted_values, k, std::less<K>{}, key_proj);
			
			if (key_it == std::cend(_key_sorted_values) || key_proj(*key_it) != k) {
				throw SymbolDoesNotExistException("lookup() The requested symbol does not exist");
			}

			return key_it->second;
		}

		const K& lookup_by_value(const V& v) const {
			auto value_it = std::ranges::lower_bound(_value_index, v, std::less<V>{}, [&](const auto& o) {return value_index_proj(o);});
			
			if (value_it == std::cend(_value_index) || value_index_proj(*value_it) != v) {
				throw SymbolDoesNotExistException("lookup_by_value() The requested value does not exist");
			}

			return _key_sorted_values[*value_it].first;
		}

		void store(K k, V v) {
			auto key_it = std::ranges::lower_bound(_key_sorted_values, k, std::less<K>{}, key_proj);
			if (key_it != std::cend(_key_sorted_values) && key_proj(*key_it) == k) {
				throw SymbolAlreadyExistsException("store() The key already exists");
			}
			auto value_it = std::ranges::lower_bound(_value_index, v, std::less<V>{}, [&](const auto& o) {return value_index_proj(o);});
			if (value_it != std::cend(_value_index) && value_index_proj(*value_it) == v) {
				throw SymbolAlreadyExistsException("store() The value already exists");
			}

			auto emplaced_it = _key_sorted_values.emplace(key_it, std::make_pair(std::move(k), std::move(v)));

			auto inserted_index = static_cast<std::size_t>(std::distance(std::begin(_key_sorted_values), emplaced_it));

			for (auto& value : _value_index) {
				if (value >= inserted_index) {
					value++;
				}
			}

			_value_index.emplace(value_it, inserted_index);
		}

		coro::generator<const std::pair<K, V>&> symbol_table() const {
			for (const auto& value : _key_sorted_values) {
				co_yield value;
			}
			co_return;
		}
		
	};
}