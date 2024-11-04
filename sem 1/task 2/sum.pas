program Sum;

type
  Indicies = 1..100;

var
  size: Indicies;
  i: Indicies;
  max_i, min_i, tmp: Integer;
  numbers: array[Indicies] of Integer;
  total, cur_max, cur_min: Integer;

begin
  Read(size);

  for i := 1 to size do Read(numbers[i]);
  for i := size + 1 to 100 do numbers[i] := 0;
  
  max_i := -1;
  min_i := -1;
  total := 0;
  cur_max := 0;
  cur_min := 0;

  
  for i := 1 to size do begin
    if (max_i = -1) or (cur_max < numbers[i]) then begin
      cur_max := numbers[i];
      max_i := i;
    end;
    if (min_i = -1) or (cur_min > numbers[i]) then begin
      cur_min := numbers[i];
      min_i := i;
    end;
  end;

  if min_i > max_i then begin
    tmp := min_i;
    min_i := max_i;
    max_i := tmp;
  end;

  for i := min_i to max_i do total := total + numbers[i];

  WriteLn(total);
end.
