program Progress;

label TheEnd;

var
  count, current, previous, difference: Integer;
  i: Integer;
  arithmetic: Boolean;

begin
  arithmetic := true;
  read(count);
  if count = 1 then goto TheEnd;
  read(previous, current);
  difference := current - previous;
  for i := 3 to count do begin
    previous := current;
    read(current);
    if difference <> (current - previous) then
      arithmetic := false;
  end;


TheEnd:
  if arithmetic then WriteLn('YES') else WriteLn('NO');
end.