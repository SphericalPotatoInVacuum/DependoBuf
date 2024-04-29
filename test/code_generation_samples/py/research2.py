from dataclasses import dataclass
from typing import Any


@dataclass
class A:
    pass

@dataclass
class B:
    a: A

class Nat:
    @dataclass
    class __Zero:
        pass
    
    @dataclass
    class __Succ:
        prev: Any
    
    def zero(self):
        return self.__Zero()
    
    def succ(self):
        pass
  