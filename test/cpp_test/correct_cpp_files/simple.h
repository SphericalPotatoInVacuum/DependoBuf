#include <string>

template <int n>
struct A {

	bool check() const {
		return true;
	}
};

template <A<5> a>
struct B {
	int f;

	bool check() const {
		return true;
	}
};

struct B_a {
	int f;

	bool check(A<5> a) const {
		return true;
	}
};

struct C {
	A<1 + 3 + 5 - 4> a;
	B_a b;

	bool check() const {
		return true && b.check(a);
	}
};

