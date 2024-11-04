program Justrow;

const MAX_COUNT = 100;

var
  row: array[1..MAX_COUNT] of integer;
  b: array[1..MAX_COUNT] of integer;
  rows, cols: 1..MAX_COUNT;
  i, j, k: integer;
  unique: boolean;

begin
  read(rows, cols);
  readln;
  for i := 1 to rows do begin
    for j := 1 to cols do read(row[j]);

    unique := true;
    for j := 1 to cols-1 do
      for k := j + 1 to cols do
        if row[j] = row[k] then
          unique := false;

    if unique then b[i] := 0 else b[i] := 1;
  end;

  for i := 1 to rows do write(b[i], ' ');
  writeln;
end.
