program FUPASCALEqual;

type
  Node = record val: Integer; next: ^Node end;
  List = ^Node;

var
  c1, c2: Integer;
  l1, l2: List;

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

function Equal(l1, l2: List): Boolean;
begin
  while (l1 <> nil) and (l2 <> nil) and (l1^.val = l2^.val) do begin
    l1 := l1^.next;
    l2 := l2^.next;
  end;

  if (l1 = nil) and (l2 = nil) then Equal := true
  else Equal := false;
end;

begin
  Read(c1, c2);
  ReadList(l1, c1);
  ReadList(l2, c2);
  if Equal(l1, l2) then WriteLn('YES') else WriteLn('NO');
  DisposeList(l1);
  DisposeList(l2);
end.
