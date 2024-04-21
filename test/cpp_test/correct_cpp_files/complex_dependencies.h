#include <string>

template <int a>
struct Sum {

	bool check() const {
		return true;
	}
};

template <int a, int b>
struct Foo {
	Sum<a + b> sum;

	bool check() const {
		return true;
	}
};

struct Sum_a {

	bool check(int a) const {
		return true;
	}
};

struct Foo_a_b {
	Sum_a sum;

	bool check(int a, int b) const {
		return true && sum.check(a + b);
	}
};

template <int a>
struct Foo_b {
	Sum_a sum;

	bool check(int b) const {
		return true && sum.check(a + b);
	}
};

template <int c>
struct Bar {
	int e;
	int d;
	Foo_a_b f;
	Foo_b<c> g;

	bool check() const {
		return true && g.check(e + d) && f.check(e, d);
	}
};

