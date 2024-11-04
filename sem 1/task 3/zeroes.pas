program Zeroes;

type Matrix = array[1..500] of array[1..500] of Integer;
var
  A, B, C: Matrix;
  N, M, ac, bc, cc: Integer;

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
      Write(A[i, j], ' ');
    WriteLn;
  end;
end;

function ZeroValuedRow(var A: Matrix; i, M: Integer): Boolean;
var j: Integer; zeroValued: Boolean;
begin
  zeroValued := true;
  for j := 1 to M do zeroValued := zeroValued and (A[i, j] = 0);
  ZeroValuedRow := zeroValued;
end;

function CountNullRows(var A: Matrix; N, M: Integer): Integer;
var i, total: Integer;
begin
  total := 0;
  for i := 1 to N do if ZeroValuedRow(A, i, M) then total := total + 1;
  CountNullRows := total;
end;

begin
  Read(N, M);
  ReadMatrix(A, N, M);
  ReadMatrix(B, N, M);
  ReadMatrix(C, N, M);
  ac := CountNullRows(A, N, M);
  bc := CountNullRows(B, N, M);
  cc := CountNullRows(C, N, M);

  if ((bc < ac) and (cc < ac)) then PrintMatrix(A, N, M);
  if ((ac < bc) and (cc < bc)) then PrintMatrix(B, N, M);
  if ((ac < cc) and (bc < cc)) then PrintMatrix(C, N, M);
  if ((bc = ac) and (cc < ac)) then begin PrintMatrix(A, N, M); PrintMatrix(B, N, M); end;
  if ((bc < ac) and (cc = ac)) then begin PrintMatrix(A, N, M); PrintMatrix(C, N, M); end;
  if ((ac < bc) and (cc = bc)) then begin PrintMatrix(B, N, M); PrintMatrix(C, N, M); end;
  if (ac = bc) and (bc = cc) then begin PrintMatrix(A, N, M); PrintMatrix(B, N, M); PrintMatrix(C, N, M); end;

end.