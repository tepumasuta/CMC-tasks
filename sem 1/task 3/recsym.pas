program Recsym;

type pstring = packed array [1..1000] of char;
var
  s: pstring;
  size, i, start, stop: Integer;

function sym(var s: pstring; i, j: Integer): Boolean;
begin
  if i > j then sym := true
  else sym := (s[i] = s[j]) and sym(s, i + 1, j - 1);
end;

begin
  ReadLn(size);
  for i := 1 to size do Read(s[i]);
  Read(start, stop);

  if sym(s, start, stop) then WriteLn('YES') else WriteLn('NO');
end.
