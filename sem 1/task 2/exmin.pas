program Exmin;

var
  first_size, second_size, min: Integer;
  first_seq, second_seq: array[1..100] of Integer;
  i: Integer;

function Has(x, size: Integer; var seq: array of Integer): Boolean;
var
  i: Integer;
  found: Boolean;
begin
  found := false;
  for i := 0 to size - 1 do found := found or (seq[i] = x);
  Has := found;
end;

procedure ReadArray(size: Integer; var seq: array of Integer);
var
  i: Integer;
begin
  for i := 0 to size-1 do Read(seq[i]);
  for i := size to 99 do seq[i] := 0;
end;

begin
  Read(first_size, second_size);
  ReadArray(first_size, first_seq);
  ReadArray(second_size, second_seq);

  min := 32001;
  for i := 1 to first_size do
    if (not Has(first_seq[i], second_size, second_seq)) and (first_seq[i] < min) then
      min := first_seq[i];
  
  WriteLn(min);
end.
