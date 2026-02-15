import matrix
import random_gen

import abc
import dataclasses


class LinearSystemSolver(abc.ABC):
    @dataclasses.dataclass
    class Solution:
        solution: 'matrix.Matrix'
        solvable: bool
    
    @abc.abstractmethod
    def solve(self, system: "matrix.Matrix", rhs: "matrix.Matrix") -> Solution: ...


def basic_test(solver: LinearSystemSolver):
    for _ in range(100):
        system = random_gen.generate_random_linear_system(size=4, low=-100, high=100)
        solution = solver.solve(matrix.Matrix.from_mat(system.system), matrix.Matrix.from_mat(system.rhs))
        assert solution.solution == system.solution
        assert solution.solvable
    for _ in range(100):
        system = random_gen.generate_random_linear_system(size=4, low=-100, high=100)
        solution = solver.solve(system.system, system.rhs)
        assert solution.solution == system.solution
        assert solution.solvable
    for _ in range(100):
        system = random_gen.generate_random_unsolvable_linear_system(size=4, low=-100, high=100)
        solution = solver.solve(system.system, system.rhs)
        assert not solution.solvable

def test_kronecker():
    basic_test(LinearSystemKronecker())
