program Magic;

label STOP;

const MAX_SIZE = 100;

var
  square: array[1..MAX_SIZE, 1..MAX_SIZE] of integer;
  size: 1..MAX_SIZE;
  i, j, k, s1, s2: integer;
  magical: boolean;

begin
  readln(size);

  for i := 1 to size do
    for j := 1 to size do
      read(square[i, j]);
  
  magical := true;

  for i := 1 to size do
    for j := 1 to size do begin
      s1 := 0;
      s2 := 0;
      for k := 1 to size do begin
        s1 := s1 + square[i, k];
        s2 := s2 + square[k, j];
      end;
      if s1 <> s2 then begin magical := false; goto STOP end;
    end;

STOP:
  if magical then writeln('YES') else writeln('NO');
end.
