program funcaoSimples;
  var n: integer;

  // retorna true se a eh divisivel por b
  function divisivel(a: integer; b: integer): boolean;
  begin
    while a > 0 do
      a := a - b;
    divisivel := (a = 0);
  end;

  // retorna true se n eh primo
  function primo(n: integer): boolean;
    var i, x: integer;
        p: boolean;
  begin
    p := true;
    i := 2;
    while (i <= n div 2) and p do
    begin
      if divisivel(n, i) then
        p := false;
      i := i + 1;
    end;
    primo := p;
  end;

begin
  read(n);
  write(primo(n));
end.