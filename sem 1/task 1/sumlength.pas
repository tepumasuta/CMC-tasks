program Sumlength;

label die, TheEnd;

var
  sym: Char;
  total, len: Integer;

begin
  len := 0;
  total := 0;

  read(sym);
  while sym <> '.' do begin
    if ('0' <= sym) and (sym <= '9') then
      total := total + ord(sym) - ord('0');
    len := len + 1;
    read(sym);
  end;

  if total <> len then goto die;
  WriteLn('YES');
  goto TheEnd;
  die:
    WriteLn('NO');
  TheEnd:
end.