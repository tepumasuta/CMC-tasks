program Various;

var
  count, previous, current, total: Integer;
  i: Integer;

begin
  previous := -32000;
  total := 0;

  readln(count);
  for i := 1 to count do begin
    read(current);
    if current <> previous then
      total += 1;
    previous := current;
  end;

  WriteLn(total);
end.