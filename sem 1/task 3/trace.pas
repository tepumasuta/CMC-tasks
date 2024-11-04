program FUPASCALTrace; { Иначе оно бы заняло драгоценное имя Trace для функции }

type
  InputRange = 1..100;
  Matrix = array[InputRange] of array[InputRange] of Real;

var
  A, B: Matrix;
  size: InputRange;

procedure ReadMatrix(var A: Matrix);
var i, j: Integer;
begin
  for i := 1 to size do
    for j := 1 to size do
      Read(A[i, j]);
end;

procedure PrintMatrix(var A: Matrix);
var i, j: Integer;
begin
  for i := 1 to size do begin
    for j := 1 to size do
      Write(A[i, j]:1:6, ' ');
    WriteLn;
  end;
end;

procedure MatMul(var A, B, C: Matrix);
var i, j, k: Integer;
begin
  for i := 1 to size do
    for j := 1 to size do begin
      C[i, j] := 0;
      for k := 1 to size do
        C[i, j] := C[i, j] + A[i, k] * B[k, j];
    end;
end;

function Trace(var A: Matrix): Real;
var i: Integer; sum: Real;
begin
  sum := 0;
  for i := 1 to size do
    sum := sum + A[i, i];
  Trace := sum;
end;

begin
  Read(size);
  ReadMatrix(A);
  ReadMatrix(B);
  if Trace(A) < Trace(B) then begin
    MatMul(A, A, B);
    PrintMatrix(B);
  end else begin
    MatMul(B, B, A);
    PrintMatrix(A);
  end;
end.
