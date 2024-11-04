program FUPASCALMergeOr;

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

procedure merge_or(var l1, l2, l: List);
var tmp, p, preTail: List; found: Boolean;
begin
  l := l1;
  if l1 <> nil then begin
    l1 := l1^.next;
    l^.next := nil;
  end;

  p := l2;
  if p <> nil then
    while p^.next <> nil do
      p := p^.next;
  
  if p <> nil then p^.next := l1
  else l2 := l1;
  l1 := nil;

  if l2 <> nil then begin
    if l = nil then begin
      l := l2;
      l2 := l2^.next;
      l^.next := nil;
    end;

    preTail := l;
    while preTail^.next <> nil do preTail := preTail^.next;

    while l2 <> nil do begin
      found := false;
      p := l;
      while p <> nil do begin
        if p^.val = l2^.val then found := true;
        p := p^.next;
      end;

      if not found then begin
        preTail^.next := l2;
        l2 := l2^.next;
        preTail := preTail^.next;
        preTail^.next := nil;
      end else begin
        p := l2;
        l2 := l2^.next;
        dispose(p);
      end;
    end;
  end;
end;

begin
  Read(c1, c2);
  ReadList(l1, c1);
  ReadList(l2, c2);
  merge_or(l1, l2, l);
  PrintList(l);
  WriteLn;
  DisposeList(l);
end.
