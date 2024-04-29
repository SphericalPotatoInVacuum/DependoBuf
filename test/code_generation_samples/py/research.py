from dataclasses import dataclass


class SomeEnum:
    @dataclass
    class __FromOne:
        val: float
        xy: int

        @staticmethod
        def check(n: int) -> bool:
            return n == 1

    @dataclass
    class __BaseConstructor:
        val: float

        @staticmethod
        def check(n: int) -> bool:
            return n != 1

    def __init__(self, n: int) -> None:
        self.n = n

    def from_one(self, val: float, xy: int) -> __FromOne:
        if self.n != 1:
            raise Exception

        return self.__FromOne(val, xy)

    def base_constructor(self, val: float) -> __BaseConstructor:
        if self.n == 1:
            raise Exception

        return self.__BaseConstructor(val)


class MyMessage:
    @dataclass
    class __C:
        name: str
        surname: str

    def __init__(self, u: int, f: float, n: int):
        self.u = u
        self.f = f
        self.n = n

    def construct(self, name: str, surname: str):
        return self.__C(name, surname)


# s1 = SomeEnum(2).base_constructor(val=3.7)
# s2 = SomeEnum(1).from_one(val=1.7, xy=3)

sf = SomeEnum(1)
print(sf)
