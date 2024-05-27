# This file was automatically generated by DependoBuf.
# Please do not change it.

from __future__ import annotations

from annotated_types import Ge
from dataclasses import dataclass
from typing import Annotated

Unsigned = Annotated[int, Ge(0)]


class DbufError(TypeError):
    pass


class Discriminated:
    @dataclass
    class __Good:
        prize: str

        def check(self, b: bool) -> None:
            if type(self) not in Discriminated._possible_types(b):
                raise DbufError(
                    'Type Discriminated.__Good does not match given dependencies.'
                )

    @dataclass
    class __Bad:
        def check(self, b: bool) -> None:
            if type(self) not in Discriminated._possible_types(b):
                raise DbufError(
                    'Type Discriminated.__Bad does not match given dependencies.'
                )

    discriminated_type = __Good | __Bad

    @classmethod
    def _possible_types(cls, b: bool) -> set[type]:
        actual = (b, )
        expected = (True, )  # type: ignore[attr-defined, assignment]
        if _is_consistent(actual, expected):
            return {cls.__Good}

        expected = (False, )  # type: ignore[attr-defined, assignment]
        if _is_consistent(actual, expected):
            return {cls.__Bad}

        return {}

    def __init__(self, b: bool) -> None:
        self.dependencies = (b, )

    def good(self, prize: str) -> __Good:
        obj = self.__Good(prize)
        obj.check(*self.dependencies)
        return obj

    def bad(self) -> __Bad:
        obj = self.__Bad()
        obj.check(*self.dependencies)
        return obj


class Status:
    @dataclass
    class __Status:
        def check(self, n: int) -> None:
            pass

    status_type = __Status

    def __init__(self, n: int) -> None:
        self.dependencies = (n, )

    def construct(self) -> __Status:
        obj = self.__Status()
        obj.check(*self.dependencies)
        return obj


class Person:
    @dataclass
    class __Person:
        dis: Discriminated.discriminated_type
        status: Status.status_type

        def check(self, age: int, male: bool, white: bool) -> None:
            dis_deps = ((male and white) or (not (not (not (male or white)))), )
            self.dis.check(*dis_deps)

            status_deps = (age - (3 * 17), )
            self.status.check(*status_deps)

    person_type = __Person

    def __init__(self, age: int, male: bool, white: bool) -> None:
        self.dependencies = (age, male, white, )

    def construct(self, dis: Discriminated.discriminated_type, status: Status.status_type) -> __Person:
        obj = self.__Person(dis, status)
        obj.check(*self.dependencies)
        return obj


def _is_consistent(actual: tuple, expected: tuple) -> bool:
    for i in range(len(actual)):
        if expected[i] is None:
            continue

        if actual[i] != expected[i]:
            return False

    return True
