import pytest

import math
import numbers
from fractions import Fraction
from typing import Self


def is_integral(value: int | float | Fraction | numbers.Complex) -> bool:
    if isinstance(value, int):
        return True
    if isinstance(value, float):
        return value.is_integer()
    if isinstance(value, Fraction):
        return value.denominator == 1
    if isinstance(value, numbers.Complex):
        return isinstance(value, numbers.Integral)
    return False


def to_integer(value: int | float | Fraction | numbers.Complex) -> int:
    if isinstance(value, int):
        return value
    if isinstance(value, float):
        return int(value)
    if isinstance(value, Fraction):
        return value.numerator
    if isinstance(value, Int):
        return value.value
    if isinstance(value, numbers.Complex):
        return int(value)
    raise ValueError(f"Unknown value type: {type(value)}; passed: {value}")


def integer_nth_root(n: int, q: int) -> tuple[int, bool]:
    neg = n < 0
    if neg:
        assert q % 2 == 1
        n = abs(n)
    if n == 0:
        return 0, True
    hi = 1 << ((n.bit_length() + q - 1) // q)
    lo = 1
    while lo <= hi:
        mid = (lo + hi) // 2
        p = pow(mid, q)
        if p == n:
            return mid * (-1 if neg else 1), True
        if p < n:
            lo = mid + 1
        else:
            hi = mid - 1
    return hi, False


def is_exact_2_neg_power(x: float) -> tuple[bool, int | None]:
    if not math.isfinite(x) or x <= 0.0:
        return False, None
    m, e = math.frexp(x)
    if m == 0.5:
        k = 1 - e
        if k >= 0:
            return True, k
    return False, None


class Rat(numbers.Rational): ...


class Int(numbers.Integral):
    def __init__(self, value: numbers.Complex = 0):
        super().__init__()
        if is_integral(value):
            self.value = to_integer(value)
            return
        raise ValueError(f"Non integral value: {value}")

    def __int__(self):
        return self.value

    def __pos__(self) -> Self:
        return Int(+self.value)

    def __neg__(self) -> Self:
        return Int(-self.value)

    def __abs__(self) -> Self:
        return Int(abs(self.value))

    def __add__(self, other: numbers.Complex) -> numbers.Complex:
        if is_integral(other):
            return Int(self.value + to_integer(other))
        return NotImplemented

    def __radd__(self, other: numbers.Complex) -> numbers.Complex:
        return self.__add__(other)

    def __mul__(self, other: numbers.Complex) -> numbers.Complex:
        if is_integral(other):
            return Int(self.value * to_integer(other))
        return NotImplemented

    def __rmul__(self, other: numbers.Complex) -> numbers.Complex:
        return self.__mul__(other)

    def __truediv__(self, other):
        if is_integral(other):
            other = to_integer(other)
            if self % other == 0:
                return self // other
            assert False, "TODO: implement"
        elif isinstance(other, Fraction):
            if self * other.denominator % other.numerator == 0:
                return self * other.denominator // other.numerator
            assert False, "TODO: implement"
        elif isinstance(other, float):
            if (self.value / other).is_integer():
                return Int(self.value / other)
            assert False, "TODO: implement"
        return NotImplemented

    def __rtruediv__(self, other):
        if is_integral(other):
            other = to_integer(other)
            if other % self == 0:
                return other // self
        return NotImplemented

    def __mod__(self, other) -> Self:
        if is_integral(other):
            return Int(self.value % to_integer(other))
        return NotImplemented

    def __rmod__(self, other) -> Self:
        if is_integral(other):
            return Int(to_integer(other)) % self
        return NotImplemented

    def __floordiv__(self, other) -> Self:
        if is_integral(other):
            return Int(self.value // to_integer(other))
        return NotImplemented

    def __rfloordiv__(self, other) -> Self:
        if is_integral(other):
            return Int(to_integer(other)) // self
        return NotImplemented

    def __pow__(self, exponent, modulus=None):  # TODO: consider modulus
        if self == 1:
            return Int(self)
        if self == 0 and exponent > 0:
            return Int()
        if self == 0 and is_integral(exponent) and to_integer(exponent) == 0:
            raise ValueError("0 ** 0 is undefined")
        if is_integral(exponent):
            return Int(self.value ** to_integer(exponent))
        if isinstance(exponent, Fraction):
            if self < 0:
                if not exponent.numerator == 1:
                    raise ValueError(f"Ambigious power: {self} ** {exponent}")
                if exponent.denominator % 2 == 0:
                    raise ValueError(f"Complex result: {self} ** {exponent}")
            if exponent.numerator == 1:
                v, succ = integer_nth_root(self.value, exponent.denominator)
                if succ:
                    return Int(v)
                assert False, "TODO: implement"
            sign = 1
            if self < 0:
                sign = -1
            v, succ = integer_nth_root(abs(self.value) ** exponent.numerator, exponent.denominator)
            if succ:
                return Int(v * sign**exponent.numerator)
            assert False, "TODO: implement"
        if isinstance(exponent, float):
            succ, val = is_exact_2_neg_power(exponent)
            if succ:
                return self ** Fraction(1, 2**val)
            assert False, "TODO: implement"
        return NotImplemented

    def __rpow__(self, base):
        if is_integral(base):
            return Int(base).__pow__(self)
        assert False, "TODO: implement"

    def __invert__(self) -> Self:
        return Int(~self.value)

    def __and__(self, other) -> Self:
        if is_integral(other):
            return Int(self.value & to_integer(other))
        return NotImplemented

    def __rand__(self, other) -> Self:
        return self.__and__(other)

    def __or__(self, other) -> Self:
        if is_integral(other):
            return Int(self.value | to_integer(other))
        return NotImplemented

    def __ror__(self, other) -> Self:
        return self.__or__(other)

    def __xor__(self, other) -> Self:
        if is_integral(other):
            return Int(self.value ^ to_integer(other))
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
        if ndigits is not None:
            assert False, "TODO: implement"
        return self

    def __eq__(self, other) -> bool:
        if not isinstance(other, numbers.Complex):
            return False
        if is_integral(other):
            return self.value == to_integer(other)
        return NotImplemented

    def __le__(self, other) -> bool:
        if is_integral(other):
            return self.value <= to_integer(other)
        return NotImplemented

    def __lt__(self, other) -> bool:
        if is_integral(other):
            return self.value < to_integer(other)
        return NotImplemented

    def __ge__(self, other) -> bool:
        if is_integral(other):
            return self.value >= to_integer(other)
        return NotImplemented

    def __gt__(self, other) -> bool:
        if is_integral(other):
            return self.value > to_integer(other)
        return NotImplemented

    def __lshift__(self, other) -> Self:
        if is_integral(other):
            return Int(self.value << to_integer(other))
        return NotImplemented

    def __rlshift__(self, other) -> Self:
        if is_integral(other):
            return Int(to_integer(other)) << self
        return NotImplemented

    def __rshift__(self, other):
        if is_integral(other):
            return Int(self.value >> to_integer(other))
        return NotImplemented

    def __rrshift__(self, other):
        if is_integral(other):
            return Int(to_integer(other)) >> self
        return NotImplemented

    def __repr__(self):
        return f"{self.value}"

    def __str__(self):
        return f"Int({self.value})"


def test_int_constructors():
    assert Int() == Int()
    assert Int() == int()
    assert Int(69) == 69
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
    assert Int(5) <= Fraction(5)
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


def test_closed_int_operations():
    result = Int(5) + Int(3)
    assert isinstance(result, Int)
    assert result == 8

    result = 5 + Int(3)
    assert isinstance(result, Int)
    assert result == 8

    result = Int(5) + 3
    assert isinstance(result, Int)
    assert result == 8

    result = Int(5) + 3.0
    assert isinstance(result, Int)
    assert result == 8

    result = 5.0 + Int(3)
    assert isinstance(result, Int)
    assert result == 8

    result = Int(5) + Fraction(3)
    assert isinstance(result, Int)
    assert result == 8

    result = Fraction(5) + Int(3)
    assert isinstance(result, Int)
    assert result == 8

    result = Int(5) - Int(3)
    assert isinstance(result, Int)
    assert result == 2

    result = 5 - Int(3)
    assert isinstance(result, Int)
    assert result == 2

    result = Int(5) - 3
    assert isinstance(result, Int)
    assert result == 2

    result = 5.0 - Int(3)
    assert isinstance(result, Int)
    assert result == 2

    result = Int(5) - 3.0
    assert isinstance(result, Int)
    assert result == 2

    result = Fraction(5) - Int(3)
    assert isinstance(result, Int)
    assert result == 2

    result = Int(5) - Fraction(3)
    assert isinstance(result, Int)
    assert result == 2

    result = Int(5) * Int(3)
    assert isinstance(result, Int)
    assert result == 15

    result = 5 * Int(3)
    assert isinstance(result, Int)
    assert result == 15

    result = Int(5) * 3
    assert isinstance(result, Int)
    assert result == 15

    result = 5.0 * Int(3)
    assert isinstance(result, Int)
    assert result == 15

    result = Int(5) * 3.0
    assert isinstance(result, Int)
    assert result == 15

    result = Fraction(5) * Int(3)
    assert isinstance(result, Int)
    assert result == 15

    result = Int(5) * Fraction(3)
    assert isinstance(result, Int)
    assert result == 15

    result = Int(5) // Int(2)
    assert isinstance(result, Int)
    assert result == 2

    result = 5 // Int(2)
    assert isinstance(result, Int)
    assert result == 2

    result = Int(5) // 2
    assert isinstance(result, Int)
    assert result == 2

    result = 5.0 // Int(2)
    assert isinstance(result, Int)
    assert result == 2

    result = Int(5) // 2.0
    assert isinstance(result, Int)
    assert result == 2

    result = Fraction(5) // Int(2)
    assert isinstance(result, Int)
    assert result == 2

    result = Int(5) // Fraction(2)
    assert isinstance(result, Int)
    assert result == 2

    with pytest.raises(ZeroDivisionError):
        Int(5) // Int(0)
    with pytest.raises(ZeroDivisionError):
        Int(5) // 0
    with pytest.raises(ZeroDivisionError):
        Int(5) // 0.0
    with pytest.raises(ZeroDivisionError):
        Int(5) // Fraction(0)

    result = Int(5) % Int(2)
    assert isinstance(result, Int)
    assert result == 1

    result = 5 % Int(2)
    assert isinstance(result, Int)
    assert result == 1

    result = Int(5) % 2
    assert isinstance(result, Int)
    assert result == 1

    result = Int(5) % 2.0
    assert isinstance(result, Int)
    assert result == 1

    result = 5.0 % Int(2)
    assert isinstance(result, Int)
    assert result == 1

    result = Fraction(5) % Int(2)
    assert isinstance(result, Int)
    assert result == 1

    result = Int(5) % Fraction(2)
    assert isinstance(result, Int)
    assert result == 1

    with pytest.raises(ZeroDivisionError):
        Int(5) % Int(0)
    with pytest.raises(ZeroDivisionError):
        Int(5) % 0
    with pytest.raises(ZeroDivisionError):
        Int(5) % 0.0
    with pytest.raises(ZeroDivisionError):
        Int(5) % Fraction(0)

    result = Int(-1) % Int(5)
    assert isinstance(result, Int)
    assert result == 4

    result = -1 % Int(5)
    assert isinstance(result, Int)
    assert result == 4

    result = -1.0 % Int(5)
    assert isinstance(result, Int)
    assert result == 4

    result = Fraction(-1) % Int(5)
    assert isinstance(result, Int)
    assert result == 4

    result = Int(4) / Int(2)
    assert isinstance(result, Int)
    assert result == 2

    result = 4 / Int(2)
    assert isinstance(result, Int)
    assert result == 2

    result = Int(4) / 2
    assert isinstance(result, Int)
    assert result == 2

    result = Int(5) / Fraction(5, 2)
    assert isinstance(result, Int)
    assert result == 2

    result = Fraction(5) / Int(1)
    assert isinstance(result, Int)
    assert result == 5

    result = Int(15) / 3.0
    assert isinstance(result, Int)
    assert result == 5

    result = Int(15) / 1.5
    assert isinstance(result, Int)
    assert result == 10

    result = 15.0 / Int(3)
    assert isinstance(result, Int)
    assert result == 5

    with pytest.raises(ZeroDivisionError):
        Int(5) / Int(0)

    result = Int(2) ** Int(3)
    assert isinstance(result, Int)
    assert result == 8

    result = 2 ** Int(3)
    assert isinstance(result, Int)
    assert result == 8

    result = Int(2) ** 3
    assert isinstance(result, Int)
    assert result == 8

    result = Int(2) ** 3.0
    assert isinstance(result, Int)
    assert result == 8

    result = Int(2) ** Fraction(3)
    assert isinstance(result, Int)
    assert result == 8

    result = Int(1) ** -1
    assert isinstance(result, Int)
    assert result == 1

    result = Int(1) ** Fraction(6, 25)
    assert isinstance(result, Int)
    assert result == 1

    result = Int(1) ** 0.45234543
    assert isinstance(result, Int)
    assert result == 1

    result = Int(9) ** Fraction(1, 2)
    assert isinstance(result, Int)
    assert result == 3

    result = Int(27) ** Fraction(2, 3)
    assert isinstance(result, Int)
    assert result == 9

    result = Int(4) ** 0.5
    assert isinstance(result, Int)
    assert result == 2

    result = Int(-8) ** Fraction(1, 3)
    assert isinstance(result, Int)
    assert result == -2

    with pytest.raises(ValueError):
        result = Int(-32) ** Fraction(2, 5)

    with pytest.raises(ValueError):
        Int(0) ** Int(0)
    with pytest.raises(ValueError):
        Int(0) ** 0
    with pytest.raises(ValueError):
        0 ** Int(0)

    result = Int(0) ** Int(5)
    assert isinstance(result, Int)
    assert result == 0

    result = Int(0) ** 5
    assert isinstance(result, Int)
    assert result == 0

    result = 0 ** Int(5)
    assert isinstance(result, Int)
    assert result == 0

    result = Int(5) ** Int(0)
    assert isinstance(result, Int)
    assert result == 1

    result = Int(5) ** 0
    assert isinstance(result, Int)
    assert result == 1

    result = 5 ** Int(0)
    assert isinstance(result, Int)
    assert result == 1

    result = Int(-5) ** Int(0)
    assert isinstance(result, Int)
    assert result == 1

    result = Int(-5) ** 0
    assert isinstance(result, Int)
    assert result == 1

    result = (-5) ** Int(0)
    assert isinstance(result, Int)
    assert result == 1

    assert Int(10) ** 100 == 10**100

    with pytest.raises(ZeroDivisionError):
        Int(0) ** -1

    result = -Int(5)
    assert isinstance(result, Int)
    assert result == -5

    result = -Int(-5)
    assert isinstance(result, Int)
    assert result == 5

    result = +Int(5)
    assert isinstance(result, Int)
    assert result == 5

    result = abs(Int(-5))
    assert isinstance(result, Int)
    assert result == 5

    result = abs(Int(5))
    assert isinstance(result, Int)
    assert result == 5

    result = Int(5) & Int(3)
    assert isinstance(result, Int)
    assert result == 1

    result = 5 & Int(3)
    assert isinstance(result, Int)
    assert result == 1

    result = Int(5) & 3
    assert isinstance(result, Int)
    assert result == 1

    result = 5.0 & Int(3)
    assert isinstance(result, Int)
    assert result == 1

    result = Int(5) & 3.0
    assert isinstance(result, Int)
    assert result == 1

    result = Fraction(5) & Int(3)
    assert isinstance(result, Int)
    assert result == 1

    result = Int(5) & Fraction(3)
    assert isinstance(result, Int)
    assert result == 1

    result = Int(5) | Int(3)
    assert isinstance(result, Int)
    assert result == 7

    result = Int(5) | 3
    assert isinstance(result, Int)
    assert result == 7

    result = 5 | Int(3)
    assert isinstance(result, Int)
    assert result == 7

    result = 5.0 | Int(3)
    assert isinstance(result, Int)
    assert result == 7

    result = Int(5) | 3.0
    assert isinstance(result, Int)
    assert result == 7

    result = Fraction(5) | Int(3)
    assert isinstance(result, Int)
    assert result == 7

    result = Int(5) | Fraction(3)
    assert isinstance(result, Int)
    assert result == 7

    result = Int(5) ^ Int(3)
    assert isinstance(result, Int)
    assert result == 6

    result = 5 ^ Int(3)
    assert isinstance(result, Int)
    assert result == 6

    result = Int(5) ^ 3
    assert isinstance(result, Int)
    assert result == 6

    result = 5.0 ^ Int(3)
    assert isinstance(result, Int)
    assert result == 6

    result = Int(5) ^ 3.0
    assert isinstance(result, Int)
    assert result == 6

    result = Fraction(5) ^ Int(3)
    assert isinstance(result, Int)
    assert result == 6

    result = Int(5) ^ Fraction(3)
    assert isinstance(result, Int)
    assert result == 6

    result = Int(5) << Int(1)
    assert isinstance(result, Int)
    assert result == 10

    result = 5 << Int(1)
    assert isinstance(result, Int)
    assert result == 10

    result = Int(5) << 1
    assert isinstance(result, Int)
    assert result == 10

    result = 5.0 << Int(1)
    assert isinstance(result, Int)
    assert result == 10

    result = Int(5) << 1.0
    assert isinstance(result, Int)
    assert result == 10

    result = Fraction(5) << Int(1)
    assert isinstance(result, Int)
    assert result == 10

    result = Int(5) << Fraction(1)
    assert isinstance(result, Int)
    assert result == 10

    result = Int(5) >> Int(1)
    assert isinstance(result, Int)
    assert result == 2

    result = 5 >> Int(1)
    assert isinstance(result, Int)
    assert result == 2

    result = Int(5) >> 1
    assert isinstance(result, Int)
    assert result == 2

    result = 5.0 >> Int(1)
    assert isinstance(result, Int)
    assert result == 2

    result = Int(5) >> 1.0
    assert isinstance(result, Int)
    assert result == 2

    result = Fraction(5) >> Int(1)
    assert isinstance(result, Int)
    assert result == 2

    result = Int(5) >> Fraction(1)
    assert isinstance(result, Int)
    assert result == 2

    result = ~Int(5)
    assert isinstance(result, Int)
    assert result == -6

    with pytest.raises(ValueError):
        Int(5) << Int(-1)
    with pytest.raises(ValueError):
        Int(5) >> Int(-1)

    result = Int(-5) + Int(-3)
    assert isinstance(result, Int)
    assert result == -8

    result = Int(-5) - Int(-3)
    assert isinstance(result, Int)
    assert result == -2

    result = Int(-5) * Int(-3)
    assert isinstance(result, Int)
    assert result == 15

    result = Int(-5) // Int(2)
    assert isinstance(result, Int)
    assert result == -3

    result = Int(-5) % Int(2)
    assert isinstance(result, Int)
    assert result == 1

    huge = 10**100
    result = Int(huge) + Int(huge)
    assert isinstance(result, Int)
    assert result == 2 * huge

    result = Int(huge) * Int(2)
    assert isinstance(result, Int)
    assert result == 2 * huge

    result = math.floor(Int(69))
    assert isinstance(result, Int)
    assert result == 69

    result = math.ceil(Int(69))
    assert isinstance(result, Int)
    assert result == 69

    result = round(Int(69))
    assert isinstance(result, Int)
    assert result == 69

    result = math.trunc(Int(69))
    assert isinstance(result, Int)
    assert result == 69
