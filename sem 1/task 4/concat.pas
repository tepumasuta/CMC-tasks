program FUPASCALConcat;

type
  Node = record val: Integer; next: ^Node end;
  List = ^Node;

var
  c1, c2: Integer;
  l1, l2: List;

procedure PrintList(l: list);
begin
  while l <> nil do begin
    Write(l^.val, ' ');
    l := l^.next;
  end
end;

procedure ReadList(var l: List; count: Integer);
var tail: List; i: Integer;
begin
  if count > 0 then begin
    new(l);
    read(l^.val);
    l^.next := nil;
    tail := l;
    for i := 2 to count do begin
      new(tail^.next);
      read(tail^.next^.val);
      tail^.next^.next := nil;
      tail := tail^.next;
    end
  end;
end;

procedure DisposeList(l: list);
var tail: List;
begin
  while l <> nil do begin
    tail := l^.next;
    dispose(l);
    l := tail;
  end
end;

procedure Concat(var l1: List; l2: List);
var l, t: List;
begin
  l := nil;
  if l2 <> nil then begin
    new(l);
    t := l;
    l^.next := nil;
    t^.val := l2^.val;
    l2 := l2^.next;
  end;
  while l2 <> nil do begin
    new(t^.next);
    t^.next^.next := nil;
    t^.next^.val := l2^.val;
    t := t^.next;
    l2 := l2^.next;
  end;

  if l1 = nil then l1 := l
  else begin
    t := l1;
    while t^.next <> nil do begin
      t := t^.next;
    end;
    t^.next := l;
  end;
end;

begin
  Read(c1, c2);
  ReadList(l1, c1);
  ReadList(l2, c2);
  Concat(l1, l2);
  Write('L1: ');
  PrintList(l1);
  WriteLn;
  Write('L2: ');
  PrintList(l2);
  WriteLn;
  DisposeList(l1);
  DisposeList(l2);
end.
