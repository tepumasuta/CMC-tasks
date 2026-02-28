import pytest

import numbers
from fractions import Fraction
from typing import Self


class Int(numbers.Integral):
    def __init__(self, value: numbers.Complex = 0):
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

    def __add__(self, other: numbers.Complex) -> numbers.Complex:
        if isinstance(other, numbers.Integral):
            return Int(self.value + Int(other).value)
        return NotImplemented

    def __radd__(self, other: numbers.Complex) -> numbers.Complex:
        return self.__add__(other)

    def __mul__(self, other: numbers.Complex) -> numbers.Complex:
        if isinstance(other, numbers.Integral):
            return Int(self.value * Int(other).value)
        return NotImplemented

    def __rmul__(self, other: numbers.Complex) -> numbers.Complex:
        return self.__mul__(other)

    def __truediv__(self, other):
        assert False, "TODO: implement"

    def __rtruediv__(self, other):
        assert False, "TODO: implement"

    def __mod__(self, other) -> Self:
        if isinstance(other, numbers.Integral):
            return Int(self.value % Int(other).value)
        return NotImplemented

    def __rmod__(self, other) -> Self:
        if isinstance(other, numbers.Integral):
            return Int(other) % self
        return NotImplemented

    def __floordiv__(self, other) -> Self:
        if isinstance(other, numbers.Integral):
            return Int(self.value // Int(other).value)
        return NotImplemented

    def __rfloordiv__(self, other) -> Self:
        if isinstance(other, numbers.Integral):
            return Int(other) // self
        return NotImplemented

    def __pow__(self, exponent, modulus=None):
        assert False, "TODO: implement"

    def __rpow__(self, base):
        assert False, "TODO: implement"

    def __invert__(self) -> Self:
        return Int(~self.value)

    def __and__(self, other) -> Self:
        if isinstance(other, numbers.Integral):
            return Int(self.value & Int(other).value)
        return NotImplemented

    def __rand__(self, other) -> Self:
        return self.__and__(other)

    def __or__(self, other) -> Self:
        if isinstance(other, numbers.Integral):
            return Int(self.value | Int(other).value)
        return NotImplemented

    def __ror__(self, other) -> Self:
        return self.__or__(other)

    def __xor__(self, other) -> Self:
        if isinstance(other, numbers.Integral):
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

    def __round__(self, ndigits: None = None) -> numbers.Complex:
        assert False, "TODO: implement"

    def __eq__(self, other) -> bool:
        if not isinstance(other, numbers.Complex):
            return False
        if isinstance(other, numbers.Integral):
            return self.value == Int(other).value
        return NotImplemented

    def __le__(self, other) -> bool:
        if isinstance(other, numbers.Integral):
            return self.value <= Int(other).value
        return NotImplemented

    def __lt__(self, other) -> bool:
        if isinstance(other, numbers.Integral):
            return self.value < Int(other).value
        return NotImplemented

    def __lshift__(self, other) -> Self:
        if isinstance(other, numbers.Integral):
            return Int(self.value << Int(other).value)
        return NotImplemented

    def __rlshift__(self, other) -> Self:
        if isinstance(other, numbers.Integral):
            return Int(other) << self
        return NotImplemented

    def __rshift__(self, other):
        if isinstance(other, numbers.Integral):
            return Int(self.value >> Int(other).value)
        return NotImplemented

    def __rrshift__(self, other):
        if isinstance(other, numbers.Integral):
            return Int(other) >> self
        return NotImplemented
    
    def __repr__(self):
        return f'{self.value}'

    def __str__(self):
        return f'Int({self.value})'


def test_int_constructors():
    assert Int() == Int()
    assert Int() == int()
    assert Int(69) == 69
    assert 69 == Int(69)
    assert Int(69.0) == Int(69)
    assert Int(Fraction(42)) == Int(42)


def test_int_comparisons():
    assert Int(5) == Int(5)
    assert Int(-5) == Int(-5)
    assert Int(0) == Int(0)
    assert Int(10**100) == Int(10**100)
    assert Int(10**100) != Int(10**100 + 1)

    assert Int(5) == 5
    assert 5 == Int(5)
    assert Int(-5) == -5
    assert -5 == Int(-5)
    assert Int(0) == 0
    assert 0 == Int(0)
    assert Int(5) != 6
    assert 6 != Int(5)

    assert Int(5) == 5.0
    assert 5.0 == Int(5)
    assert 5.0 + 1e-14 == Int(5)
    assert Int(5) == 5.0 + 1e-14
    assert 5.0 - 1e-14 == Int(5)
    assert Int(5) != 5.0 + 1e-7
    assert 5.0 + 1e-7 != Int(5)
    assert 6.0 != Int(5)
    assert Int(5) != 6.0
    assert Int(5) == 5.0 + 1e-12
    assert Int(5) != 5.0 + 1e-8

    assert Fraction(5) == Int(5)
    assert Int(5) == Fraction(5)
    assert Int(5) != Fraction(5, 2)
    assert Fraction(5, 2) != Int(2)

    assert Int(5) != "5"
    assert "5" != Int(5)
    assert Int(5) != [5]
    assert [5] != Int(5)
    assert Int(5) != (5,)
    assert (5,) != Int(5)
    assert Int(5) != {5: 5}
    assert Int(5) != None
    assert None != Int(5)
    assert Int(5) != object()

    assert (Int(5) != Int(5)) is False
    assert (Int(5) != 5) is False
    assert (5 != Int(5)) is False
    assert (Int(5) != 5.0) is False
    assert (5.0 != Int(5)) is False
    assert (Fraction(5) != Int(5)) is False
    assert (Int(5) != Fraction(5)) is False
    assert (Int(5) != "5") is True

    assert Int(5) < Int(6)
    assert Int(5) <= Int(5)
    assert not (Int(5) < Int(5))
    assert Int(6) > Int(5)
    assert Int(6) >= Int(5)
    assert Int(-10) < Int(-5)
    assert Int(-5) <= Int(-5)

    assert Int(5) < 6
    assert 5 < Int(6)
    assert Int(5) <= 5
    assert 5 <= Int(5)
    assert not (Int(5) < 5)
    assert not (5 < Int(5))
    assert Int(6) > 5
    assert 6 > Int(5)
    assert Int(6) >= 5
    assert 6 >= Int(5)
    assert Int(-5) < -4
    assert -4 > Int(-5)

    assert Int(5) <= 5.0
    assert 5.0 <= Int(5)
    assert Int(5) >= 5.0
    assert 5.0 >= Int(5)
    assert not (Int(5) < 5.0)
    assert not (5.0 < Int(5))
    assert not (Int(5) > 5.0)
    assert not (5.0 > Int(5))
    assert Int(5) < 5.0 + 1e-8
    assert not (5.0 + 1e-8 < Int(5))
    assert 5.0 + 1e-8 > Int(5)
    assert not (Int(5) > 5.0 + 1e-8)
    assert Int(5) <= 5.0 + 1e-8
    assert not (5.0 + 1e-8 <= Int(5))
    assert 5.0 + 1e-8 >= Int(5)
    assert not (Int(5) >= 5.0 + 1e-8)
    assert Int(5) > 5.0 - 1e-8
    assert not (5.0 - 1e-8 > Int(5))
    assert 5.0 - 1e-8 < Int(5)
    assert not (Int(5) < 5.0 - 1e-8)
    assert Int(5) >= 5.0 - 1e-8
    assert not (5.0 - 1e-8 >= Int(5))
    assert 5.0 - 1e-8 <= Int(5)
    assert not (Int(5) <= 5.0 - 1e-8)
    f_close = 5.0 + 1e-14
    assert Int(5) == f_close
    assert f_close == Int(5)
    assert Int(5) <= f_close
    assert f_close <= Int(5)
    assert Int(5) >= f_close
    assert f_close >= Int(5)
    assert not (Int(5) < f_close)
    assert not (f_close < Int(5))
    assert not (Int(5) > f_close)
    assert not (f_close > Int(5))
    assert Int(-5) < -4.9
    assert Int(-5) > -5.1
    assert -4.9 > Int(-5)
    assert -5.1 < Int(-5)

    assert Int(5) < Fraction(6)
    assert not (Fraction(6) < Int(5))
    assert Int(5) <= Fraction(5)
    assert Fraction(5) <= Int(5)
    assert Int(5) >= Fraction(5)
    assert Fraction(5) >= Int(5)
    assert not (Int(5) < Fraction(5))
    assert not (Fraction(5) < Int(5))
    assert not (Int(5) > Fraction(5))
    assert not (Fraction(5) > Int(5))
    assert Int(6) > Fraction(5)
    assert not (Fraction(5) > Int(6))
    assert Fraction(6) > Int(5)
    assert not (Int(5) > Fraction(6))
    assert Int(6) >= Fraction(5)
    assert not (Fraction(5) >= Int(6))
    assert Fraction(5) <= Int(5)
    assert not (Int(5) <= Fraction(5))
    assert Fraction(4) < Int(5)
    assert not (Int(5) < Fraction(4))
    assert Int(5) > Fraction(5, 2)
    assert not (Fraction(5, 2) > Int(5))
    assert Int(5) < Fraction(11, 2)
    assert not (Fraction(11, 2) < Int(5))
    assert Int(5) <= Fraction(10, 2)
    assert Fraction(10, 2) <= Int(5)
    assert Int(5) >= Fraction(10, 2)
    assert Fraction(10, 2) >= Int(5)
    assert Int(-5) < Fraction(-4)
    assert Int(-5) > Fraction(-6)
    assert Int(-5) <= Fraction(-5)
    assert Int(-5) >= Fraction(-5)

    with pytest.raises(TypeError):
        Int(5) < "5"
    with pytest.raises(TypeError):
        Int(5) <= "5"
    with pytest.raises(TypeError):
        Int(5) > [5]
    with pytest.raises(TypeError):
        Int(5) >= None
    with pytest.raises(TypeError):
        Int(5) < object()
    with pytest.raises(TypeError):
        "5" < Int(5)
    with pytest.raises(TypeError):
        [5] <= Int(5)
    with pytest.raises(TypeError):
        None > Int(5)
    with pytest.raises(TypeError):
        object() >= Int(5)

    class Dummy:
        pass

    assert (Int(5) == Dummy()) is False
