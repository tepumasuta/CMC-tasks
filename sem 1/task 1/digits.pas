program Digits;

var
  c: Char;
  numerical: Boolean;
  previous, current, following: Integer;

begin
  read(c);
  numerical := true;
  previous := -1;
  current := -1;
  following := -1;

  while c <> '.' do begin
    if ('0' <= c) and (c <= '9') then begin
      previous := current;
      current := following;
      following := ord(c) - ord('0');
      if previous >= 0 then
        if (previous + following) <> (2 * current) then
          numerical := false;
    end else
      numerical := false;
    read(c);
  end;

  if numerical then WriteLn('YES') else WriteLn('NO');
end.
