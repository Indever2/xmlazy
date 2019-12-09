defmodule Xmlazy.Properties do
  def parse(properties) when is_binary(properties) do
    get_next(String.codepoints(properties), [], [], :not_in_brackets)
    |> Enum.map(fn x -> property_string_to_tuple(x) end)
    |> Enum.sort
  end
  def parse(nil) do
    nil
  end

  def property_string_to_tuple(string) when is_binary(string) do
    [name, value] = String.split(string, "=")
    {:property, name, value}
  end

  defp get_next([], [], result, _) do
    result
  end
  defp get_next([], acc, result, _) do
    result ++ [acc]
  end
  defp get_next(["\""| remains], acc, result, :not_in_brackets) do
    get_next(remains, acc, result, :in_brackets)
  end
  defp get_next(["\""| remains], acc, result, :in_brackets) do
    get_next(remains, [], result ++ [to_string(acc)], :not_in_brackets)
  end
  defp get_next([" "| remains], acc, result, :not_in_brackets) do
    get_next(remains, acc, result, :not_in_brackets)
  end
  defp get_next([ch| remains], acc, result, :not_in_brackets) do
    get_next(remains, acc ++ [ch], result, :not_in_brackets)
  end
  defp get_next([ch| remains], acc, result, :in_brackets) do
    get_next(remains, acc ++ [ch], result, :in_brackets)
  end
end
