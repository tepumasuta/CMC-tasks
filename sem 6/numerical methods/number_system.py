from numbers import Integral, Complex
from fractions import Fraction
from typing import Self


class Int(Integral):
    def __init__(self, value: Complex = 0):
        super().__init__()
        if isinstance(value, int):
            self.value = value
            return
        elif isinstance(value, float):
            if value.is_integer():
                self.value = int(value)
                return
        elif isinstance(value, Fraction):
            if value.denominator == 1:
                self.value = value.numerator
                return
        elif isinstance(value, Int):
            self.value = value.value
            return
        assert False, "TODO: implement other types"

    def __int__(self):
        return self.value

    def __pos__(self) -> Self:
        return Int(+self.value)

    def __neg__(self) -> Self:
        return Int(-self.value)

    def __abs__(self) -> Self:
        return Int(abs(self.value))

    def __add__(self, other: Complex) -> Complex:
        if isinstance(other, Integral):
            return Int(self.value + Int(other).value)
        return NotImplemented

    def __radd__(self, other: Complex) -> Complex:
        return self.__add__(other)

    def __mul__(self, other: Complex) -> Complex:
        if isinstance(other, Integral):
            return Int(self.value * Int(other).value)
        return NotImplemented

    def __rmul__(self, other: Complex) -> Complex:
        return self.__mul__(other)

    def __truediv__(self, other):
        assert False, "TODO: implement"

    def __rtruediv__(self, other):
        assert False, "TODO: implement"

    def __mod__(self, other) -> Self:
        if isinstance(other, Integral):
            return Int(self.value % Int(other).value)
        return NotImplemented

    def __rmod__(self, other) -> Self:
        if isinstance(other, Integral):
            return Int(other) % self
        return NotImplemented

    def __floordiv__(self, other) -> Self:
        if isinstance(other, Integral):
            return Int(self.value // Int(other).value)
        return NotImplemented

    def __rfloordiv__(self, other) -> Self:
        if isinstance(other, Integral):
            return Int(other) // self
        return NotImplemented

    def __pow__(self, exponent, modulus=None):
        assert False, "TODO: implement"

    def __rpow__(self, base):
        assert False, "TODO: implement"

    def __invert__(self) -> Self:
        return Int(~self.value)

    def __and__(self, other) -> Self:
        if isinstance(other, Integral):
            return Int(self.value & Int(other).value)
        return NotImplemented

    def __rand__(self, other) -> Self:
        return self.__and__(other)

    def __or__(self, other) -> Self:
        if isinstance(other, Integral):
            return Int(self.value | Int(other).value)
        return NotImplemented

    def __ror__(self, other) -> Self:
        return self.__or__(other)

    def __xor__(self, other) -> Self:
        if isinstance(other, Integral):
            return Int(self.value ^ Int(other).value)
        return NotImplemented

    def __rxor__(self, other) -> Self:
        return self.__xor__(other)

    def __trunc__(self) -> Self:
        return Int(self)

    def __ceil__(self) -> Self:
        return Int(self)

    def __floor__(self) -> Self:
        return Int(self)

    def __round__(self, ndigits: None = None) -> Complex:
        assert False, "TODO: implement"

    def __eq__(self, other) -> bool:
        if not isinstance(other, Complex):
            return False
        if isinstance(other, Integral):
            return self.value == Int(other).value
        return NotImplemented

    def __le__(self, other) -> bool:
        if isinstance(other, Integral):
            return self.value <= Int(other).value
        return NotImplemented

    def __lt__(self, other) -> bool:
        if isinstance(other, Integral):
            return self.value < Int(other).value
        return NotImplemented

    def __lshift__(self, other) -> Self:
        if isinstance(other, Integral):
            return Int(self.value << Int(other).value)
        return NotImplemented

    def __rlshift__(self, other) -> Self:
        if isinstance(other, Integral):
            return Int(other) << self
        return NotImplemented

    def __rshift__(self, other):
        if isinstance(other, Integral):
            return Int(self.value >> Int(other).value)
        return NotImplemented

    def __rrshift__(self, other):
        if isinstance(other, Integral):
            return Int(other) >> self
        return NotImplemented


def test_int_constructors():
    assert Int() == Int()
    assert Int() == int()
    assert Int(69) == 69
    assert 69 == Int(69)
    assert Int(69.0) == Int(69)
    assert Int(Fraction(42)) == Int(42)
