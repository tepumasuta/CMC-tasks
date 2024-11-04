program FUPASCALMergeAnd;

type
  Node = record val: Integer; next: ^Node end;
  List = ^Node;

var
  c1, c2: Integer;
  l, l1, l2: List;

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

function InList(l: List; element: Integer): Boolean;
var found: Boolean;
begin
  found := false;
  while (l <> nil) and not found do begin
    if l^.val = element then found := true;
    l := l^.next;
  end;
  InList := found;
end;

procedure merge_and(var l1, l2, l: List);
var tmp, preTail: List;
begin
  while (l1 <> nil) and not InList(l2, l1^.val) do begin
    tmp := l1;
    l1 := l1^.next;
    dispose(tmp);
  end;

  if l1 <> nil then begin
    l := l1;
    l1 := l1^.next;
    l^.next := nil;
    preTail := l;

    while l1 <> nil do
      if (not InList(l, l1^.val)) and InList(l2, l1^.val) then begin
        preTail^.next := l1;
        l1 := l1^.next;
        preTail := preTail^.next;
        preTail^.next := nil;
      end else begin
        tmp := l1;
        l1 := l1^.next;
        dispose(tmp);
      end;
  end;
end;

begin
  Read(c1, c2);
  ReadList(l1, c1);
  ReadList(l2, c2);
  merge_and(l1, l2, l);
  PrintList(l);
  WriteLn;
  DisposeList(l);
  DisposeList(l2);
  { assert(l1 = nil);
    Обнаружил, что в fpc есть assert-ы) Пользовался для тестов. }
end.
