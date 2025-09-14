from task15 import hello, int_to_roman, longest_common_prefix, BankCard, primes
import itertools

print(hello()) # Hello!
print(hello('')) # Hello!
print(hello('Masha')) # Hello, Masha!

print(int_to_roman(3)) # III
print(int_to_roman(54)) # LIV
print(int_to_roman(1996)) # MCMXCVI

print(longest_common_prefix(["flo","flow","flight"])) # fl
print(longest_common_prefix(["dog","racecar","car"])) #
print(longest_common_prefix([" flo","flow","fl"])) # f

a = BankCard(10, 2)
print(a.balance) # 10
print(a.balance_limit) # 1
a(5) # You spent 5 dollars.
print(a.total_sum) # 5
print(a) # To learn the balance call balance.
print(a.balance) # 5
try:
    a(6) # Not enough money to spend 6 dollars.
except ValueError:
    pass
a(5) # You spent 5 dollars.
try:
    a.balance # Balance check limits exceeded.
except ValueError:
    pass
a.put(2) # You put 2 dollars.
print(a.total_sum) # 2

for i in primes():
    print(i)
    if i > 3:
        break

l = itertools.takewhile(lambda x : x <= 17, primes())
print(list(l)) # [2, 3, 5, 7, 11, 13, 17]
