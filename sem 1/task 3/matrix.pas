program FUPASCALMatrix;

const MAXN = 1000;
type matrix = array[1..MAXN, 1..MAXN] of real;
var
  A, B: matrix;
  N1, M1, N2, M2: Integer;


procedure ReadMatrix(var A: Matrix; N, M: Integer);
var i, j: Integer;
begin
  for i := 1 to N do
    for j := 1 to M do
      Read(A[i, j]);
end;

procedure PrintMatrix(var A: Matrix; N, M: Integer);
var i, j: Integer;
begin
  for i := 1 to N do begin
    for j := 1 to M do
      Write(A[i, j]:1:6, ' ');
    WriteLn;
  end;
end;


procedure swap(var A, B: matrix; N1, M1, N2, M2: Integer);
var
  mxai, mxaj, mxbi, mxbj, i, j: Integer;
  tmp: Real;
begin
  mxai := 1;
  mxaj := 1;
  mxbi := 1;
  mxbj := 1;
  for i := 1 to N1 do
    for j := 1 to M1 do
      if A[mxai, mxaj] < A[i, j] then begin
        mxai := i;
        mxaj := j;
      end;
  for i := 1 to N2 do
    for j := 1 to M2 do
      if B[mxbi, mxbj] < B[i, j] then begin
        mxbi := i;
        mxbj := j;
      end;
  tmp := A[mxai, mxaj];
  A[mxai, mxaj] := B[mxbi, mxbj];
  B[mxbi, mxbj] := tmp;
end;

begin
  Read(N1, M1, N2, M2);
  ReadMatrix(A, N1, M1);
  ReadMatrix(B, N2, M2);
  swap(A, B, N1, M1, N2, M2);
  PrintMatrix(A, N1, M1);
  PrintMatrix(B, N2, M2);
end.
