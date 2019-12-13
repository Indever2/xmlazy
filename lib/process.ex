defmodule Xmlazy.Process do
  def wrap(parsed) when is_list(parsed) do
    wrap_stacked([], parsed)
  end

  ### WRAP STACKED

  def wrap_stacked(
    [{{:otag, otag_value, otag_properties}, []}],
    [{:data, [data]}, {:ctag, otag_value}]
  ) do
    {otag_value, {:properties, otag_properties}, {:data, data}}
  end
  def wrap_stacked(
    [{{:otag, otag_value, otag_properties}, []}],
    [{:data, data}, {:ctag, otag_value}]
  ) do
    {otag_value, {:properties, otag_properties}, {:data, data}}
  end
  def wrap_stacked(
    [{{:otag, otag_value, otag_properties}, [data]}],
    [{:ctag, otag_value}]
  ) do
    {otag_value, {:properties, otag_properties}, {:data, data}}
  end
  def wrap_stacked(
    [{{:otag, otag_value, otag_properties}, data}],
    [{:ctag, otag_value}]
  ) do
    {otag_value, {:properties, otag_properties}, {:data, data}}
  end
  def wrap_stacked(
    [{{:otag, otag_value, otag_props} = otag, otag_data}, {{:otag, master_otag_value, master_otag_props} = master_otag, master_otag_data} | otags],
    [{:data, data}, {:ctag, otag_value} | remains]
  ) do
    wrap_stacked([{master_otag, master_otag_data ++ [{otag_value, {:properties, otag_props}, {:data, data}}]}] ++ otags, remains)
  end
  def wrap_stacked(
    [{{:otag, otag_value, otag_props} = otag, otag_data}, {{:otag, master_otag_value, master_otag_props} = master_otag, master_otag_data} | otags],
    [{:ctag, otag_value}| remains]
  ) do
    wrap_stacked([{master_otag, master_otag_data ++ [{otag_value, {:properties, otag_props}, {:data, otag_data}}]}] ++ otags, remains)
  end
  def wrap_stacked(otags, [{:otag, _, _} = otag| remains]) do
    wrap_stacked([{otag, []}] ++ otags, remains)
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

  def get_next_step(["/", ">"| remains], tag, :otag_opened) do
    {{:otag, to_string(tag), nil}, {:data, nil}, {:ctag, to_string(tag)}, remains}
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
