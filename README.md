# Xmlazy

## about Xmlazy
Xmlazy - simpe library for working with XML files in Elixir.
Now it supports only basic functions (the project is so young), but it's enough
to handle most of tasks.

## Installation

Download the source and add to yourn mix.exs file:
```elixir
defp deps do
[
   {:xmlazy, path: "path/to/xmlazy/folder"}
]
```

## Example

```elixir
iex(1)> parsed =
"<a><b inner=\"tags\"><tag property=\"one\">First value</tag><tag property=\"two\">Second value</tag></b><b>One more value</b></a>"
|> Xmlazy.parse

{"a", {:properties, nil},
 {:data,
  [
    {"b", {:properties, [{:property, "inner", "tags"}]},
     {:data,
      [
        {"tag", {:properties, [{:property, "property", "one"}]},
         {:data, "First value"}},
        {"tag", {:properties, [{:property, "property", "two"}]},
         {:data, "Second value"}}
      ]}},
    {"b", {:properties, nil}, {:data, "One more value"}}
  ]}}

iex(2)> parsed |> Xmlazy.get_path("a.b{inner=tags}")
[
  {"tag", {:properties, [{:property, "property", "one"}]},
   {:data, "First value"}},
  {"tag", {:properties, [{:property, "property", "two"}]},
   {:data, "Second value"}}
]
iex(3)> parsed |> Xmlazy.get_path("a.b{inner=tags}.tag")
["First value", "Second value"]
```

