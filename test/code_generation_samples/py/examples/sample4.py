# This file was automatically generated by DependoBuf.
# Please do not change it.

from annotated_types import Ge
from dataclasses import dataclass
from typing import Any, Annotated


Message3Type = Any


class Message4:
    @dataclass
    class __Message4:
        m3: Message3Type
    
    def construct(m3: Message3Type):
        m3_dep = [Message2().construct("Evgeny")]
        if not m3.check(*m3_dep):
            raise Exception