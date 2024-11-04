program Seqsum;

var
  max, total: LongInt;
  current: Integer;

begin
  max := -1;
  total := 0;

  read(current);
  while current <> 0 do begin
    total += current;
    if current > max then
      max := current;
    read(current);
  end;

  if total - max = max then WriteLn('YES') else WriteLn('NO');
end.
