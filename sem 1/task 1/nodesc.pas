program NoDesc;
label
  brexit;
var
  count, previous, current: Integer;
  i: Integer;
  nonDescending: Boolean;

begin
  previous := -32000;
  nonDescending := true;

  readln(count);
  for i := 1 to count do begin
    read(current);
    if current < previous then begin
      nonDescending := false;
      goto brexit;
    end else
      previous := current;
  end;

brexit:
  if nonDescending then
    WriteLn('YES')
  else
    WriteLn('NO')
end.