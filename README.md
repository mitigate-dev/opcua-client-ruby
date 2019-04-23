# opcua-client-ruby

Incomplete OPC-UA client library for Ruby. Wraps open62541: <https://open62541.org>.

## Installation

Add it to your Gemfile:

```ruby
gem 'opcua_client'
```

## Usage

Use `start` helper to automatically close connections:

```ruby
require 'opcua_client'

OPCUAClient.start("opc.tcp://127.0.0.1:4840") do |client|
  # write to ns=2;s=1
  client.write_int16(2, "1", 888)
  puts client.read_int16(2, "1")
end
```

Or handle connections manually:

```ruby
require 'opcua_client'

client = OPCUAClient::Client.new
begin
  client.connect("opc.tcp://127.0.0.1:4840")
  # write to ns=2;s=1
  client.write_int16(2, "1", 888)
  puts client.read_int16(2, "1")
ensure
  client.disconnect
end
```

## Contribute

### Set up

```console
bundle
```

### Try out changes

```console
$ rake compile
$ pry
pry> require 'opcua_client'
pry> client = OPCUAClient::Client.new
```
