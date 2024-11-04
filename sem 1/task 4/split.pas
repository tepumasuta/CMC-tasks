program FUPASCALSplit;

type
  Node = record val: Integer; next: ^Node end;
  List = ^Node;

var
  count: Integer;
  l, l1, l2: List;


procedure PrintList(l: list);
begin
  while l <> nil do begin
    Write(l^.val, ' ');
    l := l^.next;
  end
end;

procedure ReadList(var l: List; count: Integer);
var head: List; i: Integer;
begin
  for i := 1 to count do begin
    new(head);
    read(head^.val);
    head^.next := l;
    l := head;
  end
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

function Split(var l, l1, l2: List): Boolean;
var tmp: List;
begin
  l1 := nil;
  l2 := nil;

  while l <> nil do begin
    tmp := l^.next;
    if l^.val > 0 then begin
      l^.next := l1;
      l1 := l;
    end else begin
      l^.next := l2;
      l2 := l;
    end;
    l := tmp;
  end;
end;

begin
  Read(count);
  ReadList(l, count);
  Split(l, l1, l2);
  Write('L1: ');
  PrintList(l1);
  WriteLn;
  Write('L2: ');
  PrintList(l2);
  WriteLn;
  DisposeList(l);
  DisposeList(l1);
  DisposeList(l2);
end.
