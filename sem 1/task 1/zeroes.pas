program Zeroes;

var
  count, number, current, max: Integer;
  i: Integer;

begin
  current := 0;
  max := 0;

  readln(count);
  for i := 1 to count do begin
    read(number);
    if number = 0 then begin
      current := current + 1;
      if current > max then
        max := current;
    end else
      current := 0;
  end;

  WriteLn(max);
end.