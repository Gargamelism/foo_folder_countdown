#pragma once

namespace foo_countdown {
	template <class T>
	class array_t {
	public:
		array_t() : _size(0) {};
		array_t(int size) : _size(size) {};

		int size() {
			return _size;
		}

		T& operator[](int idx) {
			return _vals[idx];
		}

		void clear() {
			_size = 0;
		}

		void push_back(T& t) {
			_vals[_size] = t;
			_size++;
		}

	private:
		static const int max_limit = 100;

		T _vals[max_limit];
		int _size;
	};
}