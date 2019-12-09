defmodule Xmlazy.Process do
  def wrap(parsed) when is_list(parsed) do
    #IO.puts("1) parsed: #{inspect parsed}")
    wrap_reduce(nil, parsed, [], [])
  end
  def wrap_reduce(nil, [], [], [result]) do
    #IO.puts("2) result: #{inspect result}")
    result
  end
  def wrap_reduce(nil, [], [], result) do
    #IO.puts("3) result: #{inspect result}")
    result
  end
  def wrap_reduce(nil, [{:otag, _otag_value, _properties} = otag| remains] = to_process, [], []) when is_list(to_process) do
    #IO.puts("4) new otag: #{inspect otag_value}| remains: #{inspect remains}")
    wrap_reduce(otag, remains, [], [])
  end
  def wrap_reduce(nil, [{:otag, _otag_value, _properties} = otag| remains], [], result) do
    #IO.puts("5) new otag: #{inspect otag_value}| remains: #{inspect remains}| result: #{inspect result}")
    wrap_reduce(otag, remains, [], result)
  end
  def wrap_reduce({:otag, otag_value, props}, [{:ctag, otag_value}| remains] = parsed, processed, result) when is_list(parsed) do
    #IO.puts("6) ctag_found: #{inspect otag_value}| remains: #{inspect remains}| processed: #{inspect processed}| result: #{inspect result}")
    wrap_reduce(
      nil,
      remains,
      [],
      result ++ [
        {otag_value, {:properties, props}, {:data, wrap_reduce(nil, processed, [], [])}}
      ]
    )
  end
  def wrap_reduce({:otag, otag_value, props}, [{:data, data}, {:ctag, otag_value}| remains] = parsed, [], result) when is_list(parsed) do
    #IO.puts("7) simple block #{inspect otag_value}| parsed: #{inspect parsed} | result: #{inspect result}")
    wrap_reduce(nil, remains, [], result ++ [{otag_value, {:properties, props}, {:data, data}}])
  end
  def wrap_reduce(otag_value, [h| remains] = parsed, processed, result) when is_list(parsed) do
    #IO.puts("8) add_tag #{inspect h}| parsed: #{inspect remains} | processed: #{inspect processed} | result: #{inspect result}")
    wrap_reduce(otag_value, remains, processed ++ [h], result)
  end


  def parse({:error, _reason} = err, _), do: err
  def parse({whatever, []}, acc), do: acc ++ [whatever]
  def parse(xml, acc) when is_list(xml) or is_binary(xml) do
    parse(get_next(xml), acc)
  end
  def parse({{:otag, value, properties}, {:data, nil} = data, {:ctag, value} = ctag, remains}, acc) do
    parse(
      get_next(remains),
      acc ++ [{:otag, value, Xmlazy.Properties.parse(properties)}, data, ctag]
    )
  end
  def parse({{:otag, value, properties}, remains}, acc) do
    parse(
      get_next(remains),
      acc ++ [{:otag, value, Xmlazy.Properties.parse(properties)}]
    )
  end
  def parse({{:data, _value} = data, remains}, acc) do
    parse(get_next(remains), acc ++ [data])
  end
  def parse({{:ctag, _value} = tag, remains}, acc) do
    parse(get_next(remains), acc ++ [tag])
  end

  def get_next(""), do: {:error, :nodata}
  def get_next([]), do: {:error, :nodata}
  def get_next(xml) when is_binary(xml) or is_list(xml) do
    get_next_step(xml)
  end

  def get_next_step(xml) when is_list(xml) do
    get_next_step(xml, [], nil)
  end
  def get_next_step(xml) when is_binary(xml) do
    get_next_step(String.codepoints(xml), [], nil)
  end

  def get_next_step(["<", "/"| remains], [], nil) do
    get_next_step(remains, [], :ctag_opened)
  end
  def get_next_step(["<"| remains], [], nil) do
    get_next_step(remains, [], :otag_opened)
  end
  def get_next_step([h| remains], [], nil) do
    get_next_step(remains, [h], :data)
  end

  def get_next_step([], processed, :data) do
    {:error, "Unexpected end of term after: '#{to_string(processed)}'"}
  end

  def get_next_step(["<"| _] = remains, processed, :data) do
    {{:data, to_string(processed)}, remains}
  end
  def get_next_step([h| remains], processed, :data) do
    get_next_step(remains, processed ++ [h], :data)
  end

  def get_next_step([">"| remains], processed, :otag_opened) do
    {{:otag, to_string(processed), nil}, remains}
  end
  # PROPERTY
  def get_next_step([" "| remains], processed, :otag_opened) do
    get_next_step(remains, processed, [], :otag_property)
  end
  def get_next_step([h | remains], processed, :otag_opened) do
    get_next_step(remains, processed ++ [h], :otag_opened)
  end

  def get_next_step([">"| remains], processed, :ctag_opened) do
    {{:ctag, to_string(processed)}, remains}
  end
  def get_next_step([h | remains], processed, :ctag_opened) do
    get_next_step(remains, processed ++ [h], :ctag_opened)
  end

  def get_next_step(["/", ">"| remains], tag, property, :otag_property) do
    {{:otag, to_string(tag), to_string(property)}, {:data, nil}, {:ctag, to_string(tag)}, remains}
  end
  def get_next_step([">"| remains], tag, property, :otag_property) do
    {{:otag, to_string(tag), to_string(property)}, remains}
  end
  def get_next_step([h| remains], tag, property, :otag_property) do
    get_next_step(remains, tag, property ++ [h], :otag_property)
  end

  def cleanup([{:data, _}| tail]) do
    cleanup(tail)
  end
  def cleanup(data) when is_list(data) do
    cleanup(data, [])
  end
  def cleanup([{:otag, _, _} = otag| tail], result) do
    cleanup(tail, [otag] ++ result)
  end
  def cleanup([{:ctag, _} = ctag| tail], result) do
    cleanup(tail, [ctag] ++ result)
  end
  def cleanup([{:data, _} = data, {:ctag, _} = ctag| tail], [{:otag, _, _}| _] = result) do
    cleanup(tail, [ctag, data] ++ result)
  end
  def cleanup([], result) do
    Enum.reverse result
  end
  def cleanup([{:data, _}], result) do
    Enum.reverse result
  end
  def cleanup([_h|t], result), do: cleanup(t, result)
end
