from memory_profiler import profile


class Big:
    big_mas = [0] * 10_000_000

    def __init__(self, i):
        self.arr = [i] * 1000


class Regular:
    def get_inner(self, i):
        return Big(abs(i))


class Outer:
    class Inner:
        big_mas = [0] * 10_000_000

        def __init__(self, i):
            self.arr = [i] * 1000

    def get_inner(self, i):
        return self.Inner(abs(i))


@profile
def check_regular():
    rs = []
    for i in range(1000):
        rs.append(Regular())


@profile
def check_outer():
    os = []
    for i in range(1000):
        os.append(Outer())


if __name__ == '__main__':
    check_regular()
    check_outer()
