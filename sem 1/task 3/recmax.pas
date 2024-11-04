program Recmax;

function Max(cur: Integer): Integer;
var next: Integer;
begin
  Read(next);
  if (next = 0) then Max := cur
  else if (next <= cur) then Max := Max(cur)
  else Max := Max(next);
end;

begin
  WriteLn(Max(-1));
end.
