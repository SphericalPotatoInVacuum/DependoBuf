# This file was automatically generated by DependoBuf.
# Please do not change it.

from __future__ import annotations

from annotated_types import Ge
from dataclasses import dataclass
from typing import Annotated, TypeAlias

Unsigned = Annotated[int, Ge(0)]


class _DbufError(TypeError):
    pass


class Color:
    @dataclass
    class __Red:
        r: int

        def check(self, s: str) -> None:
            if type(self) not in Color._possible_types(s):
                raise _DbufError(
                    'Type Color.__Red does not match given dependencies.'
                )

    @dataclass
    class __Green:
        g: int

        def check(self, s: str) -> None:
            if type(self) not in Color._possible_types(s):
                raise _DbufError(
                    'Type Color.__Green does not match given dependencies.'
                )

    color_type: TypeAlias = __Red | __Green

    @classmethod
    def _possible_types(cls, s: str) -> set[type]:
        actual = (s, )
        expected = ('red', )
        if _is_consistent(actual, expected):
            return {cls.__Red}

        expected = ('green', )
        if _is_consistent(actual, expected):
            return {cls.__Green}

        return set()

    def __init__(self, s: str) -> None:
        self.dependencies = (s, )

    def red(self, r: int) -> __Red:
        obj = self.__Red(r)
        obj.check(*self.dependencies)
        return obj

    def green(self, g: int) -> __Green:
        obj = self.__Green(g)
        obj.check(*self.dependencies)
        return obj


class House:
    @dataclass
    class __GreenHouse:
        address: str

        def check(self, s: str, col: Color.color_type) -> None:
            if type(self) not in House._possible_types(s, col):
                raise _DbufError(
                    'Type House.__GreenHouse does not match given dependencies.'
                )

    @dataclass
    class __DefaultHouse:
        def check(self, s: str, col: Color.color_type) -> None:
            if type(self) not in House._possible_types(s, col):
                raise _DbufError(
                    'Type House.__DefaultHouse does not match given dependencies.'
                )

    @dataclass
    class __DefaultHouse2:
        def check(self, s: str, col: Color.color_type) -> None:
            if type(self) not in House._possible_types(s, col):
                raise _DbufError(
                    'Type House.__DefaultHouse2 does not match given dependencies.'
                )

    house_type: TypeAlias = __GreenHouse | __DefaultHouse | __DefaultHouse2

    @classmethod
    def _possible_types(cls, s: str, col: Color.color_type) -> set[type]:
        actual = (s, col, )
        expected = ('green', Color._Color__Green(12), )  # type: ignore[attr-defined, assignment]
        if _is_consistent(actual, expected):
            return {cls.__GreenHouse}

        expected = (None, None, )  # type: ignore[attr-defined, assignment]
        if _is_consistent(actual, expected):
            return {cls.__DefaultHouse, cls.__DefaultHouse2}

        return set()

    def __init__(self, s: str, col: Color.color_type) -> None:
        col_deps = ('green', )
        col.check(*col_deps)

        self.dependencies = (s, col, )

    def green_house(self, address: str) -> __GreenHouse:
        obj = self.__GreenHouse(address)
        obj.check(*self.dependencies)
        return obj

    def default_house(self) -> __DefaultHouse:
        obj = self.__DefaultHouse()
        obj.check(*self.dependencies)
        return obj

    def default_house2(self) -> __DefaultHouse2:
        obj = self.__DefaultHouse2()
        obj.check(*self.dependencies)
        return obj


class Village:
    @dataclass
    class __DefVillage:
        def check(self, n: int, h: House.house_type) -> None:
            if type(self) not in Village._possible_types(n, h):
                raise _DbufError(
                    'Type Village.__DefVillage does not match given dependencies.'
                )

    village_type: TypeAlias = __DefVillage

    @classmethod
    def _possible_types(cls, n: int, h: House.house_type) -> set[type]:
        actual = (n, h, )
        expected = (None, House._House__DefaultHouse(), )  # type: ignore[attr-defined, assignment]
        if _is_consistent(actual, expected):
            return {cls.__DefVillage}

        return set()

    def __init__(self, n: int, h: House.house_type) -> None:
        h_deps = ('my', Color._Color__Green(n + (2 * 4)), )  # type: ignore[attr-defined]
        h.check(*h_deps)

        self.dependencies = (n, h, )

    def def_village(self) -> __DefVillage:
        obj = self.__DefVillage()
        obj.check(*self.dependencies)
        return obj


def _is_consistent(actual: tuple, expected: tuple) -> bool:
    for i in range(len(actual)):
        if expected[i] is None:
            continue

        if actual[i] != expected[i]:
            return False

    return True
