#include <string>
#include <variant>

namespace dbuf {
template <int a, int b>
struct Dependent;

template <int a, int b>
struct First {
	int a1;
	bool b1;
	bool check() const {
		return true;
	}
};

template <int b>
struct Dependent<5, b> {
	std::variant<First<5, b>> value;
	bool check() const {
		return false || (std::holds_alternative<First<5, b>>(value) && std::get<First<5, b>>(value).check());
	}
};

template <int a, int b>
struct Second {
	bool a2;
	int b2;
	bool check() const {
		return true;
	}
};

template <int a>
struct Dependent<a, 1> {
	std::variant<Second<a, 1>> value;
	bool check() const {
		return false || (std::holds_alternative<Second<a, 1>>(value) && std::get<Second<a, 1>>(value).check());
	}
};

template <int a, int b>
struct Third {
	std::string a3;
	std::string b3;
	bool check() const {
		return true;
	}
};

template <int a, int b>
struct Dependent {
	std::variant<Third<a, b>> value;
	bool check() const {
		return false || (std::holds_alternative<Third<a, b>>(value) && std::get<Third<a, b>>(value).check());
	}
};

template <int a>
struct First_1_b {
	int a1;
	bool b1;
	bool check(int b) const {
		return true;
	}
};

template <int a>
struct Second_2_b {
	bool a2;
	int b2;
	bool check(int b) const {
		return true;
	}
};

template <int a>
struct Third_3_b {
	std::string a3;
	std::string b3;
	bool check(int b) const {
		return true;
	}
};

template <int a>
struct Dependent_b {
	std::variant<First_1_b<a>, Second_2_b<a>, Third_3_b<a>> value;
	bool check(int b) const {
		if (a == 5)
			return (std::holds_alternative<First_1_b<a>>(value) && std::get<First_1_b<a>>(value).check(b));
		else if (b == 1)
			return (std::holds_alternative<Second_2_b<a>>(value) && std::get<Second_2_b<a>>(value).check(b));
		else 
			return (std::holds_alternative<Third_3_b<a>>(value) && std::get<Third_3_b<a>>(value).check(b));
		return false;
	}
};

struct First_1_a_b {
	int a1;
	bool b1;
	bool check(int a, int b) const {
		return true;
	}
};

struct Second_2_a_b {
	bool a2;
	int b2;
	bool check(int a, int b) const {
		return true;
	}
};

struct Third_3_a_b {
	std::string a3;
	std::string b3;
	bool check(int a, int b) const {
		return true;
	}
};

struct Dependent_a_b {
	std::variant<First_1_a_b, Second_2_a_b, Third_3_a_b> value;
	bool check(int a, int b) const {
		if (a == 5)
			return (std::holds_alternative<First_1_a_b>(value) && std::get<First_1_a_b>(value).check(a, b));
		else if (b == 1)
			return (std::holds_alternative<Second_2_a_b>(value) && std::get<Second_2_a_b>(value).check(a, b));
		else 
			return (std::holds_alternative<Third_3_a_b>(value) && std::get<Third_3_a_b>(value).check(a, b));
		return false;
	}
};

template <int a, int b>
struct Now {
	int c;
	Dependent_b<(a + b)> d1;
	Dependent_a_b d2;
	bool check() const {
		return true && d2.check(c, (c + c)) && d1.check((c + b));
	}
};

}