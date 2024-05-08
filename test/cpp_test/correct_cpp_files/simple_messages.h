#include <string>
#include <variant>

namespace dbuf {
template <int n>
struct A {
	bool check() const {
		return true;
	}
};

struct D {
	int a;
	int b;
	bool check() const {
		return true;
	}
};

template <D d>
struct B {
	int i;
	unsigned u;
	std::string s;
	double f;
	bool b;
	bool check() const {
		return true;
	}
};

struct C {
	A<((1 + 3) + (5 - 4))> a;
	B<D{5, 6}> b;
	bool check() const {
		return true;
	}
};

}