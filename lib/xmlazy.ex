defmodule Xmlazy do
  @moduledoc """
  Documentation for Xmlazy.
  """

  @doc """
  parse

  ## Examples

      iex> Xmlazy.parse("<hello>World!</hello>")
      {"hello", {:properties, nil}, {:data, "World!"}}

  """
  def parse(xml) when is_binary(xml) do
    Xmlazy.Process.parse(xml, [])
    |> Xmlazy.Process.cleanup
    |> Xmlazy.Process.wrap
  end

  def test() do
    {:ok, file} = File.open("test2.xml")
    data = IO.binread(file, :all)

    #Xmlazy.Niftest.hello("abcd абвг €")
    Xmlazy.Niftest.hello(data)
    #Xmlazy.Niftest.hello("<a>b</a>")
  end

  def test1() do
    {:ok, file} = File.open("test2.xml")

    data = IO.binread(file, :all)

    for _ <- Range.new(0, 1000) do
      data
      |> Xmlazy.Niftest.hello
    end
  end

  def test2() do
    {:ok, file} = File.open("test2.xml")

    data = IO.binread(file, :all)

    for _ <- Range.new(0, 1000) do
      data
      |> Xmlazy.Process.parse([])
    end
  end


  def get_path(parsed_xml, path) when is_binary(path) do
    Xmlazy.Path.get_path(parsed_xml, path)
  end

  def normalize_binary(binary) when is_binary(binary) do
    String.replace(binary, "&lt;", "<")
    |> String.replace("&gt;", ">")
  end
  def normalize_binary(non_binary) do
    non_binary
  end
end
