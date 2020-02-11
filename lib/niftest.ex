defmodule Xmlazy.Niftest do
  @on_load :load_nifs

  def load_nifs do
    :erlang.load_nif('./priv/elixir_niftest', 1)
  end

  def hello(_a) do
    raise "NIF hello not implemented"
  end
end
