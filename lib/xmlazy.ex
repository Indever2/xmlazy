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
    #|> IO.inspect
    |> Xmlazy.Process.wrap
  end

  def test() do
    {:ok, file} = File.open("\\\\172.25.1.233\\Share\\1C_Integration\\temp\\test.xml")
    IO.binread(file, :all)
    |> parse
  end

  def get_path(parsed, path) when (is_tuple(parsed) or is_list(parsed)) and is_binary(path) do
    get_path(parsed, String.split(path, "."))
  end
  def get_path(parsed, [last_node]) when (is_list(parsed) or is_tuple(parsed)) do
    get_nodes(parsed, last_node)
  end
  def get_path(parsed, [current| remains]) when is_tuple(parsed) or is_list(parsed) do
    get_path(get_nodes(parsed, current), remains)
  end

  def get_nodes(parsed, condition) when is_tuple(parsed) and is_binary(condition) do
    {node, property} = IO.inspect parse_condition(condition)

    IO.inspect parsed
    IO.puts "\n"
    if property == nil do
      case parsed do
        {
          ^node,
          {:properties, _},
          {:data, result}
        } ->
          result
        _ ->
          []
      end
    else
      case parsed do
        {
          ^node,
          {:properties, ^property},
          {:data, result}
        } ->
          result
        _ ->
          []
      end
    end
  end
  def get_nodes(parsed, condition) when is_list(parsed) and is_binary(condition) do
    {node, property} = parse_condition(condition)

    result =
    if property == nil do
      Enum.filter(parsed, fn {node_name, {:properties, _props}, {:data, _data}} -> node == node_name end)
    else
      Enum.filter(parsed, fn {node_name, {:properties, props}, {:data, _data}} -> node == node_name and property == props end)
    end

    case result do
      [] -> []
      [{^node, {:properties, _}, {:data, result}}] -> result
      _ ->
        Enum.map(result, fn {^node, {:properties, _}, {:data, data}} -> data end)
    end
  end

  def parse_condition(condition) when is_binary(condition) do
    case String.split(condition, "@") do
      [node, property] ->
        {node, Xmlazy.Properties.parse(property)}
      [node] ->
        {node, nil}
    end
  end
end
