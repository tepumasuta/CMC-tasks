program Count;

const MAX_SIZE = 1000;

var
  i, j, distinct_count: integer;
  c: char;
  keep: boolean;
  size: 1..MAX_SIZE;
  distinct: array[0..255] of char;

begin
  for i := 0 to 255 do distinct[i] := ' ';
  distinct_count := 0;

  readln(size);
  for i := 0 to size-1 do begin
    read(c);
    keep := true;
    for j := 0 to distinct_count-1 do
      if distinct[j] = c then keep := false;
    if keep then begin
      distinct[distinct_count] := c;
      distinct_count := distinct_count + 1;
    end;
  end;

  WriteLn(distinct_count);
end.
