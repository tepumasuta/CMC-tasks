from numbers import Integral, Complex
from fractions import Fraction


class Int(Integral):
    ...


def test_int_constructors():
    assert Int() == Int()
    assert Int() == int()
    assert Int(69) == 69
    assert 69 == Int(69)
    assert Int(69.0) == Int(69)
    assert Int(Fraction(42)) == Int(42)
