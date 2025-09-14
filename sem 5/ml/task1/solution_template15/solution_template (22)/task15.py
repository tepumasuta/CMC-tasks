from itertools import count, takewhile

def hello(name: str | None = None) -> str:
    if not name: return 'Hello!'
    return f'Hello, {name}!'

def int_to_roman(num: int) -> str:
    assert 1 <= num <= 3999
    def odd_ones(digit: int, low: str, mid: str, high: str) -> str:
        return ('' if not digit
                else low * digit if digit < 4
                else low + mid if digit == 4
                else low + high if digit == 9
                else mid + low * (digit - 5))
    return ('M' * (num // 1000)
            + odd_ones(num % 1000 // 100, 'C', 'D', 'M')
            + odd_ones(num % 100 // 10, 'X', 'L', 'C')
            + odd_ones(num % 10, 'I', 'V', 'X'))

def longest_common_prefix(strings: list[str]) -> str:
    if not strings: return ''
    first = strings[0].lstrip()
    strings = zip(*map(str.lstrip, strings))
    return first[:len(tuple(takewhile(lambda x: len(set(x)) == 1, strings)))]

class BankCard:
    def __init__(self, total_sum: int, balance_limit: int | None = None):
        assert balance_limit is None or balance_limit >= 0
        self.total_sum = total_sum
        self._balance_limit = balance_limit
    
    @property
    def balance(self):
        if self._balance_limit is None: return self.total_sum
        if not self._balance_limit:
            print('Balance check limits exceeded.')
            raise ValueError()
        self._balance_limit -= 1
        return self.total_sum
    
    def __call__(self, sum_spent: int):
        if sum_spent > self.total_sum:
            print(f'Not enough money to spend {sum_spent} dollars.')
            raise ValueError()
        print(f'You spent {sum_spent} dollars.')
        self.total_sum -= sum_spent
    
    def __repr__(self):
        return "To learn the balance call balance."
    
    def put(self, sum_put: int):
        assert sum_put >= 0
        self.total_sum += sum_put
        print(f"You put {sum_put} dollars.")
    
    @property
    def balance_limit(self):
        return self._balance_limit
        
    def __add__(self, obj: 'BankCard'):
        return BankCard(self.total_sum + obj.total_sum, max(self.balance_limit, obj.balance_limit))

def primes():
    primes = [2]
    yield 2
    for num in count(2):
        if all(num % prime != 0 for prime in primes):
            primes.append(num)
            yield num
