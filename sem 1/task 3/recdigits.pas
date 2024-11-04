program Recdigits;

function Digits: Integer;
var next: Char;
begin
  Read(next);
  if next = '.' then Digits := 0
  else if ('0' <= next) and (next <= '9') then Digits := 1 + Digits()
  else Digits := Digits();
end;

begin
  WriteLn(Digits);
end.
