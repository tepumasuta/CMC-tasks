program Quarter;

type Bin = 0..1;

const MAX_SIZE = 100;

var
  matrix: array[1..MAX_SIZE, 1..MAX_SIZE] of Bin;
  size: 1..MAX_SIZE;
  i, j: integer;
  tmp: Bin;

begin
  read(size);

  for i := 1 to size do
    for j := 1 to size do
      read(matrix[i, j]);
  
  for i := 1 to size div 2 do
    for j := 1 to size div 2 + size mod 2 do begin
      { (x, y) -> (y, size - x + 1) }
      tmp := matrix[i, j];
      matrix[i, j] := matrix[size - j + 1, i];
      matrix[size - j + 1, i] := matrix[size - i + 1, size - j + 1];
      matrix[size - i + 1, size - j + 1] := matrix[j, size - i + 1];
      matrix[j, size - i + 1] := tmp;
  end;

  for i := 1 to size do begin
    for j := 1 to size do
      write(matrix[i, j], ' ');
    writeln;
  end;
end.
