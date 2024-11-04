program Symetrie;

const MAX_COUNT = 100;

var
  row: array[1..MAX_COUNT] of integer;
  b: array[1..MAX_COUNT] of integer;
  rows, cols: 1..MAX_COUNT;
  i, j: integer;
  symmetric: boolean;

begin
  read(rows, cols);
  readln;
  for i := 1 to rows do begin
    for j := 1 to cols do read(row[j]);

    symmetric := true;
    for j := 1 to cols div 2 do symmetric := symmetric and (row[j] = row[cols - j + 1]);

    if symmetric then b[i] := 1 else b[i] := 0;
  end;

  for i := 1 to rows do write(b[i], ' ');
  writeln;
end.
