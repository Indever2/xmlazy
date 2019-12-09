defmodule XmlazyTest do
  use ExUnit.Case
  doctest Xmlazy

  @simple "<hello>World</hello>"

  #test "simple parse test" do
  #  assert Xmlazy.parse(@simple) == %{"hello" => %{:value => "World"}}
  #end

  test "get_next open" do
    assert Xmlazy.Process.get_next(@simple) == {{:otag, "hello", nil}, "World</hello>" |> String.codepoints}
  end

  test "get_next open property" do
    assert Xmlazy.Process.get_next("<hello Handshake='true'>World</hello>") == {{:otag, "hello", "Handshake='true'"}, "World</hello>" |> String.codepoints}
  end

  test "get_next data" do
    assert Xmlazy.Process.get_next("hello</World>") == {{:data, "hello"}, "</World>" |> String.codepoints}
  end

  test "get_next close" do
    assert Xmlazy.Process.get_next("</World>") == {{:ctag, "World"}, []}
  end
end
