program Scalar;

type
  Vector = array[1..100] of Real;

var
  a, b, c: Vector;
  size: 1..100;
  t, a_min, b_min, c_min: Real;

function dot(var x, y: Vector; size: Integer): Real;
var sum: Real; i: 1..100;
begin
  sum := 0;
  for i := 1 to size do sum := sum + x[i] * y[i];
  dot := sum;
end;

function Solve(var x, y, z: Vector; size: Integer): Real;
begin
  Solve := dot(x, x, size) - dot(y, z, size);
end;

procedure ReadVectorMin(var x: Vector; var x_min: Real; size: Integer);
var i: 1..100;
begin
  Read(x[1]);
  x_min := x[1];
  for i := 2 to size do begin
    Read(x[i]);
    if x[i] < x_min then x_min := x[i];
  end;
end;

begin
  Read(size);
  ReadVectorMin(a, a_min, size);
  ReadVectorMin(b, b_min, size);
  ReadVectorMin(c, c_min, size);

  if (a_min > b_min) and (a_min > c_min) then
    t := Solve(a, b, c, size)
  else if (b_min > a_min) and (b_min > c_min) then
    t := Solve(b, a, c, size)
  else
    t := Solve(c, a, b, size);

  WriteLn(t:1:6);
end.
