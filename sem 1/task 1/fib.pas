program Fib;

var
  number, current, next, tmp: LongInt;

begin
  read(number);
  current := 0;
  next := 1;
  
  repeat
    tmp := current;
    current := next;
    next := tmp + next;
  until current >= number;

  if current = number then WriteLn('YES') else WriteLn('NO');
end.
