# This file was automatically generated by DependoBuf.
# Please do not change it.

from dataclasses import dataclass


@dataclass
class Address:
  id: Unsigned | None = None
  street: str | None = None
  floor: int | None = None
  withIntercom: bool | None = None


@dataclass
class Pet:
  id: Unsigned | None = None
  kind: str | None = None
  name: str | None = None


@dataclass
class User:
  id: Unsigned | None = None
  name: str | None = None
  address: Address | None = None
  pet: Pet | None = None
