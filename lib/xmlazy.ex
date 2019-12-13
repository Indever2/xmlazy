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
  def parse(xml) when is_binary(xml) or is_list(xml) do
    Xmlazy.Process.parse(xml, [])
    |> Xmlazy.Process.cleanup
    |> Xmlazy.Process.wrap
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
