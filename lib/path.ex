 defmodule Xmlazy.Path do
  @doc """
  parse_path_string(path)

  - path - string that specifies the node(s) to get from XML

  format: node{property1=one,property2=two,property3=three,...}.subnode.sub-subnode{...}

  iex> Xmlazy.Path.parse_path_string("Object.Value{section=main}.Subsection{property=color,value=blue}")
  [
    {"Object", nil},
    {"Value", [{:property, "section", "main"}]},
    {"Subsection",
    [{:property, "property", "color"}, {:property, "value", "blue"}]}
  ]
  """
  def parse_path_string(path) when is_binary(path) do
    path_get_next([], String.to_charlist(path), [], :tag)
  end

  defp path_get_next(_result, [], [], :tag) do
    {:error, :empty_tag}
  end
  defp path_get_next(result, [], acc, :tag) do
    result ++ [process_token(acc)]
  end
  defp path_get_next(result, [], acc, :cond_end) do
    result ++ [process_token(acc)]
  end
  defp path_get_next(result, [?.|remains], acc, :tag) do
    path_get_next(result ++ [process_token(acc)], remains, [], :tag)
  end
  defp path_get_next(result, [?{|remains], acc, :tag) do
    path_get_next(result, remains, {acc, []}, :cond_started)
  end
  defp path_get_next(result, [h|remains], acc, :tag) do
    path_get_next(result, remains, acc ++ [h], :tag)
  end

  defp path_get_next(result, [?}|remains], acc, :cond_started) do
    path_get_next(result, remains, acc, :cond_end)
  end
  defp path_get_next(result, [h|remains], {tag, condition}, :cond_started) do
    path_get_next(result, remains, {tag, condition ++ [h]}, :cond_started)
  end
  defp path_get_next(result, [?.|remains], acc, :cond_end) do
    path_get_next(result ++ [process_token(acc)], remains, [], :tag)
  end
  defp path_get_next(result, [_|remains], acc, :cond_end) do
    path_get_next(result, remains, acc, :cond_end)
  end

  defp process_token({tag, condition}) do
    {
      to_string(tag),
      to_string(condition)
      |> String.split(",")
      |> Enum.map(fn x -> String.split(x, "=") end)
      |> Enum.map(fn [x, y] -> {:property, x, y} end)
      |> Enum.sort
    }
  end
  defp process_token(var), do: {to_string(var), nil}

  def get_path(parsed_xml, path) when (is_tuple(parsed_xml) or is_list(parsed_xml)) and is_binary(path) do
    get_path(parsed_xml, parse_path_string(path))
  end
  def get_path(parsed_xml, [last_node]) when (is_list(parsed_xml) or is_tuple(parsed_xml)) do
    get_nodes(parsed_xml, last_node)
  end
  def get_path(parsed_xml, [current| remains]) when is_tuple(parsed_xml) or is_list(parsed_xml) do
    get_path(get_nodes(parsed_xml, current), remains)
  end


  def get_nodes(
    {
      node,
      {:properties, node_properties},
      {:data, result}
    },
    {node, cond_properties}
  )
  do
    if condition_match?(cond_properties, node_properties) do
      Xmlazy.normalize_binary result
    else
      []
    end
  end
  def get_nodes(parsed_xml, {node, properties}) when is_list(parsed_xml) do
    result = Enum.filter(parsed_xml, fn item -> get_nodes(item, {node, properties}) != [] end)

    case result do
      [] -> []
      [{^node, {:properties, _}, {:data, result}}] -> Xmlazy.normalize_binary result
      _ ->
        Enum.map(
          result,
          fn
            {^node, {:properties, _}, {:data, data}} -> Xmlazy.normalize_binary data
            list when is_list(list) -> Xmlazy.normalize_binary(Xmlazy.get_path(list, node))
          end
        )
    end
  end
  def get_nodes(_parsed_xml, _not_matched_condition) do
    []
  end

  def condition_match?(nil, _), do: true
  def condition_match?(_, nil), do: false
  def condition_match?(conditions, node_properties) when is_list(conditions) and is_list(node_properties) do
    Xmlazy.Lib.is_subset_of(conditions, node_properties)
  end
end
