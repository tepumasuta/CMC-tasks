program Length;

label die, TheEnd;

var
  sym: Char;
  was: Boolean;
  digit, len: Integer;

begin
  len := 0;

  read(sym);
  while sym <> '.' do begin
    if (ord('0') <= ord(sym)) and (ord(sym) <= ord('9')) then
      if was then goto die else begin was := true; digit := ord(sym) - ord('0')
    end;
    len := len + 1;
    read(sym);
  end;

  if digit <> len then goto die;
  WriteLn('YES');
  goto TheEnd;
  die:
    WriteLn('NO');
  TheEnd:
end.