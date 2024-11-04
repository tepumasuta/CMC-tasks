program Prime;

label TheEnd;

var
  number, i: LongInt;
  isPrime: Boolean;

begin
  read(number);
  isPrime := number <> 1;
  i := 2;
  while i * i <= number do begin
    if (number mod i) = 0 then begin
      isPrime := false;
      goto TheEnd;
    end;
    i := i + 1;
  end;

TheEnd:
  if isPrime then WriteLn('YES') else WriteLn('NO');
end.