program Minmax;

var
  size, max_i, min_i, cur_i: Integer;
  numbers: array[1..100] of Real;
  cur_max, cur_min, tmp: Real;

begin
  Read(size);

  max_i := -1;
  min_i := -1;
  cur_max := 0.0;
  cur_min := 0.0;

  for cur_i := 1 to size do numbers[cur_i] := 0.0;

  for cur_i := 1 to size do begin
    Read(numbers[cur_i]);
    if (max_i = -1) or (cur_max < numbers[cur_i]) then begin
      cur_max := numbers[cur_i];
      max_i := cur_i;
    end;
    if (min_i = -1) or (cur_min > numbers[cur_i]) then begin
      cur_min := numbers[cur_i];
      min_i := cur_i;
    end;
  end;

  tmp := numbers[min_i];
  numbers[min_i] := numbers[max_i];
  numbers[max_i] := tmp;

  for cur_i := 1 to size do Write(numbers[cur_i], ' ');
  Writeln;

end.

