defmodule Xmlazy.Lib do
  def is_subset_of(contained, container) when is_list(contained) and is_list(container) do
    contained -- container == []
  end
end
