program Sign;

var
  count, number, prev: Integer;

begin
  count := 0;
  read(prev, number);

  while number <> 0 do begin
    if (prev * number) < 0 then count := count + 1;
    prev := number;
    read(number);
  end;

  WriteLn(count);
end.
