program Letters;

var
  stats: array['a'..'z'] of Integer;
  c: Char;

begin
  for c := 'a' to 'z' do stats[c] := 0;
  Read(c);
  while c <> '.' do begin
    stats[c] := stats[c] + 1;
    Read(c);
  end;

  for c := 'a' to 'z' do
    if stats[c] = 1 then
      Write(c, ' ');
  WriteLn;
end.
