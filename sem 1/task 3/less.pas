program FUPASCALLess;
var a, b: Integer;

function count(a: Integer): Integer;
var i, total: Integer;
begin
  total := 0;
  for i := 1 to a div 2 do if (a mod i) = 0 then total := total + 2;
  if ((a mod 2) = 1) and ((a mod (a div 2 + 1)) = 0) then total := total + 1;
  count := total;
end;

function less(a, b: Integer): Boolean;
begin
  less := count(a) < count(b);
end;

begin
  Read(a, b);
  if less(a, b) then WriteLn('YES') else WriteLn('NO');
end.
