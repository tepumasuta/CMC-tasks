import matrix

from typing import Iterable
import dataclasses
import math


class Gaussian:
    @dataclasses.dataclass(slots=True, frozen=True)
    class ForwardStepInfo[T]:
        col_index: int
        after_swap: Iterable[T]
        row_element: int
        valid: bool

        def apply[T](self, mat: "matrix.Matrix[T]"):
            mat.swap_rows(self.col_index, self.row_element)
            for i, v in zip(range(self.col_index + 1, mat.rows), self.after_swap[1:]):
                mat.row[i] -= mat.row[self.col_index] * v
            mat.row[self.col_index] *= self.after_swap[0]

    def next_forward_step[T](self, mat: "matrix.Matrix[T]", last_step=None) -> ForwardStepInfo[T] | None:
        if mat.cols != mat.rows:
            assert False, "TODO: raise ValueError"
        if last_step is None:
            last_step = self.deduce_last_step(mat)
            if last_step is None:
                return Gaussian.ForwardStepInfo(-1, (), -1, False)
        next_step = last_step + 1
        if next_step == mat.cols:
            return None
        col = tuple(mat.col[next_step])[next_step:]
        value, row_index = max(
            ((v, i) for i, v in enumerate(col, next_step) if v != 0),
            key=lambda x: x[0],
            default=(None, -1),
        )
        if row_index == -1:
            return Gaussian.ForwardStepInfo(-1, (), -1, False)
        multipliers = [v / value for v in col]
        multipliers[row_index - next_step], multipliers[0] = multipliers[0], multipliers[row_index - next_step]
        multipliers[0] = 1 / value
        return Gaussian.ForwardStepInfo(next_step, multipliers, row_index, True)

    def deduce_last_step[T](self, mat: "matrix.Matrix[T]") -> int | None:
        if mat.cols != mat.rows:
            assert False, "TODO: raise ValueError"
        for j in range(mat.cols):
            if mat[j, j] != 1 or not all(mat[i, j] == 0 for i in range(j + 1, mat.rows)):
                return j - 1
        return j


def test_gaussian_forward_process():
    gaussian = Gaussian()
    m = matrix.Matrix([[1, 3], [2, 4]])
    s = gaussian.next_forward_step(m)
    assert s.valid
    assert s.row_element == 1
    assert s.col_index == 0
    assert list(s.after_swap) == [0.5, 0.5]
    s.apply(m)
    assert m == matrix.Matrix([[1, 2], [0, 1]])
    s = gaussian.next_forward_step(m, s.col_index)
    assert s.valid
    assert s.row_element == 1
    assert s.col_index == 1
    assert list(s.after_swap) == [1]
    s.apply(m)
    assert m == matrix.Matrix([[1, 2], [0, 1]])
    s = gaussian.next_forward_step(m, s.col_index)
    assert s is None

    m = matrix.Matrix([[2, 0, 3], [6, 0, 11], [4, 0, 7]], convert_to_fractions=False)
    s = gaussian.next_forward_step(m)
    assert s.valid
    assert s.row_element == 1
    assert s.col_index == 0
    assert list(s.after_swap) == [1 / 6, 2 / 6, 4 / 6]
    s.apply(m)
    assert m == matrix.Matrix([[1, 0, 11 / 6], [0, 0, 3 - 22 / 6], [0, 0, 7 - 44 / 6]])
    s = gaussian.next_forward_step(m)
    assert not s.valid

    m = matrix.Matrix([[2, 1, 1], [4, 1, 2], [6, 3, 4]], convert_to_fractions=False)
    s = gaussian.next_forward_step(m)
    assert s.valid
    assert s.row_element == 2
    assert s.col_index == 0
    assert list(s.after_swap) == [1 / 6, 4 / 6, 2 / 6]
    s.apply(m)
    assert m == matrix.Matrix([[1, 0.5, 2 / 3], [0, -1, -2 / 3], [0, 0, -1 / 3]])
    s = gaussian.next_forward_step(m, s.col_index)
    assert s.valid
    assert s.row_element == 1
    assert s.col_index == 1
    assert list(s.after_swap) == [-1, 0]
    s.apply(m)
    assert m == matrix.Matrix([[1, 0.5, 2 / 3], [0, 1, 2 / 3], [0, 0, -1 / 3]])
    s = gaussian.next_forward_step(m, s.col_index)
    assert s.valid
    assert s.row_element == 2
    assert s.col_index == 2
    assert math.isclose(s.after_swap[0], -3) and len(s.after_swap) == 1
    s.apply(m)
    assert m == matrix.Matrix([[1, 0.5, 2 / 3], [0, 1, 2 / 3], [0, 0, 1]])
    s = gaussian.next_forward_step(m, s.col_index)
    assert s is None

    m = matrix.Matrix([[1, 0.5, 2 / 3], [0, -1, -2 / 3], [0, 0, -1 / 3]])
    s = gaussian.next_forward_step(m)
    assert s.valid
    assert s.row_element == 1
    assert s.col_index == 1
    assert list(s.after_swap) == [-1, 0]
    m = matrix.Matrix([[1, 0.5, 2 / 3], [0, 1, 2 / 3], [0, 0, -1 / 3]])
    s = gaussian.next_forward_step(m)
    assert s.valid
    assert s.row_element == 2
    assert s.col_index == 2
    assert list(s.after_swap) == [-3]
    m = matrix.Matrix([[1, 0.5, 2 / 3], [0, 1, 2 / 3], [0, 0, 1]])
    s = gaussian.next_forward_step(m)
    assert s is None

    m = matrix.Matrix([[2, 0.5, 2 / 3], [0, -1, -2 / 3], [0, 0, -1 / 3]])
    s = gaussian.next_forward_step(m)
    assert s.valid
    assert s.row_element == 0
    assert s.col_index == 0
    assert list(s.after_swap) == [0.5, 0, 0]
